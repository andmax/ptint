/**
 *   Transfer Function Editing
 *
 *  Authors:
 *    Maximo, Andre
 *    Marroquim, Ricardo
 *
 *  Date: Jan-May, 2006
 *
 */

/**
 *   transferFunction : defines a class for Transfer Function (TF)
 *                      editing without glut functions
 *
 * C++ header.
 *
 */

/// --------------------------------   Definitions   ------------------------------------

#ifndef _TRANSFERFUNCTION_H_
#define _TRANSFERFUNCTION_H_

extern "C" {
#include <GL/gl.h> // OpenGL library
}

#include <set>
#include <string>

using std::set;

extern
void glWrite(GLdouble x, GLdouble y, char *str);

/// -----------------------------   transferFunction   ----------------------------------

/// Transfer Function (TF) handle class
/// @template real number type (float, double, etc.)
/// @template natural number type (short, unsigned, long, etc.)
template< class real, class natural >
class transferFunction {

public:

	typedef vec< 4, real > vec4;

	typedef typename set< natural, less<natural> >::const_iterator setIt;

	/// Constructor -- instantiate null-tf
	transferFunction(vec4* _tf = NULL, real _bt = 1.0) : tf(_tf),
		numColors(256), brightness(_bt), minBrightness(0.0),
		maxBrightness(8.0), ppoint(0), controlPoints(),
		minOrthoSize(-0.2), maxOrthoSize(1.3),
		winWidth(400), winHeight(300), stepColor(0),
		stepBrightness(0), tfName() {

		unSetPickedPoint();

		/// Initializing 5 control points
		controlPoints.insert(  0  ); controlPoints.insert(  63 );
		controlPoints.insert( 127 ); controlPoints.insert( 191 );
		controlPoints.insert( 255 );

		stepColor = 1.0 / (GLdouble)numColors;
		stepBrightness = 1.0 / (GLdouble)(maxBrightness - minBrightness);

	}

	/// Destructor
	~transferFunction() {
		controlPoints.clear();
	}

	/// Set functions
	void setOrtho(const GLdouble& _minOrtho, const GLdouble& _maxOrtho) {
		minOrthoSize = _minOrtho;
		maxOrthoSize = _maxOrtho;
	}
	void setWindow(const GLsizei& _winW, const GLsizei& _winH) {
		winWidth = _winW;
		winHeight = _winH;
	}
	void setTFName(string _tfName) { tfName = _tfName; }
	void unSetPickedPoint(void) { ppoint = numColors+1; }

	/// Get current brightness
	/// @return brightness
	real getBrightness(void) { return brightness; }

	/// Get TF name
	/// @return file path (TF name)
	string getTFName(void) { return tfName; }

	/// Update picked point related value
	/// @arg x, y mouse position when changing
	void updatePickedPoint(int x, int y) {

		if (ppoint == numColors+1) return;

		if (ppoint < numColors) { /// Control point

			real yP = yOrtho(y);
			if (yP > 1.0) yP = 1.0;
			if (yP < 0.0) yP = 0.0;
			tf[ ppoint ][3] = yP;

		} else { /// Brigthness point

			real xP = xOrtho(x);
			if (xP > 1.0) xP = 1.0;
			if (xP < 0.0) xP = 0.0;
			brightness = (real)( xP * (maxBrightness-minBrightness) );

		}

		updateTF();

	}

	/// OpenGL Setup
	void glSetup() {
		glClearColor(1.0f, 1.0f, 1.0f, 0.0f); ///< Fixed white background
	}

	/// Picking function
	/// @arg x, y mouse position when hit
	void pick(int x, int y) {

		unSetPickedPoint();

		GLsizei xP;

		for (setIt it = controlPoints.begin(); it != controlPoints.end(); ++it) {

			xP = xScreen((*it) * stepColor);

			if ( ( y >= yScreen(1.0) ) && ( y <= yScreen(0.0) )
			     && ( x >= xP - 5 ) && ( x <= xP + 5 ) ) { /// Control Point picked

				ppoint = (*it);
				return;

			}

		}

		xP = xScreen(brightness * stepBrightness);

		if ( ( y >= yScreen(-0.15) - 5) && (y <= yScreen(-0.15) + 5)
		     && (x >= xScreen(0.0) - 5) && (x <= xScreen(1.0) - 5) ) { /// Brightness picked

			ppoint = numColors;
			return;

		}

		updateTF();

	}

	/// Create a new control point
	/// @arg x, y mouse position when hit
	void createControlPoint(int x, int y) {

		if ( (x < xScreen(0.0)) && (x > xScreen(1.0)) ) return;

		for (GLuint i = 0; i < numColors; ++i) {

			if ( (y >= yScreen(tf[i][3]) - 2) && (y <= yScreen(tf[i][3]) + 2)
			     && (x >= xScreen(i * stepColor) - 2) && (x <= xScreen(i * stepColor) + 2) ) {

				controlPoints.insert(i);
				return;

			}

		}

	}

	/// Delete the picked control point
	void deleteControlPoint(void) {

		if (ppoint >= numColors) return;

		/// Refuses delete of the two control points in the TF range limit
		if ((ppoint == 0) || (ppoint == numColors-1)) return;

		setIt it = controlPoints.find( ppoint );
		setIt itPrev(it); itPrev--;
		setIt itNext(it); itNext++;

		if (itPrev != controlPoints.end()) ppoint = (*itPrev);
		else if (itNext != controlPoints.end()) ppoint = (*itNext);
		else ppoint = numColors+1;

		controlPoints.erase( it );

		updateTF();

		ppoint = numColors+1;

	}

	/// Color code
	void colorCode(int cc) {

		natural quarter = numColors / 4;
		real stepQuarter = 1.0 / (real)quarter;

		for (natural i = 0; i < numColors; ++i) {

			if (cc == 1) {

				if (i < quarter) { tf[i][0] = 1.0; tf[i][1] = i*stepQuarter; tf[i][2] = 0.0; }
				else if (i < 2*quarter) { tf[i][0] = 1.0 - (i-quarter)*stepQuarter; tf[i][1] = 1.0; tf[i][2] = 0.0; }
				else if (i < 3*quarter) { tf[i][0] = 0.0; tf[i][1] = 1.0; tf[i][2] = (i-2*quarter)*stepQuarter; }
				else if (i < 4*quarter) { tf[i][0] = 0.0; tf[i][1] = 1.0 - (i-3*quarter)*stepQuarter; tf[i][2] = 1.0; }

			} else if (cc == 2) {

				if (i < quarter) { tf[i][0] = 0.0; tf[i][1] = i*stepQuarter; tf[i][2] = 1.0; }
				else if (i < 2*quarter) { tf[i][0] = 0.0; tf[i][1] = 1.0; tf[i][2] = 1.0 - (i-quarter)*stepQuarter; }
				else if (i < 3*quarter) { tf[i][0] = (i-2*quarter)*stepQuarter; tf[i][1] = 1.0; tf[i][2] = 0.0; }
				else if (i < 4*quarter) { tf[i][0] = 1.0; tf[i][1] = 1.0 - (i-3*quarter)*stepQuarter; tf[i][2] = 0.0; }

			} else if (cc == 3) {

				if (i < quarter) { tf[i][0] = 0.0; tf[i][1] = 1.0; tf[i][2] = i*stepQuarter; }
				else if (i < 2*quarter) { tf[i][0] = 0.0; tf[i][1] = 1.0 - (i-quarter)*stepQuarter; tf[i][2] = 1.0; }
				else if (i < 3*quarter) { tf[i][0] = (i-2*quarter)*stepQuarter; tf[i][1] = 0.0; tf[i][2] = 1.0; }
				else if (i < 4*quarter) { tf[i][0] = 1.0; tf[i][1] = 0.0; tf[i][2] = 1.0 - (i-3*quarter)*stepQuarter; }

			} else if (cc == 4) {

				if (i < quarter) { tf[i][0] = 1.0; tf[i][1] = 0.0; tf[i][2] = i*stepQuarter; }
				else if (i < 2*quarter) { tf[i][0] = 1.0 - (i-quarter)*stepQuarter; tf[i][1] = 0.0; tf[i][2] = 1.0; }
				else if (i < 3*quarter) { tf[i][0] = 0.0; tf[i][1] = (i-2*quarter)*stepQuarter; tf[i][2] = 1.0; }
				else if (i < 4*quarter) { tf[i][0] = 0.0; tf[i][1] = 1.0; tf[i][2] = 1.0-(i-3*quarter)*stepQuarter; }

			} else if (cc == 5) {

				if (i < quarter) { tf[i][0] = i*stepQuarter; tf[i][1] = 1.0; tf[i][2] = 0.0; }
				else if (i < 2*quarter) { tf[i][0] = 1.0; tf[i][1] = 1.0 - (i-quarter)*stepQuarter; tf[i][2] = 0.0; }
				else if (i < 3*quarter) { tf[i][0] = 1.0; tf[i][1] = 0.0; tf[i][2] = (i-2*quarter)*stepQuarter; }
				else if (i < 4*quarter) { tf[i][0] = 1.0 - (i-3*quarter)*stepQuarter; tf[i][1] = 0.0; tf[i][2] = 1.0; }

			} else if (cc == 6) {

				if (i < quarter) { tf[i][0] = i*stepQuarter; tf[i][1] = 0.0; tf[i][2] = 1.0; }
				else if (i < 2*quarter) { tf[i][0] = 1.0; tf[i][1] = 0.0; tf[i][2] = 1.0 - (i-quarter)*stepQuarter; }
				else if (i < 3*quarter) { tf[i][0] = 1.0; tf[i][1] = (i-2*quarter)*stepQuarter; tf[i][2] = 0.0; }
				else if (i < 4*quarter) { tf[i][0] = 1.0 - (i-3*quarter)*stepQuarter; tf[i][1] = 1.0; tf[i][2] = 0.0; }

			} else if (cc == 7) {

				tf[i][0] = i * stepColor; tf[i][1] = 1.0 - i * stepColor; tf[i][2] = 0.0;

			} else if (cc == 8) {

				tf[i][0] = 1.0 - i * stepColor; tf[i][1] = i * stepColor; tf[i][2] = 0.0;

			} else if (cc == 9) {

				tf[i][0] = 1.0 - i * stepColor; tf[i][1] = 1.0 - i * stepColor; tf[i][2] = 1.0 - i * stepColor;

			} else if (cc == 0) {

				tf[i][0] = i * stepColor; tf[i][1] = i * stepColor; tf[i][2] = i * stepColor;
			}

		}

	}

	/// Draw transfer function informations
	void draw(void) {

		if (!tf) return;

		glBegin(GL_QUADS); /// Transfer Function RGBAs

		real x0, x1;

		for (GLuint i = 0; i < numColors; ++i) {

			glColor3fv( &tf[i] );
			x0 = (i - 0.5) * stepColor; x1 = (i + 0.5) * stepColor;
			glVertex2d( x0, 0.0); glVertex2d( x1, 0.0);
			glVertex2d( x1, tf[i][3]); glVertex2d( x0, tf[i][3]);

		}

		glEnd();

		glBegin(GL_LINES); /// Brightness line
		glColor3f(1.0, 1.0, 1.0); glVertex2d(0.0, -0.15);
		glColor3f(0.0, 0.0, 0.0); glVertex2d(1.0, -0.15);
		glEnd();

		glPointSize(5.0);
		glBegin(GL_POINTS); /// Draw Control Points

		for (setIt it = controlPoints.begin(); it != controlPoints.end(); ++it) {

			if ((*it) == ppoint) glColor3d(0.7, 0.3, 0.1);
			else glColor3d(0.0, 0.0, 0.0);

			glVertex2d( (*it) * stepColor, tf[(*it)][3] ); ///  Control point

		}

		if (ppoint == numColors) glColor3d(0.7, 0.3, 0.1);
		else glColor3d(0.0, 0.0, 0.0);

		glVertex2d( brightness * stepBrightness, -0.15); /// Brightness point

		glEnd();

		glBegin(GL_LINES); /// Draw Axis
		glColor3d(0.0, 0.0, 0.0);
		glVertex2d(0.0, 0.0); glVertex2d(0.0, 1.1);
		glVertex2d(0.0, 0.0); glVertex2d(1.1, 0.0);
		glVertex2d(0.0, 1.1); glVertex2d(0.02, 1.08);
		glVertex2d(0.0, 1.1); glVertex2d(-0.02, 1.08);
		glVertex2d(1.1, 0.0); glVertex2d(1.08, -0.02);
		glVertex2d(1.1, 0.0); glVertex2d(1.08, 0.02);
		glEnd();

		glEnable(GL_LINE_STIPPLE); glLineStipple(4, 0xAAAA);
		glBegin(GL_LINES);
		glVertex2d(0.0, 1.0); glVertex2d(1.0, 1.0);
		glVertex2d(1.0, 0.0); glVertex2d(1.0, 1.0);
		glEnd();

		glDisable(GL_LINE_STIPPLE);

		glColor3d(0.0, 0.0, 0.0);
		glWrite(0.30, 1.2, "File:"); /// Write texts
		glWrite(0.45, 1.2, (char*)tfName.c_str());
		glWrite(-0.05, 1.14, "Alpha");
		glWrite(1.12, -0.01, "Scalar");
		glWrite(1.04, -0.16, "Brightness");

		if (ppoint != numColors+1) {

			glColor3d(0.7, 0.3, 0.1);

			char str[256];

			if (ppoint == numColors) { /// Brightness point

				sprintf(str, "%.2lf", brightness );
				glWrite( (brightness - 0.5) * stepBrightness, -0.12, str );

			} else { /// Control point

				sprintf(str, "%.2lf", tf[ ppoint ][3] );
				glWrite( -0.12, tf[ppoint][3], str);

				sprintf(str, "%d", ppoint );
				glWrite( (ppoint - 0.5 ) * stepColor, -0.06, str);

			}
		}

	}

private:

	/// Update TF
	void updateTF(void) {

		if (ppoint >= numColors) return;

		setIt it = controlPoints.find( ppoint );
		setIt itPrev(it); itPrev--;
		setIt itNext(it); itNext++;

		natural x, xCurr = *it;
		real y, yCurr = tf[ xCurr ][3];
		real a, b;

		if ( itPrev != controlPoints.end() ) {

			x = (*itPrev);
			y = tf[ x ][3];

			a = (yCurr - y) / (real)(xCurr - x);
			b = yCurr - a * xCurr;

			for (natural i = x+1; i < xCurr; ++i) {

				tf[ i ][3] = a * i + b;

			}
			
		}

		if ( itNext != controlPoints.end() ) {

			x = (*itNext);
			y = tf[ x ][3];

			a = (y - yCurr) / (real)(x - xCurr);
			b = yCurr - a * xCurr;

			for (natural i = xCurr+1; i < x; ++i) {

				tf[ i ][3] = a * i + b;

			}
			
		}

	}

	/// Convert x, y ortho to/from screen
	real xOrtho(GLsizei x) {
		return (real)( ((x/(real)winWidth) * (maxOrthoSize-minOrthoSize)) + minOrthoSize );
	}
	real yOrtho(GLsizei y) {
		return (real)(((((real)winHeight - y)/(real)winHeight) * (maxOrthoSize-minOrthoSize)) + minOrthoSize);
	}
	GLsizei xScreen(real x) {
		return (GLsizei)(((x - minOrthoSize)*(real)winWidth)/(maxOrthoSize-minOrthoSize));
	}
	GLsizei yScreen(real y) {
		return winHeight - (GLsizei)(((y - minOrthoSize)*(real)winHeight)/(maxOrthoSize-minOrthoSize));
	}

	vec4 *tf; ///< Pointer to transfer function values

	natural numColors; ///< Number of TF colors

	real brightness, minBrightness, maxBrightness; ///< Brightness term

	natural ppoint; ///< Picked point = [0, numColors-1]: control point id; numColors: brightness; numColors+1: no point picked

	set< natural, less<natural> > controlPoints; ///< Vector of control points

	GLdouble minOrthoSize, maxOrthoSize; /// Minimum and maximum ortho size
	GLsizei winWidth, winHeight; ///< Width x Height pixel resolution

	real stepColor, stepBrightness; ///< Step distance in color and brightness

	string tfName; ///< Transfer Function (TF) name

};

#endif
