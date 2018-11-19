
varying vec4 colour;
varying vec2 uv;

uniform sampler2D texture;

vec4 getColour()
{
    return texture2D( texture, uv ) * colour;
}
