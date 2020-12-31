//
//  RenderEngine.cpp
//  RavEngine_Static
//
//  Copyright © 2020 Ravbug.
//	Some code adapted from: https://github.com/pezcode/Cluster/blob/master/src/Renderer/DeferredRenderer.cpp
//

#include "RenderEngine.hpp"
#include <string>
#include "CameraComponent.hpp"
#include "World.hpp"
#include <memory>
#include "Material.hpp"
#include <bgfx/platform.h>
#include <bx/bx.h>
#include "Light.hpp"
#include "StaticMesh.hpp"
#include "App.hpp"
#include "GUI.hpp"

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
#include "DebugDraw.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <fmt/core.h>
#include <iostream>
#include "Debug.hpp"

using namespace std;
using namespace RavEngine;

SDL_Window* RenderEngine::window = nullptr;
RenderEngine::vs RenderEngine::VideoSettings;
//debug drawing vertex layout
static bgfx::VertexLayout debuglayout;
bgfx::VertexLayout RenderEngine::RmlLayout;

bgfx::VertexBufferHandle RenderEngine::screenSpaceQuadVert = BGFX_INVALID_HANDLE;
bgfx::IndexBufferHandle RenderEngine::screenSpaceQuadInd = BGFX_INVALID_HANDLE;

static Ref<DebugMaterialInstance> mat;

static Ref<RavEngine::DeferredBlitShader> blitShader;
Ref<GUIMaterialInstance> RenderEngine::guiMaterial;

struct bgfx_msghandler : public bgfx::CallbackI{
	static bool diagnostic_logging;
	void fatal(const char *_filePath, uint16_t _line, bgfx::Fatal::Enum _code, const char *_str) override{
		Debug::Fatal("BGFX error {} in {} line {}: {}",_code, _filePath, _line, _str);
	}
	void traceVargs(const char *_filePath, uint16_t _line, const char *_format, va_list _argList) override{
#ifdef _DEBUG
		if(diagnostic_logging){
			Debug::LogTemp("BGFX diagnostic: {} line {}: {}",_filePath, _line, fmt::format(_format, (char*)_argList));
		}
#endif
	}
	void profilerBegin(const char *_name, uint32_t _abgr, const char *_filePath, uint16_t _line) override{
		Debug::Fatal("profiler not implemented");
	}
	void profilerBeginLiteral(const char *_name, uint32_t _abgr, const char *_filePath, uint16_t _line) override{
		Debug::Fatal("profilerliteral not implemented");
	}
	void profilerEnd() override{
		Debug::Fatal("profiler not implemented");
	}
	uint32_t cacheReadSize(uint64_t _id) override{
		return 0;
	}
	bool cacheRead(uint64_t _id, void *_data, uint32_t _size) override{
		return false;
	}
	void cacheWrite(uint64_t _id, const void *_data, uint32_t _size) override{
	}
	void screenShot(const char *_filePath, uint32_t _width, uint32_t _height, uint32_t _pitch, const void *_data, uint32_t _size, bool _yflip) override{
		Debug::Fatal("screenshot not implemented");
	}
	void captureBegin(uint32_t _width, uint32_t _height, uint32_t _pitch, bgfx::TextureFormat::Enum format, bool _yflip) override {
		Debug::Fatal("video capture not implemented");
	}
	void captureEnd() override{
		Debug::Fatal("video capture not implemented");
	}
	void captureFrame(const void *_data, uint32_t _size) override{
		Debug::Fatal("frame capture not implemented");
	}
};
bool bgfx_msghandler::diagnostic_logging = false;

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
			bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z | BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_CULL_CW | BGFX_STATE_MSAA | BGFX_STATE_PT_LINES);
			//set BGFX state to lines
			break;
		case Im3d::DrawPrimitive_Points:
			//set BGFX state to points
			bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z | BGFX_STATE_DEPTH_TEST_LESS | BGFX_STATE_CULL_CW | BGFX_STATE_MSAA | BGFX_STATE_PT_POINTS);
			break;
		default:
			Debug::Fatal("Invalid Im3d state");
			break;
	}
	//perform drawing here
	const Im3d::VertexData* vertexdata = drawList.m_vertexData;
	const auto verts = drawList.m_vertexCount;
	
	stackarray(converted, VertexColor, verts);
	
	for(int x = 0; x < verts; x++){
		Im3d::VertexData d = vertexdata[x];
		converted[x] = {d.m_positionSize.x,d.m_positionSize.y,d.m_positionSize.z,d.m_color};
	}
	
	stackarray(indices, uint16_t, verts);
		for(int i = 0; i < verts; i++){
		indices[i] = i;
	}
	
	bgfx::VertexBufferHandle vbuf = bgfx::createVertexBuffer(bgfx::copy(&converted[0], verts * sizeof(converted[0])), debuglayout);
	bgfx::IndexBufferHandle ibuf = bgfx::createIndexBuffer(bgfx::copy(&indices[0], verts * sizeof(indices[0])));

	mat->Draw(vbuf,ibuf,matrix4(1),RenderEngine::Views::FinalBlit);
	bgfx::destroy(vbuf);
	bgfx::destroy(ibuf);
#endif
}

/**
Construct a render engine instance
@param w the owning world for this engine instance
*/
RenderEngine::RenderEngine() {
	Init();
	mat = new DebugMaterialInstance(Material::Manager::AccessMaterialOfType<DebugMaterial>());
	auto& data = Im3d::GetAppData();
	data.drawCallback = &DebugRender;
	
	int width, height;
	SDL_GL_GetDrawableSize(window, &width, &height);
	dims.width = width;
	dims.height = height;
	
	
	static constexpr uint64_t gBufferSamplerFlags = BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT |
	BGFX_SAMPLER_MIP_POINT | BGFX_SAMPLER_U_CLAMP |
	BGFX_SAMPLER_V_CLAMP;

	//create framebuffers
	const auto gen_framebuffer = [](bgfx::TextureFormat::Enum format) -> bgfx::TextureHandle {
		return bgfx::createTexture2D(bgfx::BackbufferRatio::Equal, false, 1, format, BGFX_TEXTURE_RT | gBufferSamplerFlags);
	};
	constexpr bgfx::TextureFormat::Enum formats[] = { bgfx::TextureFormat::RGBA32F, bgfx::TextureFormat::RGBA32F, bgfx::TextureFormat::RGBA32F, bgfx::TextureFormat::D32F};
	for (int i = 0; i < BX_COUNTOF(formats); i++) {
		attachments[i] = gen_framebuffer(formats[i]);
	}

	//lighting textures - light color, and share depth
	lightingAttachments[0] = gen_framebuffer(bgfx::TextureFormat::RGBA32F);
	lightingAttachments[1] = attachments[3];
	
	for(int i = 0; i < gbufferSize; i++){
		if (!bgfx::isValid(attachments[i])){
			Debug::Fatal("Failed to create gbuffer attachment");
		}
	}

	//create samplers
	constexpr char* buffersamplers[] = { "s_albedo","s_normal","s_pos","s_depth"};
	for (int i = 0; i < BX_COUNTOF(buffersamplers); i++) {
		gBufferSamplers[i] = bgfx::createUniform(buffersamplers[i], bgfx::UniformType::Sampler);
	}
	
	lightingSamplers[0] = bgfx::createUniform("s_light", bgfx::UniformType::Sampler);
	lightingSamplers[1] = gBufferSamplers[3];
	
	//create gbuffer and bind all the textures together
	gBuffer = bgfx::createFrameBuffer(gbufferSize, attachments, true);
	
	lightingBuffer = bgfx::createFrameBuffer(lightingAttachmentsSize, lightingAttachments, true);
	
	if(!bgfx::isValid(gBuffer)){
		Debug::Fatal("Failed to create gbuffer");
	}
	
	bgfx::setViewName(Views::FinalBlit, "Final Blit");
	bgfx::setViewName(Views::DeferredGeo, "Deferred Geometry");
	bgfx::setViewName(Views::Lighting, "Lighting Volumes");
	
	
	bgfx::setViewClear(Views::FinalBlit, BGFX_CLEAR_COLOR);
	bgfx::setViewClear(Views::DeferredGeo, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x000000FF, 1.0f);
	bgfx::setViewClear(Views::Lighting, BGFX_CLEAR_COLOR, 0x000000FF, 1.0f);
}

RavEngine::RenderEngine::~RenderEngine()
{
	bgfx::destroy(gBuffer);	//automatically destroys attached textures
	bgfx::destroy(lightingBuffer);
}

/**
 Render one frame using the current state of every object in the world
 */
void RenderEngine::Draw(Ref<World> worldOwning){    

	for(const auto& view : {Views::FinalBlit, Views::DeferredGeo, Views::Lighting}){
		bgfx::setViewRect(view, 0, 0, dims.width, dims.height);
	}
	
	bgfx::setViewFrameBuffer(Views::DeferredGeo, gBuffer);
	bgfx::setViewFrameBuffer(Views::Lighting, lightingBuffer);
	
	bgfx::touch(Views::DeferredGeo);
	bgfx::touch(Views::Lighting);
	
	//copy into backend matrix
	float viewmat[16], projmat[16];
    
    copyMat4(glm::value_ptr(Material::Manager::GetCurrentViewMatrix()), viewmat);
    copyMat4(glm::value_ptr(Material::Manager::GetCurrentProjectionMatrix()), projmat);
	
	//set the view transform - all entities drawn will use this matrix
	for(int i = 0; i < Views::Count; i++){
		bgfx::setViewTransform(i, viewmat, projmat);
	}
	
	//get the active camera
	auto allcams = worldOwning->GetAllComponentsOfTypeFastPath<CameraComponent>();
	for (const Ref<CameraComponent>& cam : allcams) {
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
		
	//bind gbuffer textures
	for(int i = 0; i < BX_COUNTOF(attachments); i++){
		bgfx::setTexture(i, gBufferSamplers[i], attachments[i]);
	}
	
	auto geometry = worldOwning->GetAllComponentsOfSubclass<StaticMesh>();
   
	//Deferred geometry pass
	for (const Ref<StaticMesh>& e : geometry) {
		bgfx::setState( (BGFX_STATE_DEFAULT & ~BGFX_STATE_CULL_MASK) | BGFX_STATE_CULL_CW );
        e->Draw(Views::DeferredGeo);
	}
	
	DrawLightsOfType<AmbientLight>(*worldOwning.get());
	DrawLightsOfType<DirectionalLight>(*worldOwning.get());
	DrawLightsOfType<PointLight>(*worldOwning.get());
	
	//blit to view 0 using the fullscreen quad
	bgfx::setTexture(0, lightingSamplers[0], lightingAttachments[0]);
	bgfx::setTexture(1, gBufferSamplers[3], attachments[3]);
	
	bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_Z);	//don't clear depth, debug wireframes are drawn forward-style afterwards
    blitShader->Draw(screenSpaceQuadVert, screenSpaceQuadInd,Views::FinalBlit);
	
	//GUI
	//TODO: thread using ECS
	auto guis = worldOwning->GetAllComponentsOfTypeFastPath<GUIComponent>();
	auto size = GetBufferSize();
	for(const Ref<GUIComponent>& gui : guis){
		if(gui->Mode == GUIComponent::RenderMode::Screenspace){
			gui->SetDimensions(size.width, size.height);
		}
		gui->Update();
		gui->Render();	//bgfx state is set in renderer before actual draw calls
	}
	
	
#ifdef _DEBUG
	Im3d::GetContext().draw();
#endif
	//discard all previous state sets
	bgfx::frame();
    bgfx::discard();

#ifdef _DEBUG
	Im3d::NewFrame();
#endif
	bgfx::dbgTextClear();
}

void RenderEngine::resize(){
	
	int width, height;
	SDL_GL_GetDrawableSize(window, &width, &height);
	bgfx::reset(width, height, GetResetFlags());
	bgfx::setViewRect(0, 0, 0, uint16_t(width), uint16_t(height));
	dims.width = width;
	dims.height = height;
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
	return (VideoSettings.vsync ? BGFX_RESET_VSYNC : BGFX_RESET_NONE);
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
	window = SDL_CreateWindow("RavEngine", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, VideoSettings.width, VideoSettings.height, SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
	
	bgfx::Init settings;
	
	#ifdef __linux__
	settings.type = bgfx::RendererType::Vulkan;	//use Vulkan on Linux
	#endif
	
	settings.callback = new bgfx_msghandler;
	
	//must be in this order
	sdlSetWindow(window);
	bgfx::renderFrame();
	bgfx::init(settings);
	
	//TODO: refactor
	int width, height;
	SDL_GL_GetDrawableSize(window, &width, &height);
	bgfx::reset(width, height, GetResetFlags());
	
	// Enable debug text.
	bgfx::setDebug(BGFX_DEBUG_TEXT /*| BGFX_DEBUG_STATS*/);
		
	bgfx::reset(width, height, GetResetFlags());
	
	bgfx::setViewRect(0, 0, 0, uint16_t(width), uint16_t(height));

	bgfx::setState(BGFX_STATE_DEFAULT);
	
	//create screenspace quad
	const uint16_t indices[] = {0,2,1, 2,3,1};
	const Vertex vertices[] = {{-1,-1,0}, {-1,1,0}, {1,-1,0}, {1,1,0}};
	bgfx::VertexLayout vl;
	vl.begin()
	.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
	.end();
	
	screenSpaceQuadVert = bgfx::createVertexBuffer(bgfx::copy(vertices, sizeof(vertices)), vl);
	screenSpaceQuadInd = bgfx::createIndexBuffer(bgfx::copy(indices, sizeof(indices)));
	blitShader = Material::Manager::AccessMaterialOfType<DeferredBlitShader>();
	guiMaterial = new GUIMaterialInstance(Material::Manager::AccessMaterialOfType<GUIMaterial>());
	
	//init lights
	LightManager::Init();
	
#ifdef _DEBUG
	Im3d::AppData& data = Im3d::GetAppData();
	data.drawCallback = &DebugRender;
#endif
	
	//vertex format for debug drawing
	debuglayout.begin()
	.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
	.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
	.end();
	
	//vertex format for ui
	RmlLayout.begin()
	.add(bgfx::Attrib::Position, 2, bgfx::AttribType::Float)
	.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
	.add(bgfx::Attrib::Color0, 4, bgfx::AttribType::Uint8, true)
	.end();
}


bgfx::FrameBufferHandle RenderEngine::createFrameBuffer(bool hdr, bool depth)
{
	bgfx::TextureHandle textures[2];
	uint8_t attachments = 0;
	
	const uint64_t samplerFlags = BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT | BGFX_SAMPLER_MIP_POINT | BGFX_SAMPLER_U_CLAMP | BGFX_SAMPLER_V_CLAMP;
	
	bgfx::TextureFormat::Enum format =
	hdr ? bgfx::TextureFormat::RGBA16F : bgfx::TextureFormat::BGRA8; // BGRA is often faster (internal GPU format)
	assert(bgfx::isTextureValid(0, false, 1, format, BGFX_TEXTURE_RT | samplerFlags));
	textures[attachments++] = bgfx::createTexture2D(bgfx::BackbufferRatio::Equal, false, 1, format, BGFX_TEXTURE_RT | samplerFlags);
	
	if(depth)
	{
		//bgfx::TextureFormat::Enum depthFormat = findDepthFormat(BGFX_TEXTURE_RT_WRITE_ONLY | samplerFlags);
		bgfx::TextureFormat::Enum depthFormat = bgfx::TextureFormat::D32F;
		assert(depthFormat != bgfx::TextureFormat::Enum::Count);
		textures[attachments++] = bgfx::createTexture2D(bgfx::BackbufferRatio::Equal, false, 1, depthFormat, BGFX_TEXTURE_RT_WRITE_ONLY | samplerFlags);
	}
	
	bgfx::FrameBufferHandle fb = bgfx::createFrameBuffer(attachments, textures, true);
	
	if(!bgfx::isValid(fb))
		Debug::Fatal("Failed to create framebuffer");
	
	return fb;
}

RenderEngine::dim RenderEngine::GetBufferSize(){
	dim d;
	SDL_GL_GetDrawableSize(window, &d.width, &d.height);
	return d;
}
