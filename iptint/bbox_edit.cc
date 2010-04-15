/**
 *
 *    Render Volume GPU
 *
 *  File: bbox_edit.cc
 *
 *  Author:
 *    Andre Maximo
 *
 *  Created: Sep 4, 2006
 *
 */

#include "bbox_edit.h"

/// Selects a control line on the screen
void bBox::pick(int x, int y)
{
  //       3
  //   ---------
  //   |       |
  // 4 |       | 2
  //   |       |
  //   ---------     0 = no line picked
  //       1
  if ((y >= (yScreen(min[1]) - BBOX_PICK_BOX)) &&
      (y <= (yScreen(min[1]) + BBOX_PICK_BOX)) ) {
    if ((x >= xScreen(min[0])) && (x <= xScreen(max[0])) )
      picked_line = 1;
  }
  if ((x >= (xScreen(max[0]) - BBOX_PICK_BOX)) &&
      (x <= (xScreen(max[0]) + BBOX_PICK_BOX)) ) {
    if ((y >= yScreen(max[1])) && (y <= yScreen(min[1])) )
      picked_line = 2;
  }
  if ((y >= (yScreen(max[1]) - BBOX_PICK_BOX)) &&
      (y <= (yScreen(max[1]) + BBOX_PICK_BOX)) ) {
    if ((x >= xScreen(min[0])) && (x <= xScreen(max[0])) )
      picked_line = 3;
  }
  if ((x >= (xScreen(min[0]) - BBOX_PICK_BOX)) &&
      (x <= (xScreen(min[0]) + BBOX_PICK_BOX)) ) {
    if ((y >= yScreen(max[1])) && (y <= yScreen(min[1])) )
      picked_line = 4;
  }
}

/// Display the bBox
void bBox::draw(void)
{
  /// Draw Max bBox
  glEnable(GL_LINE_STIPPLE);
  glLineStipple(4, 0xFFFF);
  glColor3f(0.9, 0.9, 0.9);
  glBegin(GL_LINE_STRIP);
  glVertex2d(0.0, 0.0);
  glVertex2d(1.0, 0.0);
  glVertex2d(1.0, 1.0);
  glVertex2d(0.0, 1.0);
  glVertex2d(0.0, 0.0);
  glEnd();
  glDisable(GL_LINE_STIPPLE);

  /// Draw picked line
  glColor3f(1.0, 0.0, 0.0);
  glBegin(GL_LINES);
  if (picked_line == 1) {
    glVertex2d(min[0], min[1]);
    glVertex2d(max[0], min[1]);
  }
  if (picked_line == 2) {
    glVertex2d(max[0], min[1]);
    glVertex2d(max[0], max[1]);
  }
  if (picked_line == 3) {
    glVertex2d(max[0], max[1]);
    glVertex2d(min[0], max[1]);
  }
  if (picked_line == 4) {
    glVertex2d(min[0], max[1]);
    glVertex2d(min[0], min[1]);
  }
  glEnd();

  /// Draw Current bBox
  glEnable(GL_LINE_STIPPLE);
  glLineStipple(4, 0xAAAA);
  glColor3f(0.0, 0.0, 0.0);
  glBegin(GL_LINE_STRIP);
  glVertex2d(min[0], min[1]);
  glVertex2d(max[0], min[1]);
  glVertex2d(max[0], max[1]);
  glVertex2d(min[0], max[1]);
  glVertex2d(min[0], min[1]);
  glEnd();
  glDisable(GL_LINE_STIPPLE);
}

/// Update bBox
void bBox::update(int x, int y)
{
  if (picked_line != 0) { // if has a picked line

    GLfloat x_ortho = xOrtho(x);
    if (x_ortho > 1.0) x_ortho = 1.0;
    if (x_ortho < 0.0) x_ortho = 0.0;

    GLfloat y_ortho = yOrtho(y);
    if (y_ortho > 1.0) y_ortho = 1.0;
    if (y_ortho < 0.0) y_ortho = 0.0;

    if (picked_line == 1)
      if (y_ortho < max[1])
	min[1] = y_ortho;      
    if (picked_line == 2)
      if (x_ortho > min[0])
	max[0] = x_ortho;
    if (picked_line == 3)
      if (y_ortho > min[1])
	max[1] = y_ortho;
    if (picked_line == 4)
      if (x_ortho < max[0])
	min[0] = x_ortho;
    updateOffset();
  }
}

/// Set Size
void bBox::setSize(uint r, uint c, uint s)
{
  rows = r; cols = c; nums = s;

  /* Skull-set
  offset_x_begin = 90; offset_x_end = 90;
  offset_y_begin = 25; offset_y_end = 90;
  offset_z_begin = 0; offset_z_end = 0;
  */

  /* CT-Brain-set
  offset_x_begin = 90; offset_x_end = 90;
  offset_y_begin = 25; offset_y_end = 25;
  offset_z_begin = 300; offset_z_end = 205;
  */

  /* Heart1-set
  offset_x_begin = 20; offset_x_end = 20;
  offset_y_begin = 20; offset_y_end = 20;
  offset_z_begin = 7; offset_z_end = 0;
  */

  /* Heart2-set
  offset_x_begin = 20; offset_x_end = 20;
  offset_y_begin = 20; offset_y_end = 20;
  offset_z_begin = 1; offset_z_end = 9;
  */

  /* Heart3-set */
  offset_x_begin = 45; offset_x_end = 45;
  offset_y_begin = 80; offset_y_end = 20;
  offset_z_begin = 4; offset_z_end = 6;

  uint dX, dY, dZ, nT;
  dX = getDimX(); dY = getDimY(); dZ = getDimZ();
  nT = (dX-1)*(dY-1)*(dZ-1)*5;

#ifndef NO_NVIDIA
#define MAX_TETS 6000000
#else
#define MAX_TETS 500000
#endif
  while (nT > MAX_TETS) { // crashes if more than 6.0 M Tets
    if (dX == dY) {
      offset_x_begin += 1; offset_x_end += 1;
      offset_y_begin += 1; offset_y_end += 1;
    } else if (dX == dZ) {
      offset_x_begin += 1; offset_x_end += 1;
      offset_z_begin += 1; offset_z_end += 1;
    } else if (dY == dZ) {
      offset_y_begin += 1; offset_y_end += 1;
      offset_z_begin += 1; offset_z_end += 1;
    } else {
      offset_x_begin += 1; offset_x_end += 1;
      offset_y_begin += 1; offset_y_end += 1;
      offset_z_begin += 1; offset_z_end += 1;
    }
    dX = getDimX(); dY = getDimY(); dZ = getDimZ();
    nT = (dX-1)*(dY-1)*(dZ-1)*5;
  }

  updateMinMax();
}
