//
//  RenderEngine.cpp
//  RavEngine_Static
//
//  Copyright © 2020 Ravbug.
//	Some code adapted from: https://github.com/pezcode/Cluster/blob/master/src/Renderer/DeferredRenderer.cpp
//

#include "RenderEngine.hpp"
#include <string>
#include <fstream>
#include <sstream>
#include <filesystem>
#include "CameraComponent.hpp"
#include "World.hpp"
#include <memory>
#include "Material.hpp"
#include <bgfx/platform.h>
#include <bx/bx.h>
#include "Light.hpp"

#include "RenderableComponent.hpp"
	#ifdef __linux__
	#define SDL_VIDEO_DRIVER_X11 1	//Without this X11 support doesn't work
#endif
#include <SDL_syswm.h>
#include <SDL.h>
#include <SDL_video.h>
#include <im3d.h>
#include <BuiltinMaterials.hpp>
#include "Common3D.hpp"
#include "Debug.hpp"
#include <glm/gtc/type_ptr.hpp>

#ifdef __APPLE_
#include <Cocoa/Cocoa.h>
#endif

using namespace std;
using namespace RavEngine;

SDL_Window* RenderEngine::window = nullptr;
RenderEngine::vs RenderEngine::VideoSettings;

static Ref<DebugMaterialInstance> mat;

static bgfx::VertexLayout pcvDecl;

/**
 Create an SDL window for different platforms, and reference it to bgfx
 @param _window the SDL window pointer (modified)
 @note supported platforms: Linux, BSD, OSX, Windows, SteamLink
 */
inline bool sdlSetWindow(SDL_Window* _window)
{
	SDL_SysWMinfo wmi;
	SDL_VERSION(&wmi.version);
	if (!SDL_GetWindowWMInfo(_window, &wmi)) {
		return false;
	}
	
	bgfx::PlatformData pd;
#if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
	pd.ndt = wmi.info.x11.display;
	pd.nwh = (void*)(uintptr_t)wmi.info.x11.window;
#elif BX_PLATFORM_OSX
	pd.ndt = NULL;
	pd.nwh = wmi.info.cocoa.window;
#elif BX_PLATFORM_WINDOWS
	pd.ndt = NULL;
	pd.nwh = wmi.info.win.window;
#elif BX_PLATFORM_STEAMLINK
	pd.ndt = wmi.info.vivante.display;
	pd.nwh = wmi.info.vivante.window;
#endif // BX_PLATFORM_
	pd.context = NULL;
	pd.backBuffer = NULL;
	pd.backBufferDS = NULL;
	bgfx::setPlatformData(pd);
	
	return true;
}

void DebugRender(const Im3d::DrawList& drawList){
#ifdef _DEBUG
	switch(drawList.m_primType){
		case Im3d::DrawPrimitive_Triangles:
			//Set BGFX state to triangles
			bgfx::setState(BGFX_STATE_DEFAULT);
			break;
		case Im3d::DrawPrimitive_Lines:
			bgfx::setState(0 | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z | BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_CULL_CW | BGFX_STATE_MSAA | BGFX_STATE_PT_LINES);
			//set BGFX state to lines
			break;
		case Im3d::DrawPrimitive_Points:
			//set BGFX state to points
			bgfx::setState(0 | BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z | BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_CULL_CW | BGFX_STATE_MSAA | BGFX_STATE_PT_POINTS);
			break;
		default:
			throw runtime_error("Invalid Im3d state");
			break;
	}
	//perform drawing here
	const Im3d::VertexData* vertexdata = drawList.m_vertexData;
	const size_t verts = drawList.m_vertexCount;
	
	vector<VertexColor> converted;
	converted.reserve(verts);
	
	for(int x = 0; x < verts; x++){
		Im3d::VertexData d = vertexdata[x];
		converted.push_back({d.m_positionSize.x,d.m_positionSize.y,d.m_positionSize.z,d.m_color});
	}
	vector<uint16_t> indices;
	for(int i = 0; i < verts; i++){
		indices.push_back(i);
	}
	
	bgfx::VertexBufferHandle vbuf = bgfx::createVertexBuffer(bgfx::copy(&converted[0], converted.size() * sizeof(VertexColor)), pcvDecl);
	bgfx::IndexBufferHandle ibuf = bgfx::createIndexBuffer(bgfx::copy(&indices[0], indices.size() * sizeof(uint16_t)));
	
	//auto data = DebugDraw::InstanceAt(i);
		
	auto& context = Im3d::GetContext();
		
	auto& drawmatrix = context.getMatrix();
	
	//matrix4 rvm = glm::transpose(glm::make_mat4(drawmatrix.m));
	
	mat->Draw(vbuf,ibuf,matrix4(1));
	bgfx::destroy(vbuf);
	bgfx::destroy(ibuf);
#endif
}

/**
Construct a render engine instance
@param w the owning world for this engine instance
*/
RenderEngine::RenderEngine() {
	//call Init()
	Init();
	mat = new DebugMaterialInstance(Material::Manager::AccessMaterialOfType<DebugMaterial>());
	auto& data = Im3d::GetAppData();
	data.drawCallback = &DebugRender;
	
	//vertex format
	pcvDecl.begin()
	.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
	.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
	.end();
	
	frameBuffer = createFrameBuffer(true,true);
	bgfx::setName(frameBuffer, "Render framebuffer (pre-postprocessing)");
	
	//create the samplers for the textures
	for(size_t i = 0; i < BX_COUNTOF(gBufferSamplers); i++)
	{
		gBufferSamplers[i] = bgfx::createUniform(gBufferSamplerNames[i], bgfx::UniformType::Sampler);
	}
	
	
}

RavEngine::RenderEngine::~RenderEngine()
{
	
}


/**
 Render one frame using the current state of every object in the world
 */
void RenderEngine::Draw(Ref<World> worldOwning){    
	//get the active camera
	auto components = worldOwning->Components();
	auto allcams = components.GetAllComponentsOfType<CameraComponent>();
	
	enum : bgfx::ViewId
	{
		vGeometry = 0,    // write G-Buffer
		vFullscreenLight, // write ambient + emissive to output buffer
		vLight,           // render lights to output buffer
		vTransparent      // forward pass for transparency
	};
	
	constexpr uint32_t BLACK = 0x000000FF;
	
	//touch all the views
//	bgfx::setViewName(vGeometry, "Deferred geometry pass");
//	bgfx::setViewClear(vGeometry, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, BLACK, 1.0f);
//	bgfx::setViewRect(vGeometry, 0, 0, VideoSettings.width, VideoSettings.height);
//	bgfx::setViewFrameBuffer(vGeometry, gBuffer);
//	bgfx::touch(vGeometry);
//	
//	bgfx::setViewName(vFullscreenLight, "Deferred light pass (ambient + emissive)");
//	bgfx::setViewClear(vFullscreenLight, BGFX_CLEAR_COLOR, 0x303030ff);
//	bgfx::setViewRect(vFullscreenLight, 0, 0, VideoSettings.width, VideoSettings.height);
//	bgfx::setViewFrameBuffer(vFullscreenLight, accumFrameBuffer);
//	bgfx::touch(vFullscreenLight);
//	
//	bgfx::setViewName(vLight, "Deferred light pass (point lights)");
//	bgfx::setViewClear(vLight, BGFX_CLEAR_NONE);
//	bgfx::setViewRect(vLight, 0, 0, VideoSettings.width, VideoSettings.height);
//	bgfx::setViewFrameBuffer(vLight, accumFrameBuffer);
//	bgfx::touch(vLight);
//	
//	bgfx::setViewName(vTransparent, "Transparent forward pass");
//	bgfx::setViewClear(vTransparent, BGFX_CLEAR_NONE);
//	bgfx::setViewRect(vTransparent, 0, 0, VideoSettings.width, VideoSettings.height);
//	bgfx::setViewFrameBuffer(vTransparent, accumFrameBuffer);
//	bgfx::touch(vTransparent);
//
	
	//copy into backend matrix
	float viewmat[16];
	float projmat[16];
	const decimalType* vS = (const decimalType*)glm::value_ptr(Material::Manager::GetCurrentViewMatrix());
	const decimalType* pS = (const decimalType*)glm::value_ptr(Material::Manager::GetCurrentProjectionMatrix());
	for (int i = 0; i < 16; ++i) {
		viewmat[i] = vS[i];
		projmat[i] = pS[i];
	}
	
	bgfx::setViewTransform(0, viewmat, projmat);
	//bgfx::setTransform(transmat);
	
	//set the view transform - all entities drawn will use this matrix
	for (auto& cam : allcams) {
		auto owning = Ref<CameraComponent>(cam);
		if (owning->isActive()) {
			int width,height;
			SDL_GL_GetDrawableSize(window, &width, &height);
			owning->SetTargetSize(width, height);
			Material::Manager::SetProjectionMatrix(cam->GenerateProjectionMatrix());
			Material::Manager::SetViewMatrix(cam->GenerateViewMatrix());
			
			break;
		}
	}

    //apply transforms for only entities that need to be rendered
    auto toDraw = components.GetAllComponentsOfSubclass<RenderableComponent>();
	
	//clear buffers
	bgfx::touch(0);
	
	//get all the lights
	auto lights = components.GetAllComponentsOfSubclass<Light>();
	for(const auto& light : lights){
		light->DebugDraw();
	}	

    //iterate through renderables and call Draw
    for (auto& e : toDraw) {
        e->Draw();
    }
	
#ifdef _DEBUG
	Im3d::GetContext().draw();
#endif
	bgfx::frame();

#ifdef _DEBUG
	Im3d::NewFrame();
	Im3d::AppData& data = Im3d::GetAppData();
	data.drawCallback = &DebugRender;
#endif
	bgfx::dbgTextClear();
}

void RenderEngine::resize(){
	if(!bgfx::isValid(gBuffer))
	{
		gBuffer = createGBuffer();
		
		for(size_t i = 0; i < GBufferAttachment::Depth; i++)
		{
			gBufferTextures[i].handle = bgfx::getTexture(gBuffer, (uint8_t)i);
		}
		
		// we can't use the G-Buffer's depth texture in the light pass framebuffer
		// binding a texture for reading in the shader and attaching it to a framebuffer
		// at the same time is undefined behaviour in most APIs
		// https://www.khronos.org/opengl/wiki/Memory_Model#Framebuffer_objects
		// we use a different depth texture and just blit it between the geometry and light pass
		const uint64_t flags = BGFX_TEXTURE_BLIT_DST | gBufferSamplerFlags;
		bgfx::TextureFormat::Enum depthFormat = findDepthFormat(flags);
		lightDepthTexture = bgfx::createTexture2D(bgfx::BackbufferRatio::Equal, false, 1, depthFormat, flags);
		
		gBufferTextures[GBufferAttachment::Depth].handle = lightDepthTexture;
	}
	
	if(!bgfx::isValid(accumFrameBuffer))
	{
		const bgfx::TextureHandle textures[2] = { bgfx::getTexture(frameBuffer, 0),
			bgfx::getTexture(gBuffer, GBufferAttachment::Depth) };
		accumFrameBuffer = bgfx::createFrameBuffer(BX_COUNTOF(textures), textures); // don't destroy textures
	}
	
	int width, height;
	SDL_GL_GetDrawableSize(window, &width, &height);
	bgfx::reset(width, height, GetResetFlags());
	bgfx::setViewRect(0, 0, 0, uint16_t(width), uint16_t(height));
}

/**
@return the name of the current rendering API
*/
const string RenderEngine::currentBackend(){
	
	switch (bgfx::getRendererType()) {
		case bgfx::RendererType::Noop:			return "Disabled";
		case bgfx::RendererType::Direct3D9:		return "DirectX9"; 
		case bgfx::RendererType::Direct3D11:	return "DirectX11";
		case bgfx::RendererType::Direct3D12:	return "DirectX12";
		case bgfx::RendererType::Gnm:			return "GNM";
		case bgfx::RendererType::Metal:			return "Metal";
		case bgfx::RendererType::OpenGL:		return "OpenGL";
		case bgfx::RendererType::OpenGLES:		return "OpenGL ES";
		case bgfx::RendererType::Vulkan:		return "Vulkan";
		case bgfx::RendererType::Nvn:			return "NVN";
		case bgfx::RendererType::WebGPU:		return "WebGPU";
		case bgfx::RendererType::Count: 		return "Error - Count";
	}
}

uint32_t RenderEngine::GetResetFlags(){
	return (VideoSettings.vsync ? BGFX_RESET_VSYNC : BGFX_RESET_NONE) | BGFX_RESET_HIDPI | BGFX_RESET_MSAA_X8;
}

/**
Initialize static singletons. Invoked automatically if needed.
*/
void RenderEngine::Init()
{
	//setup bgfx if it is not already setup
	if (window != nullptr)
	{
		return;
	}
	SDL_Init(0);
	SDL_Init(SDL_INIT_GAMECONTROLLER);
	window = SDL_CreateWindow("RavEngine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 800, 480, SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
	
	bgfx::Init settings;
	
	#ifdef __linux__
	settings.type = bgfx::RendererType::Vulkan;	//use Vulkan on Linux
	#endif
	
	//must be in this order
	sdlSetWindow(window);
	bgfx::renderFrame();
	bgfx::init(settings);
	
	if(!DeferredSupported()){
		throw runtime_error("Cannot initialize graphics: Deferred rendering is not supported on this system");
	}
	
	//TODO: refactor
	int width, height;
	SDL_GL_GetDrawableSize(window, &width, &height);
	bgfx::reset(width, height, GetResetFlags());
	
	// Enable debug text.
	bgfx::setDebug(BGFX_DEBUG_TEXT /*| BGFX_DEBUG_STATS*/);
	
	// Set view 0 clear state.
	bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x303030ff, 1.0f, 0);
	
	bgfx::reset(width, height, GetResetFlags());
	
	bgfx::setViewRect(0, 0, 0, uint16_t(width), uint16_t(height));

	bgfx::setState(BGFX_STATE_DEFAULT);
}

bool RenderEngine::DeferredSupported(){
	const bgfx::Caps* caps = bgfx::getCaps();
	bool supported =  // SDR color attachment
	(caps->formats[bgfx::TextureFormat::BGRA8] & BGFX_CAPS_FORMAT_TEXTURE_FRAMEBUFFER) != 0 &&
	// HDR color attachment
	(caps->formats[bgfx::TextureFormat::RGBA16F] & BGFX_CAPS_FORMAT_TEXTURE_FRAMEBUFFER) != 0 &&
	// blitting depth texture after geometry pass
	(caps->supported & BGFX_CAPS_TEXTURE_BLIT) != 0 &&
	// multiple render targets
	// depth doesn't count as an attachment
	caps->limits.maxFBAttachments >= GBufferAttachment::Count - 1;
	if(!supported)
		return false;
	
	for(bgfx::TextureFormat::Enum format : gBufferAttachmentFormats)
	{
		if((caps->formats[format] & BGFX_CAPS_FORMAT_TEXTURE_FRAMEBUFFER) == 0)
			return false;
	}
	
	return true;
}

bgfx::FrameBufferHandle RenderEngine::createGBuffer()
{
	bgfx::TextureHandle textures[GBufferAttachment::Count];
	
	const uint64_t flags = BGFX_TEXTURE_RT | gBufferSamplerFlags;
	
	for(size_t i = 0; i < GBufferAttachment::Depth; i++)
	{
		assert(bgfx::isTextureValid(0, false, 1, gBufferAttachmentFormats[i], flags));
		textures[i] = bgfx::createTexture2D(bgfx::BackbufferRatio::Equal, false, 1, gBufferAttachmentFormats[i], flags);
	}
	
	bgfx::TextureFormat::Enum depthFormat = findDepthFormat(flags);
	assert(depthFormat != bgfx::TextureFormat::Count);
	textures[Depth] = bgfx::createTexture2D(bgfx::BackbufferRatio::Equal, false, 1, depthFormat, flags);
	
	bgfx::FrameBufferHandle gb = bgfx::createFrameBuffer((uint8_t)GBufferAttachment::Count, textures, true);
	
	if(!bgfx::isValid(gb))
		throw runtime_error("Failed to create G-Buffer");
	else
		bgfx::setName(gb, "G-Buffer");
	return gb;
}

bgfx::TextureFormat::Enum RenderEngine::findDepthFormat(uint64_t textureFlags, bool stencil){
	const bgfx::TextureFormat::Enum depthFormats[] = { bgfx::TextureFormat::D16, bgfx::TextureFormat::D32 };
	
	const bgfx::TextureFormat::Enum depthStencilFormats[] = { bgfx::TextureFormat::D24S8 };
	
	const bgfx::TextureFormat::Enum* formats = stencil ? depthStencilFormats : depthFormats;
	size_t count = stencil ? BX_COUNTOF(depthStencilFormats) : BX_COUNTOF(depthFormats);
	
	bgfx::TextureFormat::Enum depthFormat = bgfx::TextureFormat::Count;
	for(size_t i = 0; i < count; i++)
	{
		if(bgfx::isTextureValid(0, false, 1, formats[i], textureFlags))
		{
			depthFormat = formats[i];
			break;
		}
	}
	
	assert(depthFormat != bgfx::TextureFormat::Enum::Count);
	
	return depthFormat;
}

void RenderEngine::bindGBuffer(){
	//set all the textures as active
	for(size_t i = 0; i < GBufferAttachment::Count; i++)
	{
		bgfx::setTexture(gBufferTextureUnits[i], gBufferSamplers[i], gBufferTextures[i].handle);
	}
}

bgfx::FrameBufferHandle RenderEngine::createFrameBuffer(bool hdr, bool depth)
{
	bgfx::TextureHandle textures[2];
	uint8_t attachments = 0;
	
	const uint64_t samplerFlags = BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT | BGFX_SAMPLER_MIP_POINT |
	BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP;
	
	bgfx::TextureFormat::Enum format =
	hdr ? bgfx::TextureFormat::RGBA16F : bgfx::TextureFormat::BGRA8; // BGRA is often faster (internal GPU format)
	assert(bgfx::isTextureValid(0, false, 1, format, BGFX_TEXTURE_RT | samplerFlags));
	textures[attachments++] =
	bgfx::createTexture2D(bgfx::BackbufferRatio::Equal, false, 1, format, BGFX_TEXTURE_RT | samplerFlags);
	
	if(depth)
	{
		bgfx::TextureFormat::Enum depthFormat = findDepthFormat(BGFX_TEXTURE_RT_WRITE_ONLY | samplerFlags);
		assert(depthFormat != bgfx::TextureFormat::Enum::Count);
		textures[attachments++] = bgfx::createTexture2D(
														bgfx::BackbufferRatio::Equal, false, 1, depthFormat, BGFX_TEXTURE_RT_WRITE_ONLY | samplerFlags);
	}
	
	bgfx::FrameBufferHandle fb = bgfx::createFrameBuffer(attachments, textures, true);
	
	if(!bgfx::isValid(fb))
		throw runtime_error("Failed to create framebuffer");
	
	return fb;
}
