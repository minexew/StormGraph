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

namespace OpenGlDriver
{
    RenderBuffer::RenderBuffer( OpenGlDriver* driver, const Vector<unsigned>& size, bool withDepthBuffer )
            : buffer( 0 ), depth( 0 )
    {
        glApi.functions.glGenFramebuffers( 1, &buffer );
        driver->pushRenderBuffer( this );

        texture = new Texture( driver, "RenderBuffer.texture", size.x, size.y );
        glApi.functions.glFramebufferTexture2D( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, texture->texture, 0 );

        if ( withDepthBuffer )
        {
            glApi.functions.glGenRenderbuffers( 1, &depth );
            glApi.functions.glBindRenderbuffer( GL_RENDERBUFFER_EXT, depth );
            glApi.functions.glRenderbufferStorage( GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, size.x, size.y );

            glApi.functions.glFramebufferRenderbuffer( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, depth );
        }

        GLenum status = glApi.functions.glCheckFramebufferStatus( GL_FRAMEBUFFER_EXT );

        if ( status != GL_FRAMEBUFFER_COMPLETE_EXT )
            throw StormGraph::Exception( "OpenGlDriver.RenderBuffer.RenderBuffer", "RenderBufferIncomplete",
                    ( String ) "Framebuffer Status = " + String::formatInt( status, -1, String::hexadecimal ) + "h" );

        driver->popRenderBuffer();
    }

    RenderBuffer::RenderBuffer( OpenGlDriver* driver, Texture* depthTexture )
            : texture( depthTexture )
    {
        SG_assert( depthTexture != nullptr )

        glApi.functions.glGenFramebuffers( 1, &buffer );
        driver->pushRenderBuffer( this );

        glDrawBuffer( GL_NONE );
	    glReadBuffer( GL_NONE );

        glApi.functions.glFramebufferTexture2D( GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_TEXTURE_2D, depthTexture->texture, 0 );

        GLenum status = glApi.functions.glCheckFramebufferStatus( GL_FRAMEBUFFER_EXT );

        if ( status != GL_FRAMEBUFFER_COMPLETE_EXT )
            throw StormGraph::Exception( "OpenGlDriver.RenderBuffer.RenderBuffer", "RenderBufferIncomplete",
                    ( String ) "Framebuffer Status = " + String::formatInt( status, -1, String::hexadecimal ) + "h" );

        driver->popRenderBuffer();
    }

    RenderBuffer::~RenderBuffer()
    {
        glApi.functions.glDeleteFramebuffers( 1, &buffer );
    }

    unsigned RenderBuffer::getHeight()
    {
        return texture->getDimensions().y;
    }

    ITexture* RenderBuffer::getTexture()
    {
        return texture->reference();
    }

    unsigned RenderBuffer::getWidth()
    {
        return texture->getDimensions().x;
    }

    bool RenderBuffer::isSetUp()
    {
        return texture != 0;
    }
}
