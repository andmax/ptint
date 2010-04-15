/// GLSL CODE

/// 2nd Fragment Shader * NO INTEGRATION *

/// Role of Fragment Shader on the 2nd step:

uniform sampler1D tfTex;
//uniform sampler1D expTex;
varying vec3 gradFront;
varying vec3 gradBack;
uniform float brightness;
uniform float max_thickness;

void main(void)
{
  vec3 lightDir = vec3(1.0, 1.0, 0.0);
  float ka = 0.25;
  float kd = 0.2;
  float ks = 0.005;
  float specExp = 1.0;
  vec4 whiteLight = vec4(0.1);

  vec3 normal = (gradFront + gradBack) * 0.5;

  vec3 R = normalize(-reflect(lightDir, normal));

  float diffuseLight = kd* max(dot(normal, lightDir), 0.0) + ka;
  float specularLight = ks*max(pow(dot(normal, R),specExp), 0.0);

  //  float gradLen = sqrt(dot(normal, normal));

  //----- Input data ( -, sf, sb, thickness ) -----
  float sf = gl_Color.g;
  float sb = gl_Color.b;
  float l = gl_Color.a; //thickness

  l /= max_thickness;

  l *= brightness;

  if (l == 0.0) // no fragment color
    discard;
    
  float s_avg = (sf + sb) * 0.5; // average scalar
   

  vec4 c_avg = texture1D(tfTex, s_avg).rgba; // average color

  if (c_avg.a == 0.0) // no fragment color
    discard;

  //c_avg.a = 1.0 - texture1D(expTex, l*c_avg.a).a; // exp alpha
  c_avg.a = l*c_avg.a;
  //c_avg.a = l*c_avg.a; // exp alpha
  c_avg.rgb *= c_avg.a;


  //----- Output color -----
  //  gl_FragColor = c_avg * diffuseLight + whiteLight * specularLight;
  gl_FragColor = c_avg;// + whiteLight * specularLight;
}
