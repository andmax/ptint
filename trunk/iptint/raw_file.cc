/**
 *
 *    Render Volume GPU
 *
 *  File: raw_file.cc
 *
 *  Authors:
 *    Andre Maximo
 *    Ricardo Marroquim
 *
 *  Last Update: May 04, 2006
 *
 */

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

typedef unsigned int uint;
typedef unsigned short ushort;

static void generateTets(volume& obj, const uint rows, const uint cols, const uint z);
static bool readRaw(const char* fn, volume& obj, const uint rows, const uint cols, const uint ns);

void generateTets(volume& obj, const uint rows, const uint cols, const uint z)
{
  int idv[8];
  uint dimY =  cols - (CULL_OFFSET_Y_END + CULL_OFFSET_Y_BEGIN) / SAMPLE_OFFSET;
  uint dimX =  rows - (CULL_OFFSET_X_END + CULL_OFFSET_X_BEGIN) / SAMPLE_OFFSET;

  for (uint y = 0; y < dimY - SAMPLE_OFFSET; y+=SAMPLE_OFFSET) {
    for (uint x = 0; x < dimX - SAMPLE_OFFSET; x+=SAMPLE_OFFSET) {

	idv[0] =   x             +                 y*(dimX) +     z*(dimX*dimY);
	idv[1] = x+1             +                 y*(dimX) +     z*(dimX*dimY);
	idv[2] =   x             +             (y+1)*(dimX) +     z*(dimX*dimY);
	idv[3] = x+1             +             (y+1)*(dimX) +     z*(dimX*dimY);
	idv[4] =   x             +                 y*(dimX) + (z+1)*(dimX*dimY);
	idv[5] = x+1             +                 y*(dimX) + (z+1)*(dimX*dimY);
	idv[6] =   x             +             (y+1)*(dimX) + (z+1)*(dimX*dimY);
	idv[7] = x+1             +             (y+1)*(dimX) + (z+1)*(dimX*dimY);

	obj.addCell(idv[1], idv[5], idv[6], idv[7]);
	obj.addCell(idv[1], idv[2], idv[3], idv[6]);
	obj.addCell(idv[1], idv[3], idv[6], idv[7]);
	obj.addCell(idv[0], idv[1], idv[2], idv[6]);
	obj.addCell(idv[1], idv[0], idv[4], idv[6]);
	obj.addCell(idv[1], idv[6], idv[4], idv[5]);

    }
  }
}

bool readRaw(const char* fn, volume& obj,
	     const uint rows, const uint cols, const uint ns)
{
  uint dimZ = (ns)/SLICE_OFFSET;
  ushort data;
  string filename;
  ifstream slice_file;

  //create buffers
  uint dimX =  (rows - (CULL_OFFSET_X_END + CULL_OFFSET_X_BEGIN)) / SAMPLE_OFFSET;  
  uint dimY =  (cols - (CULL_OFFSET_Y_END + CULL_OFFSET_Y_BEGIN)) / SAMPLE_OFFSET;

  uint numTets = (dimX-1)*(dimY-1)*(dimZ-1)*6;
  uint numVerts = dimX*dimY*dimZ;
  obj.createBuffers(numTets, numVerts);

  for (uint z = 0; z < dimZ; ++z) {

    stringstream ssfn;
    ssfn << fn << "." << (z*SLICE_OFFSET + 1);
    filename = ssfn.str();

    slice_file.open(filename.c_str(), ios::binary);

    if(slice_file.fail())
      return false;

    for (uint y = 0; y < cols; ++y) {
      for (uint x = 0; x < rows; ++x) {
	char buf[2];
	slice_file.read(buf, 2);

	data = (ushort)((unsigned char)buf[0] + 256*(unsigned char)buf[1]);
	if ((x%SAMPLE_OFFSET == 0) && (y%SAMPLE_OFFSET == 0))
	  if ((x >= CULL_OFFSET_X_BEGIN) && (x <= rows - CULL_OFFSET_X_END - SAMPLE_OFFSET) &&
	      (y >= CULL_OFFSET_Y_BEGIN) && (y <= cols - CULL_OFFSET_Y_END - SAMPLE_OFFSET))
	    obj.addVertex((GLfloat)x, (GLfloat)y, (GLfloat)z*SLICE_OFFSET, (GLfloat)data);
      }
    }

    slice_file.close();
  }

  for (uint z = 0; z < dimZ - 1; ++z)
    generateTets(obj, rows, cols, z);

  obj.Normalize();

  //obj.compressScalars (0.8, 1.0);
    
  return true;
}

void generateTetsNoOffset(volume& obj, const uint rows, const uint cols, const uint z)
{
  int idv[8];
  uint dimY =  cols;
  uint dimX =  rows;

  for (uint y = 0; y < dimY - 1; ++y) {
    for (uint x = 0; x < dimX - 1; ++x) {

	idv[0] =   x             +                 y*(dimX) +     z*(dimX*dimY);
	idv[1] = x+1             +                 y*(dimX) +     z*(dimX*dimY);
	idv[2] =   x             +             (y+1)*(dimX) +     z*(dimX*dimY);
	idv[3] = x+1             +             (y+1)*(dimX) +     z*(dimX*dimY);
	idv[4] =   x             +                 y*(dimX) + (z+1)*(dimX*dimY);
	idv[5] = x+1             +                 y*(dimX) + (z+1)*(dimX*dimY);
	idv[6] =   x             +             (y+1)*(dimX) + (z+1)*(dimX*dimY);
	idv[7] = x+1             +             (y+1)*(dimX) + (z+1)*(dimX*dimY);
	

	bool badTet = true;
	for (int i = 0; i < 8; ++i)
	  if (obj.getScalar(idv[i]) > 0)
	    badTet = false;

	//if (!badTet)
	  {
	    obj.addCell(idv[1], idv[5], idv[6], idv[7]);
	    obj.addCell(idv[1], idv[2], idv[3], idv[6]);
	    obj.addCell(idv[1], idv[3], idv[6], idv[7]);
	    obj.addCell(idv[0], idv[1], idv[2], idv[6]);
	    obj.addCell(idv[1], idv[0], idv[4], idv[6]);
	    obj.addCell(idv[1], idv[6], idv[4], idv[5]);
	  }
// 	else 
// 	  cout << "discard!" << endl;

    }
  }
}

bool readRawSingleFile16bits(const char* fn, volume& obj,
	     const uint rows, const uint cols, const uint ns)
{
  uint dimZ = (ns)/SLICE_OFFSET;
  ushort data;
  string filename;
  ifstream slice_file;

  //create buffers
  uint dimX =  (rows - (CULL_OFFSET_X_END + CULL_OFFSET_X_BEGIN)) / SAMPLE_OFFSET;  
  uint dimY =  (cols - (CULL_OFFSET_Y_END + CULL_OFFSET_Y_BEGIN)) / SAMPLE_OFFSET;

  uint numTets = (dimX-1)*(dimY-1)*(dimZ-1)*6;
  uint numVerts = dimX*dimY*dimZ;
  obj.createBuffers(numTets, numVerts);

  stringstream ssfn;
  ssfn << fn;
  filename = ssfn.str();

  slice_file.open(filename.c_str(), ios::binary);

  if(slice_file.fail())
    return false;

  for (uint z = 0; z < dimZ; ++z) {
    for (uint y = 0; y < cols; ++y) {
      for (uint x = 0; x < rows; ++x) {
	char buf[2];
	slice_file.read(buf, 2);
	data = (ushort)((unsigned char)buf[0] + 256*(unsigned char)buf[1]);	
	
	if ((x%SAMPLE_OFFSET == 0) && (y%SAMPLE_OFFSET == 0))
	  if ((x >= CULL_OFFSET_X_BEGIN) && (x <= rows - CULL_OFFSET_X_END - SAMPLE_OFFSET) &&
	      (y >= CULL_OFFSET_Y_BEGIN) && (y <= cols - CULL_OFFSET_Y_END - SAMPLE_OFFSET))
	    obj.addVertex((GLfloat)x, (GLfloat)y, (GLfloat)z, (GLfloat)data);
      }
    }
    
  }
  slice_file.close();

  for (uint z = 0; z < dimZ - 1; ++z)
    generateTets(obj, rows, cols, z);

  obj.Normalize();

    
  return true;
}

bool readRawSingleFile8bits(const char* fn, volume& obj,
	     const uint rows, const uint cols, const uint ns)
{
  uint dimZ = (ns)/SLICE_OFFSET;
  uint data;
  string filename;
  ifstream slice_file;

  //create buffers
  uint dimX =  (rows - (CULL_OFFSET_X_END + CULL_OFFSET_X_BEGIN)) / SAMPLE_OFFSET;  
  uint dimY =  (cols - (CULL_OFFSET_Y_END + CULL_OFFSET_Y_BEGIN)) / SAMPLE_OFFSET;

  uint numTets = (dimX-1)*(dimY-1)*(dimZ-1)*6;
  uint numVerts = dimX*dimY*dimZ;
  obj.createBuffers(numTets, numVerts);

  stringstream ssfn;
  ssfn << fn;
  filename = ssfn.str();

  slice_file.open(filename.c_str(), ios::binary);

  if(slice_file.fail())
    return false;

  for (uint z = 0; z < dimZ; ++z) {
    for (uint y = 0; y < cols; ++y) {
      for (uint x = 0; x < rows; ++x) {
	char buf[1];
	slice_file.read(buf, 1);
	data = (uint)((unsigned char)buf[0]);
	
	if ((x%SAMPLE_OFFSET == 0) && (y%SAMPLE_OFFSET == 0))
	  if ((x >= CULL_OFFSET_X_BEGIN) && (x <= rows - CULL_OFFSET_X_END - SAMPLE_OFFSET) &&
	      (y >= CULL_OFFSET_Y_BEGIN) && (y <= cols - CULL_OFFSET_Y_END - SAMPLE_OFFSET))
	    obj.addVertex((GLfloat)x, (GLfloat)y, (GLfloat)z, (GLfloat)data);
      }
    }
    
  }
  slice_file.close();

  for (uint z = 0; z < dimZ - 1; ++z)
    generateTets(obj, rows, cols, z);

  obj.Normalize();

    
  return true;
}
