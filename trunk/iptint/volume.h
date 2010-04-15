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

extern "C" {
#include <assert.h>
#include <math.h>
#include <stdlib.h>
//#include <GL/glut.h>
#ifndef NO_NVIDIA
//#include <GL/glext.h>
#endif
#include <float.h>
}

#include <vector>

#ifndef NO_NVIDIA
#include "glslKernel.h"
#endif

#include "transferFunction.h"
#include "illuminationControl.h"

#define MINORTHOSIZE -1.2
#define MAXORTHOSIZE 1.2

#define NUM_BUCKETS 150

/// Object file format type
enum fileType { OFF, GRADOFF, GEO, BIN8, BIN16BE, BIN16LE,
		MULTIRAW8, MULTIRAW16BE, MULTIRAW16LE,
		SINGLERAW8, SINGLEGRADRAW8, SINGLERAW16, PNM8 };

typedef unsigned int uint;
typedef unsigned short ushort;

using namespace std;

typedef struct _pairTet {

  GLuint id;
  GLfloat cZ;

} pairTet;

//#define TEX_FORMAT GL_TEXTURE_2D
//#define TEX_TYPE GL_RGBAy display
//#define TEX_FORMAT GL_TEXTURE_RECTANGLE_NV
//#define TEX_TYPE GL_FLOAT_RGBA32_NV
#define TEX_FORMAT GL_TEXTURE_2D
#define TEX_TYPE GL_RGBA32F_ARB

/// Extern global variable declarations from main

extern bool debug_shaders, debug_cout, integrating, sorting, shading;
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

  void CreateTextures(void);
  void CreateShaders(void);

  void Normalize();

  void Draw(void);

  /// Functions to handle volume OFF
  void addVertex(GLfloat x, GLfloat y, GLfloat z, GLfloat s);
  void addGradient(GLfloat gx, GLfloat gy, GLfloat gz);	         
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
  void updateIC(void);

  void centroidSorting(void);
  void bucketSorting(void);

  void SetupArrays(bool useCentroid);
  void createBuffers(const uint& nt, const uint& nv);

  uint getCurTets(void) { return curTets; }

  transferFunction tf; //< transfer function
  illuminationControl ic; //illumination control

 private:

#ifndef NO_NVIDIA

  glslKernel *shader_1st, *shaders_2nd; // currently used shaders
  glslKernel *shaders_2nd_with_int, *shaders_2nd_no_int, *shaders_2nd_with_shading; // shader with/no integrate
  GLuint frameBuffer, tetOutputTex0, tetOutputTex1, tetOutputTex2, tetOutputTex3; // FBO
  GLuint vertexPosTex, gradientTex, tetrahedralTex, orderTableTex; // Frag 1
  GLuint tfTex, expTex, psiGammaTableTex; // Frag 2

#else

  void compute_1st_shader_on_cpu(void);
  void compute_2nd_shader_on_cpu(void);

#endif

  GLfloat *tetrahedralBuffer, *positionBuffer;
  GLfloat *gradientBuffer;
  GLfloat *outputBuffer0, *outputBuffer1, *outputBuffer2, *outputBuffer3;
  
  GLfloat *tfTexBuffer;

  GLfloat *vertexArray, *colorArray, 
    *gradientFrontArray, *gradientBackArray;

  GLuint **indices;
  GLint *count;
  GLvoid **ids;

  uint numTets, numVerts;
  uint vertTexSize, tetTexSize, expTexSize, preIntTexSize;

  uint curTets, discardedTets;

  pairTet* cellSorted;

  uint dimX, dimY, dimZ;

  GLfloat ks, kd, rho[2];

  GLfloat max_thickness;

  vector<uint> centroidBucket[NUM_BUCKETS+1];

  /// Functions to read volume files
  bool readOFF(const char* filename);
  bool readGradOFF(const char* filename);
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
