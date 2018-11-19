
uniform vec4 sceneAmbient;
uniform vec4 lightAmbient[Sg_numLights];
uniform vec4 lightDiffuse[Sg_numLights];
uniform vec4 lightEmissive[Sg_numLights];
uniform vec3 lightPos[Sg_numLights];
uniform float lightRange[Sg_numLights];
uniform vec4 lightSpecular[Sg_numLights];
uniform mat4 lightTransform[Sg_numLights];
uniform int lightType[Sg_numLights];
uniform vec4 materialAmbient;
uniform vec4 materialDiffuse;
uniform float materialShininess;
uniform vec4 materialSpecular;

varying vec3 N, V,n;

/*-- StormGraph Shader Body --*/

void main()
{ 
    // Lighting, lighting, lighting...
    vec3 L;
    float attenuation, specFact = 1.0;

    vec4 amb, diff, spec, lightSum;

    lightSum = vec4( 0.0, 0.0, 0.0, 0.0 );

    for ( int i = 0; i < Sg_numLights; i++ )
    {
        // Light = off
        if ( lightType[i] == 0 )
            continue;
        // Directional Light
        else if ( lightType[i] == 1 )
        {
            L = normalize( vec3( lightTransform[i] * vec4( -lightPos[i], 0.0 ) ) );
            attenuation = 1.0;
            specFact = 0.0;
        }
        // Point Light
        else if ( lightType[i] == 2 )
        {
            vec3 lightDir = vec3( lightTransform[i] * vec4( 0.0, 0.0, 0.0, 1.0 ) ) - V;

            L = normalize( lightDir );
            attenuation = lightRange[i] / ( 2.0 * length( lightDir ) );
        }
        // Line Light
        else if ( lightType[i] == 3 )
        {
            vec3 begin = vec3( lightTransform[i] * vec4( 0.0, 0.0, 0.0, 1.0 ) );
            vec3 len = vec3( lightTransform[i] * vec4( lightPos[i], 0.0 ) );

            vec3 lightDir;

            /*if ( length( len ) == 0.0 )
                lightDir = begin - V;
            else*/
            {
                float t = clamp( dot( V - begin, len ) / pow( length( len ), 2.0 ), 0.0, 1.0 );
                lightDir = begin + t * len - V;
            }

            L = normalize( lightDir );
            attenuation = lightRange[i] / ( 2.0 * length( lightDir ) );
        }

        vec3 E = normalize( -V );
        vec3 R = normalize( -reflect( L, N ) );

        // -- Ambient --
        amb = lightAmbient[i] * materialAmbient; 

        // -- Diffuse --
        //diff = clamp( lightDiffuse[i] * dot( N, L ), 0.0, 1.0 ) * materialDiffuse;
        diff = max( lightDiffuse[i] * dot( N, L ), 0.0 ) * materialDiffuse;

        // -- Specular --
        if ( materialShininess > 0.01f )
            spec = lightSpecular[i] * pow( max( dot( R, E ), 0.0 ), 0.3 * materialShininess );
        else
            spec = lightSpecular[i];

        //spec = clamp( spec, 0.0, 1.0 ) * materialSpecular;
        spec = spec * materialSpecular * specFact;

        // Scene ambient lighting is the only one not affected by attenuation
        lightSum += ( diff + spec + amb ) * attenuation;
    }

    gl_FragColor = clamp( vec4( ( lightSum + sceneAmbient ).rgb, 1.0 ) * getColour(), 0.0, 1.0 );
}
