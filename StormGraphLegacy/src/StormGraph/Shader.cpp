
#include "Internal.hpp"

namespace StormGraph
{
    Shader::~Shader()
    {
        glFs.DeleteShader( shader );
    }

    PixelShader::PixelShader( const char* source )
    {
        if ( !shadersEnabled )
            return;

        shader = glFs.CreateShader( GL_FRAGMENT_SHADER );
        glFs.ShaderSource( shader, 1, &source, 0 );
        glFs.CompileShader( shader );

        int status = GL_FALSE;
        glFs.GetShaderiv( shader, GL_COMPILE_STATUS, &status );

        if ( status == GL_FALSE )
        {
            int logLength = 0;
            int charsWritten  = 0;
            Array<GLchar> log;

            glFs.GetShaderiv( shader, GL_INFO_LOG_LENGTH, &logLength );

            log.resize( logLength + 1 );
            glFs.GetShaderInfoLog( shader, logLength, &charsWritten, *log );
            throw Exception( "StormGraph.PixelShader", "PixelShader", "ShaderCompileError", "shader compilation log:\n" + String( *log ) );
        }
    }

    PixelShader::~PixelShader()
    {
    }

    VertexShader::VertexShader( const char* source )
    {
        if ( !shadersEnabled )
            return;

        shader = glFs.CreateShader( GL_VERTEX_SHADER );
        glFs.ShaderSource( shader, 1, &source, 0 );
        glFs.CompileShader( shader );

        int status = GL_FALSE;
        glFs.GetShaderiv( shader, GL_COMPILE_STATUS, &status );

        if ( status == GL_FALSE )
        {
            int logLength = 0;
            int charsWritten  = 0;
            Array<GLchar> log;

            glFs.GetShaderiv( shader, GL_INFO_LOG_LENGTH, &logLength );

            log.resize( logLength + 1 );
            glFs.GetShaderInfoLog( shader, logLength, &charsWritten, *log );
            throw Exception( "StormGraph.VertexShader", "VertexShader", "ShaderCompileError", "shader compilation log:\n" + String( *log ) );
        }
    }

    VertexShader::~VertexShader()
    {
    }

    Program::Program( const String& path ) : pixel( 0 ), vertex( 0 )
    {
        pixel = new PixelShader( Engine::getInstance()->loadTextAsset( path + "Pixel.glsl", true ) );
        vertex = new VertexShader( Engine::getInstance()->loadTextAsset( path + "Vertex.glsl", true ) );

        init( pixel, vertex );
    }

    Program::Program( PixelShader* pixel, VertexShader* vertex ) : pixel( pixel ), vertex( vertex )
    {
        init( pixel, vertex );
    }

    void Program::init( PixelShader* pixel, VertexShader* vertex )
    {
        if ( !shadersEnabled )
        {
            materialAmbient = -1;
            materialDiffuse = -1;
            materialShininess = -1;
            materialSpecular = -1;
            sceneAmbient = -1;
            vertexColour = -1;

            for ( unsigned i = 0; i < maxLights; i++ )
            {
                lightAmbient[i] = -1;
                lightDiffuse[i] = -1;
                lightPos[i] = -1;
                lightSpecular[i] = -1;
                lightTransform[i] = -1;
            }

            return;
        }

        program = glFs.CreateProgram();

        SG_assert( program != 0, "StormGraph.Program", "Program" )

        if ( pixel )
            glFs.AttachShader( program, pixel->shader );

        if ( vertex )
            glFs.AttachShader( program, vertex->shader );

        glFs.LinkProgram( program );

        int status = GL_FALSE;
        glFs.GetProgramiv( program, GL_LINK_STATUS, &status );

        if ( status == GL_FALSE )
        {
            int logLength = 0;
            int charsWritten  = 0;
            Array<GLchar> log;

            glFs.GetProgramiv( program, GL_INFO_LOG_LENGTH, &logLength );

            log.resize( logLength + 1 );
            glFs.GetProgramInfoLog( program, logLength, &charsWritten, *log );
            throw Exception( "StormGraph.Program", "link", "ShaderLinkError", "shader link log:\n" + String( *log ) );
        }

        materialAmbient = glFs.glGetUniformLocation( program, "materialAmbient" );
        materialDiffuse = glFs.glGetUniformLocation( program, "materialDiffuse" );
        materialShininess = glFs.glGetUniformLocation( program, "materialShininess" );
        materialSpecular = glFs.glGetUniformLocation( program, "materialSpecular" );
        sceneAmbient = glFs.glGetUniformLocation( program, "sceneAmbient" );
        vertexColour = glFs.glGetAttribLocation( program, "vertexColour" );

        for ( unsigned i = 0; i < maxLights; i++ )
        {
            lightAmbient[i] = glFs.glGetUniformLocation( program, ( String )"lightAmbient[" + i + "]" );
            lightDiffuse[i] = glFs.glGetUniformLocation( program, ( String )"lightDiffuse[" + i + "]" );
            lightPos[i] = glFs.glGetUniformLocation( program, ( String )"lightPos[" + i + "]" );
            lightRange[i] = glFs.glGetUniformLocation( program, ( String )"lightRange[" + i + "]" );
            lightSpecular[i] = glFs.glGetUniformLocation( program, ( String )"lightSpecular[" + i + "]" );
            lightTransform[i] = glFs.glGetUniformLocation( program, ( String )"lightTransform[" + i + "]" );
        }
    }

    Program::~Program()
    {
        if ( !shadersEnabled )
            return;

        //if ( Engine::getInstance() && Engine::getInstance()->currentShader == this )
        //    Engine::getInstance()->detachShader();

        if ( pixel )
        {
            glFs.DetachShader( program, pixel->shader );
            pixel->release();
        }

        if ( vertex )
        {
            glFs.DetachShader( program, vertex->shader );
            vertex->release();
        }

        glFs.DeleteProgram( program );
    }

    void Program::setSceneAmbient( const Colour& colour )
    {
        if ( sceneAmbient >= 0 )
            glFs.glUniform4f( sceneAmbient, colour.r, colour.g, colour.b, colour.a );
    }

    void Program::setLightAmbient( unsigned i, const Colour& colour )
    {
        if ( lightAmbient[i] >= 0 )
            glFs.glUniform4f( lightAmbient[i], colour.r, colour.g, colour.b, colour.a );
    }

    void Program::setLightDiffuse( unsigned i, const Colour& colour )
    {
        if ( lightDiffuse[i] >= 0 )
            glFs.glUniform4f( lightDiffuse[i], colour.r, colour.g, colour.b, colour.a );
    }

    void Program::setLightPos( unsigned i, const Vector<float>& pos, float* modelView )
    {
        if ( lightPos[i] >= 0 )
            glFs.glUniform4f( lightPos[i], pos.x, pos.y, pos.z, modelView ? 1.0f : 0.0f );

        if ( lightTransform[i] >= 0 && modelView )
            glFs.glUniformMatrix4fv( lightTransform[i], 1, false, modelView );
    }

    void Program::setLightRange( unsigned i, float range )
    {
        if ( lightRange[i] >= 0 )
            glFs.glUniform1f( lightRange[i], range );
    }

    void Program::setLightSpecular( unsigned i, const Colour& colour )
    {
        if ( lightSpecular[i] >= 0 )
            glFs.glUniform4f( lightSpecular[i], colour.r, colour.g, colour.b, colour.a );
    }

    void Program::setMaterialAmbient( const Colour& colour )
    {
        if ( materialAmbient >= 0 )
            glFs.glUniform4f( materialAmbient, colour.r, colour.g, colour.b, colour.a );
    }

    void Program::setMaterialDiffuse( const Colour& colour )
    {
        if ( materialDiffuse >= 0 )
            glFs.glUniform4f( materialDiffuse, colour.r, colour.g, colour.b, colour.a );
    }

    void Program::setMaterialShininess( float shininess )
    {
        if ( materialShininess >= 0 )
            glFs.glUniform1f( materialShininess, shininess );
    }

    void Program::setMaterialSpecular( const Colour& colour )
    {
        if ( materialSpecular >= 0 )
            glFs.glUniform4f( materialSpecular, colour.r, colour.g, colour.b, colour.a );
    }

    void Program::setVertexColour( const Colour& colour )
    {
        if ( vertexColour >= 0 )
            glFs.glVertexAttrib4f( vertexColour, colour.r, colour.g, colour.b, colour.a );
    }

    void Program::use()
    {
        if ( !shadersEnabled )
            return;

        Engine::getInstance()->currentShader = this;
        glFs.UseProgram( program );
    }
}
