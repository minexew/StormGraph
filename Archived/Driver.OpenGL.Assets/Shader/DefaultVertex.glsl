
attribute vec4 vertexColour;

varying vec4 colour;
varying vec2 uv;

vec4 getVertex()
{
	colour = vertexColour;
    uv = vec2( gl_TextureMatrix[0] * gl_MultiTexCoord0 );

	return gl_Vertex;
}