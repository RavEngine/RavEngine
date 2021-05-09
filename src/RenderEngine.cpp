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
#include <RmlUi/Debugger.h>
#include "InputManager.hpp"

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
#include <chrono>

#ifdef __APPLE__
	#include "AppleUtilities.h"
#endif

using namespace std;
using namespace RavEngine;

SDL_Window* RenderEngine::window = nullptr;
RenderEngine::vs RenderEngine::VideoSettings;
//debug drawing vertex layout
static bgfx::VertexLayout debuglayout;
bgfx::VertexLayout RenderEngine::RmlLayout;

bgfx::VertexBufferHandle RenderEngine::screenSpaceQuadVert = BGFX_INVALID_HANDLE;
bgfx::IndexBufferHandle RenderEngine::screenSpaceQuadInd = BGFX_INVALID_HANDLE;
bgfx::ProgramHandle RenderEngine::skinningShaderHandle = BGFX_INVALID_HANDLE, RenderEngine::skinningIdentityShaderHandle = BGFX_INVALID_HANDLE;
decltype(RenderEngine::skinningOutputLayout) RenderEngine::skinningOutputLayout, RenderEngine::skinningInputLayout;

#ifdef _DEBUG
Ref<Entity> RenderEngine::debuggerContext;
Ref<InputManager> RenderEngine::debuggerInput;
phmap::flat_hash_map<uint16_t, RenderEngine::DebugMsg> RenderEngine::debugprints;
SpinLock RenderEngine::dbgmtx;
static DebugDraw dbgdraw;	//for rendering debug primitives
#endif

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
inline bgfx::PlatformData sdlSetWindow(SDL_Window* _window)
{
	SDL_SysWMinfo wmi;
	SDL_VERSION(&wmi.version);
	if (!SDL_GetWindowWMInfo(_window, &wmi)) {
		Debug::Fatal("Cannot get native window information");
	}
	
	bgfx::PlatformData pd;
#if BX_PLATFORM_LINUX || BX_PLATFORM_BSD
	pd.ndt = wmi.info.x11.display;
	pd.nwh = (void*)(uintptr_t)wmi.info.x11.window;
#elif BX_PLATFORM_OSX
	pd.ndt = NULL;
	pd.nwh = cbSetupMetalLayer(wmi.info.cocoa.window);
#elif BX_PLATFORM_IOS
	pd.ndt = NULL;
	pd.nwh = cbSetupMetalLayer(wmi.info.uikit.window);
#elif BX_PLATFORM_WINDOWS
	pd.ndt = NULL;
	pd.nwh = wmi.info.win.window;
#elif BX_PLATFORM_STEAMLINK
	pd.ndt = wmi.info.vivante.display;
	pd.nwh = wmi.info.vivante.window;
#else
	#error This system / display manager is not supported
#endif // BX_PLATFORM_
	pd.context = NULL;
	pd.backBuffer = NULL;
	pd.backBufferDS = NULL;
	
	return pd;
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
	
	bgfx::VertexBufferHandle vbuf;
	{
		maybestackarray(converted, VertexColor, verts)
		for (int x = 0; x < verts; x++) {
			Im3d::VertexData d = vertexdata[x];
			converted[x] = { d.m_positionSize.x,d.m_positionSize.y,d.m_positionSize.z,d.m_color };
		}
		vbuf = bgfx::createVertexBuffer(bgfx::copy(&converted[0], verts * sizeof(converted[0])), debuglayout);
	}
	
	bgfx::IndexBufferHandle ibuf;
	{
		maybestackarray(indices, uint16_t, verts)
		for (int i = 0; i < verts; i++) {
			indices[i] = i;
		}
		ibuf = bgfx::createIndexBuffer(bgfx::copy(&indices[0], verts * sizeof(indices[0])));
	}
	
	mat->Draw(vbuf,ibuf,matrix4(1),RenderEngine::Views::FinalBlit);
	bgfx::destroy(vbuf);
	bgfx::destroy(ibuf);
#endif
}

/**
 The render thread function, invoked on a separate thread
 */
void RenderEngine::runAPIThread(bgfx::PlatformData pd, int width, int height) {
	bgfx::Init settings;

#ifdef __linux__
	settings.type = bgfx::RendererType::Vulkan;	//use Vulkan on Linux
#elif defined _WIN32
	settings.type = bgfx::RendererType::Direct3D12;
#endif

	settings.callback = new bgfx_msghandler;

	//must be in this order
	settings.platformData = pd;
	
	settings.resolution.width = width;
	settings.resolution.height = height;
	settings.resolution.reset = RenderEngine::GetResetFlags();
	settings.resolution.maxFrameLatency = 1;	// 0 = default = 3
	if (!bgfx::init(settings)){
		Debug::Fatal("bgfx::init Failed");
	}

	// Enable debug text.
	bgfx::setDebug(BGFX_DEBUG_TEXT /*| BGFX_DEBUG_STATS*/);

	bgfx::setViewRect(0, 0, 0, uint16_t(width), uint16_t(height));

	bgfx::setState(BGFX_STATE_DEFAULT);

	//this unblocks the static Init thread
	bgfx_thread_finished_init = true;

	//begin main render loop

	while (!render_thread_exit) {
		//do queued tasks
		std::function<void(void)> func;
		while (RenderThreadQueue.try_dequeue(func)) {
			func();
		}

		if (App::Renderer) {			//skip if the App has not set its renderer yet
			//invoke World rendering call
			Ref<World> wtd = worldToDraw.lock();
			if (wtd) {
				if (wtd->newFrame){
#ifdef _DEBUG
					//display debug print messages
					bgfx::dbgTextClear();
					RenderEngine::dbgmtx.lock();
					for(const auto msg : RenderEngine::debugprints){
						bgfx::dbgTextPrintf(0, msg.first, msg.second.color, msg.second.message.c_str());
					}
					RenderEngine::dbgmtx.unlock();
#endif
					auto before = std::chrono::high_resolution_clock::now();
					App::Renderer->Draw(wtd);
					auto after = std::chrono::high_resolution_clock::now();
					currentFrameTime = std::chrono::duration<float, std::chrono::milliseconds::period>(after - before).count();;
				}
				//otherwise this world does not have a new frame ready yet, don't waste time re-rendering the same frame again
			}
		}
	}
	
	bgfx_thread_finished_init = false;

	//bgfx::shutdown must be called on this thread
}

void RenderEngine::BlockUntilFinishDraw() {
	render_thread_exit = true;			//signal to exit render thread loop
	while (bgfx_thread_finished_init);	//block until render engine has finished drawing
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
	if (SDL_Init(SDL_INIT_GAMECONTROLLER | SDL_INIT_EVENTS | SDL_INIT_HAPTIC) != 0){
		Debug::Fatal("Unable to initialize SDL2: {}",SDL_GetError());
	}
	
	window = SDL_CreateWindow("RavEngine", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, VideoSettings.width, VideoSettings.height, SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
	
	if (window == NULL){
		Debug::Fatal("Unable to create main window: {}",SDL_GetError());
	}

	//start the render thread here
	{
		auto pd = sdlSetWindow(RenderEngine::GetWindow());
#if BX_PLATFORM_IOS
		metalLayer = pd.nwh;
#endif
		int width, height;
		SDL_GL_GetDrawableSize(RenderEngine::GetWindow(), &width, &height);
		renderThread.emplace(&RenderEngine::runAPIThread,this,pd, width, height);
		renderThread.value().detach();
	}
	//wait for the render thread to be finished initializing
	while (!bgfx_thread_finished_init);

	//check capabilities
	const auto caps = bgfx::getCaps();
	Debug::Assert(caps->supported & BGFX_CAPS_COMPUTE, "Cannot proceed: this platform does not support compute shaders.");

	//create screenspace quad
	const uint16_t indices[] = { 0,2,1, 2,3,1 };
	const Vertex vertices[] = { {-1,-1,0}, {-1,1,0}, {1,-1,0}, {1,1,0} };
	bgfx::VertexLayout vl;
	vl.begin()
		.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
		.end();

	screenSpaceQuadVert = bgfx::createVertexBuffer(bgfx::copy(vertices, sizeof(vertices)), vl);
	screenSpaceQuadInd = bgfx::createIndexBuffer(bgfx::copy(indices, sizeof(indices)));
	blitShader = Material::Manager::AccessMaterialOfType<DeferredBlitShader>();
	guiMaterial = make_shared<GUIMaterialInstance>(Material::Manager::AccessMaterialOfType<GUIMaterial>());

	//load compute shader for skinning
	{
		vector<uint8_t> shaderdata;
		App::Resources->FileContentsAt("shaders/skincompute/compute.bin", shaderdata);
		const bgfx::Memory* mem = bgfx::copy(&shaderdata[0], shaderdata.size());
		skinningShaderHandle = bgfx::createProgram(bgfx::createShader(mem),true);	//auto destroys shader when program is destroyed

		App::Resources->FileContentsAt("shaders/skinstaticcompute/compute.bin", shaderdata);
		const bgfx::Memory* mem2 = bgfx::copy(&shaderdata[0], shaderdata.size());
		skinningIdentityShaderHandle = bgfx::createProgram(bgfx::createShader(mem2), true);
	}

	//create compute shader buffers
	skinningOutputLayout.begin()
		.add(bgfx::Attrib::Position, 4, bgfx::AttribType::Float)
		.add(bgfx::Attrib::Position, 4, bgfx::AttribType::Float)
		.add(bgfx::Attrib::Position, 4, bgfx::AttribType::Float)
		.add(bgfx::Attrib::Position, 4, bgfx::AttribType::Float)
		.end();
	
	skinningInputLayout.begin()
		.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
		.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
		.add(bgfx::Attrib::Position, 4, bgfx::AttribType::Float)
		.end();

	//init lights
	LightManager::Init();

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


/**
Construct a render engine instance
@param w the owning world for this engine instance
*/
RenderEngine::RenderEngine() {
	Init();

	SDL_GetWindowSize(window, &windowdims.width, &windowdims.height);
	
	mat = make_shared<DebugMaterialInstance>(Material::Manager::AccessMaterialOfType<DebugMaterial>());
	auto& data = Im3d::GetAppData();
	data.drawCallback = &DebugRender;
	
	int width, height;
	SDL_GL_GetDrawableSize(window, &width, &height);
	bufferdims.width = width;
	bufferdims.height = height;
	
	static constexpr uint64_t gBufferSamplerFlags = BGFX_SAMPLER_MIN_POINT | BGFX_SAMPLER_MAG_POINT |
	BGFX_SAMPLER_MIP_POINT | BGFX_SAMPLER_U_CLAMP |
	BGFX_SAMPLER_V_CLAMP;

	//create framebuffers
	const auto gen_framebuffer = [](bgfx::TextureFormat::Enum format) -> bgfx::TextureHandle {
		return bgfx::createTexture2D(bgfx::BackbufferRatio::Equal, false, 1, format, BGFX_TEXTURE_RT | gBufferSamplerFlags);
	};
	constexpr bgfx::TextureFormat::Enum formats[] = { bgfx::TextureFormat::RGBA32F, bgfx::TextureFormat::RGBA16F, bgfx::TextureFormat::RGBA16F, bgfx::TextureFormat::D16F};
	for (int i = 0; i < BX_COUNTOF(formats); i++) {
		attachments[i] = gen_framebuffer(formats[i]);
	}

	//lighting textures - light color, and share depth
	lightingAttachments[0] = gen_framebuffer(bgfx::TextureFormat::RGBA16F);
	lightingAttachments[1] = attachments[3];
	
	for(int i = 0; i < gbufferSize; i++){
		if (!bgfx::isValid(attachments[i])){
			Debug::Fatal("Failed to create gbuffer attachment");
		}
	}

	numRowsUniform = Vector4Uniform("NumObjects");
	//create samplers
	constexpr char const* buffersamplers[] = { "s_albedo","s_normal","s_pos","s_depth"};
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
	
	//render view 0 last
	bgfx::ViewId vieworder[]{Views::DeferredGeo, Views::Lighting, Views::FinalBlit};
	assert(Views::Count == BX_COUNTOF(vieworder));	//if this assertion fails, a view was added but its order was not defined in the list above
	bgfx::setViewOrder(0, Views::Count, vieworder);
}

RavEngine::RenderEngine::~RenderEngine()
{
	bgfx::destroy(gBuffer);	//automatically destroys attached textures
	bgfx::destroy(lightingBuffer);
}

void RenderEngine::DrawNext(Ref<World> world) {
	//mark what world to render
	worldToDraw = world;
}

/**
 Render one frame using the current state of every object in the world
 */
void RenderEngine::Draw(Ref<World> worldOwning){
	for(const auto& view : {Views::FinalBlit, Views::DeferredGeo, Views::Lighting}){
		bgfx::setViewRect(view, 0, 0, bufferdims.width, bufferdims.height);
	}
	
	bgfx::setViewFrameBuffer(Views::DeferredGeo, gBuffer);
	bgfx::setViewFrameBuffer(Views::Lighting, lightingBuffer);
	
	bgfx::touch(Views::DeferredGeo);
	bgfx::touch(Views::Lighting);
	
	//copy world framedata into local copy
	fd = worldOwning->GetFrameData();
	worldOwning->newFrame = false;	//we are processing this frame now
	
	//setup matrices
	float viewmat[16], projmat[16];
	
	copyMat4(glm::value_ptr(fd.viewmatrix),viewmat);
	copyMat4(glm::value_ptr(fd.projmatrix),projmat);
	

	//set the view transform - all entities drawn will use this matrix
	for(int i = 0; i < Views::Count; i++){
		bgfx::setViewTransform(i, viewmat, projmat);
	}

	//bind gbuffer textures
	for (int i = 0; i < BX_COUNTOF(attachments); i++) {
		bgfx::setTexture(i, gBufferSamplers[i], attachments[i]);
	}
	auto execdraw = [&](const auto& row, const auto& skinningfunc, const auto& bindfunc) {
		//call Draw with the staticmesh
		if (std::get<1>(row.first)) {

			skinningfunc(row);

			//fill the buffer using the material to write the material data for each instance
				//get the stride for the material (only needs the matrix, all others are uniforms?
			constexpr auto stride = closest_multiple_of(16 * sizeof(float), 16);
			bgfx::InstanceDataBuffer idb;
			bgfx::allocInstanceDataBuffer(&idb, row.second.items.size(), stride);
			size_t offset = 0;
			for (const auto& mesh : row.second.items) {
				//write the data into the idb
				auto matrix = glm::value_ptr(mesh);
				float* ptr = (float*)(idb.data + offset);

				copyMat4(matrix, ptr);

				offset += stride;
			}
			bgfx::setInstanceDataBuffer(&idb);
			//set BGFX state
			bgfx::setState((BGFX_STATE_DEFAULT & ~BGFX_STATE_CULL_MASK) | BGFX_STATE_CULL_CW);

			bindfunc();
			
			float values[4] = {static_cast<float>(row.second.items.size()),static_cast<float>(std::get<0>(row.first)->GetNumVerts()),0,0};
			numRowsUniform.SetValues(&values, 1);
			std::get<1>(row.first)->Draw(std::get<0>(row.first)->getVertexBuffer(), std::get<0>(row.first)->getIndexBuffer(), matrix4(), Views::DeferredGeo);
		}
	};
		
	//Deferred geometry pass
	//iterate over each row of the table
	unsigned long max_verts = 0;
	unsigned long max_objects = 0;
	for (const auto& row : fd.opaques) {
		auto nverts = std::get<0>(row.first)->GetNumVerts();
		if (nverts > max_verts) {
			max_verts = nverts;
		}
		auto numobjects = row.second.items.size();
		if (numobjects > max_objects) {
			max_objects = numobjects;
		}
	}
	bgfx::DynamicVertexBufferHandle opaquemtxhandle = BGFX_INVALID_HANDLE;
	if (fd.opaques.size() > 0) {
		auto row = fd.opaques.begin();
		opaquemtxhandle = bgfx::createDynamicVertexBuffer(max_verts * max_objects, skinningOutputLayout, BGFX_BUFFER_COMPUTE_WRITE);
		// exec compute shader that writes identity matrix into outputs
			// 1 invocation per vertex per object
				// no inputs
				// output buffer A: posed output transformations for vertices
		bgfx::setBuffer(0, opaquemtxhandle, bgfx::Access::Write);
		float values[4] = {static_cast<float>(max_objects),static_cast<float>(max_verts),0,0};
		numRowsUniform.SetValues(&values, 1);
		bgfx::dispatch(Views::DeferredGeo, skinningIdentityShaderHandle, std::ceil(max_verts / 32.0), std::ceil(max_objects / 16.0), 1);	//vertices x number of objects to pose
	}
	for(const auto& row : fd.opaques){
		execdraw(row, [](const auto& row) {
			//do nothing here
		}, [&opaquemtxhandle]() {
			bgfx::setBuffer(11, opaquemtxhandle, bgfx::Access::Read);
		});
	}
	typedef std::pair<bgfx::DynamicVertexBufferHandle,bgfx::VertexBufferHandle> bufferspair ;
	auto numskinned = fd.skinnedOpaques.size();
	stackarray(buffers,bufferspair, numskinned);
	
	uint16_t idx = 0;
	for (const auto& row : fd.skinnedOpaques) {
		buffers[idx].first = BGFX_INVALID_HANDLE;
		buffers[idx].second = BGFX_INVALID_HANDLE;
		execdraw(row, [idx,&buffers,this](const auto& row) {
			// seed compute shader for skinning
			// input buffer A: skeleton bind pose
			Ref<SkeletonAsset> skeleton = std::get<2>(row.first);
			// input buffer B: vertex weights by bone ID
			auto mesh = std::get<0>(row.first);
			// input buffer C: unposed vertices in mesh
			
			// output buffer A: posed output transformations for vertices
			auto numverts = mesh->GetNumVerts();
			auto numobjects = row.second.items.size();
			auto buf = bgfx::createDynamicVertexBuffer(numverts * 4 * numobjects, skinningOutputLayout, BGFX_BUFFER_COMPUTE_WRITE);
			buffers[idx].first = buf;

			bgfx::setBuffer(0, buf, bgfx::Access::Write);
			bgfx::setBuffer(2, mesh->getWeightsHandle(), bgfx::Access::Read);
			
			struct soafloat{
				float translate[3], scale[3];
				float rotate[4];
			};
			
			//pose SOA values
			if(auto& pose = row.second.skinningdata){
				//convert to float from double
				auto& p = pose.value();
				stackarray(pose_float, soafloat, p.size());
				for(int i = 0; i < pose.value().size(); i++){
					//in case of double mode, need to convert to float
					glm::vec3 translate = p[i].translate, scale = p[i].scale;
					glm::quat rotate = p[i].rotate;
					
					//populate stack array values
					std::memcpy(pose_float[i].translate, glm::value_ptr(translate), sizeof(soafloat::translate));
					std::memcpy(pose_float[i].scale, glm::value_ptr(scale), sizeof(soafloat::scale));
					std::memcpy(pose_float[i].rotate, glm::value_ptr(rotate), sizeof(soafloat::rotate));
				}
				
				auto posebuf = bgfx::createVertexBuffer(bgfx::copy(pose_float, pose.value().size() * sizeof(pose_float[0])), skinningInputLayout);
				buffers[idx].second = posebuf;
				bgfx::setBuffer(1, posebuf, bgfx::Access::Read);
			}
			else{
				//TODO: make bindpose available as SOA
				//bgfx::setBuffer(3, skeleton->getBindposeHandle(), bgfx::Access::Read);
			}
			
			float values[4] = {static_cast<float>(numobjects),static_cast<float>(numverts),0,0};
			numRowsUniform.SetValues(&values, 1);
			
			bgfx::dispatch(Views::DeferredGeo,skinningShaderHandle,std::ceil(numobjects/8.0),std::ceil(numverts/32.0),1);	//vertices x number of objects to pose
		}, [idx,&buffers]() {
			bgfx::setBuffer(11, buffers[idx].first, bgfx::Access::Read);
		});
		idx++;
	}

	// Lighting pass
	bool al = DrawLightsOfType<AmbientLight>(fd.ambients);
	bool dl = DrawLightsOfType<DirectionalLight>(fd.directionals);
	bool pl = DrawLightsOfType<SpotLight>(fd.spots);
	bool sl = DrawLightsOfType<PointLight>(fd.points);
		
	//blit to view 0 using the fullscreen quad
	bgfx::setTexture(0, lightingSamplers[0], lightingAttachments[0]);
	if (al || dl || pl || sl ){
		bgfx::setTexture(1, gBufferSamplers[3], attachments[3]);
	}
	
	bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_Z);	//don't clear depth, debug wireframes are drawn forward-style afterwards
    blitShader->Draw(screenSpaceQuadVert, screenSpaceQuadInd,Views::FinalBlit);
	
	//GUI
	//TODO: thread using ECS?
	auto guis = worldOwning->GetAllComponentsOfType<GUIComponent>();
	auto size = GetBufferSize();
	for(const auto g : guis){
		auto gui = std::static_pointer_cast<GUIComponent>(g);
		if(gui->Mode == GUIComponent::RenderMode::Screenspace){
			gui->SetDimensions(size.width, size.height);
			gui->SetDPIScale(GetDPIScale());
		}
		gui->Update();
		gui->Render();	//bgfx state is set in renderer before actual draw calls
	}
	
	
#ifdef _DEBUG
	//render debug GUI
	auto comp = debuggerContext->GetComponent<GUIComponent>().value();
	comp->SetDimensions(size.width, size.height);
	comp->SetDPIScale(GetDPIScale());
	comp->Update();
	comp->Render();
	
	auto shapesToDraw = worldOwning->GetAllComponentsOfType<IDebugRenderer>();
	for(const auto s : shapesToDraw){
		auto shape = std::static_pointer_cast<IDebugRenderer>(s);
		shape->DrawDebug(dbgdraw);
	}

	Im3d::GetContext().draw();
#endif
	bgfx::frame();
	// deallocate buffers
	for (int i = 0; i < numskinned; i++) {
		if (bgfx::isValid(buffers[i].first)) {
			bgfx::destroy(buffers[i].first);
		}
		if (bgfx::isValid(buffers[i].second)){
			bgfx::destroy(buffers[i].second);
		}
	}
	if (bgfx::isValid(opaquemtxhandle)) {
		bgfx::destroy(opaquemtxhandle);
	}

#ifdef _DEBUG
	Im3d::NewFrame();
#endif
	bgfx::dbgTextClear();
}

void RenderEngine::resize(){
	SDL_GL_GetDrawableSize(window, &bufferdims.width, &bufferdims.height);
	SDL_GetWindowSize(window, &windowdims.width, &windowdims.height);
#if BX_PLATFORM_IOS
	resizeMetalLayer(metalLayer,bufferdims.width, bufferdims.height);	//view must be manually sized on iOS
#endif
	RenderThreadQueue.enqueue([=]() {
		bgfx::reset(bufferdims.width, bufferdims.height, GetResetFlags());
		bgfx::setViewRect(Views::FinalBlit, 0, 0, uint16_t(bufferdims.width), uint16_t(bufferdims.height));
	});
	
}

void RenderEngine::SyncVideoSettings(){
	SDL_SetWindowSize(window, VideoSettings.width, VideoSettings.height);
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

float RavEngine::RenderEngine::GetCurrentFPS()
{
	return 1000.0f / currentFrameTime;
}

float RavEngine::RenderEngine::GetLastFrameTime()
{
	return currentFrameTime;
}

uint32_t RenderEngine::GetResetFlags(){
	return (VideoSettings.vsync ? BGFX_RESET_VSYNC : BGFX_RESET_NONE);
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

#ifdef _DEBUG
void RenderEngine::InitDebugger() const{
	Im3d::AppData& data = Im3d::GetAppData();
	data.drawCallback = &DebugRender;
	
	debuggerContext = make_shared<Entity>();
	auto ctx = debuggerContext->EmplaceComponent<GUIComponent>(10,10);
	
	bool status = Rml::Debugger::Initialise(ctx->context);
	
	debuggerInput = make_shared<InputManager>();
	
	debuggerInput->BindAnyAction(ctx);
	debuggerInput->AddAxisMap("MouseX", Special::MOUSEMOVE_X);
	debuggerInput->AddAxisMap("MouseY", Special::MOUSEMOVE_Y);
	
	debuggerInput->AddAxisMap("ScrollY", Special::MOUSEWHEEL_Y);

	debuggerInput->BindAxis("MouseX", ctx, &GUIComponent::MouseX, CID::ANY, 0);	//no deadzone
	debuggerInput->BindAxis("MouseY", ctx, &GUIComponent::MouseY, CID::ANY, 0);
	debuggerInput->BindAxis("ScrollY", ctx, &GUIComponent::ScrollY, CID::ANY, 0);
}

void RenderEngine::DeactivateDebugger() const{
	debuggerContext = nullptr;
	debuggerInput = nullptr;
}
#endif
