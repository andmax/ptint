/**
 *
 *    Render Volume GPU
 *
 *  File: Trasfer Function
 *
 *  Authors:
 *    Andre Maximo
 *    Ricardo Marroquim
 *
 *  Created: May 5, 2006
 *
 */

#include "illuminationControl.h"

/**
 * Update the position and value of the picked point
 * in the illumination control window
 * @param x
 * @param y
 **/
void illuminationControl::updateIC(int x, int y)
{
  if (picked_point != -1)
    {    
      if (picked_point == KS)
	{
	  GLfloat x_ortho = xOrtho(x);
	  if (x_ortho > 1.0) x_ortho = 1.0;
	  if (x_ortho < 0.0) x_ortho = 0.0;
	  ks = (GLfloat)( x_ortho * KS_MAX );
	}
      else if (picked_point == KD)
	{
	  GLfloat x_ortho = xOrtho(x);
	  if (x_ortho > 1.0) x_ortho = 1.0;
	  if (x_ortho < 0.0) x_ortho = 0.0;
	  kd = (GLfloat)( x_ortho * KD_MAX );
	}
      else if (picked_point == KA)
	{
	  GLfloat x_ortho = xOrtho(x);
	  if (x_ortho > 1.0) x_ortho = 1.0;
	  if (x_ortho < 0.0) x_ortho = 0.0;
	  ka = (GLfloat)( x_ortho * KA_MAX );
	}
      else if (picked_point == ALPHAI)
	{
	  GLfloat x_ortho = xOrtho(x);
	  if (x_ortho > 1.0) x_ortho = 1.0;
	  if (x_ortho < 0.0) x_ortho = 0.0;
	  alphai = (GLfloat)( x_ortho * ALPHAI_MAX );
	}
      else if (picked_point == RHO_0)
	{
	  GLfloat x_ortho = xOrtho(x);
	  if (x_ortho > 1.0) x_ortho = 1.0;
	  if (x_ortho < 0.0) x_ortho = 0.0;
	  GLfloat y_ortho = yOrtho(y);	 
	  if (y_ortho > 1.0) y_ortho = 1.0;
	  if (y_ortho < 0.0) y_ortho = 0.0;
	  
	  rho[0][0] = (GLfloat)( x_ortho );
	  rho[0][1] = (GLfloat)( y_ortho );	  
	}
      else if (picked_point == RHO_1)
	{
	  GLfloat x_ortho = xOrtho(x);
	  if (x_ortho > 1.0) x_ortho = 1.0;
	  if (x_ortho < 0.0) x_ortho = 0.0;
	  GLfloat y_ortho = yOrtho(y);	 
	  if (y_ortho > 1.0) y_ortho = 1.0;
	  if (y_ortho < 0.0) y_ortho = 0.0;
	  
	  rho[1][0] = (GLfloat)( x_ortho );
	  rho[1][1] = (GLfloat)( y_ortho );	  
	}
      else if (picked_point == RHO_2)
	{
	  GLfloat x_ortho = xOrtho(x);
	  if (x_ortho > 1.0) x_ortho = 1.0;
	  if (x_ortho < 0.0) x_ortho = 0.0;
	  GLfloat y_ortho = yOrtho(y);	 
	  if (y_ortho > 1.0) y_ortho = 1.0;
	  if (y_ortho < 0.0) y_ortho = 0.0;
	  
	  rho[2][0] = (GLfloat)( x_ortho );
	  rho[2][1] = (GLfloat)( y_ortho );	  
	}
    }
}


bool illuminationControl::writeIC(char* filename)
{

  cout << "Wrote illumination control to file" << endl;

  ofstream output(filename);

  if(output.fail())
    return false;

  output << "# Illumination Control #" << endl;

  output << rho[0][0] << " " << rho[0][1] << 
    rho[1][0] << " " << rho[1][1] << 
    rho[2][0] << " " << rho[2][1] << 
    " " << ks << " " << kd << " " << ka << " " << alphai << endl;

  output.close();

  return true;
}

/**
 * Read the control variables from a given file.
 * @param filename Name of the file.
 * @return 0 if failed, 1 otherwise.
 **/
bool illuminationControl::readIC(char* filename)
{
  char buffer[200];

  ifstream input(filename);

  if(input.fail())
    return false;

  //read the first line
  input.getline(buffer, 200);

  input >> rho[0][0] >> rho[0][1] >> 
    rho[1][0] >> rho[1][1] >> 
    rho[2][0] >> rho[2][1] >> 
    ks >> kd >> ka >> alphai;

  input.close();

  return true;
}


/**
 * Renders the illumination control window.
 **/
void illuminationControl::draw(void)
{

  /// Draw ks Line

  glBegin(GL_LINES);
  glColor3f(1.0, 1.0, 1.0);
  glVertex2d(0.0, KS_Y_POS);
  glColor3f(0.0, 0.0, 0.0);
  glVertex2d(1.0, KS_Y_POS);
  glEnd();

  /// Draw kd Line

  glBegin(GL_LINES);
  glColor3f(1.0, 1.0, 1.0);
  glVertex2d(0.0, KD_Y_POS);
  glColor3f(0.0, 0.0, 0.0);
  glVertex2d(1.0, KD_Y_POS);
  glEnd();

  /// Draw ka Line

  glBegin(GL_LINES);
  glColor3f(1.0, 1.0, 1.0);
  glVertex2d(0.0, KA_Y_POS);
  glColor3f(0.0, 0.0, 0.0);
  glVertex2d(1.0, KA_Y_POS);
  glEnd();

  /// Draw alphai Line

  glBegin(GL_LINES);
  glColor3f(1.0, 1.0, 1.0);
  glVertex2d(0.0, ALPHAI_Y_POS);
  glColor3f(0.0, 0.0, 0.0);
  glVertex2d(1.0, ALPHAI_Y_POS);
  glEnd();

  // Draw Axis' Background

  glBegin(GL_QUADS);
  glColor3f(1.0, 0.0, 0.0);
  glVertex2d(0.0, 0.0);

  glColor3f(1.0, 1.0, 1.0);
  glVertex2d(0.0, 1.0);

  glColor3f(0.0, 0.0, 1.0);
  glVertex2d(1.0, 1.0);

  glColor3f(0.0, 0.0, 0.0);
  glVertex2d(1.0, 0.0);
  glEnd();

  /// Draw Axis

  glColor3f(0.0, 0.0, 0.0);
  glBegin(GL_LINES);
  glVertex2d(0.0, 0.0);
  glVertex2d(0.0, 1.1);
  glVertex2d(0.0, 0.0);
  glVertex2d(1.1, 0.0);
	
  glVertex2d(0.0, 1.1);
  glVertex2d(0.02, 1.08);
  glVertex2d(0.0, 1.1);
  glVertex2d(-0.02, 1.08);
	
  glVertex2d(1.1, 0.0);
  glVertex2d(1.08, -0.02);
  glVertex2d(1.1, 0.0);
  glVertex2d(1.08, 0.02);
  glEnd();
	
  glEnable(GL_LINE_STIPPLE);
  glLineStipple(4, 0xAAAA);
  glBegin(GL_LINES);
  glVertex2d(0.0, 1.0);
  glVertex2d(1.0, 1.0);
  glVertex2d(1.0, 0.0);
  glVertex2d(1.0, 1.0);
  glEnd();
  glDisable(GL_LINE_STIPPLE);

  /// Draw Control Points

  glPointSize(5.0);
  glBegin(GL_POINTS);	
  glColor3f(1.0, 1.0, 0.0);
  glVertex2d(rho[0][0], rho[0][1]);
  glVertex2d(rho[1][0], rho[1][1]);
  glVertex2d(rho[2][0], rho[2][1]);
  glColor3f(0.0, 0.0, 0.0);
  glVertex2d(ks/KS_MAX, KS_Y_POS);
  glVertex2d(kd/KD_MAX, KD_Y_POS);
  glVertex2d(ka/KA_MAX, KA_Y_POS);
  glVertex2d(alphai/ALPHAI_MAX, ALPHAI_Y_POS);
  glEnd();
}

/*
  Selects a transfer function control point on the screen
*/
void illuminationControl::pick(int x, int y)
{
  picked_point = -1;

  if ((y >= yScreen(KS_Y_POS) - IC_PICK_BOX) &&
      (y <= yScreen(KS_Y_POS) + IC_PICK_BOX))
    {
      if ( (x >= xScreen((GLfloat)((ks/KS_MAX) - IC_PICK_BOX)) )
	   && (x <= xScreen((GLfloat)((ks/KS_MAX) + IC_PICK_BOX))) )
	{
	  picked_point = KS;
	  return;
	}
    }

  if ((y >= yScreen(KD_Y_POS) - IC_PICK_BOX) &&
      (y <= yScreen(KD_Y_POS) + IC_PICK_BOX))
    {
      if ( (x >= xScreen((GLfloat)((kd/KD_MAX) - IC_PICK_BOX)) )
	   && (x <= xScreen((GLfloat)((kd/KD_MAX) + IC_PICK_BOX))) )
	{
	  picked_point = KD;
	  return;
	}
    }

  if ((y >= yScreen(KA_Y_POS) - IC_PICK_BOX) &&
      (y <= yScreen(KA_Y_POS) + IC_PICK_BOX))
    {
      if ( (x >= xScreen((GLfloat)((ka/KA_MAX) - IC_PICK_BOX)) )
	   && (x <= xScreen((GLfloat)((ka/KA_MAX) + IC_PICK_BOX))) )
	{
	  picked_point = KA;
	  return;
	}
    }

  if ((y >= yScreen(ALPHAI_Y_POS) - IC_PICK_BOX) &&
      (y <= yScreen(ALPHAI_Y_POS) + IC_PICK_BOX))
    {
      if ( (x >= xScreen((GLfloat)((alphai/ALPHAI_MAX) - IC_PICK_BOX)) )
	   && (x <= xScreen((GLfloat)((alphai/ALPHAI_MAX) + IC_PICK_BOX))) )
	{
	  picked_point = ALPHAI;
	  return;
	}
    }

  if ( (y >= yScreen((GLfloat)rho[0][1]) - IC_PICK_BOX) &&
       (y <= yScreen((GLfloat)rho[0][1]) + IC_PICK_BOX) )
    { 
      if ( (x >= xScreen((GLfloat)rho[0][0]) - IC_PICK_BOX) &&
	   (x <= xScreen((GLfloat)rho[0][0]) + IC_PICK_BOX) )
	{
	  picked_point = RHO_0;
	  return;
	}
    }
  if ( (y >= yScreen((GLfloat)rho[1][1]) - IC_PICK_BOX) &&
       (y <= yScreen((GLfloat)rho[1][1]) + IC_PICK_BOX) )
    { 
      if ( (x >= xScreen((GLfloat)rho[1][0]) - IC_PICK_BOX) &&
	   (x <= xScreen((GLfloat)rho[1][0]) + IC_PICK_BOX) )
	{
	  picked_point = RHO_1;
	  return;
	}
    }
  if ( (y >= yScreen((GLfloat)rho[2][1]) - IC_PICK_BOX) &&
       (y <= yScreen((GLfloat)rho[2][1]) + IC_PICK_BOX) )
    { 
      if ( (x >= xScreen((GLfloat)rho[2][0]) - IC_PICK_BOX) &&
	   (x <= xScreen((GLfloat)rho[2][0]) + IC_PICK_BOX) )
	{
	  picked_point = RHO_2;
	  return;
	}
    }
}
