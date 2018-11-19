
/*-- StormGraph Shader Body --*/

void main()
{
    gl_Position = gl_ModelViewProjectionMatrix * getVertex();
}