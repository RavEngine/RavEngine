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
#ifndef __RenderToVertexBuffer_H__
#define __RenderToVertexBuffer_H__

#include "OgrePrerequisites.h"
#include "OgreRenderOperation.h"

namespace Ogre {
namespace v1 {
    /** \addtogroup Core
    *  @{
    */
    /** \addtogroup RenderSystem
    *  @{
    */
    /**
        An object which renders geometry to a vertex.
    @remarks
        This is especially useful together with geometry shaders, as you can
        render procedural geometry which will get saved to a vertex buffer for
        reuse later, without regenerating it again. You can also create shaders
        that run on previous results of those shaders, creating stateful 
        shaders.
    */
    class _OgreExport RenderToVertexBuffer
    {
    public:
        /** C'tor */
        RenderToVertexBuffer();
        /** D'tor */
        virtual ~RenderToVertexBuffer();

        /**
           Get the vertex declaration that the pass will output.
           @remarks
           Use this object to set the elements of the buffer. Object will calculate
           buffers on its own. Only one source allowed!
        */
        VertexDeclaration* getVertexDeclaration();

        /**
           Get the maximum number of vertices that the buffer will hold
        */
        unsigned int getMaxVertexCount() const { return mMaxVertexCount; }

        /**
           Set the maximum number of vertices that the buffer will hold
        */
        void setMaxVertexCount(unsigned int maxVertexCount) { mMaxVertexCount = maxVertexCount; }

        /**
           What type of primitives does this object generate?
        */
        OperationType getOperationType() const { return mOperationType; }

        /**
           Set the type of primitives that this object generates
        */
        void setOperationType(OperationType operationType) { mOperationType = operationType; }

        /**
           Set whether this object resets its buffers each time it updates.
        */
        void setResetsEveryUpdate(bool resetsEveryUpdate) { mResetsEveryUpdate = resetsEveryUpdate; }

        /**
           Does this object reset its buffer each time it updates?
        */
        bool getResetsEveryUpdate() const { return mResetsEveryUpdate; }

        /**
           Get the render operation for this buffer
        */
        virtual void getRenderOperation(RenderOperation& op) = 0;

        /**
           Update the contents of this vertex buffer by rendering
        */
        virtual void update(SceneManager* sceneMgr) = 0;

        /**
           Reset the vertex buffer to the initial state. In the next update,
           the source renderable will be used as input.
        */
        virtual void reset() { mResetRequested = true; }

        /**
           Set the source renderable of this object. During the first (and
           perhaps later) update of this object, this object's data will be
           used as input)
        */
        void setSourceRenderable(Renderable* source) { mSourceRenderable = source; }

        /**
           Get the source renderable of this object
        */
        const Renderable* getSourceRenderable() const { return mSourceRenderable; }

        /**
           Get the material which is used to render the geometry into the
           vertex buffer.
        */
        const MaterialPtr& getRenderToBufferMaterial() { return mMaterial; }

        /**
           Set the material name which is used to render the geometry into
           the vertex buffer
        */
        void setRenderToBufferMaterialName(const String& materialName);

    protected:
        OperationType mOperationType;
        bool mResetsEveryUpdate;
        bool mResetRequested;
        MaterialPtr mMaterial;
        Renderable* mSourceRenderable;
        VertexData* mVertexData;
        unsigned int mMaxVertexCount;
    };

    /** @} */
    /** @} */
}
}

#endif
