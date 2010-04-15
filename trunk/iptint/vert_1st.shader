/// GLSL CODE

/// 1st Vertex Shader

/// Role of Vertex Shader on the 1st step:
/// - On the first step (update step) a quad with a
///   texture map is drawn. This quad cannot be
///   rotated/scaled/translated, so we'll apply
///   only the projection matrix to the vertex.

void main(void)
{
	gl_TexCoord[0] = gl_MultiTexCoord0;
	gl_Position = gl_ProjectionMatrix * gl_Vertex;
}
