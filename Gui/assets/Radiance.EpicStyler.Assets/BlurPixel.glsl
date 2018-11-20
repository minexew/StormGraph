
uniform float blurProgress;
uniform vec2 blurRadius;
varying vec2 uv;

uniform sampler2D texture;

vec4 getColour()
{
	vec4 sum = texture2D( texture, uv + vec2( -blurRadius.x, -blurRadius.y ) ) * 1.0 / 16.0;
	sum += texture2D( texture, uv + vec2( 0.0, -blurRadius.y ) ) * 2.0 / 16.0;
	sum += texture2D( texture, uv + vec2( blurRadius.x, -blurRadius.y ) ) * 1.0 / 16.0;
	sum += texture2D( texture, uv + vec2( -blurRadius.x, 0.0 ) ) * 2.0 / 16.0;
	sum += texture2D( texture, uv + vec2( 0.0, 0.0 ) ) * 4.0 / 16.0;
	sum += texture2D( texture, uv + vec2( blurRadius.x, 0.0 ) ) * 2.0 / 16.0;
	sum += texture2D( texture, uv + vec2( -blurRadius.x, blurRadius.y ) ) * 1.0 / 16.0;
	sum += texture2D( texture, uv + vec2( 0.0, blurRadius.y ) ) * 2.0 / 16.0;
	sum += texture2D( texture, uv + vec2( blurRadius.x, blurRadius.y ) ) * 1.0 / 16.0;

    float avg = dot( sum.rgb, vec3( 0.299, 0.587, 0.114 ) );
	return mix( sum, vec4( avg, avg, avg, sum.a ), blurProgress );
}
