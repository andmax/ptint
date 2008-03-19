/**
 *   Second Step Vertex Shader
 *
 * Maximo, Andre -- March, 2008
 *
 */

/**
 *   secondStep.vert : in this step the vertices of the volume are drawn, the thin
 *                     vertices has w = 1.0, while the thick vertices has w = 0.0,
 *                     which is used to apply ModelViewProjection only to the thin
 *                     vertices.
 *
 * GLSL code.
 *
 */

void main(void) {

	if (gl_Vertex.w == 0.0) gl_Position = vec4(gl_Vertex.xyz, 1.0);

	if (gl_Vertex.w == 1.0) gl_Position = ftransform();

	gl_FrontColor = gl_Color;

}
