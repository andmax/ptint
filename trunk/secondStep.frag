/**
 *   Second Step Fragment Shader
 *
 *  Authors:
 *    Maximo, Andre
 *    Marroquim, Ricardo
 *
 *  Date: Jan-May, 2006
 *
 */

/**
 *   secondStep.frag :
 *     [1] Determine the colors for scalar front and back (2 access);
 *     [2] Compute psi gamma table parameters;
 *     [3] Evaluate opacity;
 *     [4] Read psi (1 access);
 *     [5] Paint the fragment.
 *
 * GLSL code.
 *
 */

uniform sampler1D tfTex;
uniform float preIntTexSize;
uniform sampler2D psiGammaTableTex;

uniform float maxEdgeLength;

/// Exponential Function

float exp(float x) {

	return ( pow(2.7182, x) ); // # number # e = 2.7182

}

void main(void)
{
    //----- Input data ( sf, sb, thickness ) -----
    float sf = gl_Color.r;
    float sb = gl_Color.g;
    float l = gl_Color.b; //thickness

    if (l == 0.0) // no fragment color
	discard;

    l = l / maxEdgeLength;

    vec4 colorFront = texture1D(tfTex, sf).rgba;
    vec4 colorBack = texture1D(tfTex, sb).rgba;

    vec4 color;

    vec2 tau = vec2(colorFront.a, colorBack.a) * l;

    vec2 halfVec = vec2(0.5);

    //float zeta = texture1D(expTex, dot(tau, halfVec)).a; // using tex1d for exponential

    float zeta = exp( -dot(tau, halfVec) );

    if (zeta == 1.0) // no fragment color
	discard;

    vec2 gamma = tau / (1.0 + tau);

    float psi = texture2D(psiGammaTableTex, gamma + (halfVec / vec2(preIntTexSize))).a;

    color.rgb = colorFront.rgb*(1.0 - psi) + colorBack.rgb*(psi - zeta);
    color.a = 1.0 - zeta;

    //----- Output color -----
    gl_FragColor = color;
}
