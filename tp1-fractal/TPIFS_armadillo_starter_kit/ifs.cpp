#include "ifs.h"
#include <GL/glut.h>
#include <iostream>


using namespace arma;
using namespace std;

Ifs::Ifs(void) {
    arma::mat T0={{1, 0.5, 0.5},
                  {0, 0.5, 0}, 
                  {0, 0  , 0.5}};

    arma::mat T1={{0.5, 0,  0},
                  {0.5, 1, 0.5}, 
                  {0  , 0, 0.5}};

    arma::mat T2={{0.5, 0  , 0},
                  {0  , 0.5, 0}, 
                  {0.5, 0.5, 1}};

    mIfs = {T0, T1, T2};

    arma::mat points ={{1, 0, 0},
                       {0, 1, 0}, 
                       {0, 0, 1}}; 
    mControlPoints = points;

    /*arma::mat rectangle = {{0.5, 0  , 0.75, 0.25},
                           {0  , 0.5, 0.25, 0.75}, 
                           {0.5, 0.5, 0   , 0   }};*/
    mPrimitive = points;

    
}

Ifs::~Ifs(void)
{
}

void Ifs::display(int level)
{
    ComputeApproximation(level);

    for (int i = 0; i < mApproximation.size(); i++) {
        arma::mat m = mApproximation[i];
        glBegin(GL_TRIANGLES);
        glColor3f(0.0,0.0,1.0);
        for (int j = 0; j < m.n_cols; j++) {
            glVertex3f(m.col(j)[0], m.col(j)[1], m.col(j)[2]);
        }
        glEnd(); 
    }

    /*for (int i = 0; i < mApproximation.size(); ++i)
    {
        arma::mat m = mApproximation[i];
        glBegin(GL_TRIANGLES);
        for (int j = 0; j < m.n_cols-1; ++j)
        {
            glVertex3f(m(0, j), m(1, j), m(2, j));
        }
        glEnd();
        glBegin(GL_TRIANGLES);
        for (int j = 1; j < m.n_cols; ++j)
        {
            glVertex3f(m(0, j), m(1, j), m(2, j));
        }
        glEnd();
    }*/

}

void Ifs::ComputeApproximation(int profondeur)
{   
    mApproximation.clear();
    // Generate all possible combinations of matrices in mifs
    int numMatrices = mIfs.size();
    int numCombinations = pow(numMatrices, profondeur); // Change 3 to the desired depth of combinations

    for (int i = 0; i < numCombinations; ++i) {
        arma::mat combination = arma::eye<arma::mat>(3, 3); // Identity matrix of size 3x3
        int temp = i;
        for (int j = 0; j < profondeur; ++j) { // Change 3 to the desired depth of combinations
            combination *= mIfs[temp % numMatrices];
            temp /= numMatrices;
        }
        //cout << "Combination " << i << ":\n" << combination << endl;
        mApproximation.push_back(mControlPoints * combination * mPrimitive);
    }

}
