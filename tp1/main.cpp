#include <iostream>
#include <stdlib.h>
#include <GL/glut.h>
#include <GL/glut.h>
#include <vector>
#include <assert.h>
#include <math.h>
#include <sstream>


#include<string>

using namespace std ;
void affichage(void);

void clavier(unsigned char touche,int x,int y);
void affiche_repere(void);

void mouse(int, int, int, int);
void mouseMotion(int, int);
//void reshape(int,int);
float t=.5 ;

// variables globales pour OpenGL
bool mouseLeftDown;
bool mouseRightDown;
bool mouseMiddleDown;
float mouseX, mouseY;
float cameraAngleX;
float cameraAngleY;
float cameraDistance=-3.;

// constantes pour les materieux
float no_mat[] = {0.0f, 0.0f, 0.0f, 1.0f};
float mat_ambient[] = {0.7f, 0.7f, 0.7f, 1.0f};
float mat_ambient_color[] = {0.8f, 0.8f, 0.2f, 1.0f};
float mat_diffuse[] = {0.1f, 0.5f, 0.8f, 1.0f};
float mat_specular[] = {1.0f, 1.0f, 1.0f, 1.0f};
float no_shininess = 0.0f;
float low_shininess = 5.0f;
float high_shininess = 100.0f;
float mat_emission[] = {0.3f, 0.2f, 0.2f, 0.0f};

struct Points
{
    float x;
    float y;
    float z;

    Points operator*(float f) const
    {
        return Points {.x = x * f, .y = y * f, .z = z * f};
    }
    Points operator+(Points p1) const
    {
        return Points {.x = p1.x + x, .y = p1.y + y, .z = p1.z + z};
    }
};

const int d = 3;
const int n = 7;
const int v_size = n + d + 1;
const std::vector<GLfloat> nodal_vector { 0, 0, 0, 0, 0.25, 0.5, 0.75, 1, 1, 1, 1};
const std::vector<Points> points {
  {.x = 0, .y=1, .z = 0},
  {.x = 1, .y=1, .z = 0},
  {.x = 1, .y=2, .z = 0},
  {.x = 2, .y=2, .z = 0},
  {.x = 2.5, .y=1, .z = 0},
  {.x = 1.7, .y=0, .z = 0},
  {.x = 1, .y=0.7, .z = 0},
};

void initOpenGl() 
{ 

//lumiere 

	glClearColor( .5, .5, 0.5, 0.0 );
 
	glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  GLfloat l_pos[] = { 3.,3.5,3.0,1.0 };
  glLightfv(GL_LIGHT0,GL_POSITION,l_pos);

  glLightfv(GL_LIGHT0,GL_DIFFUSE,l_pos);
  glLightfv(GL_LIGHT0,GL_SPECULAR,l_pos);
  glEnable(GL_COLOR_MATERIAL);

  glDepthFunc(GL_LESS);
  glEnable(GL_DEPTH_TEST);
//glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
// glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE|GLUT_RGB);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
         gluPerspective(45.0f,(GLfloat)200/(GLfloat)200,0.1f,10.0f);
	glMatrixMode(GL_MODELVIEW);
      gluLookAt(0.,0.,4., 0.,0.,0., 0.,1.,0.);

}

//------------------------------------------------------

//////////////////////
GLfloat cox_de_boor(GLfloat t, int d, int  j, const std::vector<GLfloat> &nodal_vector)  {
  GLfloat tj = nodal_vector[j];
  GLfloat tj1 = nodal_vector[j+1];
  GLfloat tjd = nodal_vector[j+d];
  GLfloat tjd1 = nodal_vector[j+d+1];
  
  if(d == 0) {
    return t >= tj && t < tj1; 
  }
  //calcul des coeffs, si tj confondus, alors 0
  GLfloat Nd1j = (tjd-tj) == 0 ? 0 : (t-tj) / (tjd-tj);
  GLfloat Nd1j1 = (tjd1 - tj1) == 0 ? 0 : (tjd1 - t) / (tjd1 - tj1);

  //calcul récursive des N
  return Nd1j * cox_de_boor(t, d-1, j, nodal_vector)
   + Nd1j1 * cox_de_boor(t, d-1, j + 1, nodal_vector);
}

Points computeNurbs(float t) {  
  assert(nodal_vector.size() == v_size);

  Points sommePointCox = {0, 0};
  GLfloat sommeCox = 0.f;
  GLfloat wi = 1.f / n; // poids uniforme pour le moment
  for(int i = 0; i < n; i++) {
    GLfloat Njd = cox_de_boor(t, d, i, nodal_vector);
    sommePointCox = sommePointCox + (points[i] * Njd * wi);
    sommeCox += Njd * wi;
  }

  Points pointNurbs = sommePointCox * (1.f/sommeCox);
  return pointNurbs;
}

GLfloat derive_cox(GLfloat t, int d, int  j, const std::vector<GLfloat> &nodal_vector) {
  GLfloat tj = nodal_vector[j];
  GLfloat tj1 = nodal_vector[j+1];
  GLfloat tjd = nodal_vector[j+d];
  GLfloat tjd1 = nodal_vector[j+d+1];

  GLfloat f1 = (tjd - tj) == 0 ? 0 : n/(tjd - tj);
  GLfloat f2 = (tjd1 - tj1) == 0 ? 0 : n/(tjd1 - tj1);

  return f1 * cox_de_boor(t, d - 1, j, nodal_vector) 
       - f2 * cox_de_boor(t, d - 1, j + 1, nodal_vector);
}

Points computeDeriveNurbs(float t) {
  assert(nodal_vector.size() == v_size);

  Points sommePointCox = {0, 0};
  Points sommePointCoxDerive = {0, 0};
  GLfloat sommeCox = 0.f;
  GLfloat sommeCoxDerive = 0.f;

  for(int i = 0; i < n ; i++) {
    GLfloat Njd = cox_de_boor(t, d, i, nodal_vector);
    GLfloat deriveNjd = derive_cox(t, d, i, nodal_vector);
    sommePointCox = sommePointCox + (points[i] * Njd);
    sommePointCoxDerive = sommePointCoxDerive + (points[i] * deriveNjd);
    sommeCox += Njd;
    sommeCox += deriveNjd;
  }

  GLfloat inverseSommeDiviseurCarre = 1.f / (sommeCox * sommeCox);

  Points a = (sommePointCoxDerive * sommeCox) * inverseSommeDiviseurCarre;
  Points b = (sommePointCox * sommeCoxDerive) * inverseSommeDiviseurCarre;

  return a + (b * -1); // a- b
}

////////////////////////

Points normalize(Points a) {
  float inverseDistance = 1 / sqrt(a.x * a.x + a.y * a.y + a.z * a.z);
  return a * inverseDistance;
}

Points crossProduct(Points a, Points b) {
  return Points {
    .x = a.y * b.z - a.z * b.y,
    .y = a.z * b.x - a.x * b.z,
    .z = a.x * b.y - a.y * b.x,
  };
}

void traceFrenet(float t) {
  Points k = Points {0.0,0.0,1.0};
  Points p = computeNurbs(t);
  Points tangente = computeDeriveNurbs(t);
  tangente = normalize(tangente);
  Points normal = normalize(crossProduct(k, tangente));

  tangente = tangente + p;
  normal = normal + p;
  k = k + p;

  //trace tangente
  glBegin(GL_LINES);
  glColor3f(1.0,0.0,0.0);
  glVertex2f(p.x, p.y);
  glVertex2f(tangente.x,tangente.y);
  glEnd(); 

  //trace normal
  glBegin(GL_LINES);
  glColor3f(1.0,1.0,0.0);
  glVertex2f(p.x, p.y);
  glVertex2f(normal.x,normal.y);
  glEnd(); 
  
  //trace k
  glBegin(GL_LINES);
  glColor3f(0.0,1.0,1.0);
  glVertex3f(p.x, p.y, p.z);
  glVertex3f(k.x,k.y, k.z);
  glEnd(); 
}

//function to draw a circle, find here : https://stackoverflow.com/questions/22444450/drawing-circle-with-opengl
void DrawCircle(float cx, float cy, float r)
{
    const int num_segments = 64;
    glBegin(GL_LINE_LOOP);
    for(int ii = 0; ii < num_segments; ii++)
    {
        float theta = 2.0f * 3.1415926f * float(ii) / float(num_segments);//get the current angle

        float z = r * cosf(theta);//calculate the z component
        float y = r * sinf(theta);//calculate the y component

        glVertex3f(cx, y + cy, z);//output vertex

    }
    glEnd();
}

void displayCourbe(void)
{
  //display curve
  glBegin(GL_LINE_STRIP);
  for(float i = 0; i < 1; i += 0.01){
      glColor3f(1.0f, 1.0f, 0.0f);
      Points pen;
      pen = computeNurbs(i);
      glVertex2f(pen.x, pen.y);
  }       
  glEnd();

  //display curve
  for(float i = 0; i < 1; i += 0.001){
      glColor3f(0.0f, i, 1.0f);
      Points pen;
      pen = computeNurbs(i);
      DrawCircle(pen.x, pen.y, 0.1f);
  }

  //display frenet
  traceFrenet(t);
}

int main(int argc,char **argv)
{
  /* initialisation de glut et creation
     de la fenetre */
  glutInit(&argc,argv);
  glutInitDisplayMode(GLUT_RGB);
  glutInitWindowPosition(200,200);
  glutInitWindowSize(600,600);
  glutCreateWindow("ifs");

  /* Initialisation d'OpenGL */
  glClearColor(0.0,0.0,0.0,0.0);
  glColor3f(1.0,1.0,1.0);
  glPointSize(1.0);
	
	//ifs = new Ifs();
  /* enregistrement des fonctions de rappel */
  glutDisplayFunc(affichage);
  glutKeyboardFunc(clavier);
  glutMouseFunc(mouse);
  glutMotionFunc(mouseMotion);
  //-------------------------------


  //-------------------------------
    initOpenGl() ;
//-------------------------------

/* Entree dans la boucle principale glut */
  glutMainLoop();
  return 0;
}
//------------------------------------------------------
void affiche_repere(void)
{
  glBegin(GL_LINES);
  glColor3f(1.0,0.0,0.0);
  glVertex2f(0.,0.);
  glVertex2f(1.,0.);
  glEnd(); 

	 glBegin(GL_LINES);
  glColor3f(0.0,1.0,0.0);
  glVertex2f(0.,0.);
  glVertex2f(0.,1.);
  glEnd(); 
   glBegin(GL_LINES);
  glColor3f(0.0,0.0,1.0);
  glVertex3f(0.,0.,0.);
  glVertex3f(0.,0.,1.);
  glEnd(); 
}

//-----------------------------------------------------



//------------------------------------------------------
void affichage(void)
{
	glMatrixMode(GL_MODELVIEW);
  /* effacement de l'image avec la couleur de fond */
  //glClear(GL_COLOR_BUFFER_BIT);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  //glClearDepth(10.0f);                         // 0 is near, >0 is far

  glPushMatrix();
	glTranslatef(0,0,cameraDistance);
	glRotatef(cameraAngleX,1.,0.,0.)	;
	glRotatef(cameraAngleY,0.,1.,0.);
	affiche_repere();
  displayCourbe();
  glPopMatrix();
  /* on force l'affichage du resultat */

          glFlush();
  glutSwapBuffers();

}

//------------------------------------------------------


//------------------------------------------------------
void clavier(unsigned char touche,int x,int y)
{

  switch (touche)
    {
    case '+': //
      t+=.1;
       if (t > 1 ) t=1;
      glutPostRedisplay();
      break;
    case '-': //* ajustement du t
       t-=.1;
        if (t < 0 ) t=0;
      glutPostRedisplay();
      break;
    case 'f': //* affichage en mode fil de fer 
      glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
      glutPostRedisplay();
      break;
      case 'p': //* affichage du carre plein 
      glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
      glutPostRedisplay();
      break;
  case 's' : //* Affichage en mode sommets seuls 
      glPolygonMode(GL_FRONT_AND_BACK,GL_POINT);
      glutPostRedisplay();
      break;

    case 'q' : //*la touche 'q' permet de quitter le programme 
      exit(0);
    }
    
}
void mouse(int button, int state, int x, int y)
{
    mouseX = x;
    mouseY = y;

    if(button == GLUT_LEFT_BUTTON)
    {
        if(state == GLUT_DOWN)
        {
            mouseLeftDown = true;
        }
        else if(state == GLUT_UP)
            mouseLeftDown = false;
    }

    else if(button == GLUT_RIGHT_BUTTON)
    {
        if(state == GLUT_DOWN)
        {
            mouseRightDown = true;
        }
        else if(state == GLUT_UP)
            mouseRightDown = false;
    }

    else if(button == GLUT_MIDDLE_BUTTON)
    {
        if(state == GLUT_DOWN)
        {
            mouseMiddleDown = true;
        }
        else if(state == GLUT_UP)
            mouseMiddleDown = false;
    }
}


void mouseMotion(int x, int y)
{
    if(mouseLeftDown)
    {
        cameraAngleY += (x - mouseX);
        cameraAngleX += (y - mouseY);
        mouseX = x;
        mouseY = y;
    }
    if(mouseRightDown)
    {
        cameraDistance += (y - mouseY) * 0.2f;
        mouseY = y;
    }

    glutPostRedisplay();
}

    
    
