#include <iostream>
#include <stdlib.h>
#include <GL/glut.h>
#include <GL/glut.h>
#include <vector>
#include <array>
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
float tu=0;
float tv=0;

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
const int du = 2;
const int dv = 2;
const int nu = 3;
const int nv = 3;
//const int v_size = n + d + 1;
const std::vector<GLfloat> nodal_vectorU { 0,0,0,1,2,2,2};
const std::vector<GLfloat> nodal_vectorV { 0,0,0,1,1,1 };
const std::vector<std::vector<arma::vec>> points {
  {
    {0,0,0},
    {1,1,0},
    {2,0,0},
  }, {
    {0,1,2},
    {1,1,2},
    {2,1,2},
  }, {
    {0,2,0},
    {1,2,0},
    {2,1,1},
  },
};

const float step = 0.01;
std::vector<std::vector<arma::vec>> nubs;


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

arma::vec compute3DNubs(float u, float v) {  
  arma::vec dividende {0,0,0};
  float diviseur = 0.0f;
  for(int i = 0; i < points.size(); i++) {
      for(int j = 0; j < points[i].size(); j++) {
        float Nu = cox_de_boor(u, du, i, nodal_vectorU);
        float Nv =  cox_de_boor(v, dv, j, nodal_vectorV);
        float N = Nu * Nv;
        diviseur += N;
        dividende += N * points[i][j];
      } 
  }
  return dividende / diviseur;
}

GLfloat derive_cox(GLfloat t, int d, int  j, int n, const std::vector<GLfloat> &nodal_vector) {
  GLfloat tj = nodal_vector[j];
  GLfloat tj1 = nodal_vector[j+1];
  GLfloat tjd = nodal_vector[j+d];
  GLfloat tjd1 = nodal_vector[j+d+1];

  GLfloat f1 = (tjd - tj) == 0 ? 0 : n/(tjd - tj);
  GLfloat f2 = (tjd1 - tj1) == 0 ? 0 : n/(tjd1 - tj1);

  return f1 * cox_de_boor(t, d - 1, j, nodal_vector) 
       - f2 * cox_de_boor(t, d - 1, j + 1, nodal_vector);
}

std::array<arma::vec,2> computeDerive3DNubs(float u, float v) {
  arma::vec R = {0, 0, 0};
  arma::vec Ru = {0, 0, 0};
  arma::vec Rv = {0, 0, 0};
  float omega = 0.f;
  float omegau = 0.f;
  float omegav = 0.f;
  
  for(int i = 0; i < points.size(); i++) {
    for(int j = 0; j < points[i].size(); j++) {
      float Nu = cox_de_boor(u, du, i, nodal_vectorU);
      float Nv =  cox_de_boor(v, dv, j, nodal_vectorV);
      float deriveNu = derive_cox(u, du, i, nu, nodal_vectorU);
      float deriveNv = derive_cox(v, dv, i, nv, nodal_vectorV);

      float NuNv = Nu * Nv;
      float dNuNv = deriveNu * Nv;
      float dNvNu = deriveNv * Nu;
      R += NuNv * points[i][j];
      Ru += dNuNv * points[i][j];
      Rv += dNvNu * points[i][j];
      omega += NuNv;
      omegau += dNuNv;
      omegav += dNvNu;
    } 
  }

  float omega2 = omega * omega;
  arma::vec du = (Ru*omega - R*omegau) / omega2;
  arma::vec dv = (Rv*omega - R*omegav) / omega2;
  return std::array{du,dv};
}

////////////////////////

void traceFrenet(float u, float v) {
  arma::vec p = compute3DNubs(u, v);
  std::array<arma::vec,2> tangentes = computeDerive3DNubs(u, v);
  arma::vec tangenteU = tangentes[0];
  arma::vec tangenteV = tangentes[1];
  tangenteU = arma::normalise(tangenteU);
  tangenteV = arma::normalise(tangenteV);
  arma::vec normal =  arma::normalise(arma::cross(tangenteU, tangenteV));

  tangenteU = tangenteU + p;
  tangenteV = tangenteV + p;
  normal = normal + p;

  //trace tangente
  glBegin(GL_LINES);
  glColor3f(1.0,0.0,0.0);
  glVertex3f(p[0], p[1], p[2]);
  glVertex3f(tangenteU[0],tangenteU[1],tangenteU[2]);
  glEnd(); 

  //trace normal
  glBegin(GL_LINES);
  glColor3f(1.0,1.0,0.0);
  glVertex3f(p[0], p[1], p[2]);
  glVertex3f(normal[0],normal[1],normal[2]);
  glEnd(); 
  
  //trace tangenteV
  glBegin(GL_LINES);
  glColor3f(0.0,1.0,1.0);
  glVertex3f(p[0], p[1], p[2]);
  glVertex3f(tangenteV[0],tangenteV[1], tangenteV[2]);
  glEnd(); 
}

void precompute3DNubs() {
  int iter = 1.f / step;
  for(float i = 0; i < iter; i ++) {
    float stepi = step * i;
    nubs.push_back(std::vector<arma::vec>());
    for(float j = 0; j < iter; j++){
      float stepj = step * j;
      arma::vec p = compute3DNubs(stepi,stepj);
      nubs[i].push_back(p);
    }
  }   
}

void displayCourbe(void)
{
  //display curve
  for(float i = 0; i < nubs.size(); i ++){
    for(float j = 0; j < nubs[i].size(); j++){
      glColor3f(i * step, j * step, 0.0f);
      if(i != nubs.size() - 1 && j != nubs[i].size() - 1) {
        arma::vec p = nubs[i][j];
        arma::vec p1 = nubs[i + 1][j];
        arma::vec p2 = nubs[i][j + 1];
        arma::vec p3 = nubs[i + 1][j + 1];

        glBegin(GL_TRIANGLES);
        glVertex3f(p[0], p[1], p[2]);
        glVertex3f(p1[0], p1[1], p1[2]);
        glVertex3f(p2[0], p2[1], p2[2]);
        glEnd();

        glBegin(GL_TRIANGLES);
        glVertex3f(p1[0], p1[1], p1[2]);
        glVertex3f(p3[0], p3[1], p3[2]);
        glVertex3f(p2[0], p2[1], p2[2]);
        glEnd();
      }

    }
  }       

  //display frenet
  traceFrenet(tu, tv);
}

int main(int argc,char **argv)
{
  /* initialisation de glut et creation
     de la fenetre */
  glutInit(&argc,argv);
  glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH);
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

  precompute3DNubs();


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
      case 'v': //* ajustement du t
        tv+=.1;
        tv = fmod(tv, 1.0f);
        glutPostRedisplay();
      break;
      case 'u': //* ajustement du t
        tu+=.1;
        tu = fmod(tu, 1.0f);
        glutPostRedisplay();
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

    
    
