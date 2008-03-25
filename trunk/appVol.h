/**
 *   Volume OFF Application
 *
 * Maximo, Andre -- March, 2008
 *
 */

/**
 *   appVol : defines a class for setup the volume class using application
 *            input arguments
 *
 * C++ header.
 *
 */

/// --------------------------------   Definitions   ------------------------------------

#ifndef _APPVOL_H_
#define _APPVOL_H_

extern "C" {
#include <GL/gl.h> // OpenGL library
}

#include <string>

#include "offVol.h"

/// ----------------------------------   appVol   ------------------------------------

/// Volume Application

class appVol {

public:

	offVol< GLfloat, GLuint > volume;

	typedef offVol< GLfloat, GLuint >::vec3 vec3;

	/// Debug flag
	bool debug;

	/// Volume name
	string volName;

	/// File extensions
	string offExt, tfExt, lmtExt;

	/// Searching directory for files
	string searchDir;

	/// Constructor
	appVol(bool _d = true);

	/// Destructor
	~appVol();

	/// Volume Application Setup
	/// @arg argc main argc
	/// @arg argv main argv
	/// @return true if it succeed
	bool setup(int& argc, char** argv);

};

#endif
