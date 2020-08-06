/*
-----------------------------------------------------------------------------
This source file is part of OGRE
    (Object-oriented Graphics Rendering Engine)
For the latest info, see http://www.ogre3d.org/

Copyright (c) 2000-2014 Torus Knot Software Ltd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
-----------------------------------------------------------------------------
*/

#ifndef __CompositorPassUav_H__
#define __CompositorPassUav_H__

#include "OgreHeaderPrefix.h"

#include "Compositor/Pass/OgreCompositorPass.h"
#include "Compositor/OgreCompositorCommon.h"
#include "OgreTextureGpuListener.h"

namespace Ogre
{
    namespace v1
    {
        class Rectangle2D;
    }
    class CompositorPassUavDef;

    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup Effects
    *  @{
    */

    /** Implementation of CompositorPass
        This implementation will set UAVs.
    @author
        Matias N. Goldberg
    @version
        1.0
    */
    class _OgreExport CompositorPassUav : public CompositorPass, public TextureGpuListener
    {
        CompositorPassUavDef const *mDefinition;
    protected:
        DescriptorSetUav const *mDescriptorSetUav;

        uint32 calculateNumberUavSlots(void) const;
        void setupDescriptorSetUav(void);
        void destroyDescriptorSetUav(void);

    public:
        CompositorPassUav( const CompositorPassUavDef *definition, CompositorNode *parentNode,
                           const RenderTargetViewDef *rtv );
        virtual ~CompositorPassUav();

        virtual void execute( const Camera *lodCamera );

        virtual void _placeBarriersAndEmulateUavExecution( BoundUav boundUavs[64],
                                                           ResourceAccessMap &uavsAccess,
                                                           ResourceLayoutMap &resourcesLayout );

        virtual void notifyRecreated( const UavBufferPacked *oldBuffer, UavBufferPacked *newBuffer );
        // TextureGpuListener overloads
        virtual void notifyTextureChanged( TextureGpu *texture, TextureGpuListener::Reason reason,
                                           void *extraData );
    };

    /** @} */
    /** @} */
}

#include "OgreHeaderSuffix.h"

#endif
