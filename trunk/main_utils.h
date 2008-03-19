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
static void show_error_msg(char**& argv);

void show_error_msg(char**& argv) {

	cerr << endl
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

}

/// Read arguments

fileType read_args(int& argc, char**& argv, uint* dim)
{
  if (strcmp(argv[1], "blunt") == 0)
    {	
      vol->tf.updateBrightness(9.0);
      argv[1] = "../datasets/blunt.off";
      argv[2] = "../datasets/blunt.off.tf";
      return OFF;
    }
  else if (strcmp(argv[1], "delta") == 0)
    {
      argv[1] = "../datasets/delta.off";
      argv[2] = "../datasets/delta.off.tf";
      return OFF;
    }
  else if (strcmp(argv[1], "oceanH") == 0)
    {	
      vol->tf.updateBrightness(9.0);
      argv[1] = "../datasets/oceanH.off";
      argv[2] = "../datasets/oceanH.off.tf";
      return OFF;
    }
  else if (strcmp(argv[1], "oceanU") == 0)
    {	
      argv[1] = "../datasets/oceanU.off";
      argv[2] = "../datasets/oceanU.off.tf";
      return OFF;
    }
  else if (strcmp(argv[1], "oceanV") == 0)
    {	
      argv[1] = "../datasets/oceanV.off";
      argv[2] = "../datasets/oceanV.off.tf";
      return OFF;
    }
  else if (strcmp(argv[1], "comb") == 0)
    {
      vol->tf.updateBrightness(7.0);
      argv[1] = "../datasets/comb.off";
      argv[2] = "../datasets/comb.off.tf";
      return OFF;
    }
  else if (strcmp(argv[1], "post") == 0)
    {
      vol->tf.updateBrightness(4.0);
      argv[1] = "../datasets/post.off";
      argv[2] = "../datasets/post.off.tf";
      return OFF;
    }
  else if (strcmp(argv[1], "spx") == 0)
    {	
      argv[1] = "../datasets/spx.off";
      argv[2] = "../datasets/spx.off.tf";
      return OFF;
    }
  else if (strcmp(argv[1], "spx1") == 0)
    {	
      argv[1] = "../datasets/spx1.off";
      argv[2] = "../datasets/spx1.off.tf";
      return OFF;
    }
  else if (strcmp(argv[1], "spx2") == 0)
    {	
      vol->tf.updateBrightness(2.0);
      argv[1] = "../datasets/spx2.off";
      argv[2] = "../datasets/spx2.off.tf";
      return OFF;
    }
   else if (strcmp(argv[1], "fuel") == 0)
    {
      argv[1] = "../datasets/fuel.raw";
      argv[2] = "../datasets/fuel.tf";
      dim[0] = 64; dim[1] = 64; dim[2] = 64;
      return SINGLERAW8;
    }
  else if (strcmp(argv[1], "heart1") == 0)
    {
      vol->tf.updateBrightness(5.0);
      argv[1] = "../datasets/heart_sets/set1";
      argv[2] = "../datasets/heart_sets/set1.tf";
      dim[0] = 256; dim[1] = 256; dim[2] = 20;
      return PNM8;
    }
  else if (strcmp(argv[1], "heart2") == 0)
    {
      vol->tf.updateBrightness(5.0);
      argv[1] = "../datasets/heart_sets/set2";
      argv[2] = "../datasets/heart_sets/set2.tf";
      dim[0] = 256; dim[1] = 256; dim[2] = 20;
      return PNM8;
    }
  else if (strcmp(argv[1], "heart3") == 0)
    {
      vol->tf.updateBrightness(5.0);
      argv[1] = "../datasets/heart_sets/set3";
      argv[2] = "../datasets/heart_sets/set3.tf";
      dim[0] = 256; dim[1] = 256; dim[2] = 20;
      return PNM8;
    }
  else if (strcmp(argv[1], "head") == 0)
    {
      vol->tf.updateBrightness(5.0);
      argv[1] = "../datasets/MRI/MRI_Head_256_256_111.bin";
      argv[2] = "../datasets/MRI/MRI_Head_256_256_111.bin.tf";
      dim[0] = 256; dim[1] = 256; dim[2] = 111;
      return BIN8;
    }
  else if (strcmp(argv[1], "foot") == 0)
    {
      vol->tf.updateBrightness(5.0);
      argv[1] = "../datasets/VOLVIS/foot_256_256_256.bin";
      argv[2] = "../datasets/VOLVIS/foot_256_256_256.bin.tf";
      dim[0] = 256; dim[1] = 256; dim[2] = 256;
      return BIN8;
    }
  else if (strcmp(argv[1], "tooth") == 0)
    {
      vol->tf.updateBrightness(5.0);
      argv[1] = "../datasets/tooth/tooth";
      argv[2] = "../datasets/tooth/tooth.tf";
      dim[0] = 256; dim[1] = 256; dim[2] = 161;
      return MULTIRAW16BE;
    }
  else if (strcmp(argv[1], "knee") == 0)
    {
      vol->tf.updateBrightness(5.0);
      argv[1] = "../datasets/knee/knee";
      argv[2] = "../datasets/knee/knee.tf";
      dim[0] = 512; dim[1] = 512; dim[2] = 87;
      return MULTIRAW16BE;
    }
   else if (strcmp(argv[1], "skull") == 0)
    {
      argv[1] = "../datasets/CT/CT_Skull_256_256_113.bin";
      argv[2] = "../datasets/CT/CT_Skull_256_256_113.bin.tf";
      dim[0] = 256; dim[1] = 256; dim[2] = 113;
      return BIN8;
    }
   else if (strcmp(argv[1], "brain") == 0)
    {
      argv[1] = "../datasets/CT/ctbrain_512_256_512.bin";
      argv[2] = "../datasets/CT/ctbrain_512_256_512.bin.tf";
      dim[0] = 512; dim[1] = 256; dim[2] = 512;
      return BIN8;
    }
   else if (strcmp(argv[1], "neghip") == 0)
    {
      argv[1] = "../datasets/neghip.raw";
      argv[2] = "../datasets/neghip.tf";
      dim[0] = 64; dim[1] = 64; dim[2] = 64;
      return SINGLERAW8;
    }
   else if (strcmp(argv[1], "shockwave") == 0)
    {
      argv[1] = "../datasets/shockwave.raw";
      argv[2] = "../datasets/shockwave.tf";
      dim[0] = 64; dim[1] = 64; dim[2] = 300;
      return SINGLERAW8;
    }
   else if (strcmp(argv[1], "silicium") == 0)
    {
      argv[1] = "../datasets/silicium.raw";
      argv[2] = "../datasets/silicium.tf";
      dim[0] = 98; dim[1] = 64; dim[2] = 64;
      return SINGLERAW8;
    }
   else if (strcmp(argv[1], "hydrogen") == 0)
    {
      argv[1] = "../datasets/hydrogenAtom.raw";
      argv[2] = "../datasets/hydrogenAtom.tf";
      dim[0] = 128; dim[1] = 128; dim[2] = 128;
      return SINGLERAW8;
    }
   else if (strcmp(argv[1], "dti") == 0)
    {
      vol->tf.updateBrightness(9.0);
      argv[1] = "../datasets/DTI-MD.raw";
      argv[2] = "../datasets/DTI-MD.tf";
      dim[0] = 128; dim[1] = 128; dim[2] = 58;
      return SINGLERAW8;
    }
   else if (strcmp(argv[1], "dti2") == 0)
    {
      argv[1] = "../datasets/DTI-BO.raw";
      argv[2] = "../datasets/DTI-BO.tf";
      dim[0] = 128; dim[1] = 128; dim[2] = 58;
      return SINGLERAW16;
    }
  else if (strcmp(argv[1], "salt") == 0)
    {
      argv[1] = "../datasets/salt.off";
      argv[2] = "../datasets/salt.off.tf";
      return GEO;
    }
  else if (argc < 3)
    {

      exit(0);
      show_error_msg(argv);

    }
  return OFF;
}
