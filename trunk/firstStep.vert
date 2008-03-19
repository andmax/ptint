/**
 *   First Step Vertex Shader
 *
 * Maximo, Andre -- March, 2008
 *
 */

/**
 *   firstStep.vert : before the first step a quad textured with tetrahedron
 *                    texture is drawn, it can't be rotated/scaled/translated so
 *                    we'll apply only the projection matrix to the input vertex
 *
 * GLSL code.
 *
 */

void main(void) {

	gl_TexCoord[0] = gl_MultiTexCoord0;

	gl_Position = gl_ProjectionMatrix * gl_Vertex;

}
