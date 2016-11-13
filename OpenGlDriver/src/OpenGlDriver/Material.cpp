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
    Material::Material( OpenGlDriver* driver, const char* name, const MaterialProperties2* properties, bool finalized )
            : driver( driver ), name( name ), dynamicLighting( false ), lightMapping( false ), receivesShadows( false ), queueEntry( nullptr )
    {
        SG_assert( properties != nullptr )

        // Copy other material properties
        colour = properties->colour;

        numTextures = properties->numTextures;

        for ( size_t i = 0; i < numTextures; i++ )
        {
            texturePreloads[i] = static_cast<TexturePreload*>( properties->texturePreloads[i] );
            textures[i] = static_cast<Texture*>( properties->textures[i] );
        }

        if ( properties->dynamicLighting )
        {
            dynamicLighting = true;
            dynamicLightingResponse.ambient = properties->dynamicLightingResponse.ambient;
            dynamicLightingResponse.diffuse = properties->dynamicLightingResponse.diffuse;
            dynamicLightingResponse.specular = properties->dynamicLightingResponse.specular;
            dynamicLightingResponse.emissive = properties->dynamicLightingResponse.emissive;
            dynamicLightingResponse.shininess = properties->dynamicLightingResponse.shininess;
        }

        if ( properties->lightMapping )
        {
            lightMapping = true;

            lightMapPreload = static_cast<TexturePreload*>( properties->lightMapPreload );
            lightMap = static_cast<Texture*>( properties->lightMap );
        }

        receivesShadows = properties->receivesShadows;

        if ( driver->globalState.shadersEnabled && finalized )
            getShaderSet();

        Resource::add( this );
    }

    Material::~Material()
    {
        Resource::remove( this );
    }

    void Material::apply()
    {
        ShaderProgram* shaderProgram = nullptr;

        if ( driver->globalState.shadersEnabled )
        {
            if ( shaderProgramSet == nullptr )
                getShaderSet();

            shaderProgram = shaderProgramSet->getShaderProgram( dynamicLighting );

            if ( driver->renderState.currentMaterialColour != this )
            {
                shaderProgram->setBlendColour( colour );

                driver->renderState.currentMaterialColour = this;
            }

            if ( driver->renderState.currentMaterialTexture != this )
            {
                for ( unsigned i = 0; i < numTextures; i++ )
                    if ( textures[i] != nullptr )
                        shaderProgram->setTexture( i, textures[i]->texture );

                if ( lightMapping )
                    shaderProgram->setLightMap( lightMap->texture );

                driver->renderState.currentMaterialTexture = this;
            }

            if ( driver->renderState.currentMaterialLighting != this )
            {
                if ( dynamicLighting )
                    shaderProgram->setMaterialLighting( dynamicLightingResponse );

                driver->renderState.currentMaterialLighting = this;
            }
        }
        else
        {
            if ( driver->renderState.currentMaterialColour != this )
            {
                glColor4fv( &colour.r );

                driver->renderState.currentMaterialColour = this;
            }

            if ( driver->renderState.currentMaterialTexture != this )
            {
                glApi.functions.glActiveTexture( GL_TEXTURE0 );

                if ( numTextures > 0 )
                {
                    glEnable( GL_TEXTURE_2D );
                    if ( textures[0] != nullptr )
                        glBindTexture( GL_TEXTURE_2D, textures[0]->texture );
                    stats.numTextures++;
                }
                else
                    glDisable( GL_TEXTURE_2D );

                glApi.functions.glActiveTexture( GL_TEXTURE1 );

                if ( lightMapping )
                {
                    glEnable( GL_TEXTURE_2D );
                    glBindTexture( GL_TEXTURE_2D, lightMap->texture );
                    stats.numTextures++;

                    /*glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE );
                    glTexEnvi( GL_TEXTURE_2D, GL_COMBINE_RGB_ARB, GL_MODULATE );
                    glTexEnvi( GL_TEXTURE_2D, GL_COMBINE_ALPHA_ARB, GL_MODULATE );

                    glTexEnvi( GL_TEXTURE_2D, GL_SOURCE0_RGB_ARB, GL_TEXTURE );
                    glTexEnvi( GL_TEXTURE_2D, GL_OPERAND0_RGB_ARB, GL_SRC_COLOR );
                    glTexEnvi( GL_TEXTURE_2D, GL_SOURCE0_ALPHA_ARB, GL_TEXTURE );
                    glTexEnvi( GL_TEXTURE_2D, GL_OPERAND0_ALPHA_ARB, GL_SRC_ALPHA );

                    glTexEnvi( GL_TEXTURE_2D, GL_SOURCE1_RGB_ARB, GL_TEXTURE );
                    glTexEnvi( GL_TEXTURE_2D, GL_OPERAND1_RGB_ARB, GL_SRC_COLOR );
                    glTexEnvi( GL_TEXTURE_2D, GL_SOURCE1_ALPHA_ARB, GL_TEXTURE );
                    glTexEnvi( GL_TEXTURE_2D, GL_OPERAND1_ALPHA_ARB, GL_SRC_ALPHA );*/
                }
                else
                    glDisable( GL_TEXTURE_2D );

                glApi.functions.glActiveTexture( GL_TEXTURE0 );

                driver->renderState.currentMaterialTexture = this;
            }

            if ( driver->renderState.currentMaterialLighting != this )
            {
                if ( dynamicLighting )
                {
                    glEnable( GL_LIGHTING );

#define COMPONENTS_OF( property_ ) { dynamicLightingResponse.property_.r, dynamicLightingResponse.property_.g, dynamicLightingResponse.property_.b, dynamicLightingResponse.property_.a }

                    GLfloat ambient[] = COMPONENTS_OF( ambient );
                    GLfloat diffuse[] = COMPONENTS_OF( diffuse );
                    GLfloat emissive[] = COMPONENTS_OF( emissive );
                    GLfloat specular[] = COMPONENTS_OF( specular );

                    glMaterialfv( GL_FRONT, GL_AMBIENT, ambient );
                    glMaterialfv( GL_FRONT, GL_DIFFUSE, diffuse );
                    glMaterialfv( GL_FRONT, GL_EMISSION, emissive );
                    glMaterialfv( GL_FRONT, GL_SPECULAR, specular );
                }
                else
                    glDisable( GL_LIGHTING );

                driver->renderState.currentMaterialLighting = this;
            }
        }
    }

    void Material::apply( const Colour& blend )
    {
        apply();

        if ( driver->globalState.shadersEnabled )
        {
            ShaderProgram* shaderProgram = shaderProgramSet->getShaderProgram( dynamicLighting );

            shaderProgram->setBlendColour( blend );
            driver->renderState.currentMaterialColour = nullptr;
        }
        else
            glColor4f( blend.r, blend.g, blend.b, blend.a );
    }

    void Material::apply( const Colour& blend, Texture* texture0 )
    {
        apply();

        if ( driver->globalState.shadersEnabled )
        {
            ShaderProgram* shaderProgram = shaderProgramSet->getShaderProgram( dynamicLighting );

            shaderProgram->setBlendColour( blend );

            if ( texture0 != nullptr )
                shaderProgram->setTexture( 0, texture0->texture );
        }
        else
        {
            glColor4f( blend.r, blend.g, blend.b, blend.a );

            if ( texture0 != nullptr )
                glBindTexture( GL_TEXTURE_2D, texture0->texture );
        }

        driver->renderState.currentMaterialColour = nullptr;
        driver->renderState.currentMaterialTexture = nullptr;
    }

    Material* Material::finalize()
    {
        for ( size_t i = 0; i < numTextures; i++ )
        {
            if ( texturePreloads[i] != nullptr || textures[i] != nullptr )
                printf( "[Material.finalize] %" PRIuPTR " preload=%p, texture=%p\n", i, ( void* ) texturePreloads[i], ( void* ) textures[i] );

            if ( textures[i] == nullptr )
            {
                SG_assert( texturePreloads[i] != nullptr )

                printf( "   - have preload, obtaining finalized resource\n" );

                textures[i] = ( Texture* ) texturePreloads[i]->getFinalized();
                texturePreloads[i].release();

                printf( "   - obtained finalized texture=%p\n", ( void* ) textures[i] );
            }
        }

        if ( lightMapping && lightMap == nullptr )
        {
            SG_assert( lightMapPreload != nullptr )

            lightMap = ( Texture* ) lightMapPreload->getFinalized();
            lightMapPreload.release();
        }

        if ( driver->globalState.shadersEnabled && shaderProgramSet == nullptr )
            getShaderSet();

        return this;
    }

    void Material::getShaderSet()
    {
        ShaderProgramSetProperties shaderProgramSetProperties;
        memset( &shaderProgramSetProperties, 0, sizeof( shaderProgramSetProperties ) );

        shaderProgramSetProperties.numTextures = numTextures;
        shaderProgramSetProperties.dynamicLighting = dynamicLighting;
        shaderProgramSetProperties.lightMapping = lightMapping;
        shaderProgramSetProperties.receivesShadows = receivesShadows;

        shaderProgramSet = driver->getShaderProgramSet( &shaderProgramSetProperties );
    }

    void Material::query( unsigned flags, MaterialProperties2* properties )
    {
        if ( flags & getColour )
            properties->colour = colour;

        if ( flags & getNumTextures )
            properties->numTextures = numTextures;
    }

    /*void Material::query( MaterialProperties* properties )
    {
        if ( properties->query & MaterialProperties::getAmbient )
            properties->ambient = ambient;

        if ( properties->query & MaterialProperties::getDiffuse )
            properties->diffuse = diffuse;

        if ( properties->query & MaterialProperties::getSpecular )
            properties->specular = specular;

        if ( properties->query & MaterialProperties::getEmissive )
            properties->emissive = emissive;

        if ( properties->query & MaterialProperties::getColour )
            properties->colour = colour;

        if ( properties->query & MaterialProperties::getShininess )
            properties->shininess = shininess;

        if ( properties->query & MaterialProperties::getTexture )
            properties->texture = texture->reference();

        if ( properties->query & MaterialProperties::setAmbient )
            ambient = properties->ambient;

        if ( properties->query & MaterialProperties::setDiffuse )
            diffuse = properties->diffuse;

        if ( properties->query & MaterialProperties::setSpecular )
            specular = properties->specular;

        if ( properties->query & MaterialProperties::setEmissive )
            emissive = properties->emissive;

        if ( properties->query & MaterialProperties::setColour )
            colour = properties->colour;

        if ( properties->query & MaterialProperties::setShininess )
            shininess = properties->shininess;

        if ( properties->query & MaterialProperties::setTexture )
            texture = ( Texture* ) properties->texture;
    }*/

    void Material::setColour( const Colour& colour )
    {
        this->colour = colour;

        SG_assert( driver->globalState.shadersEnabled )

        if ( driver->renderState.currentMaterialColour == this )
            driver->renderState.currentShaderProgram->setBlendColour( colour );
    }
}

