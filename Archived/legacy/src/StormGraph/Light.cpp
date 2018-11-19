
#include <StormGraph/StormGraph.hpp>

namespace StormGraph
{
    Light::Light( LightType type, const Vector<float>& loc, const Vector<float> direction,
                    const Colour& ambient, const Colour& diffuse, float range )
            : type( type ), ambient( ambient ), diffuse( diffuse ), direction( direction ),
            cutoffAngle( 360.f ), range( range )
    {
        this->loc = loc;
    }

    Light::~Light()
    {
    }

    void Light::pick( Picking* picking )
    {
    }

    void Light::render()
    {
        unsigned id = Engine::getInstance()->getLightId();

        if ( Engine::getInstance()->currentShader )
        {
            Program* shader = Engine::getInstance()->currentShader;

            shader->setLightAmbient( id, ambient );
            shader->setLightDiffuse( id, diffuse );
            shader->setLightSpecular( id, diffuse );

            if ( type == Light_positional )
            {
                static GLfloat modelView[16];
                glGetFloatv( GL_MODELVIEW_MATRIX, modelView );

                shader->setLightPos( id, loc, modelView );
            }
            else
                shader->setLightPos( id, -direction, 0 );

            shader->setLightRange( id, range );
        }
        else if ( id < GL_MAX_LIGHTS )
        {
            glEnable( GL_LIGHTING );
            glEnable( GL_LIGHT0 + id );

            GLfloat ambientRGBA[] = { ambient.r, ambient.g, ambient.b, ambient.a };
            GLfloat diffuseRGBA[] = { diffuse.r, diffuse.g, diffuse.b, diffuse.a };

            glLightfv( GL_LIGHT0 + id, GL_AMBIENT, ambientRGBA );
            glLightfv( GL_LIGHT0 + id, GL_DIFFUSE, diffuseRGBA );
            glLightfv( GL_LIGHT0 + id, GL_SPECULAR, diffuseRGBA );

            if ( type == Light_positional )
            {
                // 1.f == positional

                GLfloat position[] = { loc.x, loc.y, loc.z, 1.f };
                glLightfv( GL_LIGHT0 + id, GL_POSITION, position );
            }
            else
            {
                // dir[] actually specifies a relative position in the space
                // a HACK becouse of BUGS
                GLfloat dir[] = { -direction.x - 0.0001f, -direction.y - 0.0001f, -direction.z - 0.0001f, 0.f };
                glLightfv( GL_LIGHT0 + id, GL_POSITION, dir );
            }

            glLightf( GL_LIGHT0 + id, GL_CONSTANT_ATTENUATION, 0.f );
            glLightf( GL_LIGHT0 + id, GL_LINEAR_ATTENUATION, 2.f / range );
            glLightf( GL_LIGHT0 + id, GL_QUADRATIC_ATTENUATION, 0.f );

            /*glLightf( GL_LIGHT0 + id, GL_SPOT_CUTOFF, cutoffAngle / 2 );

            GLfloat dir[] = { direction.x, direction.y, direction.z };
            glLightfv( GL_LIGHT0 + id, GL_SPOT_DIRECTION, dir );*/
        }
    }

    Light* Light::setCutoff( float angle )
    {
        cutoffAngle = angle;

        return this;
    }
}
