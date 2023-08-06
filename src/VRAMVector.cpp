#include "VRAMVector.hpp"
#include "GetApp.hpp"
#include "App.hpp"
#include "RenderEngine.hpp"
#include "Debug.hpp"

namespace RavEngine {
	void VRAMVectorBase::TrashOldVector(RGLBufferPtr buffer)
	{
        if (buffer){
            buffer->UnmapMemory();
            if (auto app = GetApp()) {
                app->GetRenderEngine().gcBuffers.enqueue(buffer);
            }
            buffer = nullptr;
        }
	}

    VRAMVectorBase::VRAMVectorBase(){
        if (!GetApp() || !GetApp()->HasRenderEngine()){
            Debug::Fatal("Cannot create VRAMVector -- Render engine has not been initialized yet");
        }
        owningDevice = GetApp()->GetDevice();
    }
}
