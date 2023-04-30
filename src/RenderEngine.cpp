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
#include "StaticMesh.hpp"
#include "App.hpp"
#include "GUI.hpp"
#include <RmlUi/Debugger.h>
#include "Utilities.hpp"
#include "InputManager.hpp"
#include "AnimatorComponent.hpp"
#include "Skybox.hpp"

	#if defined __linux__ && !defined(__ANDROID__)
	#define SDL_VIDEO_DRIVER_X11 1		//Without this X11 support doesn't work
	#define SDL_VIDEO_DRIVER_WAYLAND 1
#endif
#include <SDL_syswm.h>
#include <SDL.h>
#include <SDL_video.h>
#include <im3d.h>
#include <BuiltinMaterials.hpp>
#include "Common3D.hpp"
#include "DebugDrawer.hpp"
#include <glm/gtc/type_ptr.hpp>
#include <fmt/core.h>
#include <iostream>
#include "Debug.hpp"
#include <chrono>
#include <cstdio>
#include <RGL/RGL.hpp>
#include <RGL/Device.hpp>
#include <RGL/Synchronization.hpp>
#include <RGL/Surface.hpp>
#include <RGL/Swapchain.hpp>
#include <RGL/CommandBuffer.hpp>
#include <RGL/Pipeline.hpp>
#include <RGL/RenderPass.hpp>

#ifdef __APPLE__
	#include "AppleUtilities.h"
#endif

#if _WIN32 && !_UWP
#define _WIN32_WINNT _WIN32_WINNT_WIN10
#include <ShellScalingApi.h>
#pragma comment(lib, "Shcore.lib")
#elif _UWP
#include <winrt/Windows.Graphics.Display.h>
using namespace winrt;
#endif

using namespace std;
using namespace RavEngine;

SDL_Window* RenderEngine::window = nullptr;
RenderEngine::vs RenderEngine::VideoSettings;


#ifndef NDEBUG
static std::optional<GUIComponent> debuggerContext;
STATIC(RenderEngine::debuggerInput);
#endif

static constexpr uint16_t shadowMapSize = 2048;


#ifndef NDEBUG
static DebugDrawer dbgdraw;	//for rendering debug primitives
#endif


void DebugRender(const Im3d::DrawList& drawList){
#if 0
#ifndef NDEBUG
	switch(drawList.m_primType){
		case Im3d::DrawPrimitive_Triangles:
			//Set BGFX state to triangles
			bgfx::setState(BGFX_STATE_DEFAULT);
			break;
		case Im3d::DrawPrimitive_Lines:
			bgfx::setState(BGFX_STATE_WRITE_RGB | BGFX_STATE_WRITE_A | BGFX_STATE_WRITE_Z /*| BGFX_STATE_DEPTH_TEST_LESS*/ | BGFX_STATE_CULL_CW | BGFX_STATE_LINEAA | BGFX_STATE_PT_LINES);
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
		for (uint32_t x = 0; x < verts; x++) {
			Im3d::VertexData d = vertexdata[x];
			converted[x] = { d.m_positionSize.x,d.m_positionSize.y,d.m_positionSize.z,d.m_color };
		}
		vbuf = bgfx::createVertexBuffer(bgfx::copy(&converted[0], verts * sizeof(converted[0])), debuglayout);
	}
	
	bgfx::IndexBufferHandle ibuf;
	{
		maybestackarray(indices, uint16_t, verts)
		for (uint32_t i = 0; i < verts; i++) {
			indices[i] = i;
		}
		ibuf = bgfx::createIndexBuffer(bgfx::copy(&indices[0], verts * sizeof(indices[0])));
	}
	
	mat->Draw(vbuf,ibuf,matrix4(1),RenderEngine::Views::FinalBlit);
	bgfx::destroy(vbuf);
	bgfx::destroy(ibuf);
#endif
#endif
}

/**
Initialize static singletons. Invoked automatically if needed.
*/
void RenderEngine::Init(const AppConfig& config)
{
	//setup RGL if it is not already setup
	if (window != nullptr)
	{
		return;
	}
	
	window = SDL_CreateWindow("RavEngine", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, VideoSettings.width, VideoSettings.height, SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
	
	if (window == NULL){
		Debug::Fatal("Unable to create main window: {}",SDL_GetError());
	}

	RGL::API api = RGL::API::PlatformDefault;
	{
#if _UWP
		size_t n_elt;
		char* envv;
		_dupenv_s(&envv, &n_elt, "RGL_BACKEND");
#else
		auto envv = std::getenv("RGL_BACKEND");
#endif
		if (envv == nullptr) {
			goto cont;
		}
		auto backend = std::string_view(envv);

		const std::unordered_map<std::string_view, RGL::API> apis{
			{"metal", decltype(apis)::value_type::second_type::Metal},
			{"d3d12", decltype(apis)::value_type::second_type::Direct3D12},
			{"vulkan", decltype(apis)::value_type::second_type::Vulkan},
		};

		auto it = apis.find(backend);
		if (it != apis.end()) {
			api = (*it).second;
		}
		else {
			std::cerr << "No backend \"" << backend << "\", expected one of:\n";
			for (const auto& api : apis) {
				std::cout << "\t - " << RGL::APIToString(api.second) << "\n";
			}
		}
	}
cont:

	RGL::InitOptions opt{
		.api = api,
		.engineName = "RavEngine",
	};
	RGL::Init(opt);

	// for debug wireframes
	auto& data = Im3d::GetAppData();
	data.drawCallback = &DebugRender;

#if __APPLE__
	if (!AppleGPUMeetsMinSpec()) {
		char buf[30]{ 0 };
		AppleGPUName(buf, 30);
		Debug::Fatal("Cannot proceed: device \"{}\" is under the minimum spec!", buf);
	}
#endif
	
}


/**
Construct a render engine instance
@param w the owning world for this engine instance
*/
RenderEngine::RenderEngine(const AppConfig& config) {
	Init(config);

	UpdateBufferDims();

	SDL_SysWMinfo wmi;
	SDL_VERSION(&wmi.version);
	if (!SDL_GetWindowWMInfo(window, &wmi)) {
		throw std::runtime_error("Cannot get native window information");
}
	surface = RGL::CreateSurfaceFromPlatformHandle(
#if _UWP
		{ &wmi.info.winrt.window },
#elif _WIN32
		{ &wmi.info.win.window },
#elif TARGET_OS_IPHONE
		{ wmi.info.uikit.window },
#elif __APPLE__
		{ wmi.info.cocoa.window },
#elif __linux__
		{ wmi.info.x11.display, wmi.info.x11.window },
#else
#error Unknown platform
#endif
		true
	);

	device = RGL::IDevice::CreateSystemDefaultDevice();
	mainCommandQueue = device->CreateCommandQueue(RGL::QueueType::AllCommands);
	swapchain = device->CreateSwapchain(surface, mainCommandQueue, bufferdims.width, bufferdims.height);
	mainCommandBuffer = mainCommandQueue->CreateCommandBuffer();
	swapchainFence = device->CreateFence(true);

	createGBuffers();

	// create "fixed-function" pipeline layouts
	lightRenderPipelineLayout = device->CreatePipelineLayout({
		.bindings = {
				{
				.binding = 0,
				.type = RGL::PipelineLayoutDescriptor::LayoutBindingDesc::Type::CombinedImageSampler,
				.stageFlags = RGL::PipelineLayoutDescriptor::LayoutBindingDesc::StageFlags::Fragment,
			},
				{
				.binding = 1,
				.type = RGL::PipelineLayoutDescriptor::LayoutBindingDesc::Type::CombinedImageSampler,
				.stageFlags = RGL::PipelineLayoutDescriptor::LayoutBindingDesc::StageFlags::Fragment,
			},
				{
				.binding = 2,
				.type = RGL::PipelineLayoutDescriptor::LayoutBindingDesc::Type::CombinedImageSampler,
				.stageFlags = RGL::PipelineLayoutDescriptor::LayoutBindingDesc::StageFlags::Fragment,
			},
				{
				.binding = 3,
				.type = RGL::PipelineLayoutDescriptor::LayoutBindingDesc::Type::CombinedImageSampler,
				.stageFlags = RGL::PipelineLayoutDescriptor::LayoutBindingDesc::StageFlags::Fragment,
			},
			{
				.binding = 4,
				.type = RGL::PipelineLayoutDescriptor::LayoutBindingDesc::Type::SampledImage,
				.stageFlags = RGL::PipelineLayoutDescriptor::LayoutBindingDesc::StageFlags::Fragment,
			},
			{
				.binding = 5,
				.type = RGL::PipelineLayoutDescriptor::LayoutBindingDesc::Type::SampledImage,
				.stageFlags = RGL::PipelineLayoutDescriptor::LayoutBindingDesc::StageFlags::Fragment,
			},
			{
				.binding = 6,
				.type = RGL::PipelineLayoutDescriptor::LayoutBindingDesc::Type::SampledImage,
				.stageFlags = RGL::PipelineLayoutDescriptor::LayoutBindingDesc::StageFlags::Fragment,
			},
			{
				.binding = 7,
				.type = RGL::PipelineLayoutDescriptor::LayoutBindingDesc::Type::SampledImage,
				.stageFlags = RGL::PipelineLayoutDescriptor::LayoutBindingDesc::StageFlags::Fragment,
			},
		},
		.boundSamplers = {
			textureSampler,
			textureSampler,
			textureSampler,
			textureSampler,
		},
		.constants = {
				
		}
	});

	// create render passes
	deferredRenderPass = RGL::CreateRenderPass({
		   .attachments = {
			   {
				   .format = colorTexFormat,
				   .loadOp = RGL::LoadAccessOperation::Clear,
				   .storeOp = RGL::StoreAccessOperation::Store,
			   },
			   {
				   .format = normalTexFormat,
				   .loadOp = RGL::LoadAccessOperation::Clear,
				   .storeOp = RGL::StoreAccessOperation::Store,
			   },
			   {
				   .format = posTexFormat,
				   .loadOp = RGL::LoadAccessOperation::Clear,
				   .storeOp = RGL::StoreAccessOperation::Store,
			   },
			   {
				   .format = idTexFormat,
				   .loadOp = RGL::LoadAccessOperation::Clear,
				   .storeOp = RGL::StoreAccessOperation::Store,
			   }

		   },
		   .depthAttachment = RGL::RenderPassConfig::AttachmentDesc{
			   .format = RGL::TextureFormat::D32SFloat,
			   .loadOp = RGL::LoadAccessOperation::Clear,
			   .storeOp = RGL::StoreAccessOperation::Store,
			   .clearColor = {1,1,1,1}
		   }
		});

	lightingRenderPass = RGL::CreateRenderPass({
		.attachments = {
			{
				.format = colorTexFormat,
				.loadOp = RGL::LoadAccessOperation::Clear,
				.storeOp = RGL::StoreAccessOperation::Store,
			}
		},
		.depthAttachment = RGL::RenderPassConfig::AttachmentDesc{
			.format = RGL::TextureFormat::D32SFloat,
			.loadOp = RGL::LoadAccessOperation::Load,
			.storeOp = RGL::StoreAccessOperation::Store,
		}
		});

	finalRenderPass = RGL::CreateRenderPass({
		.attachments = {
			{
				.format = RGL::TextureFormat::BGRA8_Unorm,
				.loadOp = RGL::LoadAccessOperation::Clear,
				.storeOp = RGL::StoreAccessOperation::Store,
			},
		},
		 .depthAttachment = RGL::RenderPassConfig::AttachmentDesc{
			.format = RGL::TextureFormat::D32SFloat,
			.loadOp = RGL::LoadAccessOperation::Load,
			.storeOp = RGL::StoreAccessOperation::Store,
		}
	});
}

void RavEngine::RenderEngine::createGBuffers()
{
	uint32_t width = bufferdims.width;
	uint32_t height = bufferdims.height;
	gcTextures.enqueue(depthStencil);
	gcTextures.enqueue(diffuseTexture);
	gcTextures.enqueue(normalTexture);
	gcTextures.enqueue(positionTexture);
	gcTextures.enqueue(lightingTexture);

	depthStencil = device->CreateTexture({
		.usage = { .DepthStencilAttachment = true },
		.aspect = { .HasDepth = true },
		.width = width,
		.height = height,
		.format = RGL::TextureFormat::D32SFloat,
		.debugName = "Depth Texture"
		}
	);
	diffuseTexture = device->CreateTexture({
		.usage = { .Sampled = true, .ColorAttachment = true },
		.aspect = { .HasColor = true },
		.width = width,
		.height = height,
		.format = colorTexFormat,
		.initialLayout = RGL::ResourceLayout::Undefined,
		.debugName = "Color gbuffer"
		}
	);
	normalTexture = device->CreateTexture({
		.usage = { .Sampled = true, .ColorAttachment = true },
		.aspect = { .HasColor = true },
		.width = width,
		.height = height,
		.format = normalTexFormat,
		.initialLayout = RGL::ResourceLayout::Undefined,
		.debugName = "Normal gbuffer"
		}
	);
	positionTexture = device->CreateTexture({
		.usage = { .Sampled = true, .ColorAttachment = true },
		.aspect = { .HasColor = true },
		.width = width,
		.height = height,
		.format = posTexFormat,
		.initialLayout = RGL::ResourceLayout::Undefined,
		.debugName = "Position gbuffer"
		}
	);
	lightingTexture = device->CreateTexture({
		.usage = { .Sampled = true, .ColorAttachment = true },
		.aspect = { .HasColor = true },
		.width = width,
		.height = height,
		.format = colorTexFormat,
		.initialLayout = RGL::ResourceLayout::Undefined,
		.debugName = "Lighting texture"
		}
	);
}

RavEngine::RenderEngine::~RenderEngine()
{
	swapchainFence->Wait();
	DestroyUnusedResources();
	device->BlockUntilIdle();
}

void RenderEngine::DestroyUnusedResources() {
	// deallocate the resources that have been freed

	auto clear = [](auto& queue) {
		typename std::remove_reference_t<decltype(queue)>::value_type item;
		while (queue.try_dequeue(item)) {}

	};

	clear(gcBuffers);
	clear(gcTextures);
	clear(gcPipelineLayout);
	clear(gcRenderPipeline);
}


/**
 Render one frame using the current state of every object in the world
 */
void RenderEngine::Draw(Ref<RavEngine::World> worldOwning){

	// queue up the next swapchain image as soon as possible, 
	// it will become avaiable in the background
	RGL::SwapchainPresentConfig presentConfig{
	};
	swapchain->GetNextImage(&presentConfig.imageIndex);

	// execute when render fence says its ok
	// did we get the swapchain image yet? if not, block until we do

	swapchainFence->Wait();
	swapchainFence->Reset();
	DestroyUnusedResources();
	mainCommandBuffer->Reset();
	mainCommandBuffer->Begin();

	auto nextimg = swapchain->ImageAtIndex(presentConfig.imageIndex);
	auto nextImgSize = nextimg->GetSize();

	// the on-screen render pass
	// contains the results of the previous stages, as well as the UI, skybox and any debugging primitives
	finalRenderPass->SetAttachmentTexture(0, nextimg);
	finalRenderPass->SetDepthAttachmentTexture(depthStencil.get());

	mainCommandBuffer->TransitionResource(nextimg, RGL::ResourceLayout::Undefined, RGL::ResourceLayout::ColorAttachmentOptimal, RGL::TransitionPosition::Top);
	mainCommandBuffer->BeginRendering(finalRenderPass);

	mainCommandBuffer->SetViewport({
		.width = static_cast<float>(nextImgSize.width),
		.height = static_cast<float>(nextImgSize.height),
	});
	mainCommandBuffer->SetScissor({
		.extent = {nextImgSize.width, nextImgSize.height}
	});

	// start with the skybox
	mainCommandBuffer->BindRenderPipeline(worldOwning->skybox->skyMat->mat->renderPipeline);
	mainCommandBuffer->SetVertexBuffer(worldOwning->skybox->skyMesh->vertexBuffer);
	mainCommandBuffer->SetIndexBuffer(worldOwning->skybox->skyMesh->indexBuffer);

	auto& cam = worldOwning->GetComponent<CameraComponent>();
	auto viewproj = cam.GenerateProjectionMatrix()* cam.GenerateViewMatrix();

	mainCommandBuffer->SetVertexBytes(viewproj , 0);
	mainCommandBuffer->DrawIndexed(worldOwning->skybox->skyMesh->totalIndices);

/*
	worldOwning->Filter([](GUIComponent& gui) {
		gui.Render();	// kicks off commands for rendering UI
	});
	*/
#ifndef NDEBUG
	// process debug shapes
	worldOwning->FilterPolymorphic([](PolymorphicGetResult<IDebugRenderable, World::PolymorphicIndirection> dbg, const PolymorphicGetResult<Transform, World::PolymorphicIndirection> transform) {
		for (int i = 0; i < dbg.size(); i++) {
			auto& ptr = dbg[i];
			if (ptr.debugEnabled) {
				ptr.DebugDraw(dbgdraw, transform[0]);
			}
		}
		});
	Im3d::GetContext().draw();
	/*
	if (debuggerContext) {
		auto& dbg = *debuggerContext;
		dbg.SetDimensions(bufferdims.width, bufferdims.height);
		dbg.SetDPIScale(GetDPIScale());
		dbg.Update();
		dbg.Render();
	}
	*/
	Im3d::NewFrame();
#endif
	mainCommandBuffer->EndRendering();
	mainCommandBuffer->TransitionResource(nextimg, RGL::ResourceLayout::ColorAttachmentOptimal, RGL::ResourceLayout::Present, RGL::TransitionPosition::Bottom);
	mainCommandBuffer->End();

	// show the results to the user
	RGL::CommitConfig commitconfig{
			.signalFence = swapchainFence,
	};
	mainCommandBuffer->Commit(commitconfig);

	swapchain->Present(presentConfig);
}

void RenderEngine::resize(){
	UpdateBufferDims();
	createGBuffers();
#if TARGET_OS_IPHONE
	//view must be manually sized on iOS
	//also this API takes screen points not pixels
	resizeMetalLayer(metalLayer,windowdims.width, windowdims.height);
#endif
	mainCommandQueue->WaitUntilCompleted();
	swapchain->Resize(bufferdims.width, bufferdims.height);
}

void RenderEngine::SyncVideoSettings(){
	SDL_SetWindowSize(window, VideoSettings.width, VideoSettings.height);
}

/**
@return the name of the current rendering API
*/
const string_view RenderEngine::GetCurrentBackendName(){
	
	return RGL::APIToString(RGL::CurrentAPI());
}

float RavEngine::RenderEngine::GetCurrentFPS()
{
	return 1.0f / currentFrameTime;
}

float RavEngine::RenderEngine::GetLastFrameTime()
{
	return currentFrameTime * 1000;
}


void RenderEngine::UpdateBufferDims(){
	// on non-apple platforms this is in pixels, on apple platforms it is in "screen points"
	// which will be dealt with later
	SDL_GetWindowSize(window, &windowdims.width, &windowdims.height);

	// update bufferdims
	bufferdims.width = windowdims.width;
	bufferdims.height = windowdims.height;

	// get the canvas size in pixels
# if _WIN32 && !_UWP
	
	SDL_SysWMinfo wmi;
	SDL_VERSION(&wmi.version);
	if (!SDL_GetWindowWMInfo(window, &wmi)) {
		Debug::Fatal("Cannot get native window information");
	}
	auto monitor = MonitorFromWindow(wmi.info.win.window, MONITOR_DEFAULTTONEAREST);
	DEVICE_SCALE_FACTOR fac;
	if (GetScaleFactorForMonitor(monitor,&fac) == S_OK) {
		win_scalefactor = (static_cast<int>(fac) / 100.0);
	}
	else {
		Debug::Fatal("GetScaleFactorForMonitor failed");
	}
#elif _UWP
	auto dinf = Windows::Graphics::Display::DisplayInformation::GetForCurrentView();
	win_scalefactor = static_cast<int32_t>(dinf.ResolutionScale()) / 100.0;
#elif __APPLE__
	// since iOS and macOS do not use OpenGL we cannot use the GL call here
	// instead we derive it by querying display data
	float scale = GetWindowScaleFactor(window);
	bufferdims.width = windowdims.width * scale;
	bufferdims.height = windowdims.height * scale;
#endif
}

#ifndef NDEBUG
void RenderEngine::InitDebugger() const{
	if (!debuggerContext){
		Im3d::AppData& data = Im3d::GetAppData();
		data.drawCallback = &DebugRender;
		
		debuggerContext.emplace(10,10);
		auto ctxd = (*debuggerContext).GetData();
		bool status = Rml::Debugger::Initialise(ctxd->context);
		
		debuggerInput = make_unique<InputManager>();
		
		debuggerInput->BindAnyAction(ctxd);
		debuggerInput->AddAxisMap("MouseX", Special::MOUSEMOVE_X);
		debuggerInput->AddAxisMap("MouseY", Special::MOUSEMOVE_Y);
		
		debuggerInput->AddAxisMap("ScrollY", Special::MOUSEWHEEL_Y);

		auto dbg = PointerInputBinder<GUIComponent>(&*debuggerContext);
		debuggerInput->BindAxis("MouseX", dbg, &GUIComponent::MouseX, CID::ANY, 0);	//no deadzone
		debuggerInput->BindAxis("MouseY", dbg, &GUIComponent::MouseY, CID::ANY, 0);
		debuggerInput->BindAxis("ScrollY", dbg, &GUIComponent::ScrollY, CID::ANY, 0);
	}
}

void RenderEngine::DeactivateDebugger() const{
	debuggerContext.reset();
	debuggerInput = nullptr;
}
#endif

void RenderEngine::SetWindowMode(WindowMode mode){
    int flag;
    switch(mode){
        case WindowMode::Windowed:
            flag = 0;
            break;
        case WindowMode::BorderlessFullscreen:
            flag = SDL_WINDOW_FULLSCREEN_DESKTOP;
            break;
        case WindowMode::Fullscreen:
            flag = SDL_WINDOW_FULLSCREEN;
            break;
    }
    SDL_SetWindowFullscreen(window, flag);
}
