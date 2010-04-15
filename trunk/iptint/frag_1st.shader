/// GLSL CODE

/// 1st Fragment Shader *** With Sorting ***

/// Role of Fragment Shader on the 1st step:
/// - On the first step (update step) the fragments are
///   vertex indices of each tetrahedral. With this ids
///   the vertex position and color are loaded, the
///   Projection Tetrahedra algorithm is computed, and
///   the Framebuffer is updated with the thick vertex
///   position, color, thickness and PT classification.
///   * With sorting returns the Z coordinate of the
///   tetrahedra centroid.

#extension GL_ARB_draw_buffers : enable

uniform sampler2D tetrahedralTex;
uniform sampler2D vertexPosTex;
uniform sampler2D gradientTex;

uniform sampler1D orderTableTex;

uniform float vertTexSize;

vec3 vert_proj[4]; // vertex position in homogenous clip-space (screen space)
vec3 vert_order[4]; //vertices in right order (basis graph)
float scalar_order[4]; // color in right order (basis graph)
float scalar_orig[4]; // vertex original color
ivec4 tests; // four classification tests
float paramU1, paramU2; // line intersection parameters
vec3 grad_orig[4]; //vertices gradients
vec3 grad_order[4];

void vertex_data_retrieval(out float cZ)
{
  cZ = 0.0;
  vec4 ids = texture2D(tetrahedralTex, gl_TexCoord[0].st).xyzw;

  for (int i = 0; i < 4; i++)
    {
      //      vec2 texParam = vec2(mod(ids[i],vertTexSize), floor(ids[i]/vertTexSize));

      vec2 texParam = vec2((mod(ids[i],vertTexSize)+0.5)/vertTexSize, (floor(ids[i]/vertTexSize)+0.5)/vertTexSize);
      vec4 texInfo = texture2D(vertexPosTex, texParam).xyzw;
      grad_orig[i] = texture2D(gradientTex, texParam).xyz;

      scalar_orig[i] = texInfo.w;

      vec4 v = vec4(texInfo.xyz, 1.0);
      vert_proj[i].z = (gl_ModelViewMatrix * v).z; 
      cZ += vert_proj[i].z;
      vert_proj[i].xy = (gl_ModelViewProjectionMatrix * v).xy;
    }
  cZ *= 0.25;
}

float my_cross(vec3 a, vec3 b)
{
  return (a.x * b.y) - (a.y * b.x);
}

void compute_crosses(out float c1, out float c2, out float c3, out float c4)
{
  vec3 vc1_0 = vert_proj[1] - vert_proj[0];
  vec3 vc2_0 = vert_proj[2] - vert_proj[0];
  vec3 vc3_0 = vert_proj[3] - vert_proj[0];
  vec3 vc1_2 = vert_proj[1] - vert_proj[2];
  vec3 vc1_3 = vert_proj[1] - vert_proj[3];
    
  c1 = my_cross(vc1_0, vc2_0);
  c2 = my_cross(vc1_0, vc3_0);
  c3 = my_cross(vc2_0, vc3_0);
  c4 = my_cross(vc1_2, vc1_3);
}

void pt_classification(out int id, inout int cvtx)
{
  vec4 cross;
  
  int num_crosses0 = 0;

  compute_crosses(cross[0], cross[1], cross[2], cross[3]);

  for (int i = 0; i < 4; i++) {
    tests[i] = int(sign(cross[i])) + 1;
    if (tests[i] == 1)
      num_crosses0++;
  }

  if (num_crosses0 > 2)
    discard;
 
  if (num_crosses0 == 2)
    {cvtx = 3;}
  if (num_crosses0 == 1)
    {cvtx = 4;}
  if (num_crosses0 == 0)
    {cvtx = 5;}
 
  id = tests.x * 27 + tests.y * 9 + tests.z * 3 + tests.w * 1;
}

void order_vertices(in int id)
{
  //----------- read texture info and place it in var tableRow -----------
  ivec4 tableRow = ivec4(  texture1D(orderTableTex, float(id)/80.0).xyzw * 4.0 + 0.5);

  for(int i = 0; i < 4; i++)
    {
      if (tableRow[i] == 0)
	{
	  vert_order[i] = vert_proj[0];
	  scalar_order[i] = scalar_orig[0];
	  grad_order[i] = grad_orig[0];
	}
      if (tableRow[i] == 1)
	{
	  vert_order[i] = vert_proj[1];
	  scalar_order[i] = scalar_orig[1];
	  grad_order[i] = grad_orig[1];
	}
      if (tableRow[i] == 2)
	{
	  vert_order[i] = vert_proj[2];
	  scalar_order[i] = scalar_orig[2];
	  grad_order[i] = grad_orig[2];
	}
      if (tableRow[i] == 3)
	{
	  vert_order[i] = vert_proj[3];
	  scalar_order[i] = scalar_orig[3];
	  grad_order[i] = grad_orig[3];
	}
    }
}

void compute_params(in int cvtx)
{
  float denominator, numeratorU1, numeratorU2;

  if (cvtx == 3)
    {
      paramU1 = 1.0;
      paramU2 = 1.0;
    }
  if (cvtx == 4)
    {
      //line intersection denominator between v0->v2 and v1->v3
      denominator = ((vert_order[3].y - vert_order[1].y) * (vert_order[2].x - vert_order[0].x)) -
	((vert_order[3].x - vert_order[1].x) * (vert_order[2].y - vert_order[0].y));

      //line defined by vector v1->v3
      numeratorU2 = ((vert_order[2].x - vert_order[0].x) * (vert_order[0].y - vert_order[1].y)) -
	((vert_order[2].y - vert_order[0].y) * (vert_order[0].x - vert_order[1].x));

      paramU1 = 1.0;
      paramU2 = numeratorU2 / denominator;
    }
  if (cvtx == 5)
    {
      //line intersection denominator between v0->v2 and v1->v3
      denominator = ((vert_order[3].y - vert_order[1].y) * (vert_order[2].x - vert_order[0].x)) -
	((vert_order[3].x - vert_order[1].x) * (vert_order[2].y - vert_order[0].y));
	
      //line defined by vector v0->v2
      numeratorU1 = ((vert_order[3].x - vert_order[1].x) * (vert_order[0].y - vert_order[1].y)) -
	((vert_order[3].y - vert_order[1].y) * (vert_order[0].x - vert_order[1].x));
	
      //line defined by vector v1->v3
      numeratorU2 = ((vert_order[2].x - vert_order[0].x) * (vert_order[0].y - vert_order[1].y)) -
	((vert_order[2].y - vert_order[0].y) * (vert_order[0].x - vert_order[1].x));
	
      paramU1 = numeratorU1 / denominator;
      paramU2 = numeratorU2 / denominator;
    }  
}

void compute_intersection(out vec3 ipoint, out float t, in int cvtx)
{
  t = 0.0;
  ipoint = vec3(0.0);
  if (cvtx == 3)
    {
      t = (vert_order[0].z - vert_order[1].z);
    }
  if (cvtx == 4)
    {
      float zBackIntersection = vert_order[1].z + paramU2*(vert_order[3].z - vert_order[1].z);
      t = (vert_order[2].z - zBackIntersection);
    }
  if (cvtx == 5)
    {
      //find z coordinate of back intersection point by interpolating original vertices (not projected)
      float zBackIntersection = vert_order[1].z + paramU2*(vert_order[3].z - vert_order[1].z);
	
      //find ordered intersection point between the two ordered lines (basis graph)
      ipoint = (vert_order[0] + paramU1*(vert_order[2] - vert_order[0]));
	
      t = (ipoint.z - zBackIntersection);
    }
}

void compute_scalars(out float sf, out float sb, in int cvtx)
{
  sf = 0.0;
  sb = 0.0;
  if (cvtx == 6)
    {      
      //Interpolate scalar values the same manner as done for intersection vertex
      sf = scalar_order[0] + paramU1*(scalar_order[2] - scalar_order[0]);
      sb = scalar_order[1] + paramU2*(scalar_order[3] - scalar_order[1]);
    }
  if (cvtx == 5)
    {
      sf = scalar_order[2];
      float sb_tmp = scalar_order[1] + paramU2*(scalar_order[3] - scalar_order[1]);
      sb = scalar_order[0] + paramU1*(sb_tmp - scalar_order[0]);
    }
  if (cvtx == 4)
    {
      sf = scalar_order[2];
      sb = scalar_order[1] + paramU2*(scalar_order[3] - scalar_order[1]);
    }
  if (cvtx == 3)
    {
      sf = scalar_order[0];
      sb = scalar_order[1];
    }
}

void compute_grad(out vec3 gradFront, out vec3 gradBack, in int cvtx)
{
  gradFront = vec3(0.0);
  gradBack = vec3(0.0);

  if (cvtx == 6)
    {
      gradFront = grad_order[0] + paramU1*(grad_order[2] - grad_order[0]);
      gradBack = grad_order[1] + paramU2*(grad_order[3] - grad_order[1]);
    }
  if (cvtx == 5)
    {
      gradFront = grad_order[2];
      vec3 grad_tmp = grad_order[1] + paramU2*(grad_order[3] - grad_order[1]);
      gradBack = grad_order[0] + (grad_tmp - grad_order[0])*paramU1;
    }
  if (cvtx == 4)
    {
      gradFront = grad_order[2];
      gradBack = grad_order[1] + paramU2*(grad_order[3] - grad_order[1]);
    }
  if (cvtx == 3)
    {
      gradFront = grad_order[0];
      gradBack = grad_order[1];
    }
}


void main(void)
{
  //------------------------ Output Variables definition --------------------------
  int id_order = 0; //Id of the order table
  vec3 intersectionPoint = vec3(0.0); //Intersection point (already projected)
  float thickness = 0.0; //Distance between intersection point (projected and original)
    
  float centroidZ; // centroid of the tetrahedron
  int count_tfan = 3; // triangle fan count
    
  float scalar_front = 0.0, scalar_back = 0.0; //Scalar of front and back vertex	
  
  vec3 gradFront = vec3(0.0, 0.0, 0.0);
  vec3 gradBack = vec3(0.0, 0.0, 0.0);

  for (int i = 0; i < 4; i++)
    {
      vert_order[i] = vec3(0.0);
      scalar_order[i] = 0.0;
      grad_order[i] = vec3(0.0);
    }
  paramU1 = 1.0;
  paramU2 = 1.0;

  vertex_data_retrieval(centroidZ);

  pt_classification(id_order, count_tfan);
    
  order_vertices(id_order);

  compute_params(count_tfan);

  compute_intersection(intersectionPoint, thickness, count_tfan);

  //if Class 1 then paramU1 is greater than 1.0 (middle vertex inside projected triangle)
  //in this case the thickness value must be multiplied by the
  //ratio r=(|V0V4|/|V0I|), that is, r=1/paramU1,
  //where V0V4 is the distance from V0 to the middle vertex
  //and V0I is the distance from V0 to the intersection point

  if (paramU1 >= 1.0)
    {
      if (count_tfan == 5) {
	thickness /= paramU1;
	paramU1 = 1.0 / paramU1;
      }
    }
  else // count == 6 ; thick vertex == intersection vertex
    {
      count_tfan = 6;
    }

  compute_scalars(scalar_front, scalar_back, count_tfan);
  compute_grad(gradFront, gradBack, count_tfan);

  //vec3 gradThick = (gradFront + gradBack) * 0.5;

  if (thickness < 0.0)
    {
      float tmp = scalar_back;
      scalar_back = scalar_front;
      scalar_front = tmp;
    }

  gradFront += vec3(1.0);
  gradFront *= 0.5;

  gradBack += vec3(1.0);
  gradBack *= 0.5;


  //------- Output Multiple Render Targets (MRT) to Frame Buffer Objects (FBO) -------
  gl_FragData[0] = vec4 (intersectionPoint.xy, centroidZ, id_order);
  gl_FragData[1] = vec4 (scalar_front, scalar_back, abs(thickness), count_tfan);
  gl_FragData[2] = vec4 (gradFront, 1.0);
  gl_FragData[3] = vec4 (gradBack, 1.0);
}
