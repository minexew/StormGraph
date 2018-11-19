
#include "Internal.hpp"

#include <SDL_image.h>

namespace StormGraph
{
    Texture::Texture( const char* fileName ) : texture( 0 ), width( 0 ), height( 0 )
    {
        SDL_Surface* surface = 0;

        if ( fileName )
            surface = load( fileName );

        if ( surface )
        {
            init( surface );
            SDL_FreeSurface( surface );
        }
        else
            throw Exception( "StormGraph.Texture", "Texture", "TextureLoadError",
                    ( String )"Failed to load `" + fileName + "`. The file probably either doesn't exist at all or is not a well-formed image." );

        /*
        SeekableInputStream* input = Engine::getInstance()->open( fileName );

        if ( !input )
            throw Exception( "StormGraph.Texture", "Texture", "TextureLoadError", ( String )"Failed to load `" + fileName + "`. The file was not found." );

        texture = Engine::driver->createTextureFromStream( input, fileName );
        */
    }

    Texture::Texture( SDL_Surface* surface ) : texture( 0 ), width( 0 ), height( 0 )
    {
        if ( surface )
        {
            init( surface );
            SDL_FreeSurface( surface );
        }

        /*
        */
    }

    Texture::Texture( unsigned width, unsigned height )
    {
        glGenTextures( 1, &texture );
        glBindTexture( GL_TEXTURE_2D, texture );

        // set the scaling methods
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

#if StormGraph_Render_Mode == StormGraph_Render_OpenGL_2_Fixed || StormGraph_Render_Mode == StormGraph_Render_OpenGL_Multi
        glTexParameteri( GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE );
#else
#error Set-up GL3 mipmapping...
#endif

        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0 );

        /*
        texture = Engine::driver->createEmptyTexture( width, height );
        */
    }

    Texture::~Texture()
    {
        glDeleteTextures( 1, &texture );

        /*
        release( texture );
        */
    }

    Texture* Texture::tryLoad( const char* fileName )
    {
        SDL_Surface* surface = 0;

        if ( fileName )
            surface = load( fileName );

        if ( surface )
            return new Texture( surface );
        else
            return 0;

        /*
        SeekableInputStream* input = Engine::getInstance()->open( fileName );

        if ( !input )
            return 0;

        return new Texture( Engine::driver->createTextureFromStream( input, fileName ) );
        */
    }

#ifndef StormGraph_No_Helium
    Helium::Variable Texture::initMembers( Helium::Variable object )
    {
        object.setMember( "size", Helium::Variable::newVector( width, height, 0.0 ) );
        return object;
    }
#endif

    void Texture::init( SDL_Surface* surface )
    {
        SG_assert( surface != NULL, "StormGraph.Texture", "init" )

        width = surface->w;
        height = surface->h;

        //* Create a texture handle
	    glGenTextures( 1, &texture );
        glBindTexture( GL_TEXTURE_2D, texture );

        // set the scaling methods
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

        SG_assert2( useOpenGL < 300, "StormGraph.Texture", "init", "Using old GL_GENERATE_MIPMAP in OpenGL >= 3.0" );
        glTexParameteri( GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE );

	    //* Finally paste the data into the texture
	    if ( SDL_MUSTLOCK( surface ) )
            SDL_LockSurface( surface );

        glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, getSurfaceFormat( surface ), GL_UNSIGNED_BYTE, surface->pixels );

        if ( SDL_MUSTLOCK( surface ) )
            SDL_UnlockSurface( surface );

        /*
        */
    }

    void Texture::blitSurface( SDL_Surface* surface, int x, int y )
    {
        SG_assert( surface != NULL, "StormGraph.Texture", "blitSurface" )

        glBindTexture( GL_TEXTURE_2D, texture );

	    if ( SDL_MUSTLOCK( surface ) )
            SDL_LockSurface( surface );

        glTexSubImage2D( GL_TEXTURE_2D, 0, x, y, surface->w, surface->h, getSurfaceFormat( surface ), GL_UNSIGNED_BYTE, surface->pixels );

        if ( SDL_MUSTLOCK( surface ) )
            SDL_UnlockSurface( surface );

        /*
        TO MOVE ?
        */
    }

    void Texture::centerOrigin()
    {
        setOrigin( width / 2.0f,  height / 2.0f );
        /*
        ??
        */
    }

    GLenum Texture::getSurfaceFormat( const SDL_Surface* surface )
    {
        SG_assert( surface != NULL, "StormGraph.Texture", "getSurfaceFormat" )

        const int numColorChannels = surface->format->BytesPerPixel;

        if ( numColorChannels == 4 )
        {
            if ( surface->format->Rmask == 0x000000FF )
                return GL_RGBA;
            else
                return GL_BGRA;    // GL_BGRA_EXT
        }
        else if ( numColorChannels == 3 )
        {
            if ( surface->format->Rmask == 0x000000FF )
                return GL_RGB;
            else
                return GL_BGR;
        }
        else
            throw Exception( "StormGraph.Texture", "getSurfaceFormat", "InvalidImageFormat", "The input needs to be a 24-/32-bit pixel map." );

        /*
        */
    }

    SDL_Surface* Texture::load( const char* fileName )
    {
        SeekableInputStream* input = Engine::getInstance()->open( fileName );

        if ( !input )
            return 0;

        return IMG_Load_RW( getRwOps( input ), 1 );

        /*
        */
    }

    void Texture::render2D( float x, float y )
    {
        return render2D( x, y, ( float )width, ( float )height );

        /*
        Graphics::draw()
        */
    }

    void Texture::render2D( float x, float y, float w, float h, float u0, float v0, float u1, float v1, const Colour& blend )
    {
        const float uv[] = { u1, v0, u0, v0, u1, v1, u1, v1, u0, v0, u0, v1 };

        static const float verts[] =
        {
            1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 0.0f,
            1.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f
        };

        // Transform
        glPushMatrix();

        glTranslatef( x, y, 0.0f );
        // <rotate here>
        glTranslatef( -origin.x, -origin.y, 0.0f );
        glScalef( w, h, 1.0f );

        // Set-up
		glEnable( GL_TEXTURE_2D );
        glBindTexture( GL_TEXTURE_2D, texture );

        Engine::getInstance()->setColour( blend );

        glEnableClientState( GL_TEXTURE_COORD_ARRAY );
        glTexCoordPointer( 2, GL_FLOAT, 0, uv );

        glEnableClientState( GL_VERTEX_ARRAY );
        glVertexPointer( 3, GL_FLOAT, 0, verts );

        // Render
        glDrawArrays( GL_TRIANGLES, 0, 6 );
        numPolysThisFrame += 2;

        // Clean up
        glDisableClientState( GL_VERTEX_ARRAY );
        glDisableClientState( GL_TEXTURE_COORD_ARRAY );

		glPopMatrix();

		/*
        Graphics::draw()
        */
    }

    void Texture::renderBillboard( const Vector<float>& center, float width, float height, float angle, float alpha )
    {
        static const float uv[] = { 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f };

        static const float verts[] =
        {
            0.5f, -0.5f, 0.0f,
            -0.5f, -0.5f, 0.0f,
            0.5f, 0.5f, 0.0f,
            0.5f, 0.5f, 0.0f,
            -0.5f, -0.5f, 0.0f,
            -0.5f, 0.5f, 0.0f
        };

        // Billboard computation code
        // Source: http://www.lighthouse3d.com/opengl/billboarding/

        const static Vector<float> lookAt( 0.0f, 0.0f, 1.0f );

        Vector<float> objToCamProj = Vector<float>( Camera::currentCamera.x - center.x, 0.0f, Camera::currentCamera.z - center.z ).normalize();
        Vector<float> upVector = lookAt.crossProduct( objToCamProj );

        // Transform

        glPushMatrix();
        glTranslatef( center.x, center.y, center.z );

        float angleCos = lookAt.dotProduct( objToCamProj );

        if ( angleCos < 0.9999 && angleCos > -0.9999 )
		    glRotatef( acos( angleCos ) * 180.0f / ( float )M_PI, upVector.x, upVector.y, upVector.z );

        Vector<float> objToCam = ( Camera::currentCamera - center ).normalize();
        angleCos = objToCamProj.dotProduct( objToCam );

        if ( angleCos < 0.9999 && angleCos > -0.9999 )
        {
		    if ( objToCam.y < 0 )
    			glRotatef( acos( angleCos ) * 180.0f / ( float )M_PI, 1.0f, 0.0f, 0.0f );
		    else
			    glRotatef( acos( angleCos ) * 180.0f / ( float )M_PI, -1.0f, 0.0f, 0.0f );
        }

        glRotatef( -angle * 180.0f / ( float )M_PI, 0.0f, 0.0f, 1.0f );
        glScalef( width, height, 1.0f );

        // Set-up

        glEnable( GL_TEXTURE_2D );
        glBindTexture( GL_TEXTURE_2D, texture );

        Engine::getInstance()->setColour( Colour( 1.0f, 1.0f, 1.0f, alpha ) );

        glEnableClientState( GL_TEXTURE_COORD_ARRAY );
        glTexCoordPointer( 2, GL_FLOAT, 0, uv );

        glEnableClientState( GL_VERTEX_ARRAY );
        glVertexPointer( 3, GL_FLOAT, 0, verts );

        // Render
        glDrawArrays( GL_TRIANGLES, 0, 6 );
        numPolysThisFrame += 2;

        // Clean up
        glDisableClientState( GL_VERTEX_ARRAY );
        glDisableClientState( GL_TEXTURE_COORD_ARRAY );

        glPopMatrix();

        /*
        Texture::()
        */
    }

    void Texture::renderBillboard2( const Vector<float>& center, float width, float height, float angle, float alpha )
    {
        static const float uv[] = { 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f };

        static const float verts[] =
        {
            0.5f, -0.5f, 0.0f,
            -0.5f, -0.5f, 0.0f,
            0.5f, 0.5f, 0.0f,
            0.5f, 0.5f, 0.0f,
            -0.5f, -0.5f, 0.0f,
            -0.5f, 0.5f, 0.0f
        };

        Vector<float> delta( Camera::currentCamera - center );
        const float distance = delta.getLength();

        // Transform

        glPushMatrix();
        glTranslatef( center.x, center.y, center.z );

        glRotatef( -atan2( delta.x, delta.y ) * 180.0f / ( float )M_PI, 0.0f, 0.0f, 1.0f );
        glRotatef( -acos( delta.z / distance ) * 180.0f / ( float )M_PI, 1.0f, 0.0f, 0.0f );

        glRotatef( -angle * 180.0f / ( float )M_PI, 0.0f, 0.0f, 1.0f );
        glScalef( width, height, 1.0f );

        // Set-up

        glEnable( GL_TEXTURE_2D );
        glBindTexture( GL_TEXTURE_2D, texture );

        Engine::getInstance()->setColour( Colour( 1.0f, 1.0f, 1.0f, alpha ) );

        glEnableClientState( GL_TEXTURE_COORD_ARRAY );
        glTexCoordPointer( 2, GL_FLOAT, 0, uv );

        glEnableClientState( GL_VERTEX_ARRAY );
        glVertexPointer( 3, GL_FLOAT, 0, verts );

        // Render
        glDrawArrays( GL_TRIANGLES, 0, 6 );
        numPolysThisFrame += 2;

        // Clean up
        glDisableClientState( GL_VERTEX_ARRAY );
        glDisableClientState( GL_TEXTURE_COORD_ARRAY );

        glPopMatrix();

        /*
        Texture::...()
        */
    }

    void Texture::renderPlanar( const Vector<float>& center, float a, float angle )
    {
        static const float uv[] = { 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f };

        static const float verts[] =
        {
            0.5f, -0.5f, 0.0f,
            -0.5f, -0.5f, 0.0f,
            0.5f, 0.5f, 0.0f,
            0.5f, 0.5f, 0.0f,
            -0.5f, -0.5f, 0.0f,
            -0.5f, 0.5f, 0.0f
        };

        // Transform
        glPushMatrix();
        glTranslatef( center.x, center.y, center.z );
        glRotated( -angle * 180.0 / M_PI, 0.0, 0.0, 1.0 );
        glScalef( a, a, 1.0f );

        // Set-up
        glEnable( GL_TEXTURE_2D );
        glBindTexture( GL_TEXTURE_2D, texture );

        Engine::getInstance()->setColour( Colour( 1.0f, 1.0f, 1.0f, 1.0f ) );

        glEnableClientState( GL_TEXTURE_COORD_ARRAY );
        glTexCoordPointer( 2, GL_FLOAT, 0, uv );

        glEnableClientState( GL_VERTEX_ARRAY );
        glVertexPointer( 3, GL_FLOAT, 0, verts );

        // Render
        glDrawArrays( GL_TRIANGLES, 0, 6 );
        numPolysThisFrame += 2;

        // Clean up
        glDisableClientState( GL_VERTEX_ARRAY );
        glDisableClientState( GL_TEXTURE_COORD_ARRAY );

        glPopMatrix();

        /*
        Texture:::....
        */
    }

    void Texture::renderQuad( const Vector<float>& a, const Vector<float>& b, const Vector<float>& c, const Vector<float>& d )
    {
        static const float uv[] = { 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f };

        float verts[] =
        {
            b.x, b.y, b.z, a.x, a.y, a.z, c.x, c.y, c.z,
            c.x, c.y, c.z, a.x, a.y, a.z, d.x, d.y, d.z
        };

        glEnable( GL_TEXTURE_2D );
        glBindTexture( GL_TEXTURE_2D, texture );

        glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );

        glEnableClientState( GL_TEXTURE_COORD_ARRAY );
        glTexCoordPointer( 2, GL_FLOAT, 0, uv );

        glEnableClientState( GL_VERTEX_ARRAY );
        glVertexPointer( 3, GL_FLOAT, 0, verts );

        glDrawArrays( GL_TRIANGLES, 0, 6 );
        numPolysThisFrame += 2;

        glDisableClientState( GL_VERTEX_ARRAY );
        glDisableClientState( GL_TEXTURE_COORD_ARRAY );

        /*
        Texture:::....
        */
    }

    void Texture::setOrigin( float x, float y )
    {
        origin.x = x;
        origin.y = y;

        /*
        ??
        **/
    }
}
