/**
 *
 *    Render Volume GPU
 *
 *  File: render_volume_gpu.cc
 *
 *  Authors:
 *    Andre Maximo
 *    Ricardo Marroquim
 *
 *  Last Update: May 10, 2006
 *
 */
   
#include <string>

#include <sstream>
#include <iostream>

#include <iomanip>
#include <cmath>

#include <cstring>

#include "volume.h"

#include "ftrackball.h"

using namespace std;

/// Global Variables

/// the volume
static volume* vol;

#include "main_utils.h"

// names

#ifndef NO_NVIDIA

const char titleModelWin[30] = "GPU Volume Rendering";
const char runingIn[4] = "GPU";

#else

const char titleModelWin[30] = "CPU Volume Rendering";
const char runingIn[4] = "CPU";

#endif

const char titleTFWin[30] = "Transfer Function";

const char titleICWin[30] = "Illumination Control";

static char *volume_name, *tfName;

static fileType fType;

// windows
static int modelWindow, tfWindow, icWindow;

// fps computation
static int time_frame = 0,
  gpu_frames = 0;

// mouse click variables
static bool button_pressedModelWin[2] = {false, false};
static bool button_pressedTFWin = false;
static bool button_pressedICWin = false;

// trackball
static FTrackBall track;

static GLfloat rotX = 0.0, rotY = 0.0, rotZ = 0.0;

// screen variable
GLint modelWinWidth=800, modelWinHeight=800,
  tfWinWidth=400, tfWinHeight=300,
  icWinWidth=400, icWinHeight=300;

// flag controls
bool integrating = true,
  shading = true,
  debug_cout = true,
  debug_shaders = true,
  sorting = true;

static bool rotate_always = false,
  show_debug = true,
  show_help = false,
  tf_visible = true,
  ic_visible = true,
  write_in_file = false;

static bool rotated = true,
  first_still_frame = false,
  tf_updated = false;

// debug timers
static double update_time=0.0,
  sorting_time=0.0,
  setup_time=0.0,
  draw_time=0.0,
  total_time=0.0;

static unsigned long int time_elapsed = 0;
static bool write_once = false;

//static double gpu_fps = 0.0;

static uint num_tets = 0, num_verts = 0;

static uint dimensionX, dimensionY, dimensionZ;

static bool clear_white = false,
            text_white = true;

// information file
static ofstream info_file;

/// Static Local Functions

static void init(void);
static void reshapeModelWin(int w, int h);
static void reshapeTFWin(int w, int h);
static void reshapeICWin(int w, int h);
static void idle(void);
static void glWrite(GLdouble x, GLdouble y, char *str);
static bool displayDebugModel(void);
static void displayDebugTF(void);
static void displayDebugIC(void);
static void displayModelWin(void);
static void displayTFWin(void);
static void displayICWin(void);
static void mouseModelWin(GLint button, GLint state, GLint x, GLint y);
static void motionModelWin(GLint x, GLint y);
static void keyboardModelWin(unsigned char key, int x, int y);
static void mouseTFWin(GLint button, GLint state, GLint x, GLint y);
static void motionTFWin(GLint x, GLint y);
static void keyboardTFWin(unsigned char key, int x, int y);
static void mouseICWin(GLint button, GLint state, GLint x, GLint y);
static void motionICWin(GLint x, GLint y);
static void keyboardICWin(unsigned char key, int x, int y);
static void commandMenuModel(int value);

/// Initialization (global)

void init(void) 
{
  vol->CreateTextures();
    
  num_verts = vol->getNumVerts();
  num_tets = vol->getNumTets();

  track.setZoom(1.0);

  if (write_in_file) {
    info_file.open("last_info_file.txt");

    if (info_file.fail())
      exit(0);

    info_file << "* " << runingIn << " Volume Rendering - Information file" << endl;
    info_file << "* Filename: " << volume_name << endl;
    if (dimensionX > 0) {
      info_file << "* Dimension: " << dimensionX << " x " << dimensionY << " x " << dimensionZ
		<< " Total Tets: " << (dimensionX-1)*(dimensionY-1)*(dimensionZ-1)*5 << endl;
    }
    info_file << "* Resolution: " << modelWinWidth << " x " << modelWinHeight << endl;
    info_file << "* Num Tets: " << num_tets << " ; Num Vertices: " << num_verts << endl;
    info_file << "* Psi-Gama Table: " << vol->getPsiGamaTableSize() << endl;
    info_file << "* Exponential Table: " << vol->getExpTexSize() << endl;
    info_file << "* Table (average times in seconds):" << endl;
    info_file << "---------------------------------------------------------------------------" << endl;
    info_file << "Time |   FPS   |  K Tets  |  Update   | Sorting   |  Setup    |   Draw    |" << endl;
    info_file << "---------------------------------------------------------------------------" << endl;
  }
}

/// Reshape (model)

void reshapeModelWin(int w, int h)
{
  glutSetWindow(modelWindow);
  glViewport(0, 0, modelWinWidth=w, modelWinHeight=h);
}

/// Reshape (tf)

void reshapeTFWin(int w, int h)
{
  glutSetWindow(tfWindow);
  glViewport(0, 0, tfWinWidth=w, tfWinHeight=h);
}

/// Reshape (IC)

void reshapeICWin(int w, int h)
{
  glutSetWindow(icWindow);
  glViewport(0, 0, icWinWidth=w, icWinHeight=h);
}

/// Idle (global)

void idle(void)
{
  if (rotate_always) {
    track.trackBallInAction(track.getOldX()+4, track.getOldY()+8,
			    modelWinWidth, modelWinHeight);
    rotated = true;
    glutSetWindow(modelWindow);
    glutPostRedisplay();
    return; // don't redisplay again
  }

  if (show_debug) {
    glutSetWindow(modelWindow);
    glutPostRedisplay(); // to update the debug information
  }
}

/// OpenGL Write (global)

void glWrite(GLdouble x, GLdouble y, char *str) {
  glRasterPos2d(x, y);
  for (char *s = str; *s; ++s)
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *s);
}

/// Display Debug Text (model)

bool displayDebugModel(void)
{
  ostringstream oss_update, oss_sort, oss_setup, oss_draw;
  ostringstream oss_fps, oss_tps, oss_brightness;
  ostringstream oss_verts, oss_tets, oss_res;

  GLfloat actual_brightness = vol->tf.getBrightness();

  uint cur_num_tets = vol->getCurTets();

  // trunc 3 decimals (and 2 for percentuals and rates)
  double update = (uint)(update_time*1000.0)/1000.0;
  double update_perc = (uint)((update_time/total_time)*10000.0)/100.0;
  double sort = (uint)(sorting_time*1000.0)/1000.0;
  double sort_perc = (uint)((sorting_time/total_time)*10000.0)/100.0;
  double setup = (uint)(setup_time*1000.0)/1000.0;
  double setup_perc = (uint)((setup_time/total_time)*10000.0)/100.0;
  double draw = (uint)(draw_time*1000.0)/1000.0;
  double draw_perc = (uint)((draw_time/total_time)*10000.0)/100.0;
  double fps = (uint)((1.0/total_time)*100.0)/100.0;
  //long double gpu_tets = gpu_fps*num_tets/1000.0; // K Tets / sec

  long double gpu_tets = ((long double)cur_num_tets/total_time)/1000.0; // K Tets / sec
  long double tets = (unsigned long int)(gpu_tets*100.0)/100.0; // trunc 2 decimals

  oss_update << runingIn << " Update: " << update << " s (" << update_perc << "%)";

  oss_sort << "CPU Sort: " << sort << " s (" << sort_perc << " %)";

  oss_setup << "CPU Setup: " << setup << " s (" << setup_perc << " %)";

  oss_draw << runingIn << " Draw: " << draw << " s (" << draw_perc << " %)";

  oss_fps << "FPS: " << fps;
  oss_tps << "Tet/s: " << tets << " K";
    
  oss_brightness << "Brightness: " << actual_brightness;

  oss_verts << "# Vertices: " << num_verts;	
  oss_tets << "# Tetrahedra: " << cur_num_tets << " / " << num_tets;
  oss_res << "Resolution: " << modelWinWidth << " x " << modelWinHeight;	

  if (text_white)
    glColor4f(1.0, 1.0, 1.0, 1.0);
  else
    glColor4f(0.0, 0.0, 0.0, 1.0);

  glWrite(-0.3, 1.1, volume_name);

  glWrite(-1.1, 0.9, (char*) oss_update.str().c_str());
  glWrite(-1.1, 0.8, (char*) oss_sort.str().c_str());
  glWrite(-1.1, 0.7, (char*) oss_setup.str().c_str());
  glWrite(-1.1, 0.6, (char*) oss_draw.str().c_str());
  glWrite(-1.1, 0.5, (char*) oss_fps.str().c_str());
  glWrite(-1.1, 0.4, (char*) oss_tps.str().c_str());
  
  glWrite(-1.1, -0.7, (char*) oss_brightness.str().c_str());
  glWrite(-1.1, -0.8, (char*) oss_verts.str().c_str());
  glWrite(-1.1, -0.9, (char*) oss_tets.str().c_str());
  glWrite(-1.1, -1.0, (char*) oss_res.str().c_str());

  if (sorting)
    glWrite(0.8, -0.8, "* sorting *");
  if (rotate_always)
    glWrite(0.8, -0.9, "* rotating *");
  if (integrating)
    glWrite(0.8, -1.0, "* integrating *");
  if (shading)
    glWrite(0.8, -1.1, "* shading *");

  if (write_in_file)
    {
      // total average values
      static long double total_fps = 0.0, total_tets = 0.0, total_update = 0.0,
	total_sorting = 0.0, total_setup = 0.0, total_draw = 0.0, num_sums = 0.0;
      // final average (fa) values
      static long double fa_total_fps = 0.0, fa_total_tets = 0.0, fa_total_update = 0.0,
	fa_total_sorting = 0.0, fa_total_setup = 0.0, fa_total_draw = 0.0, fa_num_sums = 0.0;

      total_fps += fps;
      total_tets += tets;
      total_update += update;
      total_sorting += sort;
      total_setup += setup;
      total_draw += draw;
      num_sums += 1.0;

      if (write_once) { // write in file every 16 s
	
	info_file << setw(5) << time_elapsed << " "
		  << setw(9) << total_fps / num_sums << " "
		  << setw(9) << total_tets / num_sums << " "
		  << setw(11) << total_update / num_sums << " "
		  << setw(11) << total_sorting / num_sums << " "
		  << setw(11) << total_setup / num_sums << " "
		  << setw(11) << total_draw / num_sums << " "
		  << endl;

	fa_total_fps += total_fps;
	fa_total_tets += total_tets;
	fa_total_update += total_update;
	fa_total_sorting += total_sorting;
	fa_total_setup += total_setup;
	fa_total_draw += total_draw;
	fa_num_sums += num_sums;

	total_fps = 0.0; total_tets = 0.0; total_update = 0.0;
	total_sorting = 0.0; total_setup = 0.0; total_draw = 0.0;
	num_sums = 0;
	write_once = false;
      }

      if (time_elapsed >= 128) { // exit after 128 s
	// write the final average values
	
	info_file << "---------------------------------------------------------------------------" << endl;
	info_file << " avg "
		  << setw(9) << fa_total_fps / fa_num_sums << " "
		  << setw(9) << fa_total_tets / fa_num_sums << " "
		  << setw(11) << fa_total_update / fa_num_sums << " "
		  << setw(11) << fa_total_sorting / fa_num_sums << " "
		  << setw(11) << fa_total_setup / fa_num_sums << " "
		  << setw(11) << fa_total_draw / fa_num_sums << " "
		  << endl;
	info_file << "---------------------------------------------------------------------------" << endl;
	info_file << endl;

	glutDestroyWindow(modelWindow);
	glutDestroyWindow(tfWindow);
	glutDestroyWindow(icWindow);
	return false;
      }
    }
  return true;
}

/// Display (model)

void displayModelWin(void)
{
  static int sta_sh=0, end_sh=0, now=0;
  static double total_dt=0;
  double update_dt=0, sorting_dt=0, setup_dt=0, draw_dt=0;
   
  glClear(GL_COLOR_BUFFER_BIT);

  now = glutGet(GLUT_ELAPSED_TIME);
  total_dt = ( now - time_frame ) / 1000.0;
  if (total_dt >= 8.0) {
    //gpu_fps = gpu_frames / total_dt;
    gpu_frames = 0;
    time_frame = now;
    time_elapsed += 8;
    if ((time_elapsed % 16) == 0)
      write_once = true;
  }

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glPushMatrix();

  glRotated(rotX, 1.0, 0.0, 0.0);
  glRotated(rotY, 0.0, 1.0, 0.0);
  glRotated(rotZ, 0.0, 0.0, 1.0);

  track.multToOpenGL();

  double zoom = track.getZoom();
  glScaled(zoom, zoom, zoom);

  //--- First Frame ---
  if (first_still_frame)
    {
      //--- Sorting ---
      if (show_debug)
	sta_sh = glutGet(GLUT_ELAPSED_TIME);

      vol->centroidSorting();
  
      if (show_debug) {
	end_sh = glutGet(GLUT_ELAPSED_TIME);
	sorting_dt = (end_sh - sta_sh) / 1000.0;	
      }

      //--- Setup Array ---
      if (show_debug)
	sta_sh = glutGet(GLUT_ELAPSED_TIME);
	
      vol->SetupArrays(true);
	
      if (show_debug) {
	end_sh = glutGet(GLUT_ELAPSED_TIME);
	setup_dt = (end_sh - sta_sh) / 1000.0;
      }

      first_still_frame = false;
    }

  //--- Update ---
  if (rotated)
    {
      //--- Update data ---
      if (show_debug)
	sta_sh = glutGet(GLUT_ELAPSED_TIME);
	
      vol->UpdateData();
	
      if (show_debug) {
	end_sh = glutGet(GLUT_ELAPSED_TIME);
	update_dt = (end_sh - sta_sh) / 1000.0;	
      }
	
      //--- Sorting ---
      if (show_debug)
	sta_sh = glutGet(GLUT_ELAPSED_TIME);

      vol->bucketSorting();
  
      if (show_debug) {
	end_sh = glutGet(GLUT_ELAPSED_TIME);
	sorting_dt = (end_sh - sta_sh) / 1000.0;	
      }
	
      //--- Setup Array ---
      if (show_debug)
	sta_sh = glutGet(GLUT_ELAPSED_TIME);
	
      vol->SetupArrays(false);
	
      if (show_debug) {
	end_sh = glutGet(GLUT_ELAPSED_TIME);
	setup_dt = (end_sh - sta_sh) / 1000.0;
      }
	
      rotated = false;
      first_still_frame = true;
    }

  //--- TF Updated  ---
  if (tf_updated)
    {
      //--- Update data ---
      if (show_debug)
	sta_sh = glutGet(GLUT_ELAPSED_TIME);
	
      vol->UpdateData();
	
      if (show_debug) {
	end_sh = glutGet(GLUT_ELAPSED_TIME);
	update_dt = (end_sh - sta_sh) / 1000.0;	
      }

      //--- Setup Array ---
      if (show_debug)
	sta_sh = glutGet(GLUT_ELAPSED_TIME);
	
      vol->SetupArrays(true);
	
      if (show_debug) {
	end_sh = glutGet(GLUT_ELAPSED_TIME);
	setup_dt = (end_sh - sta_sh) / 1000.0;
      }

      tf_updated = false;
    }

  //--- Draw ---
  if (show_debug)
    sta_sh = glutGet(GLUT_ELAPSED_TIME);
  
  vol->Draw();
    
  if (show_debug) {
    end_sh = glutGet(GLUT_ELAPSED_TIME);
    draw_dt = (end_sh - sta_sh) / 1000.0;
  }

  glPopMatrix();

  //--- Text ---
  if (show_debug) {


    //if (gpu_frames == 0)
    //{
    update_time = update_dt;
    sorting_time = sorting_dt;
    setup_time = setup_dt;
    draw_time = draw_dt;
    total_time = update_time + sorting_time + setup_time + draw_time;
    if (!displayDebugModel())
      return;
    //}
  }

  glutSwapBuffers();

  ++gpu_frames;
}

/// Display Debug Text (tf)

void displayDebugTF(void)
{
  //--- Text ---
  glColor3f(0.0, 0.0, 0.0);
  glWrite(0.25, 1.2, tfName);

  glWrite(-0.05, 1.12, "Alpha");
  glWrite(1.12, -0.01, "Scalar");
  glWrite(1.04, -0.16, "Brightness");

  int pPoint = vol->tf.getPickedPoint();

  if (pPoint != -1) {
    glColor3f(0.7, 0.3, 0.1);
    if (pPoint == vol->tf.getBrightnessId()) { // brightness
      GLfloat curBrightness = (int)(vol->tf.getBrightness()*100.0)/100.0;
      ostringstream oss_brightness;
      oss_brightness << curBrightness;
      glWrite(curBrightness/BRIGHTNESS_MAX - 0.03, -0.12,
	      (char*)oss_brightness.str().c_str());      
    }
    else { // control points
      GLfloat curAlpha = (int)(vol->tf.getCurAlpha()*100.0)/100.0;
      GLfloat curScalar = (int)(vol->tf.getCurScalar()*100.0)/100.0;
      ostringstream oss_alpha, oss_scalar;
      oss_alpha << curAlpha;
      oss_scalar << curScalar;
      glWrite(-0.12, curAlpha, (char*)oss_alpha.str().c_str());
      glWrite(curScalar/255.0 - 0.03, -0.05,
	      (char*)oss_scalar.str().c_str());
    }
  }
}

/// Display Debug Text (ic)

void displayDebugIC(void)
{
  //--- Text ---
  glColor3f(0.0, 0.0, 0.0);
  glWrite(0.25, 1.2, "Illumination Control");

  glWrite(-0.05, 1.12, "Threshold");
  glWrite(1.12, -0.01, "Range");
  glWrite(1.04, -0.06, "Specular");
  glWrite(1.04, -0.16, "Diffuse");
  glWrite(1.04, -0.26, "Ambient");
  glWrite(1.04, -0.36, "Alpha I");
}

/// Display (tf)

void displayTFWin(void)
{
  glClearColor(1.0, 1.0, 1.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT);
  
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  //--- Draw ---
  vol->tf.draw();

  if (show_debug)
    displayDebugTF();

  glutSwapBuffers();
}

/// Display (IC)

void displayICWin(void)
{
  glClearColor(1.0, 1.0, 1.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT);
  
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  //--- Draw ---
  vol->ic.draw();

  //--- Text ---
  glColor3f(0.0, 0.0, 0.0);

  if (show_debug)
    displayDebugIC();

  glutSwapBuffers();
}

/// Mouse (model)

void mouseModelWin(GLint button, GLint state, GLint x, GLint y) {

  if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_DOWN))
    button_pressedModelWin[0] = true;
    
  if ((button == GLUT_MIDDLE_BUTTON) && (state == GLUT_DOWN))
    button_pressedModelWin[1] = true;

  if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_UP))
      button_pressedModelWin[0] = false;

  if ((button == GLUT_MIDDLE_BUTTON) && (state == GLUT_UP))
    button_pressedModelWin[1] = false;

  track.setOld(x, y);
}

/// Mouse (tf)

void mouseTFWin(GLint button, GLint state, GLint x, GLint y)
{
  if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_DOWN))
    {
      vol->tf.pick(x, y);	  
      button_pressedTFWin = true;
      glutPostRedisplay();
    }

  if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_UP))
    {
      vol->tf.updateTF(x, y);
      vol->tf.setPickedPointNull();
      vol->tf.computeTF();
      button_pressedTFWin = false;
      glutSetWindow(modelWindow);
      vol->reloadTFTex();
      tf_updated = true;
      glutPostRedisplay();
      glutSetWindow(tfWindow);
      glutPostRedisplay();
    }
}

/// Mouse (IC)

void mouseICWin(GLint button, GLint state, GLint x, GLint y)
{
  if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_DOWN))
    {
      vol->ic.pick(x, y);
      button_pressedICWin = true;
      glutPostRedisplay();
    }
  if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_UP))
    {
      vol->ic.updateIC(x, y);
      vol->ic.setPickedPointNull();
      button_pressedICWin = false;
      glutSetWindow(modelWindow);
      vol->updateIC();
      glutPostRedisplay();
      glutSetWindow(icWindow);
      glutPostRedisplay();
    }

}

/// Motion (model)

void motionModelWin(GLint x, GLint y)
{
  if (button_pressedModelWin[0]) { // rotate
    track.trackBallInAction(x, y, modelWinWidth, modelWinHeight);
    rotated = true;
    glutPostRedisplay();
  }
    
  if (button_pressedModelWin[1]) { // zoom
    track.zoomInAction(y);
    rotated = true;
    glutPostRedisplay();
  }

  track.setOld(x, y);
}

/// Motion (tf)

void motionTFWin(GLint x, GLint y)
{
  if (button_pressedTFWin) 
    {
      vol->tf.updateTF(x, y);
      vol->tf.computeTF();
      glutSetWindow(modelWindow);
      vol->reloadTFTex();
      tf_updated = true;
      glutPostRedisplay();
      glutSetWindow(tfWindow);
      glutPostRedisplay();
    }
}

/// Motion (IC)

void motionICWin(GLint x, GLint y)
{
  if (button_pressedICWin) {
    vol->ic.updateIC(x, y);
    glutSetWindow(modelWindow);
    vol->updateIC();
    glutPostRedisplay();
    glutSetWindow(icWindow);
    glutPostRedisplay();
  }
}

/// keyboard (model)

void keyboardModelWin(unsigned char key, int x, int y)
{
  switch(key) {
  case '0': case '1': case '2': case '3':
  case '4': case '5': case '6': case '7':
  case '8': case '9':
    x = ((int)key)-48; // char to int
    //vol->tf.setColorCode(atoi((const char*)&key));
    vol->tf.setColorCode(x);
    glutSetWindow(tfWindow);
    glutPostRedisplay();
    tf_updated = true;
    glutSetWindow(modelWindow);
    vol->reloadTFTex();
    glutPostRedisplay();
    break;
  case '+':
    vol->tf.updateBrightness(+0.2);
    glutSetWindow(tfWindow);
    glutPostRedisplay();
    rotated = true;
    glutSetWindow(modelWindow);
    glutPostRedisplay();
    break;
  case '-':
    vol->tf.updateBrightness(-0.2);
    glutSetWindow(tfWindow);
    glutPostRedisplay();
    rotated = true;
    glutSetWindow(modelWindow);
    glutPostRedisplay();
    break;
  case 'u': case 'U': // bbox
    ic_visible = !ic_visible;
    glutSetWindow(icWindow);
    if (ic_visible) {
      glutShowWindow();
      glutPostRedisplay();
    }
    else
      glutHideWindow();
    glutSetWindow(modelWindow);
    break;
  case 'c': // touch
    rotX += 1.0; rotY += 1.0; rotZ += 1.0;
    rotated = true;
    glutPostRedisplay();
    break;
  case 'C': // touch
    rotX -= 1.0; rotY -= 1.0; rotZ -= 1.0;
    rotated = true;
    glutPostRedisplay();
    break;
  case 'f': case 'F': // fullscreen
    glutFullScreen();
    break;
  case 'g': case 'G': // backgroung
    text_white = !text_white;
    clear_white = !clear_white;
    glutSetWindow(modelWindow);
    if (clear_white)
      glClearColor(1.0, 1.0, 1.0, 0.0);
    else
      glClearColor(0.0, 0.0, 0.0, 0.0);
    glutPostRedisplay();
    break;
  case 'h': case 'H': // help
    show_debug = false;
    show_help = true;
    break;
  case 'i': case 'I': // integrating
    integrating = !integrating;
    glutPostRedisplay();
    break;
  case 'l': case 'L': // integrating
    shading = !shading;
    glutPostRedisplay();
    break;
  case 'n': case 'N': // no sorting
    sorting = !sorting;
    break;
  case 'o': case 'O': // origin
    rotX = 0.0; rotY = 0.0; rotZ = 0.0;
    track.reset(1.0); 
    rotated = true;
    glutPostRedisplay();
    break;
  case 'q': case 'Q': case 27: // quit
    glutDestroyWindow(modelWindow);
    glutDestroyWindow(tfWindow);
    glutDestroyWindow(icWindow);
    break;
  case 'r': case 'R': // rotate
    rotate_always = !rotate_always;
    rotated = true;
    glutPostRedisplay();
    break;
  case 's': case 'S': // sort
    show_debug = !show_debug;
    glutPostRedisplay();
    break;
  case 't': case 'T': // tf
    tf_visible = !tf_visible;
    glutSetWindow(tfWindow);
    if (tf_visible) {
      glutShowWindow();
      glutPostRedisplay();
    }
    else
      glutHideWindow();
    glutSetWindow(modelWindow);
    break;
  case 'x':
    rotX += 90.0;
    rotated = true;
    glutPostRedisplay();
    break;
  case 'X':
    rotX -= 90.0;
    rotated = true;
    glutPostRedisplay();
    break;
  case 'y':
    rotY += 90.0;
    rotated = true;
    glutPostRedisplay();
    break;
  case 'Y':
    rotY -= 90.0;
    rotated = true;
    glutPostRedisplay();
    break;
  case 'z':
    rotZ += 90.0;
    rotated = true;
    glutPostRedisplay();
    break;
  case 'Z':
    rotZ -= 90.0;
    rotated = true;
    glutPostRedisplay();
    break;
  default: break;
  }
}

/// Keyboard (tf)

void keyboardTFWin(unsigned char key, int x, int y)
{
  switch(key) {
  case 'w': case 'W':
    vol->tf.writeTF(tfName);
    break;
  case 'q': case 'Q': case 'h': case 'H': case 27:
    glutHideWindow();
    break;
  default:
    keyboardModelWin(key, x, y);
    break;
  }
}

/// Keyboard (BBox)

void keyboardICWin(unsigned char key, int x, int y)
{
  switch(key) {
  case 'w': case 'W':
    //vol->ic.writeIC(icName);
    break;
  case 'q': case 'Q': case 'h': case 'H': case 27:
    glutHideWindow();
    break;
  default:
    keyboardModelWin(key, x, y);
    break;
  }
}

/// Command Menu (model)

void commandMenuModel(int value)
{
  keyboardModelWin((unsigned char)value, 0, 0);
}

/// Main

int main(int argc, char** argv)
{
  int sta_in=0, end_in=0;
  double init_time;

  vol = new volume;

  if (show_debug)
    sta_in = glutGet(GLUT_ELAPSED_TIME);

  // Extended options
  if ( ((argc == 4) && (strcmp(argv[3], "-t") == 0)) ||
       ((argc == 3) && (strcmp(argv[2], "-t") == 0)) ) {
    write_in_file = true;
    rotate_always = true;
  }

  if (argc == 1) { // for commodity...
    argv[1] = "spxGrad";
    argc = 3;
  }

  uint dim[3] = {0, 0, 0};

  fType = read_args(argc, argv, dim);

  volume_name = argv[1];
  tfName = argv[2];
  dimensionX = dim[0];
  dimensionY = dim[1];
  dimensionZ = dim[2];

  vol->readFile(volume_name, fType, dim[0], dim[1], dim[2]);
  vol->tf.readTF(tfName);

  //-- Model Window --

  // initialize opengl
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);

  glutIdleFunc(idle);

  //glDisable(GL_POINT_SMOOTH);
  //glDisable(GL_LINE_SMOOTH);
  //glDisable(GL_POLYGON_SMOOTH);
  glDisable(GL_CULL_FACE); 

  glDisable(GL_DEPTH_TEST);
  glDisable(GL_STENCIL_TEST);
  glDisable(GL_FOG);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);

  GLfloat global_ambient[] = { 0.5f, 0.5f, 0.5f, 1.0f };
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, global_ambient);
  GLfloat specular[] = {0.0f, 0.0f, 0.0f , 0.0f};
  glLightfv(GL_LIGHT0, GL_SPECULAR, specular);
  GLfloat diffuse[] = {0.5f, 0.5f, 0.5f , 1.0f};
  glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
  GLfloat ambient[] = { 1.0f, 1.0f, 1.0f };
  glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
  //GLfloat position[] = { -2.5f, 0.5f, 4.0f, 1.0f };
  GLfloat position[] = { 0.0f, 0.0f, 0.0f, 1.0f };
  glLightfv(GL_LIGHT0, GL_POSITION, position);

  glDisable(GL_LIGHTING);
  glDisable(GL_LIGHT0);

  glDisable(GL_TEXTURE_1D);
  glDisable(GL_TEXTURE_2D);
  glDisable(GL_TEXTURE_3D);

  glPolygonMode(GL_FRONT, GL_FILL);

  glutInitWindowSize(modelWinWidth, modelWinHeight);
  glutInitWindowPosition(50, 80);
  modelWindow = glutCreateWindow(titleModelWin);

  if (clear_white)
    glClearColor(1.0, 1.0, 1.0, 0.0);
  else
    glClearColor(0.0, 0.0, 0.0, 0.0);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  glOrtho(MINORTHOSIZE, MAXORTHOSIZE, MINORTHOSIZE, MAXORTHOSIZE, MINORTHOSIZE, MAXORTHOSIZE);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  init();

  glutReshapeFunc(reshapeModelWin);
  glutDisplayFunc(displayModelWin);

  // user interaction
  glutMotionFunc(motionModelWin);
  glutMouseFunc(mouseModelWin);
  glutKeyboardFunc(keyboardModelWin);

  // user menu
  glutCreateMenu(commandMenuModel);
  glutAddMenuEntry("[h]  Show Help on/off", 'h');
  glutAddMenuEntry("[q] Quit", 'q');

  glutAttachMenu(GLUT_RIGHT_BUTTON);
    
  if (debug_cout) {
    end_in = glutGet(GLUT_ELAPSED_TIME);
    init_time = (end_in - sta_in) / 1000.0;
    cout << "*********************************************" << endl;
    cout << "*** Initialization time      : " << setw(8) << init_time << " s ***" << endl;
    cout << "*********************************************" << endl;
    cout << endl;
    time_frame = glutGet(GLUT_ELAPSED_TIME);
  }

  //-- TF Window --
  glutInitWindowSize(tfWinWidth, tfWinHeight);
  glutInitWindowPosition(900, 80);

  tfWindow = glutCreateWindow(titleTFWin);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(TF_ORTHO_MIN, TF_ORTHO_MAX, TF_ORTHO_MIN, TF_ORTHO_MAX);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  if (!tf_visible)
    glutHideWindow();

  glutMotionFunc(motionTFWin);
  glutMouseFunc(mouseTFWin);
  glutKeyboardFunc(keyboardTFWin);
  glutReshapeFunc(reshapeTFWin);
  glutDisplayFunc(displayTFWin);

  //-- IC Window --
  glutInitWindowSize(icWinWidth, icWinHeight);
  glutInitWindowPosition(900, 500);

  icWindow = glutCreateWindow(titleICWin);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluOrtho2D(IC_ORTHO_MIN, IC_ORTHO_MAX, IC_ORTHO_MIN, IC_ORTHO_MAX);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  if (!ic_visible)
    glutHideWindow();

  glutMotionFunc(motionICWin);
  glutMouseFunc(mouseICWin);
  glutKeyboardFunc(keyboardICWin);
  glutReshapeFunc(reshapeICWin);
  glutDisplayFunc(displayICWin);

  // set model window
  glutSetWindow(modelWindow);

  glutMainLoop();

  if (write_in_file)
    info_file.close();

  delete vol;

  return 0;
}
