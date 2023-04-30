#include "VRAMSparseSet.hpp"
#include "GetApp.hpp"
#include "App.hpp"
#include "RenderEngine.hpp"

RGLDevicePtr RavEngine::VRAMSparseSet_ImplementationDetails::GetDevice()
{
    return GetApp()->GetRenderEngine().GetDevice();
}
