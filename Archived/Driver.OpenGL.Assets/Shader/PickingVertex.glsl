
attribute vec4 vertexColour;

varying vec4 colour;

vec4 getVertex()
{
	colour = vertexColour;

	return gl_Vertex;
}