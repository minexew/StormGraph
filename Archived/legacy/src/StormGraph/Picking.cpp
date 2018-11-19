
#include <StormGraph/StormGraph.hpp>

namespace StormGraph
{
    Picking::Picking() : pickingProgram( 0 )
    {
        pickingProgram = new Program( "sg_assets/shader/Picking" );
    }

    Picking::~Picking()
    {
        release( pickingProgram );
    }

    void Picking::begin()
    {
        SG_assert( pickingProgram != NULL, "StormGraph.Picking", "begin" )

        glDisable( GL_BLEND );
        glDisable( GL_DITHER );
        glDisable( GL_FOG );
        glDisable( GL_LIGHTING );
        glDisable( GL_TEXTURE_1D );
        glDisable( GL_TEXTURE_2D );
        glDisable( GL_TEXTURE_3D );
        glShadeModel( GL_FLAT );

        pickingProgram->use();

        nextId = 1;
    }

    void Picking::end( unsigned x, unsigned y )
    {
        unsigned id = 0;
        GLint viewport[4];

	    glGetIntegerv( GL_VIEWPORT, viewport );
        glReadPixels( x, viewport[3] - y, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, &id );

        idAt = id & 0xFFFFFF;

        glEnable( GL_BLEND );
        glShadeModel( GL_SMOOTH );

        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    }

    unsigned Picking::generateId()
    {
        return nextId++;
    }

    unsigned Picking::getId() const
    {
        return idAt;
    }
}
