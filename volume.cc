/**
 *
 *    Render Volume GPU
 *
 *  File: volume.cc
 *
 *  Authors:
 *    Andre Maximo
 *    Ricardo Marroquim
 *
 *  Last Update: Apr 27, 2006
 *
 */

//#define GL_GLEXT_PROTOTYPES

#include <iostream>
#include <iomanip>

#include "volume.h"

#include "tables.h"

/// Shaders CPU version

#ifdef NO_NVIDIA
#include "../psiGammaTable512.h"
#include "cpu_frags.h"
#endif

/// Static local function

static void errcheck(char * place);
static bool lessCentroid(const pairTet& p1, const pairTet& p2);

/// Error check
/// @arg place were this functions was called

void errcheck(char * place)
{
  static GLenum errCode;
  const GLubyte *errString;
    
  if ((errCode = glGetError()) != GL_NO_ERROR)
    {
      errString = gluErrorString(errCode);
      fprintf (stderr, "%s : OpenGL Error: %s\n", place, errString);
      exit(1);
    }
}

/// Less Centroid comparison
/// @arg p1 first tet's pair 
/// @arg p2 second tet's pair 

bool lessCentroid(const pairTet& p1, const pairTet& p2) 
{
  return p1.cZ < p2.cZ;
}

/**
 * volume Class
 */

/// Constructor

volume::volume()
{
}

/// Destructor

volume::~volume()
{
#ifndef NO_NVIDIA
  deleteShaders();
  deleteTextures();
#endif
  deleteBuffers();
  deleteArrays();
}

/// Delete Shader
void volume::deleteShaders(void)
{
#ifndef NO_NVIDIA
  if (shaders_1st)
    delete shaders_1st;
  if (shaders_2nd_with_int)
    delete shaders_2nd_with_int;
  if (shaders_2nd_no_int)
    delete shaders_2nd_no_int;
#endif
}

/// Delete Textures

void volume::deleteTextures(void)
{
#ifndef NO_NVIDIA
  glDeleteFramebuffersEXT(1, &frameBuffer);
  glDeleteTextures(1, &tetOutputTex0);
  glDeleteTextures(1, &tetOutputTex1);
  glDeleteTextures(1, &tetrahedralTex);
  glDeleteTextures(1, &vertexPosTex);
  glDeleteTextures(1, &orderTableTex);
  glDeleteTextures(1, &tfTex);
  //glDeleteTextures(1, &expTex);
  glDeleteTextures(1, &psiGammaTableTex);
#endif
}

/// Delete Buffers

void volume::deleteBuffers(void)
{
  if (tetrahedralBuffer)
    delete [] tetrahedralBuffer;
  if (positionBuffer)
    delete [] positionBuffer;
  if (outputBuffer0)
    delete [] outputBuffer0;
  if (outputBuffer1)
    delete [] outputBuffer1;
}

/// Delete Arrays

void volume::deleteArrays(void)
{
  if (vertexArray)
    delete [] vertexArray;
  if (colorArray)
    delete [] colorArray;

  if (indices) {
    for (uint i = 0; i < numTets; ++i)
      delete [] indices[i];
  }

  if (count)
    delete [] count;

  if (cellSorted)
    delete [] cellSorted;

  if (centroidBucket) {
    for (uint i = 0; i < numTets; ++i)
      delete [] centroidBucket[i];
  }

  if (countBucket)
    delete [] countBucket;
}

/// Read any file type

bool volume::readFile(const char* filename, const fileType& ft,
		      const uint& rows, const uint& cols,
		      const uint& nums)
{
  bool rb = false;
  if (ft == OFF) rb = readOFF(filename);
  else if (ft == GEO) rb = readGeoOFF(filename);
  else rb = readMedBIN(filename, rows, cols, nums, ft);
  return rb;
}

/// Read Object File Format (OFF)

bool volume::readOFF(const char* filename)
{
  uint numVerts, numTets, idxV1, idxV2, idxV3, idxV4;
  GLfloat x, y, z, scalar;

  ifstream input(filename);

  if(input.fail())
    return false;

  input >> numVerts >> numTets;

  createBuffers(numTets, numVerts);

  for(uint i = 0; i < numVerts; i++)
    {
      input >> x >> y >> z >> scalar;
      addVertex(x, y, z, scalar);
    }

  for(uint i = 0; i < numTets; i++)
    {
      input >> idxV1 >> idxV2 >> idxV3 >> idxV4;
      addCell(idxV1, idxV2, idxV3, idxV4);
    }

  Normalize();

  input.close();

  return true;
}

/// Read Geological OFF

bool volume::readGeoOFF(const char* filename)
{
  uint numVerts, numTets, idxV1, idxV2, idxV3, idxV4, id, region;
  GLfloat x, y, z;

  ifstream input(filename);

  if(input.fail())
    return false;

  input >> numVerts >> numTets;

  createBuffers(numTets, numVerts);

  for(uint i = 0; i < numVerts; i++)
    {
      input >> id >> x >> y >> z;
      addVertex(x, y, z, 1.0);
    }

  for(uint i = 0; i < numTets; i++)
    {
      input >> id >> idxV1 >> idxV2 >> idxV3 >> idxV4 >> region;
      addCell(idxV1, idxV2, idxV3, idxV4);
      setScalar(idxV1, (GLfloat)region);
      setScalar(idxV2, (GLfloat)region);
      setScalar(idxV3, (GLfloat)region);
      setScalar(idxV4, (GLfloat)region);
    }

  Normalize();

  input.close();

  return true;
}

/// Read Medical BIN
/// @arg filename name of the file to be open
/// @arg rows number of rows of the binary file
/// @arg cols number of columns of the binary file
/// @arg nums number of slices of the binary file
/// @arg ft file type

bool volume::readMedBIN(const char* filename, const uint& rows,
			const uint& cols, const uint& nums,
			const fileType& ft)
{
  uint nB = 1; //< number of Bytes for each scalar
  bool ffiles = false; //< true if it is fragmented files
  bool bigE = false; //< tells if the file is in big or little endian format
  bool pnm = false; //< true if it is a Netpbm file

  // File type selection
  if (ft == BIN8) ;
  if (ft == BIN16LE) nB = 2;
  if (ft == BIN16BE) { nB = 2; bigE = true; }
  if (ft == MULTIRAW16LE) { nB = 2; ffiles = true; }
  if (ft == MULTIRAW16BE) { nB = 2; ffiles = true; bigE = true; }
  if (ft == SINGLERAW8) ;
  if (ft == SINGLERAW16) { nB = 2; bigE = true; }
  if (ft == PNM8) { ffiles = true; pnm = true; }

  // Set bounding box size
  bB.setSize(rows, cols, nums);

  /// Get Offsets
  uint offset_x_begin = bB.getOffsetMinX(); uint offset_x_end = bB.getOffsetMinX();
  uint offset_y_begin = bB.getOffsetMinY(); uint offset_y_end = bB.getOffsetMaxY();
  uint offset_z_begin = bB.getOffsetMinZ(); uint offset_z_end = bB.getOffsetMaxZ();
  uint xy_step = bB.getStepXY(); uint z_step = bB.getStepZ();

  if (!bB.testValid()) {
    cerr << "Initial bounding box values are not valid!" << endl;
    return false;
  }

  // Get Dimensions
  uint dimX = bB.getDimX();
  uint dimY = bB.getDimY();
  uint dimZ = bB.getDimZ();

  // Create Buffers
  uint numTets = (dimX-1)*(dimY-1)*(dimZ-1)*5;
  uint numVerts = dimX*dimY*dimZ;
  createBuffers(numTets, numVerts);

  // File to be read
  ifstream volume_file;

  if (!ffiles) { // if its only one binary file
    volume_file.open(filename);
    if (volume_file.fail()) {
      cerr << "Can't open " << filename << " for reading." << endl;
      return false;
    }
  }

  for (uint z = offset_z_begin; z < (nums - offset_z_end); z+=z_step) {

    if (ffiles) { // if its more than one binary file
      stringstream ss;
      if (pnm)
	ss << filename << "." << (z+1) << ".pnm";
      else
	ss << filename << "." << (z+1);
      volume_file.open(ss.str().c_str());
      if (volume_file.fail()) {
	cerr << "Can't open " << ss.str() << " for reading." << endl;
	return false;
      }
      if (pnm) { // jump the first 3 lines (header)
	char buf[256];
	volume_file.getline(buf, 256);
	volume_file.getline(buf, 256);
	volume_file.getline(buf, 256);
      }
    }

    for (uint y = offset_y_begin; y < (cols - offset_y_end); y+=xy_step) {

      for (uint x = offset_x_begin; x < (rows - offset_x_end); x+=xy_step) {

	// seek the file to the correct position
	if (ffiles) {
	  if (pnm)
	    volume_file.seekg( (uint)(y*rows*nB*3 + x*nB*3) );
	  else
	    volume_file.seekg( (uint)(y*rows*nB + x*nB) );
	}
	else
	  volume_file.seekg( (uint)(z*cols*rows*nB + y*rows*nB + x*nB) );

	char* buf;
	GLfloat data;
	buf = new char[nB];
	volume_file.read(buf, nB);

	if (nB > 1) {
	  if (bigE)
	    data = (GLfloat)((unsigned char)buf[1] + 256*(unsigned char)buf[0]);
	  else
	    data = (GLfloat)((unsigned char)buf[0] + 256*(unsigned char)buf[1]);
	}
	else
	  data = (GLfloat)((unsigned char)buf[0]);

	delete [] buf;

	if (pnm)
	  addVertex((GLfloat)x, (GLfloat)y, (GLfloat)z*20, data);
	else
	  addVertex((GLfloat)x, (GLfloat)y, (GLfloat)z, data);
      }
    }
    if (ffiles) // if its more than one binary file
      volume_file.close();
  }

  if (!ffiles) // if its only one binary file
    volume_file.close();

  generateTets();

  Normalize();

  return true;
}

/// Generate Tetrahedra (5 by Hexahedron)

void volume::generateTets(void)
{
  // Get Dimensions and step
  uint dimX = bB.getDimX();
  uint dimY = bB.getDimY();
  uint dimZ = bB.getDimZ();
  uint xy_step = bB.getStepXY(); uint z_step = bB.getStepZ();

  uint idv[8];

  for (uint z = 0; z < (dimZ - 1); z += (uint)z_step) {
    for (uint y = 0; y < (dimY - 1); y += (uint)xy_step) {
      for (uint x = 0; x < (dimX - 1); x += (uint)xy_step) {

	idv[0] = (uint)(   x +     y*(dimX) +     z*(dimX*dimY) );
	idv[1] = (uint)( x+1 +     y*(dimX) +     z*(dimX*dimY) );
	idv[2] = (uint)(   x + (y+1)*(dimX) +     z*(dimX*dimY) );
	idv[3] = (uint)( x+1 + (y+1)*(dimX) +     z*(dimX*dimY) );
	idv[4] = (uint)(   x +     y*(dimX) + (z+1)*(dimX*dimY) );
	idv[5] = (uint)( x+1 +     y*(dimX) + (z+1)*(dimX*dimY) );
	idv[6] = (uint)(   x + (y+1)*(dimX) + (z+1)*(dimX*dimY) );
	idv[7] = (uint)( x+1 + (y+1)*(dimX) + (z+1)*(dimX*dimY) );

	addCell(idv[0], idv[2], idv[3], idv[6]);
	addCell(idv[3], idv[5], idv[6], idv[7]);
	addCell(idv[0], idv[4], idv[5], idv[6]);
	addCell(idv[0], idv[1], idv[3], idv[5]);
	addCell(idv[0], idv[3], idv[5], idv[6]);
      }
    }
  }
}

/// Create Buffers
/// tetrahedralBuffer - gives tetrahedra connectivity
/// positionBuffer - gives vertices position
/// outputBuffer0,1 - gives output from the 1st fragment shader

void volume::createBuffers(const uint& nt, const uint& nv)
{
  numTets = nt;
  numVerts = nv;

  vertTexSize = (uint)ceil(sqrt(numVerts));
  tetTexSize = (uint)ceil(sqrt(numTets));

  if (tetrahedralBuffer)
    delete [] tetrahedralBuffer;
  if (positionBuffer)
    delete [] positionBuffer;

  tetrahedralBuffer = new GLfloat[numTets * 4];
  positionBuffer = new GLfloat[vertTexSize * vertTexSize * 4]; 

  // Discard n tetrahedra
  curTets = numTets;
  discardedTets = 0;

  if (outputBuffer0)
    delete [] outputBuffer0;
  if (outputBuffer1)
    delete [] outputBuffer1;

  outputBuffer0 = new GLfloat[tetTexSize * tetTexSize * 4];
  outputBuffer1 = new GLfloat[tetTexSize * tetTexSize * 4];
}

/// Create Arrays
/// vertexarray - store vertexes [_tet0_(vProj, v0, v1, v2, v3);...]
/// colorarray - store colors [_tet0_(cProj, c0, c1, c2, c3);...]
/// indices, ids, count - data structure for glMultiDrawElements

void volume::CreateArrays(void)
{
  uint idVArray = 0, idCArray = 0, i = 0, j = 0, k = 0;
  GLuint vertId;  

  if (vertexArray)
    delete [] vertexArray;
  if (colorArray)
    delete [] colorArray;

  vertexArray = new GLfloat[numTets * 4 * 5];
  colorArray = new GLfloat[numTets * 3 * 5];
    
  for (i = 0; i < numTets; ++i)
    {
      idVArray = i * 5 * 4;
      idCArray = i * 5 * 3;
	
      /// Initialize thick vertex
      for (k = 0; k < 4; ++k)
	{
	  vertexArray[idVArray + 0*4 + k] = 0.0;
	  colorArray[idCArray + 0*4 + k] = 0.0;
	}
	
      /// Fill vertex array with tetrahedron's four vertices
      for (j = 1; j < 5; ++j)
	{
	  vertId = (GLuint)tetrahedralBuffer[i*4 + j-1];
	  for (k = 0; k < 3; ++k)
	    {
	      vertexArray[idVArray + j*4 + k] = positionBuffer[vertId*4 + k];
	      colorArray[idCArray + j*3 + k] = positionBuffer[vertId*4 + 3];
	    }
	  //scalar (in blue) and w coordinate
	  vertexArray[idVArray + j*4 + 3] = 1.0;
	  colorArray[idCArray + j*3 + 2] = 0.0;
	}
    }

  if (indices) {
    for (i = 0; i < numTets; ++i)
      delete [] indices[i];
  }

  /// Alocate indexes, count and ids arrays for glMultiDrawElement
  indices = new GLuint*[numTets];

  for (i = 0; i < numTets; ++i)
    indices[i] = new GLuint[6];

  if (count)
    delete [] count;

  count = new GLint[numTets];

  ids = new GLvoid*[numTets];

  for (i = 0; i < numTets; ++i)
    { 
      ids[i] = (GLvoid*)indices[i];
      count[i] = 6;
    }

#ifndef NO_NVIDIA
  glEnableClientState(GL_COLOR_ARRAY);
  glEnableClientState(GL_VERTEX_ARRAY);
  glColorPointer(3, GL_FLOAT, 0, colorArray);
  glVertexPointer(4, GL_FLOAT, 0, vertexArray);
#endif
}

/// Create Output Textures
/// Buffer: 0 { Intersection(x, y), thickness, indexReorder }
/// Buffer: 1 { Color_Intersection(r, g, b), test.z }

void volume::CreateOutputTextures()
{
#ifndef NO_NVIDIA
  //Generate first output texture
  glGenTextures(1, &tetOutputTex0);
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(TEX_FORMAT, tetOutputTex0);
  glTexParameteri(TEX_FORMAT, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(TEX_FORMAT, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage2D(TEX_FORMAT, 0, TEX_TYPE_16, tetTexSize, tetTexSize, 0, GL_RGBA, GL_FLOAT, NULL);
    
  //Generate second output texture
  glGenTextures(1, &tetOutputTex1);
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(TEX_FORMAT, tetOutputTex1);
  glTexParameteri(TEX_FORMAT, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(TEX_FORMAT, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage2D(TEX_FORMAT, 0, TEX_TYPE_16, tetTexSize, tetTexSize, 0, GL_RGBA, GL_FLOAT, NULL);
    
  errcheck("outputTexCreation");
#endif
}

/// Reload Tetrahedra Textures
/// Textures: { tetrahedral, output0, output1, output2 }

void volume::reloadTetTex(void)
{
  /// Discard n tetrahedra
  GLfloat *curTetBuffer;
  curTetBuffer = new GLfloat[numTets * 4];

  discardedTets = 0;
  for(uint i = 0; i < numTets; ++i)
    {
      GLfloat s;
      bool discardTet = false;

      for (GLint j = 0; j < 4; ++j)
	{
	  GLuint vertId = (GLuint)tetrahedralBuffer[i*4 + j];

	  s = positionBuffer[vertId*4 + 3];

	  // all the iterations must agree in discarding the tet
	  discardTet = (tfTexBuffer[(uint)(s*255)*4 + 3] == 0.0);

	  if (!discardTet)
	    break;
	}

      if (discardTet) {
	discardedTets++;
	continue;
      }

      uint idNew = (i - discardedTets);

      /// Initialize thick vertex
      for (uint k = 0; k < 4; ++k)
	{
	  colorArray[idNew*5*3 + 0*3 + k] = 0.0;
	  vertexArray[idNew*5*4 + 0*4 + k] = 0.0;
	}
      
      /// Rebuild Tet Buffer and Arrays
      for (uint j = 0; j < 4; ++j) {

	curTetBuffer[idNew*4 + j] = tetrahedralBuffer[i*4 + j];

	GLuint vertId = (GLuint)curTetBuffer[idNew*4 + j];

	for (uint k = 0; k < 3; ++k)
	  {
	    colorArray[idNew*5*3 + (j+1)*3 + k] = positionBuffer[vertId*4 + 3];
	    vertexArray[idNew*5*4 + (j+1)*4 + k] = positionBuffer[vertId*4 + k];
	  }

	//scalar (in blue) and w coordinate
	colorArray[idNew*5*3 + (j+1)*3 + 2] = 0.0;
	vertexArray[idNew*5*4 + (j+1)*4 + 3] = 1.0;
      }
    }

  uint newCurTets = numTets - discardedTets;

  /// Discard n tetrahedra
  if (newCurTets != curTets) {

    curTets = newCurTets;
    tetTexSize = (uint)ceil(sqrt( curTets ));

    glDeleteTextures(1, &tetrahedralTex);

    /// Reload tetrahedral texture
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(TEX_FORMAT, tetrahedralTex);
    glTexParameteri(TEX_FORMAT, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(TEX_FORMAT, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(TEX_FORMAT, 0, TEX_TYPE_32, tetTexSize, tetTexSize, 0, GL_RGBA, GL_FLOAT, curTetBuffer);

    shaders_1st->use();
    shaders_1st->set_uniform("tetrahedralTex", 2);
    shaders_1st->use(0);

  }

  delete [] curTetBuffer;
}

/// Reload Transfer Function Texture
/// Texture:  { Transfer function (tf[i].r, tf[i].g, tf[i].b, tf[i].s) }

void volume::reloadTFTex(void)
{
#ifndef NO_NVIDIA

  for (uint i = 0; i < 256; ++i)
    {
      for (uint j = 0; j < 4; ++j)
	tfTexBuffer[i*4 + j] = tf[i][j];
    }

  reloadTetTex();
  
  /// Reload TF texture
  glActiveTexture(GL_TEXTURE5);
  glBindTexture(GL_TEXTURE_1D, tfTex);
  glTexSubImage1D(GL_TEXTURE_1D, 0, 0, 256, GL_RGBA, GL_FLOAT, tfTexBuffer);

  shaders_2nd_with_int->use();
  shaders_2nd_with_int->set_uniform("tfTex", 5);
  shaders_2nd_with_int->use(0);
  shaders_2nd_no_int->use();
  shaders_2nd_no_int->set_uniform("tfTex", 5);
  shaders_2nd_no_int->use(0);

#endif
}

/// Create Input Texture
/// Texture:  { Tetrahedral vertex ids (v0, v1, v2, v3) }
/// Texture:  { Vertex Position (x, y, z, 1.0) }
/// Texture:  { Ternary Truth Table (id0, id1, id2, id3) }

void volume::CreateInputTextures(void)
{
#ifndef NO_NVIDIA

  if (debug_cout) {
    cout << "*** Vertex Texture Size      : " << setw(10) << vertTexSize << " ***" << endl;
    cout << "*** Tetrahedral Texture Size : " << setw(10) << tetTexSize << " ***" << endl;
  }

  /// Generate tetrahedral texture
  glGenTextures(1, &tetrahedralTex);
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(TEX_FORMAT, tetrahedralTex);
  glTexParameteri(TEX_FORMAT, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(TEX_FORMAT, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage2D(TEX_FORMAT, 0, TEX_TYPE_32, tetTexSize, tetTexSize, 0, GL_RGBA, GL_FLOAT, tetrahedralBuffer);

  /// Generate vertex position texture
  glGenTextures(1, &vertexPosTex);
  glActiveTexture(GL_TEXTURE3);
  glBindTexture(TEX_FORMAT, vertexPosTex);
  glTexParameteri(TEX_FORMAT, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(TEX_FORMAT, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexImage2D(TEX_FORMAT, 0, TEX_TYPE_16, vertTexSize, vertTexSize, 0, GL_RGBA, GL_FLOAT, positionBuffer);

  GLfloat *orderTableBuffer;
  orderTableBuffer = new GLfloat[81*4];
//   GLubyte *orderTableBuffer;
//   orderTableBuffer = new GLubyte[81*4];

  for (int i = 0; i < 81; ++i)
	  for (int j = 0; j < 4; ++j)
		  orderTableBuffer[i*4 + j] = (GLfloat)(order_table[i][j]/4.0);
// 		  orderTableBuffer[i*4 + j] = (GLubyte)order_table[i][j];

  /// Generate orderTable texture
  glGenTextures(1, &orderTableTex);
  glActiveTexture(GL_TEXTURE4);
  glBindTexture(GL_TEXTURE_1D,  orderTableTex);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//   glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA8, 81, 0, GL_RGBA, GL_UNSIGNED_BYTE, orderTableBuffer);
  glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, 81, 0, GL_RGBA, GL_FLOAT, orderTableBuffer);

  delete [] orderTableBuffer;

  tfTexBuffer = new GLfloat[256*4];

  for (int i = 0; i < 256; ++i)
    {
      for (int j = 0; j < 4; ++j)
	tfTexBuffer[i*4 + j] = tf[i][j];
    } 

  //Generate Transfer Function texture
  glGenTextures(1, &tfTex);
  glActiveTexture(GL_TEXTURE5);
  glBindTexture(GL_TEXTURE_1D, tfTex);
//   glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//   glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, 256, 0, GL_RGBA, GL_FLOAT, tfTexBuffer);

  //Generate Exponential texture

  // This defines the exponential 1d texture size
  /**
#define EXP_SIZE 512

  expTexSize = EXP_SIZE;

  GLfloat *expTexBuffer;
  expTexBuffer = new GLfloat[expTexSize];

  for (int u = 0; u < (int)expTexSize; ++u)
    expTexBuffer[u] = (GLfloat)exp( -u / (GLfloat)(expTexSize - 1) );

  cout << "*** Exponential Texture Size : " << setw(10) << expTexSize << " ***" << endl;

  glGenTextures(1, &expTex);
  glActiveTexture(GL_TEXTURE6);
  glBindTexture(GL_TEXTURE_1D, expTex);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
//   glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//   glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//   glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//   glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA, EXP_SIZE, 0, GL_ALPHA, GL_FLOAT, expTexBuffer);

  delete [] expTexBuffer;
  **/
  //Generate PsiGammaTable texture

  // This header includes the psiGammaTable matrix variable for the texture,
  // the matrix is a local variable that is cleaned in the end of this function.

#include "../psiGammaTable512.h"
    
  preIntTexSize = PSI_GAMMA_SIZE_FRONT; // always 2D quad texture
    
  cout << "*** psiGammaTex Texture Size : " << setw(10) << preIntTexSize << " ***" << endl;

  glGenTextures(1, &psiGammaTableTex);
  glActiveTexture(GL_TEXTURE7);
  glBindTexture(TEX_FORMAT, psiGammaTableTex);
//   glTexParameteri(TEX_FORMAT, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//   glTexParameteri(TEX_FORMAT, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(TEX_FORMAT, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(TEX_FORMAT, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexImage2D(TEX_FORMAT, 0, GL_RGBA, preIntTexSize, preIntTexSize,
    	       0, GL_ALPHA, GL_FLOAT, psiGammaTable);

//   glTexParameteri(TEX_FORMAT, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//   glTexParameteri(TEX_FORMAT, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//   glTexParameteri(TEX_FORMAT, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
//   glTexParameteri(TEX_FORMAT, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

//   glTexParameteri(TEX_FORMAT, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
//   glTexParameteri(TEX_FORMAT, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  //  glEnable(TEX_FORMAT);

  errcheck("texCreation");
#else

  preIntTexSize = PSI_GAMMA_SIZE_FRONT; // always 2D quad texture

  if (debug_cout)
    cout << "*** psiGamma Table Size      : " << setw(10) << preIntTexSize << " ***" << endl;

#endif
}

/// Find maximum edge length inside the volume

void volume::findMaxEdgeLength(void) {

	GLfloat len;

	maxEdgeLength = 0.0;

	for (uint i = 0; i < numTets; ++i) { // for each tet

		uint vertId[4] = { (uint)tetrahedralBuffer[i*4 + 0],
				   (uint)tetrahedralBuffer[i*4 + 1], 
				   (uint)tetrahedralBuffer[i*4 + 2],
				   (uint)tetrahedralBuffer[i*4 + 3] };

		GLfloat vertex[4][3] = { { positionBuffer[ vertId[0]*4 + 0 ],
					   positionBuffer[ vertId[0]*4 + 1 ],
					   positionBuffer[ vertId[0]*4 + 2 ] },
					 { positionBuffer[ vertId[1]*4 + 0 ],
					   positionBuffer[ vertId[1]*4 + 1 ],
					   positionBuffer[ vertId[1]*4 + 2 ] },
					 { positionBuffer[ vertId[2]*4 + 0 ],
					   positionBuffer[ vertId[2]*4 + 1 ],
					   positionBuffer[ vertId[2]*4 + 2 ] },
					 { positionBuffer[ vertId[3]*4 + 0 ],
					   positionBuffer[ vertId[3]*4 + 1 ],
					   positionBuffer[ vertId[3]*4 + 2 ] } };
			
		GLfloat edge[6][3] = { { (vertex[0][0] - vertex[1][0]),
					 (vertex[0][1] - vertex[1][1]),
					 (vertex[0][2] - vertex[1][2]) },
				       { (vertex[0][0] - vertex[2][0]),
					 (vertex[0][1] - vertex[2][1]),
					 (vertex[0][2] - vertex[2][2]) },
				       { (vertex[0][0] - vertex[3][0]),
					 (vertex[0][1] - vertex[3][1]),
					 (vertex[0][2] - vertex[3][2]) },
				       { (vertex[1][0] - vertex[2][0]),
					 (vertex[1][1] - vertex[2][1]),
					 (vertex[1][2] - vertex[2][2]) },
				       { (vertex[1][0] - vertex[3][0]),
					 (vertex[1][1] - vertex[3][1]),
					 (vertex[1][2] - vertex[3][2]) },
				       { (vertex[2][0] - vertex[3][0]),
					 (vertex[2][1] - vertex[3][1]),
					 (vertex[2][2] - vertex[3][2]) } };

		for (uint j = 0; j < 6; ++j) { // for each edge

			len = sqrt( edge[j][0] * edge[j][0] +
				    edge[j][1] * edge[j][1] +
				    edge[j][2] * edge[j][2] );

			if (len > maxEdgeLength)
				maxEdgeLength = len;
		}

	}

}

/// Create Textures
/// Input and Output Textures

void volume::CreateTextures()
{
  if (debug_cout) {
    cout << "*********************************************" << endl;
    cout << "*** Number of Vertex         : " << setw(10) << numVerts << " ***" << endl;
    cout << "*** Number of Tetrahedrals   : " << setw(10) << numTets  << " ***" << endl;
  }   

  cellSorted = new pairTet[numTets];

  for (uint i = 0; i < numTets; ++i) {

    cellSorted[i].id = i;
    cellSorted[i].cZ = 0.0;

  }

  centroidBucket = new uint*[NUM_BUCKETS];
  countBucket = new uint[NUM_BUCKETS];

  for (uint i = 0; i < NUM_BUCKETS; ++i) {

    centroidBucket[i] = new uint[numTets/NUM_BUCKETS];
    centroidBucket[i][0] = 0;
    countBucket[i] = 0;

  }

  CreateOutputTextures();
  CreateInputTextures();

  CreateShaders();
  CreateArrays();

  reloadTetTex();

// #ifndef NO_NVIDIA

//   delete [] tetrahedralBuffer;
//   delete [] positionBuffer;

// #endif
}

///  Creates the glsl shaders.
///  * First
///     * Vertex Shader: simply apply only ProjectionMatrix to the vertex
///                      so we not rotate the vertex of the textured quad.
///     * Fragment Shader: compute intersection vertex; Scalars front
///                        and back; Thickness; IndexReorder; Order id.
///  * Second
///     * Vertex Shader: simply switch between w = 0 for the intersection
///                      vertex (already ModelViewProjected) and w = 1 for
///                      all others (doing the ModelViewProjection).
///     * Fragment Shader: look up the transfer function color by the scalar
///                        and compute the exponential alpha

void volume::CreateShaders()
{
#ifndef NO_NVIDIA
  /************ Install Shaders ******************************/
    
  // 1st Shaders
    
  shaders_1st = new glslKernel();
  assert (glsl_support()); // need to verify if has glsl
  shaders_1st->vertex_source("vert_1st.shader"); // 1st vertex shader is equal
  shaders_1st->fragment_source("frag_1st.shader");
  shaders_1st->install(debug_shaders);

  // 2nd Shaders
    
  shaders_2nd_with_int = new glslKernel();
  shaders_2nd_with_int->vertex_source("vert_2nd.shader"); // 2nd vertex shader is equal
  shaders_2nd_with_int->fragment_source("frag_2nd_with_int.shader");
  shaders_2nd_with_int->install(debug_shaders);
  shaders_2nd_no_int = new glslKernel();
  shaders_2nd_no_int->vertex_source("vert_2nd.shader"); // 2nd vertex shader is equal
  shaders_2nd_no_int->fragment_source("frag_2nd_no_int.shader");
  shaders_2nd_no_int->install(debug_shaders);

  if (integrating) shaders_2nd = shaders_2nd_with_int;
  else shaders_2nd = shaders_2nd_no_int;

  /************ Create framebuffer object *********************/
    
  glGenFramebuffersEXT(1, &frameBuffer);
  errcheck("genFrameBuffer");
    
  // Bind the framebuffer, so operations will now occur on it
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, frameBuffer);
  errcheck("bind FBO");
    
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(TEX_FORMAT, tetOutputTex0);
  glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, TEX_FORMAT, tetOutputTex0, 0);
    
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(TEX_FORMAT, tetOutputTex1);
  glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT1_EXT, TEX_FORMAT, tetOutputTex1, 0);
    
  errcheck("texture attach to FBO");
    
  // Unbind the frambuffer object, so subsequent drawing ops are not drawn into the FBO.
  // '0' means "windowing system provided framebuffer
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    
  errcheck("fboCreation");

  /************* Pass Uniform Variables ************************/

  shaders_1st->use();
  shaders_1st->set_uniform("tetrahedralTex", 2);
  shaders_1st->set_uniform("vertexPosTex", 3);
  shaders_1st->set_uniform("orderTableTex", 4);
  shaders_1st->set_uniform("vertTexSize", (GLfloat)vertTexSize);
  shaders_1st->use(0);

  shaders_2nd = shaders_2nd_with_int;
  shaders_2nd->use();
  shaders_2nd->set_uniform("tfTex", 5);
  //shaders_2nd->set_uniform("expTex", 6);
  shaders_2nd->set_uniform("psiGammaTableTex", 7);
  shaders_2nd->set_uniform("preIntTexSize", (GLfloat)preIntTexSize);
  shaders_2nd->set_uniform("maxEdgeLength", maxEdgeLength);
  shaders_2nd->use(0);

  shaders_2nd = shaders_2nd_no_int;
  shaders_2nd->use();
  shaders_2nd->set_uniform("tfTex", 5);
  //shaders_2nd->set_uniform("expTex", 6);
  shaders_2nd->use(0);

#endif
}

/// Draw Quad
/// Set the texture for Fragment Shader execution

void volume::DrawQuad()
{
#ifndef NO_NVIDIA
  GLfloat minWinWidth = MINORTHOSIZE;
  GLfloat maxWinWidth = MAXORTHOSIZE;
    
  GLfloat minWinHeight = MINORTHOSIZE;
  GLfloat maxWinHeight = MAXORTHOSIZE;
    
  glViewport(0, 0, (GLsizei) tetTexSize, (GLsizei) tetTexSize);
    
  glBegin(GL_QUADS);
  glTexCoord2d(0.0, 0.0);
  glVertex2d(minWinWidth, minWinHeight);
    
  //glTexCoord2d(0.0, tetTexSize); // NV Rect
  glTexCoord2d(0.0, 1.0);
  glVertex2f(minWinWidth, maxWinHeight);
    
  //glTexCoord2d(tetTexSize, tetTexSize); // NV Rect
  glTexCoord2d(1.0, 1.0);
  glVertex2f(maxWinWidth, maxWinHeight);
    
  //glTexCoord2d(tetTexSize, 0.0); // NV Rect
  glTexCoord2d(1.0, 0.0);
  glVertex2f(maxWinWidth, minWinHeight);
  glEnd();

  glViewport(0, 0, (GLsizei) modelWinWidth, (GLsizei) modelWinHeight);
#endif
}

/// Compute Tetrahedral Classes
/// Set and run fragment shader to update for the new viewpoint

void volume::UpdateData()
{
#ifndef NO_NVIDIA
  /************ First Pass - Fragment Shader *************/
  /** One call per tetrahedral
   * Steps:
   * - Transform vertices positions to screen space
   * - Determine projection class
   * - Calculate coordinates of intersection vertex
   * - Map vertices to basis graph
   * - Determine depth at thick vertex
   */
  /*******************************************************/
    
  GLint currentDrawBuffer;
  glGetIntegerv(GL_DRAW_BUFFER, &currentDrawBuffer);
    
  GLenum colorBuffers[2] = {GL_COLOR_ATTACHMENT0_EXT, GL_COLOR_ATTACHMENT1_EXT};
    
  // Bind the framebuffer, rendering will now occur on it
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, frameBuffer);	     
  glDrawBuffers(2, colorBuffers);

  if (!shaders_1st) return;

  shaders_1st->use();

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); 	 	  	  
    
  glEnable(TEX_FORMAT);
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(TEX_FORMAT, tetrahedralTex);
    
  DrawQuad();
  glFinish();
    
  static int dtBegin=0, dtEnd=0;
  dtBegin = glutGet(GLUT_ELAPSED_TIME);

  //capture fragment shader's output information
  glReadBuffer(GL_COLOR_ATTACHMENT0_EXT);
  glReadPixels(0, 0, tetTexSize, tetTexSize, GL_RGBA, GL_FLOAT, outputBuffer0);
  glReadBuffer(GL_COLOR_ATTACHMENT1_EXT);
  glReadPixels(0, 0, tetTexSize, tetTexSize, GL_RGBA, GL_FLOAT, outputBuffer1);

  dtEnd = glutGet(GLUT_ELAPSED_TIME);
  cout << "read pixels = " << (dtEnd - dtBegin) << " ms" << endl;

//   for (uint i=0; i < tetTexSize*tetTexSize; i += 4) {
//     outputBuffer1[i+0] = 0.5;
//     outputBuffer1[i+1] = 0.5;
//     outputBuffer1[i+2] = 0.5;
//     outputBuffer1[i+3] = 5;
//   }
    
  shaders_1st->use(0);
    
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
  glDrawBuffer(currentDrawBuffer);
  glDisable(TEX_FORMAT);
  glBindTexture(TEX_FORMAT, 0);
    
#else

  compute_1st_shader_on_cpu();

  errcheck("first fragment shader on cpu");

#endif
}

/// Setup Arrays
/// Fill (Intersection Vertex) and reorder vertex and color arrays

void volume::SetupArrays(bool useCentroid)
{
  GLuint id_order = 0, tetId = 0;
  GLuint vecArrayId = 0, vecIndicesId = 0;
  GLuint count_tfan = 0;
  GLuint curr_bucket = 0;

  //------------ debug -----------------

//   int countfans[numTets], idorders[numTets];
//   GLfloat sfs[numTets], sbs[numTets], thicks[numTets];

//   for (uint i=0; i<numTets; ++i) {
//     idorders[i] = 0;
//     countfans[i] = 0;
//     sfs[i] = 0.0;
//     sbs[i] = 0.0;
//     thicks[i] = 0.0;
//   }

  //------------------------------------

  for(uint i = 0; i < curTets; ++i)
    {
      if (sorting) {
	if (useCentroid) //centroid sorting
	  {
	    tetId = cellSorted[i].id;
	  }
	else //bucket sorting
	  {
	    while (countBucket[curr_bucket] > 0)
	      curr_bucket ++;
	    
	    tetId = centroidBucket[curr_bucket][countBucket[curr_bucket]];
	    countBucket[curr_bucket]--;
	  }
      }
      else
	tetId = i; // no sorting

      //vecIndicesId is the index of the indices array (*5 to skip 5 positions (vertices) per tetrahedron)
      vecIndicesId = tetId * 5;
	
      //vecArrayId is the index of the vertex and color array (*4 to skip four positions per vertex (xyzw coordinates))
      vecArrayId = vecIndicesId * 4;

      //retrieve the table row number and the 'count' of the triangle fan from the fragment buffer
      id_order = (int)outputBuffer0[tetId*4 + 3];
      count_tfan = (int)outputBuffer1[tetId*4 + 3];
 
      //update array indices and count for the first vertex/color of the
      //tetrahedron (the thick vertex)
      //All other vertices of tetrahedron are already in the arrays, only
      //the drawing order is redefined
      //If order == -1 -> thick vertex is the intersection vertex
      if (count_tfan == 6)
	{
	  //update first vertex of tetrahedron as intersection vertex
	  vertexArray[vecArrayId + 0] = outputBuffer0[tetId*4];
	  vertexArray[vecArrayId + 1] = outputBuffer0[tetId*4 + 1];
	  vertexArray[vecArrayId + 2] = 0.0;
	  vertexArray[vecArrayId + 3] = 0.0;
	}
      //else -> thick vertex is order[0]
      else
	{
	  //uses order[0] as the first vertex of the triangle fan
	  //order[0]+1 because in vertexArray the position 0 is the thick vertex, so
	  //the v0 of the tetrahedron is the second vertex in the array
	  for(uint j = 0; j < 3; j++)
	    {
	      vertexArray[vecArrayId + j] =
		vertexArray[vecArrayId + (1+triangle_fan_order_table[id_order][0]) * 4 + j];
	    }
	  vertexArray[vecArrayId + 3] = 1.0;
	}

      //updates the thick vertex color ( Sf, Sb, thickness )
      for(uint j = 0; j < 3; j++)
	colorArray[tetId*5*3 + j] = outputBuffer1[tetId*4 + j];

      //------------ debug -----------------

//       cout << outputBuffer0[tetId*4 + 0] << " " <<
// 	outputBuffer0[tetId*4 + 1] << " " <<
// 	outputBuffer0[tetId*4 + 2] << " " <<
// 	outputBuffer0[tetId*4 + 3] << endl;

//       cout << outputBuffer1[tetId*4 + 0] << " " <<
// 	outputBuffer1[tetId*4 + 1] << " " <<
// 	outputBuffer1[tetId*4 + 2] << " " <<
// 	outputBuffer1[tetId*4 + 3] << endl;

//       idorders[tetId] = id_order;
//       countfans[tetId] = count_tfan;
//       sfs[tetId] = colorArray[vecArrayId + 1];
//       sbs[tetId] = colorArray[vecArrayId + 2];
//       thicks[tetId] = colorArray[vecArrayId + 3];

      //       for(uint j = 0; j < 4; ++j)
      // 	colorArray[vecArrayId + j*4] = outputBuffer1[tetId*4];

      //------------------------------------

      //multiply thickness of the thick vertex by brightness      
      //colorArray[vecArrayId + 3] *= tf.getBrightness();

      //first vertex of the triangle fan is always the thick vertex
      //(the first vertex of the tetrahedron represented in the vertex array)

      indices[i][0] = vecIndicesId;

      //for Class 2 : count includes the center of the triangle fan (intersection vertex)
      //plus the four vertices of the tetrahedron, the sixth is to close the fan
      //for Classes 1, 3 and 4 : the thick vertex is a vertex of the tetrahedron, so we need one vertex
      //less from the above condition

      count[i] = count_tfan;
	
      //updates the order of the indices 
      for (uint j = 1; j < count_tfan; ++j)
	{
	  //remember first vertex of vecArray is the thick vertex, so
	  //must add 1 to index to get others
	  //v0 of tetrahedron is vecIndicesId + 1 and so on ...

	  indices[i][j] = (vecIndicesId + 1) + triangle_fan_order_table[id_order][j];
	}

    }

//       cout << endl;
//       cout << tetId << " : t : " << colorArray[vecArrayId + 3] << endl;
//       cout << "id_order : " << id_order << endl;
//       cout << "count_tfan : " << count_tfan << endl;
//       cout << "thick vertex : " << outputBuffer0[tetId*4] << " " << outputBuffer0[tetId*4 + 1] << endl;
//       cout << "             : " << vertexArray[vecArrayId + 0] << " " << vertexArray[vecArrayId + 1] << endl;


  //   for (uint i = 0; i < numTets; ++i) {
  //     cout << "Thick - Tet(" << i << ") : id_order = " << idorders[i] << " ; count = " << countfans[i]
  // 	 << " ; sf = " << sfs[i] << " ; sb = " << sbs[i] << " ; l = " << thicks[i] << endl;
  //   }
  //   cout << endl;

}


/// Draw Arrays
/// Use only one OpenGL function (glMultiDrawElement) to
/// draw all vertex and colors stored on vertexArray and
/// colorArray

void volume::Draw()
{
  glEnable(GL_CULL_FACE);

  glEnable(GL_BLEND);

  //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);

  /************ Second Pass - Vertex Shader **************/
  /** For each vertex verifies w component:
   * 0 - Intersection Vertex, change w = 1 and not
   *     multiply by ModelviewProjection Matrix;
   * 1 - Normal Vertex, multiply by ModelviewProjection
   *     Matrix.
   */
  /*******************************************************/
  glClear(GL_COLOR_BUFFER_BIT);

#ifndef NO_NVIDIA

  if (integrating) shaders_2nd = shaders_2nd_with_int;
  else shaders_2nd = shaders_2nd_no_int;

  shaders_2nd->use();

  glMultiDrawElements(GL_TRIANGLE_FAN, count, GL_UNSIGNED_INT,
		      (const GLvoid**)ids, curTets);

  shaders_2nd->use(0);

#else

  compute_2nd_shader_on_cpu();

  errcheck("second fragment shader on cpu");

#endif

  glDisable(GL_BLEND);

  glDisable(GL_CULL_FACE);
}

/// Normalize
/// Put all vertex between [-1, 1]

void volume::Normalize()
{
  GLfloat min[4], max[4];
  GLfloat scaleCoord, scaleScalar, maxCoord;
  GLfloat vert, center[3];
  
  for (int i = 0; i < 4; ++i)
    {
      min[i] = positionBuffer[i];
      max[i] = positionBuffer[i];
    }
    
  for(uint i = 1; i < numVerts; ++i)
    {
      for (int k = 0; k < 4; ++k)
	{
	  vert = positionBuffer[i*4 + k];
	  if(min[k] > vert) min[k] = vert;
	  if(max[k] < vert) max[k] = vert;
	}
    }
    
  for (int i = 0; i < 3; ++i)
    {
      center[i] = (GLfloat)( (min[i] + max[i]) / 2.0 );
      max[i] -= center[i];
    }
     
  maxCoord = (max[1] > max[2]) ? max[1] : max[2];
  maxCoord = (max[0] > maxCoord) ? max[0] : maxCoord;
  scaleCoord = (GLfloat)( 1.0 / maxCoord );
  scaleScalar = (GLfloat)( 1.0 / (max[3] - min[3]) );
  
  for(uint i = 0; i < numVerts; ++i)
    {
      for (int k = 0; k < 3; ++k)
	positionBuffer[i*4 + k] = (positionBuffer[i*4 + k] - center[k]) * scaleCoord;
      positionBuffer[i*4 + 3] = (positionBuffer[i*4 + 3] - min[3]) * scaleScalar;
    }
}

/// Add Vertex

void volume::addVertex(GLfloat x, GLfloat y, GLfloat z, GLfloat s)
{
  static uint posBufferSize = 0;

  positionBuffer[posBufferSize + 0] = x;
  positionBuffer[posBufferSize + 1] = y;
  positionBuffer[posBufferSize + 2] = z;
  positionBuffer[posBufferSize + 3] = s;

  posBufferSize += 4;
}

/// Add Cell

void volume::addCell(uint idxV0, uint idxV1, uint idxV2, uint idxV3)
{
  static uint tetBufferSize = 0;
  
  tetrahedralBuffer[tetBufferSize + 0] = (GLfloat)idxV0;
  tetrahedralBuffer[tetBufferSize + 1] = (GLfloat)idxV1;
  tetrahedralBuffer[tetBufferSize + 2] = (GLfloat)idxV2;
  tetrahedralBuffer[tetBufferSize + 3] = (GLfloat)idxV3;

  tetBufferSize += 4;
}

/*
 * Simple centroid sorting
 */
void volume::centroidSorting(void)
{
  if (!sorting) return;
  for (uint i = 0; i < curTets; ++i)
    {
      cellSorted[i].id = i;
      cellSorted[i].cZ = outputBuffer0[i*4 + 2];
      //  cout << i << " : " << cellSorted[i].cZ << endl;
    }
  //  cout << endl;

  stable_sort( cellSorted, cellSorted+curTets, lessCentroid ); // stl stable sort
}

/*
 * Simple bucket sorting:
 * divide the normalized model in NUM_BUCKETS layers,
 * put each tetrahedron in the layer that matches its centroid Z coordinate
 * render the buckets in back-to-front order
 * note: inside the buckets the tetrahedra remain unsorted
 */
void volume::bucketSorting(void)
{
  if (!sorting) return;

  uint sizeBucket = 0, bucket = 0;

  for (uint i = 0; i < curTets; ++i)
    countBucket[i] = 0;

  for (uint i = 0; i < curTets; ++i)
    {
      bucket = (int)(outputBuffer0[i*4 + 2] + 1.0) * (int)(NUM_BUCKETS * 0.5);
      centroidBucket[bucket][countBucket[bucket]] = i;
      sizeBucket++;
      countBucket[bucket] = sizeBucket;
    }
    /*
  if (!sorting) return;
  uint bucket = 0;
  for (uint i = 0; i <= NUM_BUCKETS; ++i)
    centroidBucket[i].clear();

  double min = outputBuffer0[2];
  double max = outputBuffer0[2];
  for (uint i = 0; i < curTets; ++i)
    {	  
      double cz = outputBuffer0[i*4 + 2];
      if (min > cz) min = cz;
      if (max < cz) max = cz;
    }

  double rng = max - min;
  int offset = (int)(NUM_BUCKETS);
  for (uint i = 0; i < curTets; ++i)
    {	  
      bucket = (int)(((outputBuffer0[i*4 + 2] - min) / rng) * offset);
      if (bucket > NUM_BUCKETS)
	bucket = NUM_BUCKETS;
      if (bucket < 0)
	bucket = 0;

      centroidBucket[bucket].push_back(i);
    }
  */
}
