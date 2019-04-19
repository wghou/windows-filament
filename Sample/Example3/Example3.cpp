// Example3.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include <iostream>


#include <string>
#include <map>
#include <vector>

#include <utils/Path.h>

#include <filament/Engine.h>
#include <filament/DebugRegistry.h>
#include <filament/IndirectLight.h>
#include <filament/LightManager.h>
#include <filament/Material.h>
#include <filament/MaterialInstance.h>
#include <filament/RenderableManager.h>
#include <filament/Scene.h>
#include <filament/TransformManager.h>
#include <filament/View.h>

#include <math/mat3.h>
#include <math/mat4.h>
#include <math/vec4.h>
#include <math/norm.h>

#include "Config.h"
#include "IBL.h"
#include "FilamentApp.h"

#include<libsdl2/SDL.h>
#include <libsdl2/SDL_syswm.h>

#include"resources/resources.h"

#include<OBJLoader/OBJLoader.h>

using namespace filament::math;
using namespace filament;
using namespace utils;

struct Vertex {
	float2 position;
};

static const Vertex TRIANGLE_VERTICES[3] =
{	{{1,0}},
	{{cos(M_PI * 2 / 3), sin(M_PI * 2 / 3)}},
	{{cos(M_PI * 4 / 3), sin(M_PI * 4 / 3)}},
};

static constexpr uint16_t TRIANGLE_INDICES[3] = { 0,1,2 };


int main(int argc, char *argv[])
{
	SDL_Window* m_window = nullptr;
	const int x = SDL_WINDOWPOS_CENTERED;
	const int y = SDL_WINDOWPOS_CENTERED;
	const uint32_t windowFlags = SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI;
	m_window = SDL_CreateWindow("test", x, y, 570, 570, windowFlags);

	SDL_SysWMinfo wmi;
	if (SDL_GetWindowWMInfo(m_window, &wmi) != SDL_TRUE)
	{
		std::cerr << "SDL version unsupported!" << std::endl;
	}
	//ASSERT_POSTCONDITION(SDL_GetWindowWMInfo(sdlWindow, &wmi), "SDL version unsupported!");
	HDC win = (HDC)wmi.info.win.hdc;
	void* nativeSwapChain = (void*)win;

	Engine *engine = Engine::create();
	SwapChain* mSwapChain = nullptr;
	mSwapChain = engine->createSwapChain(nativeSwapChain);
	Renderer* renderer = engine->createRenderer();
	
	Camera* camera = engine->createCamera();
	View* view = engine->createView();
	Scene* scene = engine->createScene();

	view->setCamera(camera);
	view->setScene(scene);

	Entity renderable = EntityManager::get().create();

	

	// build a quad
	VertexBuffer* vb = VertexBuffer::Builder()
		.vertexCount(3)
		.bufferCount(1)
		.attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT2, 0, 12)
		.build(*engine);
	vb->setBufferAt(*engine, 0,
	VertexBuffer::BufferDescriptor(TRIANGLE_VERTICES, 36, nullptr));

	IndexBuffer* ib = IndexBuffer::Builder()
		.indexCount(3)
		.bufferType(IndexBuffer::IndexType::USHORT)
		.build(*engine);
	ib->setBuffer(*engine,
		IndexBuffer::BufferDescriptor(TRIANGLE_INDICES, 6, nullptr));

	Material* material = Material::Builder()
		.package((void*)RESOURCES_BAKEDCOLOR_DATA, RESOURCES_BAKEDCOLOR_SIZE)
		.build(*engine);
	MaterialInstance* materialInstance = material->createInstance();

	RenderableManager::Builder(1)
		.boundingBox({ { -1, -1, -1 }, { 1, 1, 1 } })
		.material(0, materialInstance)
		.geometry(0, RenderableManager::PrimitiveType::TRIANGLES, vb, ib, 0, 3)
		.culling(false)
		.build(*engine, renderable);
	scene->addEntity(renderable);

	while (true)
	{
		// TODO: we need better timing or use SDL_GL_SetSwapInterval
		//SDL_Delay(16);

		// beginFrame() returns false if we need to skip a frame
		if (renderer->beginFrame(mSwapChain)) {
			// for each View
			renderer->render(view);
			renderer->endFrame();
		}
	}

	return 0;
}