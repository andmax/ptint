/**
 *
 *    Render Volume GPU
 *
 *  File: bbox_edit.h
 *
 *  Author:
 *    Andre Maximo
 *
 *  Created: Sep 04, 2006
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

#define BBOX_PICK_BOX 5

#define BBOX_ORTHO_MIN -0.2
#define BBOX_ORTHO_MAX 1.3
#define BBOX_ORTHO_SIZE (BBOX_ORTHO_MAX - BBOX_ORTHO_MIN)

/// Extern screen size

extern GLint bBoxWinWidth, bBoxWinHeight;

/// A class to handle the Bounding Box of the Volume

class bBox
{
 public:
  /// Constructor
  bBox() {
    picked_line = 0;
    min[0] = 0.0; min[0] = 0.0;
    max[0] = 1.0; max[1] = 1.0;
    rows = 100; cols = 100; nums = 100;
    xy_step = 1; z_step = 1;
    offset_x_begin = 0; offset_x_end = 0;
    offset_y_begin = 0; offset_y_end = 0;
    offset_z_begin = 0; offset_z_end = 0;
    updateMinMax();
  }

  /// Destructor
  ~bBox() { }

  /// Set original size
  void setSize(uint r, uint c, uint s);

  /// Test if the values are valid

  bool testValid(void) {
    return (!((rows - (offset_x_end + offset_x_begin)) <= 0 ) ||
	    ((cols - (offset_y_end + offset_y_begin)) <= 0 ) ||
	    ((nums - (offset_z_end + offset_z_begin)) <= 0 ) );
  }

  /// Get dimensions
  uint getDimX(void) { return (uint)ceil( (rows - (offset_x_end + offset_x_begin)) / (GLfloat)xy_step ); }
  uint getDimY(void) { return (uint)ceil( (cols - (offset_y_end + offset_y_begin)) / (GLfloat)xy_step ); }
  uint getDimZ(void) { return (uint)ceil( (nums - (offset_z_end + offset_z_begin)) / (GLfloat)z_step  ); }

  /// Get min and max
  GLfloat getMinX(void) { return offset_x_begin; }
  GLfloat getMinY(void) { return offset_y_begin; }
  GLfloat getMaxX(void) { return rows - offset_x_end; }
  GLfloat getMaxY(void) { return cols - offset_y_end; }

  /// Get Offsets
  uint getOffsetMinX(void) { return offset_x_begin; }
  uint getOffsetMinY(void) { return offset_y_begin; }
  uint getOffsetMinZ(void) { return offset_z_begin; }
  uint getOffsetMaxX(void) { return offset_x_end; }
  uint getOffsetMaxY(void) { return offset_y_end; }
  uint getOffsetMaxZ(void) { return offset_z_end; }

  /// Get Steps
  uint getStepXY(void) { return xy_step; }
  uint getStepZ(void) { return z_step; }

  /// Get picked line
  uint getPickedLine(void) { return picked_line; }

  /// Selects a control line on the screen
  void pick(int x, int y);

  /// Set picked line to Null
  void set_picked_null() { picked_line = 0; }

  /// Draw the bBox
  void draw(void);

  /// Update bBox (from screen)
  void update(int x, int y);

  void updateMinMax(void) {
    min[0] = offset_x_begin * uX();
    max[0] = (rows - offset_x_end) * uX();
    min[1] = offset_y_begin * uY();
    max[1] = (cols - offset_y_end) * uY();
  }

  void updateOffset(void) {
    offset_x_begin = (uint)ceil( min[0] / uX() );
    offset_x_end = (uint)ceil( rows - (max[0] / uX() ) );
    offset_y_begin = (uint)ceil( min[1] / uY() );
    offset_y_end = (uint)ceil( cols - (max[1] / uY() ) );
  }

 private:
  uint picked_line;
  GLfloat min[2], max[2];
  uint rows, cols, nums;
  uint xy_step, z_step,
    offset_x_begin, offset_x_end,
    offset_y_begin, offset_y_end,
    offset_z_begin, offset_z_end;

  inline GLfloat uX(void) { return ( 1.0 / (GLfloat) rows ); }
  inline GLfloat uY(void) { return ( 1.0 / (GLfloat) cols ); }

  inline GLfloat xOrtho(int x) { return (GLfloat)( ((x/(GLfloat)bBoxWinWidth) * BBOX_ORTHO_SIZE) + BBOX_ORTHO_MIN ); }
  inline GLfloat yOrtho(int y) { return (GLfloat)(((((GLfloat)bBoxWinHeight - y)/(GLfloat)bBoxWinHeight) * BBOX_ORTHO_SIZE) + BBOX_ORTHO_MIN); }
  inline int xScreen(GLfloat x) { return (int)(((x - BBOX_ORTHO_MIN)*(GLfloat)bBoxWinWidth)/BBOX_ORTHO_SIZE); }
  inline int yScreen(GLfloat y) { return bBoxWinHeight - (int)(((y - BBOX_ORTHO_MIN)*(GLfloat)bBoxWinHeight)/BBOX_ORTHO_SIZE); }

};
