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

/// Static local functions

static fileType read_args(int& argc, char**& argv, uint* dim);

/// Read arguments

fileType read_args(int& argc, char**& argv, uint* dim)
{
  if (strcmp(argv[1], "test1") == 0)
    {	
      argv[1] = "../../tet_offs/test1.grad.off";
      argv[2] = "../../tet_offs/test.tf";
      return GRADOFF;
    }
  else if (strcmp(argv[1], "blunt") == 0)
    {	
      argv[1] = "../../tet_offs/blunt.off";
      argv[2] = "../../tet_offs/blunt.off.tf";
      return OFF;
    }
  else if (strcmp(argv[1], "delta") == 0)
    {
      argv[1] = "../../tet_offs/delta.off";
      argv[2] = "../../tet_offs/delta.off.tf";
      return OFF;
    }
  else if (strcmp(argv[1], "oceanH") == 0)
    {	
      argv[1] = "../../tet_offs/oceanH.off";
      argv[2] = "../../tet_offs/oceanH.off.tf";
      return OFF;
    }
  else if (strcmp(argv[1], "oceanU") == 0)
    {	
      argv[1] = "../../tet_offs/oceanU.off";
      argv[2] = "../../tet_offs/oceanU.off.tf";
      return OFF;
    }
  else if (strcmp(argv[1], "oceanV") == 0)
    {	
      argv[1] = "../../tet_offs/oceanV.off";
      argv[2] = "../../tet_offs/oceanV.off.tf";
      return OFF;
    }
  else if (strcmp(argv[1], "comb") == 0)
    {
      argv[1] = "../../tet_offs/comb.off";
      argv[2] = "../../tet_offs/comb.off.tf";
      return OFF;
    }
  else if (strcmp(argv[1], "post") == 0)
    {
      argv[1] = "../../tet_offs/post.off";
      argv[2] = "../../tet_offs/post.off.tf";
      return OFF;
    }
  else if (strcmp(argv[1], "spx") == 0)
    {	
      argv[1] = "../../tet_offs/spx.off";
      argv[2] = "../../tet_offs/spx.off.tf";
      return OFF;
    }
  else if (strcmp(argv[1], "spx1") == 0)
    {	
      argv[1] = "../../tet_offs/spx1.off";
      argv[2] = "../../tet_offs/spx1.off.tf";
      return OFF;
    }
  else if (strcmp(argv[1], "spx2") == 0)
    {	
      argv[1] = "../../tet_offs/spx2.off";
      argv[2] = "../../tet_offs/spx2.off.tf";
      return OFF;
    }
   else if (strcmp(argv[1], "fuel") == 0)
    {
      argv[1] = "../../tet_offs/fuel.raw";
      argv[2] = "../../tet_offs/fuel.tf";
      dim[0] = 64; dim[1] = 64; dim[2] = 64;
      return SINGLERAW8;
    }
   else if (strcmp(argv[1], "sphere") == 0)
    {
      argv[1] = "../../tet_offs/sphere.raw";
      argv[2] = "../../tet_offs/sphere.tf";
      dim[0] = 16; dim[1] = 16; dim[2] = 16;
      return SINGLERAW8;
    }
   else if (strcmp(argv[1], "sphereGrad") == 0)
    {
      argv[1] = "../../tet_offs/sphere.grad.raw";
      argv[2] = "../../tet_offs/sphere.tf";
      dim[0] = 16; dim[1] = 16; dim[2] = 16;
      return SINGLEGRADRAW8;
    }
   else if (strcmp(argv[1], "cube") == 0)
    {
      argv[1] = "../../tet_offs/cube.raw";
      argv[2] = "../../tet_offs/cube.tf";
      dim[0] = 16; dim[1] = 16; dim[2] = 16;
      return SINGLERAW8;
    }
   else if (strcmp(argv[1], "cubeGrad") == 0)
    {
      argv[1] = "../../tet_offs/cube.grad.raw";
      argv[2] = "../../tet_offs/cube.tf";
      dim[0] = 16; dim[1] = 16; dim[2] = 16;
      return SINGLEGRADRAW8;
    }
  else if (strcmp(argv[1], "heart1") == 0)
    {
      argv[1] = "../../tet_offs/heart_sets/set1";
      argv[2] = "../../tet_offs/heart_sets/set1.tf";
      dim[0] = 256; dim[1] = 256; dim[2] = 20;
      return PNM8;
    }
  else if (strcmp(argv[1], "heart2") == 0)
    {
      argv[1] = "../../tet_offs/heart_sets/set2";
      argv[2] = "../../tet_offs/heart_sets/set2.tf";
      dim[0] = 256; dim[1] = 256; dim[2] = 20;
      return PNM8;
    }
  else if (strcmp(argv[1], "heart3") == 0)
    {
      argv[1] = "../../tet_offs/heart_sets/set3";
      argv[2] = "../../tet_offs/heart_sets/set3.tf";
      dim[0] = 256; dim[1] = 256; dim[2] = 20;
      return PNM8;
    }
  else if (strcmp(argv[1], "head") == 0)
    {
      argv[1] = "../../tet_offs/MRI/MRI_Head_256_256_111.bin";
      argv[2] = "../../tet_offs/MRI/MRI_Head_256_256_111.bin.tf";
      dim[0] = 256; dim[1] = 256; dim[2] = 111;
      return BIN8;
    }
  else if (strcmp(argv[1], "foot") == 0)
    {
      argv[1] = "../../tet_offs/VOLVIS/foot_256_256_256.bin";
      argv[2] = "../../tet_offs/VOLVIS/foot_256_256_256.bin.tf";
      dim[0] = 256; dim[1] = 256; dim[2] = 256;
      return BIN8;
    }
  else if (strcmp(argv[1], "tooth") == 0)
    {
      argv[1] = "../../tet_offs/tooth/tooth";
      argv[2] = "../../tet_offs/tooth/tooth.tf";
      dim[0] = 256; dim[1] = 256; dim[2] = 161;
      return MULTIRAW16BE;
    }
  else if (strcmp(argv[1], "toothGrad") == 0)
    {
      argv[1] = "../../tet_offs/tooth/tooth.grad";
      argv[2] = "../../tet_offs/tooth/tooth.tf";
      dim[0] = 256; dim[1] = 256; dim[2] = 161;
      return MULTIRAW16BE;
    }
  else if (strcmp(argv[1], "knee") == 0)
    {
      argv[1] = "../../tet_offs/knee/knee";
      argv[2] = "../../tet_offs/knee/knee.tf";
      dim[0] = 512; dim[1] = 512; dim[2] = 87;
      return MULTIRAW16BE;
    }
   else if (strcmp(argv[1], "skull") == 0)
    {
      argv[1] = "../../tet_offs/CT/CT_Skull_256_256_113.bin";
      argv[2] = "../../tet_offs/CT/CT_Skull_256_256_113.bin.tf";
      dim[0] = 256; dim[1] = 256; dim[2] = 113;
      return BIN8;
    }
   else if (strcmp(argv[1], "brain") == 0)
    {
      argv[1] = "../../tet_offs/CT/ctbrain_512_256_512.bin";
      argv[2] = "../../tet_offs/CT/ctbrain_512_256_512.bin.tf";
      dim[0] = 512; dim[1] = 256; dim[2] = 512;
      return BIN8;
    }
   else if (strcmp(argv[1], "neghip") == 0)
    {
      argv[1] = "../../tet_offs/neghip.raw";
      argv[2] = "../../tet_offs/neghip.tf";
      dim[0] = 64; dim[1] = 64; dim[2] = 64;
      return SINGLERAW8;
    }
   else if (strcmp(argv[1], "shockwave") == 0)
    {
      argv[1] = "../../tet_offs/shockwave.raw";
      argv[2] = "../../tet_offs/shockwave.tf";
      dim[0] = 64; dim[1] = 64; dim[2] = 300;
      return SINGLERAW8;
    }
   else if (strcmp(argv[1], "silicium") == 0)
    {
      argv[1] = "../../tet_offs/silicium.raw";
      argv[2] = "../../tet_offs/silicium.tf";
      dim[0] = 98; dim[1] = 64; dim[2] = 64;
      return SINGLERAW8;
    }
   else if (strcmp(argv[1], "hydrogen") == 0)
    {
      argv[1] = "../../tet_offs/hydrogenAtom.raw";
      argv[2] = "../../tet_offs/hydrogenAtom.tf";
      dim[0] = 128; dim[1] = 128; dim[2] = 128;
      return SINGLERAW8;
    }
   else if (strcmp(argv[1], "dti") == 0)
    {
      argv[1] = "../../tet_offs/DTI-MD.raw";
      argv[2] = "../../tet_offs/DTI-MD.tf";
      dim[0] = 128; dim[1] = 128; dim[2] = 58;
      return SINGLERAW8;
    }
   else if (strcmp(argv[1], "dti2") == 0)
    {
      argv[1] = "../../tet_offs/DTI-BO.raw";
      argv[2] = "../../tet_offs/DTI-BO.tf";
      dim[0] = 128; dim[1] = 128; dim[2] = 58;
      return SINGLERAW16;
    }
  else if (strcmp(argv[1], "salt") == 0)
    {
      argv[1] = "../../tet_offs/salt.off";
      argv[2] = "../../tet_offs/salt.off.tf";
      return GEO;
    }
  else if (strcmp(argv[1], "spxGrad") == 0)
    {	
      argv[1] = "../../tet_offs/spx.grad.off";
      argv[2] = "../../tet_offs/spx.off.tf";
      return GRADOFF;
    }
  else if (strcmp(argv[1], "bluntGrad") == 0)
    {	
      argv[1] = "../../tet_offs/blunt.grad.off";
      argv[2] = "../../tet_offs/blunt.off.tf";
      return GRADOFF;
    }
  else if (strcmp(argv[1], "combGrad") == 0)
    {	
      argv[1] = "../../tet_offs/comb.grad.off";
      argv[2] = "../../tet_offs/comb.off.tf";
      return GRADOFF;
    }
  else if (strcmp(argv[1], "postGrad") == 0)
    {	
      argv[1] = "../../tet_offs/post.grad.off";
      argv[2] = "../../tet_offs/post.off.tf";
      return GRADOFF;
    }
  else if (strcmp(argv[1], "spx2Grad") == 0)
    {	
      argv[1] = "../../tet_offs/spx2.grad.off";
      argv[2] = "../../tet_offs/spx2.off.tf";
      return GRADOFF;
    }
   else if (strcmp(argv[1], "fuelGrad") == 0)
    {
      argv[1] = "../../tet_offs/fuel.grad.raw";
      argv[2] = "../../tet_offs/fuel.tf";
      dim[0] = 64; dim[1] = 64; dim[2] = 64;
      return SINGLEGRADRAW8;
    }
   else if (strcmp(argv[1], "neghipGrad") == 0)
    {
      vol->tf.updateBrightness(0.0);
      argv[1] = "../../tet_offs/neghip.grad.raw";
      argv[2] = "../../tet_offs/neghip.tf";
      dim[0] = 64; dim[1] = 64; dim[2] = 64;
      return SINGLEGRADRAW8;
    }
  else if (strcmp(argv[1], "footGrad") == 0)
    {
      argv[1] = "../../tet_offs/VOLVIS/foot_256_256_256.grad.bin";
      argv[2] = "../../tet_offs/VOLVIS/foot_256_256_256.bin.tf";
      dim[0] = 256; dim[1] = 256; dim[2] = 256;
      return BIN8;
    }
   else if (strcmp(argv[1], "hydrogenGrad") == 0)
    {
      argv[1] = "../../tet_offs/hydrogenAtom.grad.raw";
      argv[2] = "../../tet_offs/hydrogenAtom.tf";
      dim[0] = 128; dim[1] = 128; dim[2] = 128;
      return SINGLERAW8;
    }
  else if (argc < 3)
    {
      cout << endl
	   << "Usage: " << argv[0] << " <filename.off> <filename.tf> [extended option]  or" << endl
	   << "       " << argv[0] << " [valid model]" << endl
	   << "You can enter in [valid model] one of the following:" << endl << endl
	   << "  OFF  models : blunt   delta   oceanH comb       post      spx      spx2" << endl
	   << "  RAW 16 bits : tooth   knee    dti2" << endl
	   << "  RAW  8 bits : fuel    neghip  skull  shockwave  silicium  hydrogen bluntFin dti" << endl << endl
	   << "  BIN  8 bits : brain   skull   foot   head  " << endl
	   << "  PNM  images : heart1  heart2  heart3" << endl
	   << "  GEO datsets : salt    skull" << endl << endl
	   << "And in [extended option] you can enter: " << endl
	   << "  -t : to generate a text file in a 2 min execution" << endl << endl;
      exit(0);
    }
  return OFF;
}
