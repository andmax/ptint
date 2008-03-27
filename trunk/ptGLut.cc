/**
 *
 *    OpenGL Projected Tetrahedra PT Utilities
 *
 *  Maximo, Andre -- March, 2008
 *
 **/

/**
 *   ptGL functions
 *
 * C++ code.
 *
 */

/// ----------------------------------   Definitions   ------------------------------------

#include "ptGLut.h"

extern "C" {
#include <GL/glut.h> // gl-utility library
}

/// Global Variables

static const char titleWin[] = "PTINT"; ///< Window title

static int winWidth = 512, winHeight = 512; ///< Window size

int ptWinId; ///< PT Window id

extern int tfWinId; ///< TF Window id

static int buttonPressed = -1; ///< button state

static GLfloat oldx = 0.0, oldy = 0.0, xangle = 0.0, yangle = 0.0;
static int xmouse, ymouse;
static GLdouble zoom = 1.0;

static frameType volumeFrame = firstStill; ///< Volume frame status

static bool alwaysRotating = false; ///< Always rotating state

static GLdouble firstStepTime = 0.0, sortTime = 0.0, setupArraysTime = 0.0,
	secondStepTime = 0.0, totalTime = 0.0; ///< Time spent in each step

static bool showHelp = false; ///< show help flag
static bool showInfo = true; ///< show information flag

/// ----------------------------------   Functions   -------------------------------------

/// glPT Show Information boxes

void glPTShowInfo(void) {

	glColor3f(BLACK);

	if (showInfo) { /// Show timing and dataset informations

		totalTime = firstStepTime + sortTime + setupArraysTime + secondStepTime;
		char str[256];

		sprintf(str, "First Step: %.2lf s ( %.2lf %% )", firstStepTime, 100*firstStepTime / totalTime );
		glWrite(-1.1, 0.9, str);

		sprintf(str, "Sort: %.2lf s ( %.2lf %% )", sortTime, 100*sortTime / totalTime );
		glWrite(-1.1, 0.8, str);

		sprintf(str, "Setup Arrays: %.2lf s ( %.2lf %% )", setupArraysTime, 100*setupArraysTime / totalTime );
		glWrite(-1.1, 0.7, str);

		sprintf(str, "Second Step: %.2lf s ( %.2lf %% )", secondStepTime, 100*secondStepTime / totalTime );
		glWrite(-1.1, 0.6, str);

		sprintf(str, "# Tets / sec: %.2lf MTet/s ( %.2lf fps )", (app.volume.numTets / totalTime) / 1000000.0, 1.0 / totalTime );
		glWrite(-1.1, 0.5, str);

		sprintf(str, "# Tets: %d", app.volume.numTets );
		glWrite(-1.1, -0.5, str);

		sprintf(str, "# Verts: %d", app.volume.numVerts );
		glWrite(-1.1, -0.6, str);

		sprintf(str, "Resolution: %d x %d", winWidth, winHeight );
		glWrite(-1.1, -0.7, str);

	}

	if (showHelp) { /// Show help information

		glWrite( 0.82,  1.1, "(?) close help");
		glWrite(-0.52,  0.4, "(left-button) rotate volume");
		glWrite(-0.52,  0.3, "(middle-button) zoom volume");
		glWrite(-0.52,  0.2, "(right-button) open menu");
		glWrite(-0.52,  0.1, "(r) always rotating mode");
		glWrite(-0.52,  0.0, "(s) show/close timing information");
		glWrite(-0.52, -0.1, "(t) open transfer function window");
		glWrite(-0.52, -0.2, "(q|esc) close application");

	} else {

		glWrite(0.82, 1.1, "(?) open help");

	}

}

/// glPT Display

void glPTDisplay(void) {

	glClear(GL_COLOR_BUFFER_BIT);

	/// Clear time
	firstStepTime = 0.0; sortTime = 0.0;
	setupArraysTime = 0.0; secondStepTime = 0.0;

	/// Reset transformations
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glLoadIdentity();
	glRotatef(oldx+xangle, 1.0f, 0.0f, 0.0f);
	glRotatef(oldy+yangle, 0.0f, 1.0f, 0.0f);
	glScalef(zoom, zoom, zoom);

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

	glPTShowInfo();

	glutSwapBuffers();

}

/// glPT Reshape

void glPTReshape(int w, int h) {

	app.setWindow(w, h);
	glViewport(0, 0, winWidth=w, winHeight=h);

}

/// glPT Keyboard

void glPTKeyboard( unsigned char key, int x, int y ) {

	switch(key) {
	case 'h': case 'H': case '?': // show help
		showHelp = !showHelp;
		if (showHelp) showInfo = false;
		else showInfo = true;
		break;
	case 'r': case 'R': // always rotating flag
		alwaysRotating = !alwaysRotating;
		if (alwaysRotating) volumeFrame = rotating;
		else volumeFrame = firstStill;
		return;
	case 's': case 'S': // show/close information
		showInfo = !showInfo;
		break;
	case 't': case 'T': // open tf window
		glutSetWindow( tfWinId );
		glutShowWindow();
		glutPostRedisplay();
		glutSetWindow( ptWinId );
		break;
	case 'q': case 'Q': case 27: // quit application
		glutDestroyWindow( ptWinId );
		glutDestroyWindow( tfWinId );		
		return;
	default: // any other key
		cerr << "No key bind for " << key
		     << " in (" << x << ", " << y << ")" << endl;
		return;
	}

	glutPostRedisplay();

}

/// glPT Command

void glPTCommand(int value) {

	glPTKeyboard( (unsigned char)value, 0, 0 );

}

/// glPT Mouse

void glPTMouse(int button, int state, int x, int y) {

	buttonPressed = button;
	xmouse = x;
	ymouse = y;

	if ( (button == GLUT_LEFT_BUTTON) && (state == GLUT_DOWN) )
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

	if (button == GLUT_MIDDLE_BUTTON) {

		volumeFrame = firstStill;

		glutPostRedisplay();

	}

}

/// glPT Motion

void glPTMotion(int x, int y) {

	if (buttonPressed == GLUT_LEFT_BUTTON) {

		yangle = (x - xmouse) * 360 / (GLfloat) winWidth;
		xangle = (y - ymouse) * 180 / (GLfloat) winHeight;

		glutPostRedisplay();

	}

	if (buttonPressed == GLUT_MIDDLE_BUTTON) {

		zoom += (ymouse - y) * 2.0 / (GLfloat)winHeight;
		ymouse = y;
		if (zoom < 0.1) zoom = 0.1;

		volumeFrame = firstStill;
		glutPostRedisplay();

	}

}

/// glPT idle function

void glPTIdle(void) {

	if (alwaysRotating) {

		if (volumeFrame != rotating) volumeFrame = rotating;

		oldx += 0.5;
		oldy += 0.5;
		if (oldx > 360.0) oldx -= 360.0;
		if (oldy > 360.0) oldy -= 360.0;

		if (glutGetWindow() == tfWinId) {
			glutSetWindow( ptWinId );
			glutPostRedisplay();
			glutSetWindow( tfWinId );
		} else
			glutPostRedisplay();

	}

}

/// glPT Application Setup

void glPTSetup(void) {

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(winWidth, winHeight);
	glutInitWindowPosition(0, 0);
	ptWinId = glutCreateWindow(titleWin);

	/// Event handles
	glutReshapeFunc(glPTReshape);
	glutDisplayFunc(glPTDisplay);
	glutKeyboardFunc(glPTKeyboard);
	glutMouseFunc(glPTMouse);
	glutMotionFunc(glPTMotion);
	glutIdleFunc(glPTIdle);

	/// Command Menu
	glutCreateMenu(glPTCommand);
	glutAddMenuEntry("[h] Show/close help", 'h');
	glutAddMenuEntry("[r] Rotate always", 'r');
	glutAddMenuEntry("[s] Show/close timing information", 's');
	glutAddMenuEntry("[t] Open TF window", 't');
	glutAddMenuEntry("[q] Quit", 'q');
	glutAttachMenu(GLUT_RIGHT_BUTTON);

	/// ModelviewProjection setup (<-)
	glMatrixMode(GL_PROJECTION); glLoadIdentity();
	glOrtho(-1.2, 1.2, -1.2, 1.2, -1.2, 1.2);
	glMatrixMode(GL_MODELVIEW); glLoadIdentity();

	app.setWindow(winWidth, winHeight);
	app.setOrtho(-1.2, 1.2);

}
