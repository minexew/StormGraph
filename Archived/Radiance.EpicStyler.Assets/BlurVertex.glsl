
varying vec2 uv;

vec4 getVertex()
{
    uv = vec2( gl_TextureMatrix[0] * gl_MultiTexCoord0 );

	return gl_Vertex;
}