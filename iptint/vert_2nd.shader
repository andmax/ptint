/// GLSL CODE

/// 2nd Vertex Shader

/// Role of Vertex Shader on the 2nd step:
/// - On the second step (draw step) the vertices of the
///   volume are drawn. The thin (non-thick) vertex has
///   the homogeneous coordinate equals one (w = 1.0). And
///   the thick vertex has w = 0.0. This difference allow us
///   to apply the ModeviewProjection matrix only to the
///   thin vertex, because the thick vertex is already
///   ModelviewProjected (on the prior step).

varying vec3 gradFront;
varying vec3 gradBack;

void main(void)
{
  vec4 outVert;
  if (gl_Vertex.w == 0.0)
    outVert = vec4(gl_Vertex.xyz, 1.0);
  else
    outVert = gl_ModelViewProjectionMatrix * gl_Vertex;

  vec3 normal = gl_Normal.rgb;
  normal = (normal * 2.0) - vec3(1.0);
  vec3 n = gl_NormalMatrix * vec3(normal.xyz);
  gradFront = n.rgb;
  
  normal = gl_SecondaryColor.rgb;
  normal = (normal * 2.0) - vec3(1.0);
  n = gl_NormalMatrix * vec3(normal.xyz);
  gradBack = n.rgb;

  gl_Position = outVert;
  gl_FrontColor = gl_Color;
  gl_BackColor = vec4(0.0);
}
