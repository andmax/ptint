/**
 *
 *    OpenGL Application Utilities
 *
 *  File: glutApp.h
 *
 *  Author: Maximo, Andre
 *
 *  Last Update: March, 2008
 *
 **/

/// ----------------------------------   Definitions   ------------------------------------

#include "ptVol.h"

extern "C" {
#include <GL/glut.h> // gl-utility library
}

#define MULTFACT 1000

enum frameType { still, firstStill, rotating }; ///< Frame type status

/// Global Variables

const char titleWin[] = "PTINT"; ///< Window title

static int winWidth = 512, winHeight = 512; ///< Window size

static int buttonPressed = -1; ///< button state

static GLfloat oldx = 0.0, oldy = 0.0, xangle = 0.0, yangle = 0.0;
static int xmouse, ymouse;

static frameType volumeFrame = firstStill; ///< Volume frame status

static ptVol app; ///< PT Volume application

static GLdouble firstStepTime = 0.0, sortTime = 0.0, setupArraysTime = 0.0,
	secondStepTime = 0.0, totalTime = 0.0; ///< Time spent in each step

/// ----------------------------------   Functions   -------------------------------------

/// OpenGL Write
/// @arg x, y raster position
/// @arg str string to write
void glWrite(GLdouble x, GLdouble y, char *str) {

	// You should call glColor* before glWrite;
	// And the font color is also affected by texture and lighting
	glRasterPos2d(x, y);
	for (char *s = str; *s; s++)
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_12, *s);

}

/// OpenGL Show Information boxes

void glShowInfo(void) {

	totalTime = firstStepTime + sortTime + setupArraysTime + secondStepTime;

	/// Show nothing if all times are zero
	if (totalTime == 0.0) return;

	glColor3f(BLACK);

	char string[256];

	sprintf(string, "First Step: %.2lf s ( %.2lf %% )", firstStepTime, 100*firstStepTime / totalTime );

	glWrite(-1.1, 0.9, string);

	sprintf(string, "Sort: %.2lf s ( %.2lf %% )", sortTime, 100*sortTime / totalTime );

	glWrite(-1.1, 0.8, string);

	sprintf(string, "Setup Arrays: %.2lf s ( %.2lf %% )", setupArraysTime, 100*setupArraysTime / totalTime );

	glWrite(-1.1, 0.7, string);

	sprintf(string, "Second Step: %.2lf s ( %.2lf %% )", secondStepTime, 100*secondStepTime / totalTime );

	glWrite(-1.1, 0.6, string);

	sprintf(string, "# Tets / sec: %.2lf MTet/s ( %.2lf fps )", (app.volume.numTets / totalTime) / 1000000.0, 1.0 / totalTime );

	glWrite(-1.1, 0.5, string);

}

/// OpenGL Display

void glDisplay(void) {

	/// Clear time
	firstStepTime = 0.0; sortTime = 0.0;
	setupArraysTime = 0.0; secondStepTime = 0.0;

	/// Reset transformations
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glRotatef(oldx+xangle, 1.0f, 0.0f, 0.0f);
	glRotatef(oldy+yangle, 0.0f, 1.0f, 0.0f);

	if (volumeFrame == rotating) {

		app.firstStep(firstStepTime);
		app.sort(sortTime, bucket);
		app.setupAndReorderArrays(setupArraysTime);

	} else if (volumeFrame == firstStill) {

		app.firstStep(firstStepTime);
		app.sort(sortTime, centroid);
		app.setupAndReorderArrays(setupArraysTime);

		volumeFrame = still;

	}

	app.secondStep(secondStepTime);

	glPopMatrix();

	glShowInfo();

	glutSwapBuffers();

}

/// OpenGL Reshape
/// @arg w, h new window size

void glReshape(int w, int h) {

	app.setWindow(w, h);
	glViewport(0, 0, winWidth=w, winHeight=h);

}

/// OpenGL Keyboard
/// @arg key keyboard key hitted
/// @arg x, y mouse position when hit

void glKeyboard( unsigned char key, int x, int y ) {

	switch(key) {
	case 'q': case 'Q': case 27: // quit application
		glutDestroyWindow( glutGetWindow() );
		return;
	default: // any other key
		cerr << "No key bind for " << key
		     << " in (" << x << ", " << y << ")" << endl;
		return;
	}

	glutPostRedisplay();

}

/// OpenGL Mouse
/// @arg button mouse button event
/// @arg state mouse state event
/// @arg x, y mouse position

void glMouse(int button, int state, int x, int y) {

	buttonPressed = button;
	xmouse = x;
	ymouse = y;

	if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_DOWN))
		volumeFrame = rotating;

	if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_UP)) {

		oldx += xangle;
		oldy += yangle;
		if (oldx > 360.0) oldx -= 360.0;
		if (oldx < 0.0) oldx += 360.0;
		if (oldy > 360.0) oldy -= 360.0;
		if (oldy < 0.0) oldy += 360.0;
		xangle = 0.0;
		yangle = 0.0;

		volumeFrame = firstStill;

		glutPostRedisplay();

	}

}

/// OpenGL Motion
/// @arg x, y mouse position

void glMotion(int x, int y) {

	if (buttonPressed == GLUT_LEFT_BUTTON) {

		yangle = (x - xmouse) * 360 / (GLfloat) winWidth;
		xangle = (y - ymouse) * 180 / (GLfloat) winHeight;

		glutPostRedisplay();

	}

}

/// OpenGL idle function

void glIdle(void) {

	glutPostRedisplay();

}

/// OpenGL Application Initialization

void glAppInit(int& argc, char** argv) {

	glutInit(&argc, argv);

}

/// OpenGL Application Setup

void glAppSetup(void) {

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);

	glutInitWindowSize(winWidth, winHeight);
	glutInitWindowPosition(0, 0);
	glutCreateWindow(titleWin);

	glutReshapeFunc(glReshape);
	glutDisplayFunc(glDisplay);
	glutKeyboardFunc(glKeyboard);
	glutMouseFunc(glMouse);
	glutMotionFunc(glMotion);
	glutIdleFunc(NULL);

	/// ModelviewProjection setup (<-)
	glMatrixMode(GL_PROJECTION); glLoadIdentity();
	glOrtho(-1.2, 1.2, -1.2, 1.2, -1.2, 1.2);
	glMatrixMode(GL_MODELVIEW); glLoadIdentity();

	app.setWindow(winWidth, winHeight);
	app.setOrtho(-1.2, 1.2);

}

/// OpenGL Main Loop

void glLoop(void) {

	glutMainLoop();

}
