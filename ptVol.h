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
 * C++ header.
 *
 */

/// --------------------------------   Definitions   ------------------------------------

#ifndef _PTVOL_H_
#define _PTVOL_H_

#include <sys/time.h>

#include "glslKernel.h"

#include "appVol.h"

/// Pre-defined colors
#define WHITE 1.0f, 1.0f, 1.0f
#define BLACK 0.0f, 0.0f, 0.0f
#define BLUE  0.0f, 0.0f, 1.0f

enum sortType { none, centroid, bucket }; ///< Three types of sort methods

typedef struct _tetCentroid {
	GLuint id; ///< Tetrahedron index
	GLfloat cZ; ///< Centroid Z
	friend bool operator < (const struct _tetCentroid& t1, const struct _tetCentroid& t2) {
		return t1.cZ < t2.cZ;
	}
} tetCentroid;

/// -----------------------------------   ptVol   -------------------------------------

/// PT Volume

class ptVol : public appVol {

public:

	/// Constructor
	ptVol( bool _d = true );

	/// Destructor
	~ptVol();

	/// Size of PT Volume (OpenGL in CPU)
	/// @return openGL usage in Bytes
	int sizeOf(void);

	/// Set functions
	void setColor(const GLclampf& _r, const GLclampf& _g, const GLclampf& _b) {
		backGround = vec3( _r, _g, _b );
		glClearColor(backGround.r(), backGround.g(), backGround.b(), 0.0);
	}
	void setOrtho(const GLdouble& _minOrtho, const GLdouble& _maxOrtho) {
		minOrthoSize = _minOrtho;
		maxOrthoSize = _maxOrtho;
	}
	void setWindow(const GLsizei& _winW, const GLsizei& _winH) {
		winWidth = _winW;
		winHeight = _winH;
	}
	void setSortMethod(sortType _sT) {
		sortMethod = _sT;
	}

	/// OpenGL Setup
	/// Compute texture sizes, create buffers, arrays and textures
	/// @return true if it succeed
	bool glSetup(void);

	/// Run First Step
	///   The first step shader computes each tetrahedron
	///    projection and classify it
	/// @arg readFBOTime returns the time spent to read the FBOs
	/// @return total time spent in the first step
	void firstStep(GLdouble& totalTime) {
		static struct timeval starttime, endtime;
		gettimeofday(&starttime, 0);
		firstStep();
		gettimeofday(&endtime, 0);
		totalTime = (endtime.tv_sec - starttime.tv_sec) + (endtime.tv_usec - starttime.tv_usec)/1000000.0;
	}
	void firstStep(void);

	/// Sort
	///   Sort the tetrahedra using the selected sort method
	/// @arg totalTime returns total time spent in sorting
	void sort(GLdouble& totalTime, sortType _sT = none) {
		static struct timeval starttime, endtime;
		gettimeofday(&starttime, 0);
		sort(_sT);
		gettimeofday(&endtime, 0);
		totalTime = (endtime.tv_sec - starttime.tv_sec) + (endtime.tv_usec - starttime.tv_usec)/1000000.0;
	}
	void sort(sortType _sT) {
		setSortMethod(_sT);
		sort();
	}
	void sort(void);

	/// Setup and Reorder Arrays
	///   Between the first and second step, the vertex, color,
	///   indices and count arrays must be reorganized acoording to
	///   the first step output buffers
	/// @arg totalTime returns total time spent in the setup step
	void setupAndReorderArrays(GLdouble& totalTime) {
		static struct timeval starttime, endtime;
		gettimeofday(&starttime, 0);
		setupAndReorderArrays();
		gettimeofday(&endtime, 0);
		totalTime = (endtime.tv_sec - starttime.tv_sec) + (endtime.tv_usec - starttime.tv_usec)/1000000.0;
	}
	void setupAndReorderArrays(void);

	/// Run Second Step
	///   Draw Arrays using one OpenGL function: glMultiDrawElement
	///   to draw all vertices stored into vertexArray and colorArray
	/// @arg totalTime returns total time spent in the second step
	void secondStep(GLdouble& totalTime) {
		static struct timeval starttime, endtime;
		gettimeofday(&starttime, 0);
		secondStep();
		gettimeofday(&endtime, 0);
		totalTime = (endtime.tv_sec - starttime.tv_sec) + (endtime.tv_usec - starttime.tv_usec)/1000000.0;
	}
	void secondStep(void);

	/// Refresh Transfer Function (TF) and Brightness
	/// @arg brightness term
	void refreshTFandBrightness(GLfloat brightness = 1.0);

	/// Draw volume wireframe
	void drawWireFrame(void);

private:

	/// Create Arrays
	/// vertexArray: store vertices [_tet0_(vThick, v0, v1, v2, v3) ; ...]
	/// colorArray: store colors [_tet0_(cThick, c0, c1, c2, c3) ; ...]
	///   where vi = (x, y, z, 1|0) and ci  = (r, g, b)
	/// indices, ids, count: data structure for glMultiDrawElements
	/// @return true if it succeed
	bool createArrays(void);

	/// Create Buffers
	/// outputBuffer0,1: gives output from the 1st fragment shader
	/// @return true if it succeed
	bool createBuffers(void);

	/// Create Centroid Sorts
	/// centroidSorted: {  (tetId, centroidZ), ... }
	/// centroidBucket: {  _bucket0_(tetId_0, tetId_1, ...), ... }
	/// @return true if it succeed
	bool createCentroidSorts(void);

	/// Create Output/Input Textures
	/// Texture 0: { Intersection(x, y), thickness, indexReorder }
	/// Texture 1: { Color_Intersection(r, g, b), test.z }
	/// Texture 2: { Tetrahedral vertices ids (v0, v1, v2, v3) }
	/// Texture 3: { Vertices coordinates (x, y, z, 1.0) }
	/// Texture 4: { Ternary Truth Table (id0, id1, id2, id3) }
	/// Texture 5: { Transfer Function (r, g, b, thau) }
	/// Texture 6: { Psi Gamma Table (psi) }
	void createTextures(void);

	/// Create Shaders
	/// First Step Shader:  Computes projections and PT classification
	/// Second Step Shader: Compute ray integration with PsiGammaTable
	/// @return true if it succeed
	bool createShaders(void);

	/// Draw Quad
	/// Draw a quadrilateral matching the size of the
	///   tetrahedral texture to run first step GPGPU shader
	void drawQuad(void);

	/// PT Volume Properties
	glslKernel *firstStepShader, *secondStepShader;

	GLfloat *vertexArray, *colorArray;
	GLuint **indices;
	GLint *count;
	GLvoid **ids;

	tetCentroid *centroidSorted; ///< Stable sorting
	vector< GLuint >* centroidBucket; ///< Bucket sorting

	GLfloat *outputBuffer0, *outputBuffer1; ///< Output Buffers

	GLuint frameBuffer, tetOutputTex0, tetOutputTex1; ///< FBO
	GLuint vertListTex, tetListTex, orderTableTex; ///< Frag 1
	GLuint tfTex, psiGammaTableTex; ///< Frag 2

	GLuint vertTexSize, tetTexSize; ///< Texture sizes

	vec3 backGround; ///< Background color
	GLdouble minOrthoSize, maxOrthoSize; /// Minimum and maximum ortho size
	GLsizei winWidth, winHeight; ///< Width x Height pixel resolution

	sortType sortMethod; ///< Selected sort method

};

#endif
