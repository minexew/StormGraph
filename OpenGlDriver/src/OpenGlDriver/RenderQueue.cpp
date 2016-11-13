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
    RenderQueue::RenderQueue( OpenGlDriver* driver )
            : driver( driver )
    {
    }

    void RenderQueue::add( IModel* model, const Transform* transforms, size_t numTransforms )
    {
        //iterate2 ( i, model->me
    }

    void RenderQueue::enqueue( Mesh* mesh, const glm::mat4& transform )
    {
        /*Material* material;

        if ( driverShared.useVertexBuffers )
            material = mesh->remoteData->material;
        else
            material = mesh->localData->material;

        if ( material->queueEntry != nullptr )
        {
            material->queueEntry->last->nextQueued = mesh;
            material->queueEntry->last = mesh;
        }
        else
        {
            MatEntry entry = { material, mesh, mesh };

            materials.add( entry );
        }*/
    }

    void RenderQueue::render()
    {
    }

    /*void RenderQueue::addLight( ILight* light, const Transform* transforms, size_t numTransforms )
    {
        light->render( transforms, numTransforms );
    }

    void RenderQueue::run()
    {
    }*/
}
