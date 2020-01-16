/*
    Copyright (c) 2011 Xeatheran Minexew

    This software is provided 'as-is', without any express or implied
    warranty. In no event will the authors be held liable for any damages
    arising from the use of this software.

    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
    claim that you wrote the original software. If you use this software
    in a product, an acknowledgment in the product documentation would be
    appreciated but is not required.

    2. Altered source versions must be plainly marked as such, and must not be
    misrepresented as being the original software.

    3. This notice may not be removed or altered from any source
    distribution.
*/

#include "OpenGlDriver.hpp"

#include <StormGraph/Engine.hpp>

#include <littl/File.hpp>

namespace OpenGlDriver
{
    Shader::Shader() : shader( 0 )
    {
    }

    Shader::~Shader()
    {
        if ( shader )
            glApi.functions.glDeleteShader( shader );
    }

    void Shader::compile( const char* name, GLuint shaderType, const char* source )
    {
        shader = glApi.functions.glCreateShader( shaderType );
        glApi.functions.glShaderSource( shader, 1, &source, 0 );
        glApi.functions.glCompileShader( shader );

        int status = GL_FALSE;
        glApi.functions.glGetShaderiv( shader, GL_COMPILE_STATUS, &status );

        if ( status == GL_FALSE )
        {
            int logLength = 0;
            int charsWritten  = 0;
            Array<GLchar> log;

            glApi.functions.glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &logLength );

            log.resize( logLength + 1 );
            glApi.functions.glGetShaderInfoLog( shader, logLength, &charsWritten, log.getPtr() );

            Common::logEvent( "OpenGlDriver.Shader", ( String ) "Shader compilation failed.\nShader source:\n<pre>" + source + "</pre>" );

            throw StormGraph::Exception( "OpenGlDriver.Shader.compile", "ShaderCompileError",
                    ( String ) "Failed to compile shader `" + name + "`. Compilation log:\n\n" + String( log.getPtr() ) );
        }

        int error = glGetError();

        if ( error != GL_NO_ERROR )
            throw Exception( "OpenGlDriver.Shader.compile", "OpenGlError", ( String ) "OpenGL runtime error: " + error );
    }

    PixelShader::PixelShader( const char* name, const char* source )
    {
        compile( name, GL_FRAGMENT_SHADER, source );
    }

    PixelShader::~PixelShader()
    {
    }

    VertexShader::VertexShader( const char* name, const char* source )
    {
        compile( name, GL_VERTEX_SHADER, source );
    }

    VertexShader::~VertexShader()
    {
    }

    ShaderProgram::ShaderProgram( OpenGlDriver* driver, ShaderProgramProperties* properties )
            : driver( driver )
    {
        numTextures = properties->common.numTextures;

        String vertexShaderSource, pixelShaderSource;
        String name = "stock" + String::formatInt( numTextures ) + "T";

        if ( properties->common.dynamicLighting )
            name += String::formatInt( properties->numDirectionalLights ) + "d" + String::formatInt( properties->numPointLights ) + "p";

        name += String::formatInt( properties->common.lightMapping ) + "L" + String::formatInt( properties->common.receivesShadows ) + "S";

        // **** VERTEX SHADER ****

        vertexShaderSource += "attribute vec4 blendColour;\n";
        vertexShaderSource += "varying vec4 colour;\n";

        if ( numTextures > 0 )
            vertexShaderSource += "varying vec2 uv[" + String::formatInt( numTextures ) + "];\n";

        // Dynamic Directional & Point Lighting
        if ( properties->numDirectionalLights > 0 || properties->numPointLights > 0 )
            vertexShaderSource += "varying vec3 N, V;\n";

        // Light Mapping
        if ( properties->common.lightMapping )
            vertexShaderSource += "varying vec2 lightUv;\n";

        // Shadows
        if ( properties->numPointLights > 0 && properties->common.receivesShadows )
        {
            String index = "[" + String::formatInt( properties->numPointLights ) + "]";

            vertexShaderSource += "varying vec4 pointShadowUv" + index + ";\n";
            vertexShaderSource += "uniform mat4 pointShadowMatrix" + index + ";\n";
            vertexShaderSource += "uniform mat4 localToWorld;\n";
        }

        vertexShaderSource += "void main() {\n";
        vertexShaderSource += "    colour = blendColour;\n";

        for ( unsigned i = 0; i < numTextures; i++ )
            vertexShaderSource += "    uv[" + String::formatInt( i ) + "] = vec2( gl_TextureMatrix[" + String::formatInt( i ) + "] * gl_MultiTexCoord" + String::formatInt( i ) + " );\n";

        // Dynamic Directional & Point Lighting
        if ( properties->numDirectionalLights > 0 || properties->numPointLights > 0 )
        {
            vertexShaderSource += "    N = normalize( gl_NormalMatrix * gl_Normal );\n";
            vertexShaderSource += "    V = vec3( gl_ModelViewMatrix * gl_Vertex );\n";
        }

        // Light Mapping
        if ( properties->common.lightMapping )
            vertexShaderSource += "    lightUv = vec2( gl_TextureMatrix[" + String::formatInt( /*numTextures*/0 ) + "] * gl_MultiTexCoord" + String::formatInt( numTextures ) + " );\n";

        // Shadow Mapping
        for ( unsigned i = 0; i < properties->numPointLights && properties->common.receivesShadows; i++ )
        {
            String index = "[" + String::formatInt( i ) + "]";

            vertexShaderSource += "    pointShadowUv" + index + " = pointShadowMatrix" + index + " * ( localToWorld * gl_Vertex );\n";
        }

        vertexShaderSource += "    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n";
        vertexShaderSource += "}\n";

        vertexShader = new VertexShader( name + "vertex", vertexShaderSource );

        // **** PIXEL SHADER ****

        // Blending
        pixelShaderSource += "varying vec4 colour;\n";

        // Textures
        if ( numTextures > 0 )
        {
            pixelShaderSource += "uniform sampler2D textures[" + String::formatInt( numTextures ) + "];\n";
            pixelShaderSource += "varying vec2 uv[" + String::formatInt( numTextures ) + "];\n";
        }

        // Dynamic Lighting
        if ( properties->common.dynamicLighting )
            pixelShaderSource += "uniform vec3 sceneAmbient;\n";

        // Dynamic Directional & Point Lighting
        if ( properties->numDirectionalLights > 0 || properties->numPointLights > 0 )
        {
            pixelShaderSource += "uniform vec3 materialAmbient;\n";
            pixelShaderSource += "uniform vec3 materialDiffuse;\n";
            pixelShaderSource += "uniform float materialShininess;\n";
            pixelShaderSource += "uniform vec3 materialSpecular;\n";

            pixelShaderSource += "varying vec3 N, V;\n";
        }

        // Dynamic Directional Lighting
        if ( properties->numDirectionalLights > 0 )
        {
            pixelShaderSource += "uniform vec3 directionalAmbient[" + String::formatInt( properties->numDirectionalLights ) + "];\n";
            pixelShaderSource += "uniform vec3 directionalDiffuse[" + String::formatInt( properties->numDirectionalLights ) + "];\n";
            pixelShaderSource += "uniform vec3 directionalDir[" + String::formatInt( properties->numDirectionalLights ) + "];\n";
            pixelShaderSource += "uniform vec3 directionalSpecular[" + String::formatInt( properties->numDirectionalLights ) + "];\n";
        }

        if ( properties->numPointLights > 0 )
        {
            pixelShaderSource += "uniform vec3 pointAmbient[" + String::formatInt( properties->numPointLights ) + "];\n";
            pixelShaderSource += "uniform vec3 pointDiffuse[" + String::formatInt( properties->numPointLights ) + "];\n";
            pixelShaderSource += "uniform vec3 pointPos[" + String::formatInt( properties->numPointLights ) + "];\n";
            pixelShaderSource += "uniform float pointRange[" + String::formatInt( properties->numPointLights ) + "];\n";
            pixelShaderSource += "uniform vec3 pointSpecular[" + String::formatInt( properties->numPointLights ) + "];\n";
        }

        // Light Mapping
        if ( properties->common.lightMapping )
        {
            pixelShaderSource += "uniform sampler2D lightMap;\n";
            pixelShaderSource += "varying vec2 lightUv;\n";
        }

        // Shadows
        if ( properties->numPointLights > 0 && properties->common.receivesShadows )
        {
            String array = "[" + String::formatInt( properties->numPointLights ) + "]";

            if ( !driver->globalState.softShadows )
                pixelShaderSource += "uniform sampler2DShadow pointShadowMap" + array + ";\n";
            else
                pixelShaderSource += "uniform sampler2D pointShadowMap" + array + ";\n";

            pixelShaderSource += "varying vec4 pointShadowUv" + array + ";\n";
        }

        pixelShaderSource += "\nvoid main()\n{\n";

        if ( properties->common.dynamicLighting )
            pixelShaderSource += "    vec3 lightSum = sceneAmbient;\n";

        if ( properties->numPointLights > 0 )
        {
            pixelShaderSource += "    vec3 ray;\n";

            if ( properties->common.receivesShadows && driver->globalState.softShadows )
            {
                pixelShaderSource += "    vec4 shadowCoordinateWdivide;\n";
		        pixelShaderSource += "    float distanceFromLight, shadow, distRatio;\n";
            }
        }

        for ( unsigned i = 0; i < properties->numDirectionalLights; i++ )
        {
            pixelShaderSource += "    lightSum += max( directionalDiffuse[" + String::formatInt( i ) + "] * dot( N, directionalDir[" + String::formatInt( i ) + "] ), 0.0 ) * materialDiffuse"
                    " + directionalSpecular[" + String::formatInt( i ) + "] * pow( max( dot( normalize( -reflect( directionalDir[" + String::formatInt( i ) + "], N ) ), normalize( -V ) ), 0.0 ), 0.3 * materialShininess )"
                    " * materialSpecular"
                    " + directionalAmbient[" + String::formatInt( i ) + "] * materialAmbient;\n";
        }

        for ( unsigned i = 0; i < properties->numPointLights; i++ )
        {
            String index = "[" + String::formatInt( i ) + "]";

            if ( properties->common.receivesShadows && driver->globalState.softShadows )
            {
                pixelShaderSource += "    if ( pointShadowUv" + index + ".w > 0.0 )\n";
                pixelShaderSource += "    {\n";
                pixelShaderSource += "        shadowCoordinateWdivide = pointShadowUv" + index + " / pointShadowUv" + index + ".w;\n";

		        // Used to lower moiré pattern and self-shadowing
		        pixelShaderSource += "        shadowCoordinateWdivide.z += 0.005;\n";

		        pixelShaderSource += "        distanceFromLight = texture2D( pointShadowMap" + index + ", shadowCoordinateWdivide.st ).z;\n";
		        pixelShaderSource += "        distRatio = ( ( shadowCoordinateWdivide.z - distanceFromLight ) / distanceFromLight ) * 0.002;\n";

	 		    //pixelShaderSource += "        shadow = distanceFromLight < shadowCoordinateWdivide.z ? 0.2 : 1.0;\n";

#define sample( x, y ) "( texture2D( pointShadowMap" + index + ", shadowCoordinateWdivide.st + vec2( "#x", "#y" ) ).z < shadowCoordinateWdivide.z ? 0.2 : 1.0 )"

                pixelShaderSource += "        shadow = ( " sample( 0, 0 ) "\n";
                pixelShaderSource += "                + " sample( -distRatio, -distRatio ) "\n";
                pixelShaderSource += "                + " sample( -distRatio, +distRatio ) "\n";
                pixelShaderSource += "                + " sample( +distRatio, +distRatio ) "\n";
                pixelShaderSource += "                + " sample( +distRatio, -distRatio ) "\n";
                pixelShaderSource += "                + " sample( 0, -distRatio ) "\n";
                pixelShaderSource += "                + " sample( 0, +distRatio ) "\n";
                pixelShaderSource += "                + " sample( -distRatio, 0 ) "\n";
                pixelShaderSource += "                + " sample( +distRatio, 0 ) "\n";
                pixelShaderSource += "                ) / 9.0;\n";

	 		    pixelShaderSource += "    }\n";
	 		    pixelShaderSource += "    else\n";
	 		    pixelShaderSource += "        shadow = 1.0;\n";
            }

            pixelShaderSource += "    ray = pointPos" + index + " - V;\n";
            pixelShaderSource += "    lightSum += ( ( max( pointDiffuse" + index + " * dot( N, normalize( ray ) ), 0.0 ) * materialDiffuse"
                    " + pointSpecular" + index + " * pow( max( dot( normalize( -reflect( normalize( ray ), N ) ), normalize( -V ) ), 0.0 ), 0.3 * materialShininess ) * materialSpecular"
                    " + pointAmbient" + index + " * materialAmbient"
                    " ) * pointRange" + index + " / ( 2.0 * length( ray ) ) )";

            if ( properties->common.receivesShadows )
            {
                if ( !driver->globalState.softShadows )
                {
                    if ( !driver->globalState.shadowPcfEnabled )
                        pixelShaderSource += " * shadow2DProj( pointShadowMap" + index + ", pointShadowUv" + index + " ).r";
                    else
                    {
                        String dist = String::formatFloat( driver->globalState.shadowPcfDist );

                        pixelShaderSource += " * ( ( shadow2DProj( pointShadowMap" + index + ", pointShadowUv" + index + " ).r";
                        pixelShaderSource += " + shadow2DProj( pointShadowMap" + index + ", pointShadowUv" + index + " + vec4( -" + dist + ", -" + dist + ", 0.0, 0.0 ) ).r";
                        pixelShaderSource += " + shadow2DProj( pointShadowMap" + index + ", pointShadowUv" + index + " + vec4( -" + dist + ", " + dist + ", 0.0, 0.0 ) ).r";
                        pixelShaderSource += " + shadow2DProj( pointShadowMap" + index + ", pointShadowUv" + index + " + vec4( " + dist + ", " + dist + ", 0.0, 0.0 ) ).r";
                        pixelShaderSource += " + shadow2DProj( pointShadowMap" + index + ", pointShadowUv" + index + " + vec4( " + dist + ", -" + dist + ", 0.0, 0.0 ) ).r ) / 5.0 )";
                    }
                }
                else
                    pixelShaderSource += " * shadow";
            }

            pixelShaderSource += ";\n";
        }

        pixelShaderSource += "    gl_FragColor = ";

        //bool toon = true;

        //if ( toon )
        //    pixelShaderSource += "round( ( ";

        pixelShaderSource += "colour";

        if ( numTextures > 0 )
            pixelShaderSource += " * texture2D( textures[0], uv[0] )";

//        if ( properties->common.dynamicLighting )
//            pixelShaderSource += " * vec4( lightSum, 1.0 )";
//
//        if ( properties->common.lightMapping )
//            pixelShaderSource += " * texture2D( lightMap, lightUv )";

        //if ( toon )
        //    pixelShaderSource += ") * vec4( 5.0, 5.0, 5.0, 1.0 ) ) / vec4( 5.0, 5.0, 5.0, 1.0 )";

        pixelShaderSource += ";\n";

        pixelShaderSource += "}\n";

        //File::save( name + "pixel.glsl", pixelShaderSource );
        pixelShader = new PixelShader( name + "pixel", pixelShaderSource );

        // **** END OF SHADER GENERATION ****

        program = glApi.functions.glCreateProgram();

        SG_assert3( program != 0, "OpenGlDriver.ShaderProgram.ShaderProgram" )

        glApi.functions.glAttachShader( program, pixelShader->shader );
        glApi.functions.glAttachShader( program, vertexShader->shader );
        glApi.functions.glLinkProgram( program );

        int status = GL_FALSE;
        glApi.functions.glGetProgramiv( program, GL_LINK_STATUS, &status );

        if ( status == GL_FALSE )
        {
            int logLength = 0;
            int charsWritten  = 0;
            Array<GLchar> log;

            glApi.functions.glGetProgramiv( program, GL_INFO_LOG_LENGTH, &logLength );

            log.resize( logLength + 1 );
            glApi.functions.glGetProgramInfoLog( program, logLength, &charsWritten, log.getPtr() );

            throw Exception( "OpenGlDriver.ShaderProgram.ShaderProgram", "ShaderLinkError", "shader link log:\n" + String( log.getPtr() ) );
        }

        select();

        blendColour = glApi.functions.glGetAttribLocation( program, "blendColour" );
        materialAmbient = glApi.functions.glGetUniformLocation( program, "materialAmbient" );
        materialDiffuse = glApi.functions.glGetUniformLocation( program, "materialDiffuse" );
        materialEmissive = glApi.functions.glGetUniformLocation( program, "materialEmissive" );
        materialShininess = glApi.functions.glGetUniformLocation( program, "materialShininess" );
        materialSpecular = glApi.functions.glGetUniformLocation( program, "materialSpecular" );
        sceneAmbient = glApi.functions.glGetUniformLocation( program, "sceneAmbient" );

        for ( unsigned i = 0; i < properties->common.numTextures; i++ )
            textures[i] = glApi.functions.glGetUniformLocation( program, "textures[" + String::formatInt( i ) + "]" );

        lightMap = glApi.functions.glGetUniformLocation( program, "lightMap" );

        for ( unsigned i = 0; i < properties->numDirectionalLights; i++ )
        {
            directionalAmbient[i] = glApi.functions.glGetUniformLocation( program, ( String )"directionalAmbient[" + i + "]" );
            directionalDiffuse[i] = glApi.functions.glGetUniformLocation( program, ( String )"directionalDiffuse[" + i + "]" );
            directionalDir[i] = glApi.functions.glGetUniformLocation( program, ( String )"directionalDir[" + i + "]" );
            directionalSpecular[i] = glApi.functions.glGetUniformLocation( program, ( String )"directionalSpecular[" + i + "]" );
        }

        for ( unsigned i = 0; i < properties->numPointLights; i++ )
        {
            String index = "[" + String::formatInt( i ) + "]";

            pointAmbient[i] = glApi.functions.glGetUniformLocation( program, ( String ) "pointAmbient[" + i + "]" );
            pointDiffuse[i] = glApi.functions.glGetUniformLocation( program, ( String ) "pointDiffuse[" + i + "]" );
            pointPos[i] = glApi.functions.glGetUniformLocation( program, ( String ) "pointPos[" + i + "]" );
            pointRange[i] = glApi.functions.glGetUniformLocation( program, ( String ) "pointRange[" + i + "]" );
            pointSpecular[i] = glApi.functions.glGetUniformLocation( program, ( String ) "pointSpecular[" + i + "]" );

            if ( properties->common.receivesShadows )
            {
                pointShadowMap[i] = glApi.functions.glGetUniformLocation( program, "pointShadowMap" + index );
                pointShadowMatrix[i] = glApi.functions.glGetUniformLocation( program, "pointShadowMatrix" + index );
            }
        }

        localToWorld = glApi.functions.glGetUniformLocation( program, "localToWorld" );

        // ASSIGN TEXTURE INDICES
        unsigned nextTextureIndex = 0;

        for ( unsigned i = 0; i < properties->common.numTextures; i++ )
        {
            if ( textures[i] >= 0 )
            {
                textureIndices[i] = nextTextureIndex++;
                glApi.functions.glUniform1i( textures[i], textureIndices[i] );
            }
            else
                textureIndices[i] = -1;
        }

        if ( properties->common.lightMapping && lightMap >= 0 )
        {
            lightMapIndex = nextTextureIndex++;
            glApi.functions.glUniform1i( lightMap, lightMapIndex );
        }
        else
            lightMapIndex = -1;

        for ( unsigned i = 0; i < properties->numPointLights && properties->common.receivesShadows; i++ )
        {
            if ( pointShadowMap[i] >= 0 )
            {
                pointShadowMapIndices[i] = nextTextureIndex++;
                glApi.functions.glUniform1i( pointShadowMap[i], pointShadowMapIndices[i] );
            }
            else
                pointShadowMapIndices[i] = -1;
        }

        driver->checkErrors( "OpenGlDriver.ShaderProgram.ShaderProgram" );
    }

    ShaderProgram::~ShaderProgram()
    {
        if ( pixelShader != nullptr )
        {
            glApi.functions.glDetachShader( program, pixelShader->shader );
            pixelShader.release();
        }

        if ( vertexShader != nullptr )
        {
            glApi.functions.glDetachShader( program, vertexShader->shader );
            vertexShader.release();
        }

        glApi.functions.glDeleteProgram( program );
    }

    void ShaderProgram::select()
    {
        glApi.functions.glUseProgram( program );

        driver->renderState.currentShaderProgram = this;
        driver->renderState.currentMaterialColour = nullptr;
        driver->renderState.currentMaterialLighting = nullptr;
        driver->renderState.currentMaterialTexture = nullptr;
    }

    void ShaderProgram::setBlendColour( const Colour& colour )
    {
        if ( blendColour >= 0 )
            glApi.functions.glVertexAttrib4f( blendColour, colour.r, colour.g, colour.b, colour.a );
    }

    void ShaderProgram::setDirectionalLight( unsigned index, const DirectionalLightProperties& light )
    {
        if ( directionalAmbient[index] >= 0 )
            glApi.functions.glUniform3f( directionalAmbient[index], light.ambient.r, light.ambient.g, light.ambient.b );

        if ( directionalDiffuse[index] >= 0 )
            glApi.functions.glUniform3f( directionalDiffuse[index], light.diffuse.r, light.diffuse.g, light.diffuse.b );

        if ( directionalDir[index] >= 0 )
            glApi.functions.glUniform3f( directionalDir[index], light.direction.x, light.direction.y, light.direction.z );

        if ( directionalSpecular[index] >= 0 )
            glApi.functions.glUniform3f( directionalSpecular[index], light.specular.r, light.specular.g, light.specular.b );
    }

    void ShaderProgram::setLightMap( GLuint texture )
    {
        if ( lightMapIndex >= 0 )
        {
            glApi.functions.glActiveTexture( GL_TEXTURE0 + lightMapIndex );
            glBindTexture( GL_TEXTURE_2D, texture );

            stats.numTextures++;
        }
    }

    void ShaderProgram::setLocalToWorld( const glm::mat4& matrix )
    {
        if ( localToWorld >= 0 )
            glApi.functions.glUniformMatrix4fv( localToWorld, 1, GL_FALSE, &matrix[0][0] );
    }

    void ShaderProgram::setMaterialLighting( const MaterialLightingProperties& material )
    {
        if ( materialAmbient >= 0 )
            glApi.functions.glUniform3f( materialAmbient, material.ambient.r, material.ambient.g, material.ambient.b );

        if ( materialDiffuse >= 0 )
            glApi.functions.glUniform3f( materialDiffuse, material.diffuse.r, material.diffuse.g, material.diffuse.b );

        if ( materialEmissive >= 0 )
            glApi.functions.glUniform3f( materialEmissive, material.emissive.r, material.emissive.g, material.emissive.b );

        if ( materialShininess >= 0 )
            glApi.functions.glUniform1f( materialShininess, material.shininess );

        if ( materialSpecular >= 0 )
            glApi.functions.glUniform3f( materialSpecular, material.specular.r, material.specular.g, material.specular.b );
    }

    void ShaderProgram::setPointLight( unsigned index, const PointLightProperties& light )
    {
        if ( pointAmbient[index] >= 0 )
            glApi.functions.glUniform3f( pointAmbient[index], light.ambient.r, light.ambient.g, light.ambient.b );

        if ( pointDiffuse[index] >= 0 )
            glApi.functions.glUniform3f( pointDiffuse[index], light.diffuse.r, light.diffuse.g, light.diffuse.b );

        if ( pointPos[index] >= 0 )
            glApi.functions.glUniform3f( pointPos[index], light.pos.x, light.pos.y, light.pos.z );

        if ( pointRange[index] >= 0 )
            glApi.functions.glUniform1f( pointRange[index], light.range );

        if ( pointSpecular[index] >= 0 )
            glApi.functions.glUniform3f( pointSpecular[index], light.specular.r, light.specular.g, light.specular.b );
    }

    void ShaderProgram::setPointLightShadowMap( unsigned index, GLuint texture, const glm::mat4& matrix )
    {
        if ( pointShadowMapIndices[index] >= 0 )
        {
            glApi.functions.glActiveTexture( GL_TEXTURE0 + pointShadowMapIndices[index] );
            glBindTexture( GL_TEXTURE_2D, texture );

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE_ARB, GL_COMPARE_R_TO_TEXTURE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC_ARB, GL_LEQUAL);
            glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE_ARB, GL_INTENSITY);

            //printf( "sPLSM %u (%i) to %u\n", index, pointShadowMapIndices[index], texture );

            stats.numTextures++;
        }

        if ( pointShadowMatrix[index] >= 0 )
            glApi.functions.glUniformMatrix4fv( pointShadowMatrix[index], 1, GL_FALSE, &matrix[0][0] );
    }

    void ShaderProgram::setSceneAmbient( const Colour& colour )
    {
        if ( sceneAmbient >= 0 )
            glApi.functions.glUniform3f( sceneAmbient, colour.r, colour.g, colour.b );
    }

    void ShaderProgram::setTexture( unsigned index, unsigned texture )
    {
        if ( textureIndices[index] >= 0 )
        {
            glApi.functions.glActiveTexture( GL_TEXTURE0 + textureIndices[index] );
            glBindTexture( GL_TEXTURE_2D, texture );

            stats.numTextures++;
        }
    }

    /*int Program::getParamId( const char* name )
    {
        return glApi.functions.glGetUniformLocation( program, name );
    }*/

    /*void Program::setColourParam( int id, const Colour& colour )
    {
        if ( id >= 0 )
            glApi.functions.glUniform4f( id, colour.r, colour.g, colour.b, colour.a );
    }

    void Program::setFloatParam( int id, float a )
    {
        if ( id >= 0 )
            glApi.functions.glUniform1f( id, a );
    }

    void Program::setLightAmbient( unsigned i, const Colour& colour )
    {
        if ( lightAmbient[i] >= 0 )
            glApi.functions.glUniform4f( lightAmbient[i], colour.r, colour.g, colour.b, colour.a );
    }

    void Program::setLightDiffuse( unsigned i, const Colour& colour )
    {
        if ( lightDiffuse[i] >= 0 )
            glApi.functions.glUniform4f( lightDiffuse[i], colour.r, colour.g, colour.b, colour.a );
    }

    void Program::setLightPos( unsigned i, const Vector<float>& pos, float* modelView )
    {
        //if ( lightPos[i] >= 0 )
        //    glApi.functions.glUniform4f( lightPos[i], pos.x, pos.y, pos.z, modelView ? 1.0f : 0.0f );

        if ( lightPos[i] >= 0 )
            glApi.functions.glUniform3f( lightPos[i], pos.x, pos.y, pos.z );

        if ( lightTransform[i] >= 0 && modelView )
            glApi.functions.glUniformMatrix4fv( lightTransform[i], 1, false, modelView );
    }

    void Program::setLightRange( unsigned i, float range )
    {
        if ( lightRange[i] >= 0 )
            glApi.functions.glUniform1f( lightRange[i], range );
    }

    void Program::setLightSpecular( unsigned i, const Colour& colour )
    {
        if ( lightSpecular[i] >= 0 )
            glApi.functions.glUniform4f( lightSpecular[i], colour.r, colour.g, colour.b, colour.a );
    }

    void Program::setLightType( unsigned i, int type )
    {
        if ( lightType[i] >= 0 )
            glApi.functions.glUniform1i( lightType[i], type );
    }

    void Program::setVector2Param( int id, float x, float y )
    {
        if ( id >= 0 )
            glApi.functions.glUniform2f( id, x, y );
    }*/

    ShaderProgramSet::ShaderProgramSet( OpenGlDriver* driver, ShaderProgramSetProperties* properties )
            : driver( driver ), properties( *properties ), programs( nullptr )
    {
        if ( properties->dynamicLighting )
        {
            dirLightVars = MAX_DIRECTIONAL_LIGHTS + 1;
            pointLightVars = MAX_POINT_LIGHTS + 1;
        }
        else
        {
            dirLightVars = 1;
            pointLightVars = 1;
        }
    }

    void ShaderProgramSet::init()
    {
        programs = Allocator<ShaderProgram*>::allocate( dirLightVars * pointLightVars );

        SG_assert( programs != nullptr )

        for ( unsigned dirLights = 0; dirLights < dirLightVars; dirLights++ )
            for ( unsigned pointLights = 0; pointLights < pointLightVars; pointLights++ )
            {
                ShaderProgramProperties programProperties;

                programProperties.common = this->properties;
                programProperties.numDirectionalLights = dirLights;
                programProperties.numPointLights = pointLights;

                programs[pointLightVars * dirLights + pointLights] = new ShaderProgram( driver, &programProperties );
            }

#ifdef li_MSW
        printf( "OpenGlDriver: generated %u shader(s) ", dirLightVars * pointLightVars );
#endif
    }

    ShaderProgramSet::~ShaderProgramSet()
    {
        for ( unsigned dirLights = 0; dirLights < dirLightVars; dirLights++ )
            for ( unsigned pointLights = 0; pointLights < pointLightVars; pointLights++ )
                delete programs[pointLightVars * dirLights + pointLights];

        Allocator<ShaderProgram*>::release( programs );
    }

    ShaderProgram* ShaderProgramSet::getShaderProgram( bool dynamicLighting )
    {
        ShaderProgram* program;

        unsigned numDirectionalLights = 0, numPointLights = 0;

        if ( dynamicLighting && driver->globalState.dynamicLightingEnabled )
        {
            numDirectionalLights = driver->globalState.numDirectionalLights;
            numPointLights = driver->globalState.numPointLights;

            SG_assert( numDirectionalLights < dirLightVars )
            SG_assert( numPointLights < pointLightVars )

            program = programs[pointLightVars * numDirectionalLights + numPointLights];
        }
        else
            program = programs[0];

        if ( driver->renderState.currentShaderProgram != program )
        {
            program->select();
            program->setSceneAmbient( driver->globalState.sceneAmbient );

            if ( dynamicLighting )
            {
                for ( unsigned i = 0; i < numDirectionalLights; i++ )
                    program->setDirectionalLight( i, driver->globalState.directionalLights[i] );

                for ( unsigned i = 0; i < numPointLights; i++ )
                    program->setPointLight( i, driver->globalState.pointLights[i] );

                if ( properties.receivesShadows )
                {
                    for ( unsigned i = 0; i < numPointLights; i++ )
                        program->setPointLightShadowMap( i, driver->globalState.pointShadowMap[i]->texture, driver->globalState.pointShadowMatrix[i] );
                }
            }
        }

        return program;
    }

    bool ShaderProgramSet::matches( const ShaderProgramSetProperties* properties )
    {
        return properties->numTextures == this->properties.numTextures
                && properties->dynamicLighting == this->properties.dynamicLighting
                && properties->lightMapping == this->properties.lightMapping;
    }
}
