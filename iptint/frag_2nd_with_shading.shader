/// GLSL CODE

/// 2nd Fragment Shader * WITH INTEGRATION *

/// Role of Fragment Shader on the 2nd step:

uniform sampler1D tfTex;
uniform sampler1D expTex;
uniform float preIntTexSize;
uniform sampler2D psiGammaTableTex;

uniform float ks;
uniform float kd;
uniform float ka;
uniform float alphai;
uniform vec3 rho;
uniform vec3 brightnessIC;
uniform float brightness;
uniform float max_thickness;

varying vec3 gradFront;
varying vec3 gradBack;

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

  /*   if (l == 0.0) // no fragment color */
  /*     discard; */

  /*   if (l == 0.0) */
  /*     l = 1.0; */

  vec4 colorFront = texture1D(tfTex, sf).rgba;
  vec4 colorBack = texture1D(tfTex, sb).rgba;

  vec4 color;

  vec2 tau = vec2(colorFront.a, colorBack.a) * (l / max_thickness);
  vec2 halfVec = vec2(0.5);
  float zeta = texture1D(expTex, dot(tau, halfVec)).a; // using tex1d for exponential
  //float zeta = exp(-1.0 * (dot(tau, halfVec))); // computing exponential

  /*   if (zeta == 1.0) // no fragment color  */
  /*     discard; */

  vec2 gamma = tau / (1.0 + tau);
  float psi = texture2D(psiGammaTableTex, gamma + (halfVec / vec2(preIntTexSize))).a;

  color.rgb = colorFront.rgb*(1.0 - psi) + colorBack.rgb*(psi - zeta);
  color.a = 1.0 - zeta;

  //  float avgScalar = (sf + sb) * 0.5;
  //using scalars as threshold instead of gradient length

  /*   if (((rho[1] >= sb) && (rho[1] <= sf)) || */
  /*       ((rho[1] >= sf) && (rho[1] <= sb))) */
  //  float gradLen = dot (normal,normal);
  /*   float avgScalar = max(sf, sb); */
  /*   if ((avgScalar > rho[1]) && (avgScalar <= rho[1] + rho[0])) */

  float smin = min(sf, sb);
  float smax = max(sf, sb);

  /*   vec3 gradMax = max(gradFront, gradBack); */
  /*   vec3 gradMin = max(gradFront, gradBack); */

  int surface = 0;

  vec3 normal;
  //float delta = 0.05;
  //float delta = 0.005;
  //float delta = 0.001;
  float delta = 0.0;

  float alpha = alphai;

/*   gradFront *= color.a; */
/*   gradBack *= color.a; */

  for (int i = 0; i < 3; ++i)
    {
      if (brightnessIC[i] > 0.0) {
	if ((smax >= rho[i] - delta) && (smin <= rho[i] + delta))
	  {
	    sf = smax;
	    sb = smin;
	    
	    //if (smax >= rho[i])
	    sf = rho[i];
	    //if (smin <= rho[i])
	    sb = rho[i];
	    
	    //	    l = gl_Color.a * ((sf - sb) / (smax - smin));
	    l = (gl_Color.a/max_thickness) * ((sf - sb) / (smax - smin));

	    l *= alpha * brightnessIC[i];
	    surface = i+1;
	    
	    float lf, lb;
	    lf = abs(smax - rho[i]);
	    lb = abs(smin - rho[i]);
	    float ratio = lb / (lf + lb);
	    normal = gradFront*ratio + gradBack*(1.0 - ratio);
	    alpha *= brightnessIC[i];
	  }
      }
    }

  if (surface > 0)
    {
      colorFront = texture1D(tfTex, sf).rgba;
      colorBack = texture1D(tfTex, sb).rgba;

      if (surface == 3)
	{
	  color = colorFront;
	  //color.rgb = abs(normalize(normal));
	  color.a *= alpha + (l/max_thickness);;
	}
      else
	{
	  color = colorFront;// + colorBack) * 0.5;
	  color.a *= alpha;
	  color.rgb *= color.a;
	}


      normal = normalize(normal);

      vec3 lightDir = vec3(1.0, 0.0, -1.0);
      lightDir = normalize(lightDir);

      vec3 halfVector = -reflect(lightDir, normal);

      float specExp = 30.0;
      float NdotHV = abs(dot(normal, normalize(halfVector)));
      //float NdotHV = max(dot(normal, normalize(halfVector)), 0.0);
      vec3 specularLight = color.rgb * ks * pow(NdotHV, specExp);

      float NdotL = abs(dot(normal, lightDir));
      //float NdotL = max(dot(normal, lightDir), 0.0);
      vec3 diffuseLight = color.rgb * kd * NdotL;

      vec3 ambientLight = color.rgb * ka;
      
      color.rgb = diffuseLight.rgb + specularLight.rgb + ambientLight;
    }
/*     else */
/*       discard; */

  //----- Output color -----
  gl_FragColor = color;
}
