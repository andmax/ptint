/**
 *   Projection Tetrahedra Volume
 *
 *  Authors:
 *    Maximo, Andre
 *    Marroquim, Ricardo
 *
 *  Date: Jan-May, 2006
 *
 */

/**
 *   ptVol : defines a class for volume rendering using projected
 *           tetrahedra with partial pre-integration PTINT in GPU
 *
 * C++ implementation.
 *
 */

/// --------------------------------   Definitions   ------------------------------------

#include <iomanip>
#include <iostream>

using std::setprecision;
using std::cerr;

#include "ptVol.h"

#include "errHandle.h"

#include "tables.h"

#include "psiGammaTable512.h"

#define TEX_FORMAT GL_TEXTURE_2D
#define TEX_TYPE_32 GL_RGBA32F_ARB
#define TEX_TYPE_16 GL_RGBA16F_ARB

#define NUM_LAYERS 100 ///< Layers for bucket sorting

/// ----------------------------------   ptVol   ------------------------------------

/// Constructor
ptVol::ptVol( bool _d ) :
	appVol(_d),
	firstStepShader(NULL), secondStepShader(NULL),
	vertexArray(NULL), colorArray(NULL),
	indices(NULL), count(NULL), ids(NULL),
	centroidSorted(NULL), centroidBucket(NULL),
	outputBuffer0(NULL), outputBuffer1(NULL),
	frameBuffer(0),
	tetOutputTex0(0), tetOutputTex1(0),
	vertListTex(0), tetListTex(0),
	orderTableTex(0), tfTex(0),
	psiGammaTableTex(0),
	vertTexSize(0), tetTexSize(0),
	backGround(WHITE),
	minOrthoSize(-1.0), maxOrthoSize(1.0),
	winWidth(512), winHeight(512),
	sortMethod(none) {

}

/// Destructor
ptVol::~ptVol() {

	if (firstStepShader) delete firstStepShader;
	if (secondStepShader) delete secondStepShader;

	if (vertexArray) delete [] vertexArray;
	if (colorArray) delete [] colorArray;

	if (indices)
		for (GLuint i = 0; i < volume.numTets; ++i)
			if (indices[i]) delete [] indices[i];

	if (count) delete [] count;

	if (ids) delete [] ids;

	if (centroidSorted) delete [] centroidSorted;

	if (centroidBucket) {
		for (GLuint i = 0; i < NUM_LAYERS; ++i)
			centroidBucket[i].clear();
		delete [] centroidBucket;
	}

	if (outputBuffer0) delete [] outputBuffer0;
	if (outputBuffer1) delete [] outputBuffer1;

	glDeleteFramebuffersEXT(1, &frameBuffer);
	glDeleteTextures(1, &tetOutputTex0);
	glDeleteTextures(1, &tetOutputTex1);
	glDeleteTextures(1, &tetListTex);
	glDeleteTextures(1, &vertListTex);
	glDeleteTextures(1, &orderTableTex);
	glDeleteTextures(1, &tfTex);
	glDeleteTextures(1, &psiGammaTableTex);

}

/// OpenGL Setup
bool ptVol::glSetup() {

	try {

		/// OpenGL settings
		glEnable(GL_CULL_FACE);

		/// Source alpha is applied in second fragment shader
		glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

		/// Background color
		glClearColor(backGround.r(), backGround.g(), backGround.b(), 0.0f);

		/// Vertices and Tetrahedra textures size
		vertTexSize = (GLuint)ceil(sqrt(volume.numVerts));
		tetTexSize = (GLuint)ceil(sqrt(volume.numTets));

		if (debug) cout << "::: OpenGL :::" << endl << endl;

		/// Create OpenGL auxiliary data structures
		if (!createArrays()) throw errHandle(memoryErr);

		if (!createCentroidSorts()) throw errHandle(memoryErr);

		if (!createBuffers()) throw errHandle(memoryErr);

		createTextures();

		if (!createShaders()) throw errHandle(genericErr, "GLSL Error!");

		if (debug) cout	<< endl << "# Memory Size = " << setprecision(4)
				<< this->sizeOf() / 1000000.0 << " MB " << endl << endl;

		return true;

	} catch(errHandle& e) {

		cerr << e;

		return false;

	} catch(...) {

		throw errHandle();

	}

}

/// Size of PT Volume (OpenGL in CPU)
int ptVol::sizeOf(void) {

	return ( ( (firstStepShader) ? firstStepShader->size_of() : 0 ) + ///< First Step Shader
		 ( (secondStepShader) ? secondStepShader->size_of() : 0 ) + ///< Second Step Shader
		 ( (vertexArray) ? volume.numTets * 4 * 5 * sizeof(GLfloat) : 0 ) + ///< Vertex Array
		 ( (colorArray) ? volume.numTets * 3 * 5 * sizeof(GLfloat) : 0 ) + ///< Color Array
		 ( (indices) ? volume.numTets * 6 * sizeof(GLuint) : 0 ) + ///< Indices
		 ( (count) ? volume.numTets * sizeof(GLint) : 0 ) + ///< Count
		 ( (ids) ? volume.numTets * sizeof(int) : 0 ) + ///< Ids (pointers)
		 ( (centroidSorted) ? volume.numTets * sizeof(tetCentroid) : 0 ) + ///< Tet Centroids
		 ( (centroidBucket) ? volume.numTets * sizeof(GLuint) : 0 ) + ///< Tet ids per bucket
		 ( (outputBuffer0) ? tetTexSize * tetTexSize * 4 * sizeof(GLfloat) : 0 ) + ///< Output Buffer 0
		 ( (outputBuffer1) ? tetTexSize * tetTexSize * 4 * sizeof(GLfloat) : 0 ) + ///< Output Buffer 1
		 ( 10 * sizeof(GLuint) ) + ///< All GLuints
		 ( 12 * sizeof(int) ) + ///< pointers
		 ( PSI_GAMMA_SIZE_BACK * PSI_GAMMA_SIZE_FRONT * sizeof(float) ) ///< Psi Gamma Table
		);

}

/// Create Arrays
bool ptVol::createArrays(void) {

	GLuint idVArray = 0, idCArray = 0, i = 0, j = 0, k = 0;
	GLuint vertId, nT = volume.numTets;

	if (vertexArray) delete [] vertexArray;
	vertexArray = new GLfloat[nT * 4 * 5];
	if (!vertexArray) return false;

	if (colorArray) delete [] colorArray;
	colorArray = new GLfloat[nT * 3 * 5];
	if (!colorArray) return false;
    
	for (i = 0; i < nT; ++i) {

		idVArray = i * 5 * 4;
		idCArray = i * 5 * 3;
	
		for (k = 0; k < 4; ++k) { /// Initialize thick vertex

			vertexArray[idVArray + 0*4 + k] = 0.0;
			colorArray[idCArray + 0*4 + k] = 0.0;

		} // k

		for (j = 1; j < 5; ++j) {

			vertId = volume.tetList[i][j-1];

			for (k = 0; k < 3; ++k) {

				vertexArray[idVArray + j*4 + k] = volume.vertList[vertId][k];
				colorArray[idCArray + j*3 + k] = volume.vertList[vertId][3];

			} // k

			vertexArray[idVArray + j*4 + 3] = 1.0;
			colorArray[idCArray + j*3 + 2] = 0.0;

		} // j

	} // i

	if (indices)
		for (i = 0; i < nT; ++i)
			if (indices[i]) delete [] indices[i];
	indices = new GLuint*[nT];
	if (!indices) return false;

	for (i = 0; i < nT; ++i) {

		indices[i] = new GLuint[6];
		if (!indices[i]) return false;

	}

	if (count) delete [] count;
	count = new GLint[nT];
	if (!count) return false;

	if (ids) delete [] ids;
	ids = new GLvoid*[nT];
	if (!ids) return false;

	for (i = 0; i < nT; ++i) {

		ids[i] = (GLvoid*)indices[i];
		count[i] = 6; ///< Init with maximum value

	}

	glEnableClientState(GL_COLOR_ARRAY);
	glEnableClientState(GL_VERTEX_ARRAY);
	glColorPointer(3, GL_FLOAT, 0, colorArray);
	glVertexPointer(4, GL_FLOAT, 0, vertexArray);

	return true;

}

/// Create Centroid Sorts
bool ptVol::createCentroidSorts(void) {

	GLuint i, nT = volume.numTets;

	if (centroidSorted) delete [] centroidSorted;
	centroidSorted = new tetCentroid[nT];
	if (!centroidSorted) return false;

	for (i = 0; i < nT; ++i) {

		centroidSorted[i].id = i;
		centroidSorted[i].cZ = 0.0;

	}

	if (centroidBucket) {
		for (GLuint i = 0; i < NUM_LAYERS; ++i)
			centroidBucket[i].clear();
		delete [] centroidBucket;
	}
	centroidBucket = new vector< GLuint >[ NUM_LAYERS ];
	if (!centroidBucket) return false;

	return true;

}

/// Create Buffers
bool ptVol::createBuffers(void) {

	if (outputBuffer0) delete [] outputBuffer0;
	outputBuffer0 = new GLfloat[tetTexSize * tetTexSize * 4];
	if (!outputBuffer0) return false;

	if (outputBuffer1) delete [] outputBuffer1;
	outputBuffer1 = new GLfloat[tetTexSize * tetTexSize * 4];
	if (!outputBuffer1) return false;

	return true;

}

/// Create Output/Input Textures
void ptVol::createTextures(void) {

	/// Output Buffer 0 Texture
	glGenTextures(1, &tetOutputTex0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(TEX_FORMAT, tetOutputTex0);
	glTexParameteri(TEX_FORMAT, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(TEX_FORMAT, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(TEX_FORMAT, 0, TEX_TYPE_16, tetTexSize, tetTexSize, 0, GL_RGBA, GL_FLOAT, NULL);

	/// Output Buffer 1 Texture   
	glGenTextures(1, &tetOutputTex1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(TEX_FORMAT, tetOutputTex1);
	glTexParameteri(TEX_FORMAT, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(TEX_FORMAT, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(TEX_FORMAT, 0, TEX_TYPE_16, tetTexSize, tetTexSize, 0, GL_RGBA, GL_FLOAT, NULL);

	GLfloat *tetListTexBuffer;
	tetListTexBuffer = new GLfloat[tetTexSize*tetTexSize*4];

	for (GLuint i = 0; i < volume.numTets; ++i)
		for (GLuint j = 0; j < 4; ++j)
			tetListTexBuffer[i*4 + j] = volume.tetList[i][j];

	/// Tetrahedron List Texture
	glGenTextures(1, &tetListTex);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(TEX_FORMAT, tetListTex);
	glTexParameteri(TEX_FORMAT, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(TEX_FORMAT, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(TEX_FORMAT, 0, TEX_TYPE_32, tetTexSize, tetTexSize, 0, GL_RGBA, GL_FLOAT, tetListTexBuffer);

	delete [] tetListTexBuffer;

	GLfloat *vertListTexBuffer;
	vertListTexBuffer = new GLfloat[vertTexSize*vertTexSize*4];

	for (GLuint i = 0; i < volume.numVerts; ++i)
		for (GLuint j = 0; j < 4; ++j)
			vertListTexBuffer[i*4 + j] = volume.vertList[i][j];

	/// Vertex List Texture
	glGenTextures(1, &vertListTex);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(TEX_FORMAT, vertListTex);
	glTexParameteri(TEX_FORMAT, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(TEX_FORMAT, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(TEX_FORMAT, 0, TEX_TYPE_16, vertTexSize, vertTexSize, 0, GL_RGBA, GL_FLOAT, vertListTexBuffer);

	delete [] vertListTexBuffer;

	GLfloat *orderTableBuffer;
	orderTableBuffer = new GLfloat[81*4];

	for (GLuint i = 0; i < 81; ++i)
		for (GLuint j = 0; j < 4; ++j)
			orderTableBuffer[i*4 + j] = (GLfloat)(order_table[i][j]/4.0);

	/// Order Table Texture
	glGenTextures(1, &orderTableTex);
	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_1D,  orderTableTex);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, 81, 0, GL_RGBA, GL_FLOAT, orderTableBuffer);

	delete [] orderTableBuffer;

	GLfloat *tfTexBuffer;
	tfTexBuffer = new GLfloat[256*4];

	for (GLuint i = 0; i < 256; ++i)
		for (GLuint j = 0; j < 4; ++j)
			tfTexBuffer[i*4 + j] = volume.tf[i][j];

	/// Transfer Function Texture
	glGenTextures(1, &tfTex);
	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_1D, tfTex);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, 256, 0, GL_RGBA, GL_FLOAT, tfTexBuffer);

	delete [] tfTexBuffer;

	/// Psi Gamma Table Texture
	glGenTextures(1, &psiGammaTableTex);
	glActiveTexture(GL_TEXTURE6);
	glBindTexture(TEX_FORMAT, psiGammaTableTex);
	glTexParameteri(TEX_FORMAT, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(TEX_FORMAT, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(TEX_FORMAT, 0, TEX_TYPE_16, PSI_GAMMA_SIZE_BACK, PSI_GAMMA_SIZE_FRONT,
		     0, GL_ALPHA, GL_FLOAT, psiGammaTable);

}

/// Create Shaders
bool ptVol::createShaders(void) {

	if ( !glsl_support() ) return false;

	if (firstStepShader) delete firstStepShader;
	firstStepShader = new glslKernel();
	if (!firstStepShader) return false;

	firstStepShader->vertex_source("firstStep.vert");
	firstStepShader->fragment_source("firstStep.frag");
	firstStepShader->install(debug);

	if (secondStepShader) delete secondStepShader;
	secondStepShader = new glslKernel();
	if (!secondStepShader) return false;

	secondStepShader->vertex_source("secondStep.vert");
	secondStepShader->fragment_source("secondStep.frag");
	secondStepShader->install(debug);

	glGenFramebuffersEXT(1, &frameBuffer);

	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, frameBuffer);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(TEX_FORMAT, tetOutputTex0);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, TEX_FORMAT, tetOutputTex0, 0);
    
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(TEX_FORMAT, tetOutputTex1);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, TEX_FORMAT, tetOutputTex1, 0);
    
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

	firstStepShader->use();
	firstStepShader->set_uniform("tetrahedralTex", 2);
	firstStepShader->set_uniform("vertexPosTex", 3);
	firstStepShader->set_uniform("orderTableTex", 4);
	firstStepShader->set_uniform("vertTexSize", (GLfloat)vertTexSize);
	firstStepShader->use(0);

	secondStepShader->use();
	secondStepShader->set_uniform("tfTex", 5);
	secondStepShader->set_uniform("psiGammaTableTex", 6);
	secondStepShader->set_uniform("preIntTexSize", (GLfloat)PSI_GAMMA_SIZE_BACK);
	secondStepShader->set_uniform("maxEdgeLength", volume.maxEdgeLength);
	secondStepShader->set_uniform("brightness", (GLfloat)1.0);
	secondStepShader->use(0);

	return true;

}

/// Draw Quad
void ptVol::drawQuad() {

	glViewport(0, 0, (GLsizei) tetTexSize, (GLsizei) tetTexSize);

	glBegin(GL_QUADS);

	glTexCoord2d(0.0, 0.0); glVertex2d(minOrthoSize, minOrthoSize);

	glTexCoord2d(1.0, 0.0); glVertex2d(maxOrthoSize, minOrthoSize);
    
	glTexCoord2d(1.0, 1.0); glVertex2d(maxOrthoSize, maxOrthoSize);

	glTexCoord2d(0.0, 1.0); glVertex2d(minOrthoSize, maxOrthoSize);

	glEnd();

	glViewport(0, 0, winWidth, winHeight);

}

/// Run First Step
void ptVol::firstStep() {

	if (!firstStepShader) return;

	/// Create 2 output FBOs to return data from the first fragment shader
	GLenum colorBuffers[2] = { GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT };
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, frameBuffer);
	glDrawBuffers(2, colorBuffers);

	/// Render using the first step shader
	firstStepShader->use();

	drawQuad();

	firstStepShader->use(0);

	/// Read back output FBOs data
	glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
	glReadPixels(0, 0, tetTexSize, tetTexSize, GL_RGBA, GL_FLOAT, outputBuffer0);

	glReadBuffer(GL_COLOR_ATTACHMENT1_EXT);
	glReadPixels(0, 0, tetTexSize, tetTexSize, GL_RGBA, GL_FLOAT, outputBuffer1);

	/// Bind back the framebuffer
	glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

}

/// Sort
void ptVol::sort() {

	GLuint nT = volume.numTets;

	/// Switch to the selected sort method
	if (sortMethod == centroid) {

		/// Fill the centroid sorted array using the centroid Z computed
		///   in the first step shader
		for (GLuint i = 0; i < nT; ++i) {

			centroidSorted[i].id = i;
			centroidSorted[i].cZ = outputBuffer0[i*4 + 2];

		}

		/// STL stable sort
		std::sort( centroidSorted, centroidSorted + nT, less<tetCentroid>() );

	} else if (sortMethod == bucket) {

		/// Simple bucket sorting algorithm:
		///   Divide the normalized volume in a fixed number of bucket layers,
		///   put each tetrahedron in a layer that matches its centroid Z and
		///   render the buckets in back-to-front order.
		/// Note: Inside the buckets the tetrahedra remain unsorted

		GLuint bucket;

		for (GLuint i = 0; i < NUM_LAYERS; ++i)
			centroidBucket[i].clear();

		for (GLuint i = 0; i < nT; ++i) {

			bucket = (GLuint)(outputBuffer0[i*4 + 2] + 1.0) * (GLuint)((NUM_LAYERS-1) * 0.5);

			if (bucket < 0) bucket = 0;
			if (bucket > NUM_LAYERS-1) bucket = NUM_LAYERS-1;

			centroidBucket[bucket].push_back(i);

		}

	}

}

/// Setup and Reorder Arrays
void ptVol::setupAndReorderArrays() {

	GLuint tetId = 0, currBucket = 0, idBucket = 0, idTTT, arrayId, indicesId, cnt;

	for(GLuint i = 0; i < volume.numTets; ++i) {

		/// Switch to the selected sort method
		if (sortMethod == centroid) {

			tetId = centroidSorted[i].id;

		} else if (sortMethod == bucket) {

			if (currBucket >= NUM_LAYERS) return;

			while( centroidBucket[currBucket].size() <= idBucket ) {

				++currBucket;
				idBucket = 0;

			}

			tetId = centroidBucket[currBucket][idBucket];
			++idBucket;


		} else if (sortMethod == none) {

			tetId = i;

		}

		/// indices array index: each tetrahedron have 5 associated vertices
		indicesId = tetId * 5;
	
		/// Vertex and Color array index: each vertex/color have 4 components
		arrayId = indicesId * 4;

		/// Retrieve classification id (case 0 to 80) of the Ternary Truth Table
		///   and count triangle fan from the FBOs
		idTTT = (GLuint)outputBuffer0[tetId*4 + 3];
		cnt = (GLuint)outputBuffer1[tetId*4 + 3];

		if( cnt > 6 ) cnt = 6;
		if( idTTT > 80 ) idTTT = 80;
 
		/// If the projection is class 2 (count = 6) the thick vertex (first) must be
		///   updated to the intersection coordinates computed in the first step
		if (cnt == 6) {

			vertexArray[arrayId + 0] = outputBuffer0[tetId*4];
			vertexArray[arrayId + 1] = outputBuffer0[tetId*4 + 1];
			vertexArray[arrayId + 2] = 0.0;
			vertexArray[arrayId + 3] = 0.0; /// w = 0: computed in the first step

		} else { /// Else the thick vertex is one of the other vertices

			/// Use the Triangle Fan Order Table to determine which vertex
			///   must be copied to the thick vertex position
			for(GLuint j = 0; j < 3; ++j) {

				vertexArray[arrayId + j] =
					vertexArray[arrayId + (1+triangle_fan_order_table[idTTT][0])*4 + j];

			}

			vertexArray[arrayId + 3] = 1.0; /// w = 1: original tetrahedron vertex

		}

		/// Updates the thick vertex color: ( sf, sb, thickness )
		for(GLuint j = 0; j < 3; ++j)
			colorArray[tetId*5*3 + j] = outputBuffer1[tetId*4 + j];

		/// First vertex of the triangle fan is always the thick
		///   vertex of the tetrahedron
		indices[i][0] = indicesId;

		/// Number of vertices in the triangle fan
		count[i] = cnt;

		/// Reorder vertices
		for (GLuint j = 1; j < cnt; ++j) {

			indices[i][j] = (indicesId + 1) + triangle_fan_order_table[idTTT][j];

		}

	} // i

}

/// Run Second Step
void ptVol::secondStep() {

	glEnable(GL_BLEND);

	secondStepShader->use();

	glMultiDrawElements(GL_TRIANGLE_FAN, count, GL_UNSIGNED_INT,
			    (const GLvoid**)ids, volume.numTets);

	secondStepShader->use(0);

	glDisable(GL_BLEND);

}

/// Refresh Transfer Function (TF) and Brightness
void ptVol::refreshTFandBrightness(GLfloat brightness) {

	GLfloat *tfTexBuffer;
	tfTexBuffer = new GLfloat[256*4];

	for (GLuint i = 0; i < 256; ++i)
		for (GLuint j = 0; j < 4; ++j)
			tfTexBuffer[i*4 + j] = volume.tf[i][j];

	glActiveTexture(GL_TEXTURE5);
	glBindTexture(GL_TEXTURE_1D, tfTex);
	glTexSubImage1D(GL_TEXTURE_1D, 0, 0, 256, GL_RGBA, GL_FLOAT, tfTexBuffer);

	secondStepShader->use();
	secondStepShader->set_uniform("tfTex", 5);
	secondStepShader->set_uniform("brightness", brightness);
	secondStepShader->use(0);

	delete [] tfTexBuffer;

}

/// Draw volume wireframe
void ptVol::drawWireFrame() {

	glEnable(GL_BLEND);

	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	for (GLuint i = 0; i < volume.numTets; ++i) {

		for (GLuint f = 0; f < 4; ++f) {

			glColor4f( 1.0, 1.0, 0.0, 0.8 );

			glBegin(GL_TRIANGLES);

			for (GLuint k = 0; k < 3; ++k) {

				GLuint vId = volume.tetList[i][(f+k)%4];

				glVertex3fv( &volume.vertList[ vId ].xyz()[0] );

			}

			glEnd();

		}

	}

	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glDisable(GL_BLEND);

}
