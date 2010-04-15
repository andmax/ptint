/// GLSL CODE

/// 2nd Fragment Shader * WITH INTEGRATION *

/// Role of Fragment Shader on the 2nd step:

uniform sampler1D tfTex;
uniform sampler1D expTex;
uniform float preIntTexSize;
uniform sampler2D psiGammaTableTex;
uniform float brightness;
uniform float max_thickness;

varying vec3 normal;

float exp(float x)
{
  return pow(2.7182, x);
}

void main(void)
{
  //----- Input data ( -, sf, sb, thickness ) -----
  float sf = gl_Color.g;
  float sb = gl_Color.b;
  float l = gl_Color.a; //thickness

  l *= brightness;

  if (l == 0.0) // no fragment color
    discard;

  vec4 colorFront = texture1D(tfTex, sf).rgba;
  vec4 colorBack = texture1D(tfTex, sb).rgba;

  if (colorFront.a == 0.0)
    discard;

  vec4 color;

  vec2 tau = vec2(colorFront.a, colorBack.a) * (l/max_thickness);

  vec2 halfVec = vec2(0.5);

  float zeta = texture1D(expTex, dot(tau, halfVec)).a; // using tex1d for exponential

  //float zeta = exp(-1.0 * (dot(tau, halfVec))); // computing exponential

  if (zeta == 1.0) // no fragment color
    discard;

  vec2 gamma = tau / (1.0 + tau);

  float psi = texture2D(psiGammaTableTex, gamma + (halfVec / vec2(preIntTexSize))).a;

  color.rgb = colorFront.rgb*(1.0 - psi) + colorBack.rgb*(psi - zeta);
  color.a = 1.0 - zeta;

  //----- Output color -----
  gl_FragColor = color;
}
