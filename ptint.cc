/**
 *
 *    PTINT -- Projected Tetrahedra with Partial Pre-Integration
 *
 *  Maximo, Andre -- March, 2008
 *
 **/

/**
 *   Main
 *
 * C++ code.
 *
 */

/// ----------------------------------   Definitions   ------------------------------------

#include "ptint.h"

/// -----------------------------------   Functions   -------------------------------------

/// Main

int main(int argc, char** argv) {

	glAppInit(argc, argv);

	if ( !app.setup(argc, argv) )
		return 1;

	glTFSetup();

	glPTSetup();

	if ( !app.glSetup() )
		return 1;

	glLoop();

	return 0;

}
