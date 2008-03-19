/**
 *
 *    Render Volume GPU
 *
 *  File: volume.h
 *
 *  Authors:
 *    Andre Maximo
 *    Ricardo Marroquim
 *
 *  Last Update: Mar 03, 2006
 *
 */

#include "glslKernel.h"

extern "C" {
#include <assert.h>
#include <math.h>
#include <GL/glut.h>
#ifndef NO_NVIDIA
#include <GL/glext.h>
#endif
#include <float.h>
}

#include <vector>

#include "transferFunction.h"

#include "bbox_edit.h"

#define MINORTHOSIZE -1.2
#define MAXORTHOSIZE 1.2

#define NUM_BUCKETS 100

/// Object file format type
enum fileType { OFF, GEO, BIN8, BIN16BE, BIN16LE,
		MULTIRAW8, MULTIRAW16BE, MULTIRAW16LE,
		SINGLERAW8, SINGLERAW16, PNM8 };

typedef unsigned int uint;
typedef unsigned short ushort;

using namespace std;

typedef struct _pairTet {

  GLuint id;
  GLfloat cZ;

} pairTet;

//#define TEX_FORMAT GL_TEXTURE_2D
//#define TEX_TYPE GL_RGBA
//#define TEX_FORMAT GL_TEXTURE_RECTANGLE_NV
//#define TEX_TYPE GL_FLOAT_RGBA32_NV
#define TEX_FORMAT GL_TEXTURE_2D
#define TEX_TYPE_32 GL_RGBA32F_ARB
#define TEX_TYPE_16 GL_RGBA16F_ARB

/// Extern global variable declarations from main

extern bool debug_shaders, debug_cout, integrating, sorting;
extern GLint modelWinWidth, modelWinHeight;

class volume
{
 public:
  volume();
  ~volume();

  /// Delete procedures
  void deleteShaders(void);
  void deleteTextures(void);
  void deleteBuffers(void);
  void deleteArrays(void);

  bool readFile(const char* filename, const fileType& ft,
		const uint& rows = 0, const uint& cols = 0,
		const uint& nums = 0);

  void findMaxEdgeLength(void);

  void CreateTextures(void);
  void CreateShaders(void);

  void Normalize();

  void Draw(void);

  /// Functions to handle volume OFF
  void addVertex(GLfloat x, GLfloat y, GLfloat z, GLfloat s);	         
  void setScalar(int idVertex, GLfloat s) {
    positionBuffer[idVertex * 4 + 3] = s;
  }
  GLfloat getScalar(int idVertex) { 
    return positionBuffer[idVertex*4 + 3];
  }
  void addCell(uint idxV0, uint idxV1, uint idxV2, uint idxV3);
		
  uint getNumVerts(void) { return numVerts; }
  uint getNumTets(void) { return numTets; }
  uint getPsiGamaTableSize(void) { return preIntTexSize; }
  uint getExpTexSize(void) { return preIntTexSize; }

  void reloadTetTex(void);
  void reloadTFTex(void);
  void UpdateData(void);

  void centroidSorting(void);
  void bucketSorting(void);

  void SetupArrays(bool useCentroid);
  void createBuffers(const uint& nt, const uint& nv);

  uint getCurTets(void) { return curTets; }

  transferFunction tf; //< transfer function
  bBox bB; //< volume bounding box

 private:

#ifndef NO_NVIDIA

  glslKernel *shaders_1st, *shaders_2nd; // currently used shaders
  glslKernel *shaders_2nd_with_int, *shaders_2nd_no_int; // shader with/no integrate
  GLuint frameBuffer, tetOutputTex0, tetOutputTex1; // FBO
  GLuint vertexPosTex, tetrahedralTex, orderTableTex; // Frag 1
  GLuint tfTex, expTex, psiGammaTableTex; // Frag 2

#else

  void compute_1st_shader_on_cpu(void);
  void compute_2nd_shader_on_cpu(void);

#endif

  GLfloat *tetrahedralBuffer, *positionBuffer;
  GLfloat *outputBuffer0, *outputBuffer1;

  GLfloat *tfTexBuffer;
	
  GLfloat *vertexArray, *colorArray;
		
  GLuint **indices;
  GLint *count;
  GLvoid **ids;

  GLfloat maxEdgeLength;

  uint numTets, numVerts;

  uint curTets, discardedTets;

  uint vertTexSize, tetTexSize, expTexSize, preIntTexSize;

  pairTet *cellSorted;

  uint **centroidBucket;
  uint *countBucket;

  /// Functions to read volume files
  bool readOFF(const char* filename);
  bool readGeoOFF(const char* filename);
  bool readMedBIN(const char* filename, const uint& rows,
		  const uint& cols, const uint& nums,
		  const fileType& ft);

  void generateTets(void);

  void CreateOutputTextures();
  void CreateInputTextures();
  void CreateArrays(void);
		
  void DrawQuad(void);
	
};
