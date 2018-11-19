
#include <StormGraph/StormGraph.hpp>

namespace StormGraph
{
    Material::Material() : texture( 0 ), alpha( 1.0f ), shininess( 128.0f )
    {
    }

    Material::~Material()
    {
        if ( texture )
            texture->release();
    }

    void Material::apply()
    {
        if ( texture && texture->texture )
        {
            glEnable( GL_TEXTURE_2D );
            glBindTexture( GL_TEXTURE_2D, texture->texture );
        }
        else
            glDisable( GL_TEXTURE_2D );

        if ( Engine::getInstance()->currentShader )
        {
            Engine::getInstance()->currentShader->setMaterialAmbient( ambient );
            Engine::getInstance()->currentShader->setMaterialDiffuse( diffuse );
            Engine::getInstance()->currentShader->setMaterialShininess( shininess );
            Engine::getInstance()->currentShader->setMaterialSpecular( specular );
        }
        else
        {
            GLfloat ambientRGBA[] = { ambient.r, ambient.g, ambient.b, ambient.a };
            GLfloat diffuseRGBA[] = { diffuse.r, diffuse.g, diffuse.b, diffuse.a };
            GLfloat specularRGBA[] = { specular.r, specular.g, specular.b, specular.a };
            GLfloat emissiveRGBA[] = { emissive.r, emissive.g, emissive.b, emissive.a };

            glMaterialfv( GL_FRONT, GL_AMBIENT, ambientRGBA );
            glMaterialfv( GL_FRONT, GL_DIFFUSE, diffuseRGBA );
            glMaterialfv( GL_FRONT, GL_SPECULAR, specularRGBA );
            glMaterialfv( GL_FRONT, GL_EMISSION, emissiveRGBA );
        }

        Engine::getInstance()->setColour( Colour( 1.0f, 1.0f, 1.0f, alpha ) );
    }

    Material* Material::createSimple( const char* texture )
    {
        Material* mat = new Material;
        mat->name = texture;
        mat->ambient = Colour( 0.6f, 0.6f, 0.6f );
        mat->diffuse = Colour( 0.8f, 0.8f, 0.8f );
        mat->specular = Colour( 1.0f, 1.0f, 1.0f );
        mat->emissive = Colour();
        mat->shininess = 0.0f;
        mat->alpha = 1.0f;
        mat->texture = new Texture( texture );
        return mat;
    }
}
