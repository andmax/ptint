/**
 *
 *    Render Volume GPU
 *
 *  File: transferFunction.h
 *
 *  Authors:
 *    Andre Maximo
 *    Ricardo Marroquim
 *
 *  Created: May 06, 2006
 *
 */
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <math.h>

#include <GL/glut.h>

using namespace std;

typedef unsigned int uint;

/// Default values

#define INIT_KS 0.5
#define INIT_KD 0.5
#define INIT_KA 0.1
#define INIT_ALPHAI 1.0
#define INIT_RHO_0_X 0.2
#define INIT_RHO_0_Y 0.0
#define INIT_RHO_1_X 0.5
#define INIT_RHO_1_Y 0.0
#define INIT_RHO_2_X 0.7
#define INIT_RHO_2_Y 0.5

#define KS_MAX 1.0
#define KD_MAX 1.0
#define KA_MAX 1.0
#define ALPHAI_MAX 100.0

#define KS_Y_POS -0.05
#define KD_Y_POS -0.15
#define KA_Y_POS -0.25
#define ALPHAI_Y_POS -0.35

#define IC_PICK_BOX 5

#define IC_ORTHO_MIN -0.4
#define IC_ORTHO_MAX 1.3
#define IC_ORTHO_SIZE (IC_ORTHO_MAX - IC_ORTHO_MIN)

/// Object file format type
enum pickControl { RHO_0, RHO_1, RHO_2, KS, KD, KA, ALPHAI };

/// Extern screen size

extern GLint icWinWidth, icWinHeight;

/// A simple Color Type for the double 4-array RGBA

/// A class to handle the Transfer Function

class illuminationControl
{
 public:
  /// Constructor
  illuminationControl(GLfloat _ks = INIT_KS, GLfloat _kd = INIT_KD, GLfloat _ka = INIT_KA,
		      GLfloat _alphai = INIT_ALPHAI, 
		      GLfloat _rho_0_X = INIT_RHO_0_X, GLfloat _rho_0_Y = INIT_RHO_0_Y,
		      GLfloat _rho_1_X = INIT_RHO_1_X, GLfloat _rho_1_Y = INIT_RHO_1_Y,
		      GLfloat _rho_2_X = INIT_RHO_2_X, GLfloat _rho_2_Y = INIT_RHO_2_Y) :
    picked_point(-1), ks(_ks), kd(_kd), ka(_ka), alphai(_alphai)
    {
      rho[0][0] = _rho_0_X;
      rho[0][1] = _rho_0_Y;
      rho[1][0] = _rho_1_X;
      rho[1][1] = _rho_1_Y;
      rho[2][0] = _rho_2_X;
      rho[2][1] = _rho_2_Y;
    }

    /// Destructor
    ~illuminationControl() {  }

    void pick(int x, int y);
    void draw(void);    

    bool writeIC(char* filename);
    bool readIC(char* filename);

    void updateIC(int x, int y);

    void updateKs(GLfloat delta) {
      ks += delta;
      if (ks < 0.0) ks = 0.0;
      if (ks > 1.0) ks = 1.0;
    }

    void updateKd(GLfloat delta) {
      kd += delta;
      if (kd < 0.0) kd = 0.0;
      if (kd > 1.0) kd = 1.0;
    }

    void updateKa(GLfloat delta) {
      ka += delta;
      if (ka < 0.0) ka = 0.0;
      if (ka > 1.0) ka = 1.0;
    }

    void updateAlphai(GLfloat delta) {
      alphai += delta;
      if (ka < 0.0) alphai = 0.0;
      if (ka > 1.0) alphai = 1.0;
    }

    inline void setPickedPointNull(void) { picked_point = -1; }

    inline int getPickedPoint(void) { return picked_point; }

    inline GLfloat getAlphai(void) { return alphai; }
    inline GLfloat getKa(void) { return ka; }
    inline GLfloat getKs(void) { return ks; }
    inline GLfloat getKd(void) { return kd; }
    inline GLfloat getRho(int i, int j){ return rho[i][j]; }

 private:
    int picked_point;
    GLfloat ks, kd, ka, alphai, rho[3][2];

    inline GLfloat xOrtho(int x) { return (GLfloat)( ((x/(GLfloat)icWinWidth) * IC_ORTHO_SIZE) + IC_ORTHO_MIN ); }
    inline GLfloat yOrtho(int y) { return (GLfloat)(((((GLfloat)icWinHeight - y)/(GLfloat)icWinHeight) * IC_ORTHO_SIZE) + IC_ORTHO_MIN); }
    inline int xScreen(GLfloat x) { return (int)(((x - IC_ORTHO_MIN)*(GLfloat)icWinWidth)/IC_ORTHO_SIZE); }
    inline int yScreen(GLfloat y) { return icWinHeight - (int)(((y - IC_ORTHO_MIN)*(GLfloat)icWinHeight)/IC_ORTHO_SIZE); }
};
