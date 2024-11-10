#include <iostream>
#include <stdlib.h>
#include <GL/glut.h>
#include <GL/glut.h>
#include <vector>
#include <assert.h>
#include <math.h>
#include <sstream>
#include <armadillo>

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


//constante des courbes NUBS
const int d = 3;  //degree de la courbe
const int n = 7; //nb points de controles
const int v_size = n + d + 1;
const std::vector<GLfloat> nodal_vector { 0, 0, 0, 0, 0.25, 0.5, 0.75, 1, 1, 1, 1};
//const std::vector<GLfloat> nodal_vector { 0, 0.1, 0.2, 0.3, 0.4, 0.5, 0.6, 0.7, 0.8, 0.9, 1};
const std::vector<arma::vec> points {
  {0, 1, 0},
  {1, 1, 0},
  {1, 2, 0},
  {2, 2, 0},
  {2.5, 1, 0},
  {1.7, 0, 0},
  {1, 0.7, 0},
};

std::vector<arma::vec> cercle;

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

arma::vec computeNubs(float t) {  

  arma::vec nubs = {0, 0, 0};
  for(int i = 0; i < n; i++) {
    GLfloat Njd = cox_de_boor(t, d, i, nodal_vector);
    nubs = nubs + (points[i] * Njd);
  }

  return nubs;
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

arma::vec computeDeriveNubs(float t) {
  assert(nodal_vector.size() == v_size);

  arma::vec nubs = {0, 0, 0};
  arma::vec sommePointCoxDerive = {0, 0, 0};
  GLfloat sommeCox = 0.f;
  GLfloat sommeCoxDerive = 0.f;

  for(int i = 0; i < n ; i++) {
    GLfloat Njd = cox_de_boor(t, d, i, nodal_vector);
    GLfloat deriveNjd = derive_cox(t, d, i, nodal_vector);
    nubs += (points[i] * Njd);
    sommePointCoxDerive += (points[i] * deriveNjd);
    sommeCox += Njd;
    sommeCox += deriveNjd;
  }

  arma::vec a = (sommePointCoxDerive * sommeCox);
  arma::vec b = (nubs * sommeCoxDerive);

  return (a - b) / (sommeCox * sommeCox); 
}

arma::vec derive_seconde(float t) {
  const float epsilon = 0.001;
  return computeDeriveNubs(t + epsilon) - computeDeriveNubs(t) / epsilon;
}

float rayonCourbure(float t) {
  arma::vec derive = computeDeriveNubs(t);
  arma::vec seconde = derive_seconde(t);

  return arma::norm(arma::cross(seconde, derive)) / pow(arma::norm(derive), 3);
}

////////////////////////

void traceFrenet(float t) {
  //compute tangente and normal
  arma::vec binormale = arma::vec {0.0,0.0,1.0};
  arma::vec p = computeNubs(t);
  arma::vec tangente = computeDeriveNubs(t);
  tangente = arma::normalise(tangente);
  arma::vec normal =  arma::normalise(arma::cross(binormale, tangente));

  tangente = tangente + p;
  normal = normal + p;
  binormale = binormale + p;

  //trace tangente
  glBegin(GL_LINES);
  glColor3f(1.0,0.0,0.0);
  glVertex2f(p[0], p[1]);
  glVertex2f(tangente[0],tangente[1]);
  glEnd(); 

  //trace normal
  glBegin(GL_LINES);
  glColor3f(1.0,1.0,0.0);
  glVertex2f(p[0], p[1]);
  glVertex2f(normal[0],normal[1]);
  glEnd(); 
  
  //trace binormale
  glBegin(GL_LINES);
  glColor3f(0.0,1.0,1.0);
  glVertex3f(p[0], p[1], p[2]);
  glVertex3f(binormale[0],binormale[1], binormale[2]);
  glEnd(); 
}

//function to draw a circle, found here : https://stackoverflow.com/questions/22444450/drawing-circle-with-opengl
void createCircle(float r) {
  const int num_segments = 64;
  for(int ii = 0; ii < num_segments; ii++)
  {
    float theta = 2.0f * 3.1415926f * float(ii) / float(num_segments);//get the current angle

    float z = r * cosf(theta);//calculate the z component
    float y = r * sinf(theta);//calculate the y component
    arma::vec p{0.0f, y, z};
    cercle.push_back(p);
  }
}

void DrawCircle(arma::vec pos, float t)
{
  //compute tangente and normal
  arma::vec k = arma::vec {0.0,0.0,1.0};
  arma::vec tangente = computeDeriveNubs(t);
  tangente = arma::normalise(tangente);
  arma::vec normal =  arma::normalise(arma::cross(k, tangente));
  
  glBegin(GL_LINE_LOOP);
  for(int i = 0; i < cercle.size(); i++)
  {
      arma::vec v = cercle[i];
      
      arma::vec v1 = (v[0] * tangente + v[1] * k + v[2] * normal) + pos;
      glVertex3f(v1[0], v1[1], v1[2]);

  }
  glEnd();
}

void displayCourbe(void)
{
  //display curve
  glBegin(GL_LINE_STRIP);
  for(float i = 0; i < 1; i += 0.01){
      glColor3f(1.0f, 1.0f, 0.0f);
      arma::vec pen;
      pen = computeNubs(i);
      glVertex2f(pen[0], pen[1]);
  }       
  glEnd();

  //display circles
  for(float i = 0; i < 1; i += 0.001){ 
      glColor3f(0.0f, i, 1.0f);
      arma::vec pen;
      pen = computeNubs(i);
      DrawCircle(pen, i);
  }

  //display frenet
  traceFrenet(t);

  arma::vec pos = computeNubs(t);
  arma::vec derive = computeDeriveNubs(t);
  arma::vec seconde = derive_seconde(t);
  std::cout << "--------------------------------------------------------" << std::endl;
  std::cout << "Paramètre : t=" << t << std::endl;
  std::cout << "Position : d=" << pos[0] << ", " << pos[1] << ", " << pos[2] << ", " << std::endl;
  std::cout << "Derivée : d'=" << derive[0] << ", " << derive[1] << ", " << derive[2] << ", " << std::endl;
  std::cout << "Derivée seconde : d''=" << seconde[0] << ", " << seconde[1] << ", " << seconde[2] << ", " << std::endl;
  std::cout << "Rayon de courbure : R=" << rayonCourbure(t) << std::endl;
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

  createCircle(0.1);

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
      t+=.05;
       if (t > 1 ) t=1;
      glutPostRedisplay();
      break;
    case '-': //* ajustement du t
       t-=.05;
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

    
    
