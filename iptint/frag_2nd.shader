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

  vec4 colorFront = texture1D(tfTex, sf).rgba;
  vec4 colorBack = texture1D(tfTex, sb).rgba;

  vec4 color;

  vec2 tau = vec2(colorFront.a, colorBack.a) * l;
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
  vec3 rhoMax = vec3(rho[0], rho[1], rho[2]);
  
  for (int i = 0; i < 3; ++i)
    if ((smax > rho[i]) && (smin < rho[i]))
      {
	sf = smax;
	sb = smin;
	
	if (smax > rhoMax[i])
	  sf = rhoMax[i];
	if (smin < rho[i])
	  sb = rho[i];
	
	l = gl_Color.a * ((sf - sb) / (smax - smin));
	l *= alphai * brightnessIC[i];
	surface = i+1;

	float lf, lb;
	lf = abs(smax - rhoMax[i]);
	lb = abs(smin - rho[i]);
	float ratio = lb / (max(smax, rhoMax[i]) - min(smin, rho[i]));
	normal = gradFront*ratio + gradBack*(1.0 - ratio);
      }
/*     if (!((smax < rho[i]) || (smin > rhoMax[i]))) */
/*       { */
/* 	sf = smax; */
/* 	sb = smin;	 */
	
/* 	if (smax > rhoMax[i]) */
/* 	  sf = rhoMax[i]; */
/* 	if (smin < rho[i]) */
/* 	  sb = rho[i]; */
	
/* 	l = gl_Color.a * ((sf - sb) / (smax - smin)); */
/* 	l *= alphai * brightnessIC[i]; */
/* 	surface = i+1; */

/* 	float lf, lb; */
/* 	lf = abs(smax - rhoMax[i]); */
/* 	lb = abs(smin - rho[i]); */
/* 	float ratio = lb / (max(smax, rhoMax[i]) - min(smin, rho[i])); */
/* 	normal = gradFront*ratio + gradBack*(1.0 - ratio); */
/*       } */

  if (surface == 3)
    {
      colorFront = texture1D(tfTex, sf).rgba;
      colorBack = texture1D(tfTex, sb).rgba;

      if (surface == 3)
	{
	  //color = (colorFront + colorBack) * 0.5;
	  color.rgb = abs(normalize(normal));
	  color.a = 1.0;
	  // ka = 0.2; kd = 0.6;
	}
      else
	{
	  tau = vec2(colorFront.a, colorBack.a) * l;
	  zeta = texture1D(expTex, dot(tau, halfVec)).a; // using tex1d for exponential
	  gamma = tau / (1.0 + tau);
	  psi = texture2D(psiGammaTableTex, gamma + (halfVec / vec2(preIntTexSize))).a;
	  color.rgb = colorFront.rgb*(1.0 - psi) + colorBack.rgb*(psi - zeta);
	  color.a = 1.0 - zeta;
	}

      normal = normalize(normal);
      vec3 lightDir = vec3(1.0, -1.0, 1.0);

      vec3 halfVector = -reflect(lightDir, normal);

      float specExp = 50.0;
      float NdotHV = abs(dot(normal, normalize(halfVector)));
      //float NdotHV = max(dot(normal, normalize(halfVector)), 0.0);
      vec3 specularLight = color.rgb * ks * pow(NdotHV, specExp);

      float NdotL = abs(dot(normal, lightDir));
      //float NdotL = max(dot(normal, lightDir), 0.0);
      vec3 diffuseLight = color.rgb * kd * NdotL;

      vec3 ambientLight = color.rgb * ka;
      
      color.rgb = diffuseLight.rgb + specularLight.rgb + ambientLight;
    }
  /*   else */
  /*     discard; */

  //----- Output color -----
  gl_FragColor = color;
}
