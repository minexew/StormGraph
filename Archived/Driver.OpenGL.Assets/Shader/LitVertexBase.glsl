
uniform vec3 lightPos[Sg_numLights];
uniform mat4 lightTransform[Sg_numLights];

varying vec3 N, V, n;

/*-- StormGraph Shader Body --*/

void main()
{
    // Set up some varyings for lighting
    V = vec3( gl_ModelViewMatrix * gl_Vertex );
    N = normalize( gl_NormalMatrix * gl_Normal );
    n = gl_Normal;

    // Transform vertex & UV
    gl_Position = gl_ModelViewProjectionMatrix * getVertex();
}