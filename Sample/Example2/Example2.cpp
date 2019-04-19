// Example2.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
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

#include<OBJLoader/OBJLoader.h>

#include"resources/resources.h"

#include "Example2.h"

using namespace filament::math;
using namespace filament;
using namespace utils;

static Scene* g_scene = nullptr;

static std::map<std::string, MaterialInstance*> g_meshMaterialInstances;
static SandboxParameters g_params;
static Config g_config;
static bool g_shadowPlane = false;

static OBJLoader m_loader;
static utils::Entity renderable;

VertexBuffer* vb = nullptr;
IndexBuffer* ib = nullptr;


static void cleanup(Engine* engine, View*, Scene*) {
	for (const auto& material : g_meshMaterialInstances) {
		engine->destroy(material.second);
	}

	for (auto& i : g_params.materialInstance) {
		engine->destroy(i);
	}

	for (auto& i : g_params.material) {
		engine->destroy(i);
	}

	engine->destroy(g_params.light);
	EntityManager& em = EntityManager::get();
	em.destroy(g_params.light);
}

static void setup(Engine* engine, View* view, Scene* scene) {

	g_scene = scene;
	view->setClearColor({ 0.1, 0.125, 0.25, 1.0 });
	//view->setPostProcessingEnabled(false);
	//view->setDepthPrepass(filament::View::DepthPrepass::DISABLED);

	m_loader.loadObj(g_config.objFilename, false);

	for (int i = 0; i < 12; i++)
	{
		std::cout << "i: " << i << "  position: " << m_loader.getVertices()[i*3+0] << "  " << m_loader.getVertices()[i * 3 + 1]
			<< "  " << m_loader.getVertices()[i * 3 + 2] << std::endl;
		std::cout << "i: " << i << "  tangent: " << m_loader.getTangents()[i * 3 + 0] << "  " << m_loader.getTangents()[i * 3 + 1]
			<< "  " << m_loader.getTangents()[i * 3 + 2] << "  " << m_loader.getTangents()[i * 3 + 3] << std::endl;
	}

	//// create material and light
	createInstances(g_params, *engine);

	// vertex buffer
	VertexBuffer* vb = VertexBuffer::Builder()
		.vertexCount(m_loader.getNumVertices())
		.bufferCount(2)
		.attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT3)
		.attribute(VertexAttribute::TANGENTS, 1, VertexBuffer::AttributeType::FLOAT4)
		.normalized(VertexAttribute::TANGENTS)
		.build(*engine);
	vb->setBufferAt(*engine, 0,
		VertexBuffer::BufferDescriptor(m_loader.getVertices().data(), m_loader.getVertices().size()*sizeof(float), nullptr));
	vb->setBufferAt(*engine, 1,
		VertexBuffer::BufferDescriptor(m_loader.getTangents().data(), m_loader.getTangents().size() * sizeof(float), nullptr));


	// index buffer
	IndexBuffer* ib = IndexBuffer::Builder()
		.indexCount(m_loader.getFaces().size())
		.bufferType(IndexBuffer::IndexType::USHORT)
		.build(*engine);
	ib->setBuffer(*engine,
		IndexBuffer::BufferDescriptor(m_loader.getFaces().data(), m_loader.getFaces().size() * sizeof(uint16_t), nullptr));
	
	// create renderable entity
	EntityManager::get().create(1, &renderable);

	RenderableManager::Builder(1)
		.boundingBox({ {-1.f, -1.f, -1.f}, {1.f, 1.f, 1.f} })
		.geometry(0, RenderableManager::PrimitiveType::TRIANGLES, vb, ib, 0, m_loader.getFaces().size())
		.build(*engine, renderable);

	// set transform
	auto& tcm = engine->getTransformManager();
	auto ei = tcm.getInstance(renderable);
	tcm.setTransform(ei, mat4f{ mat3f(g_config.scale), float3(0.0f, 0.0f, 0.0f) } *
		tcm.getWorldTransform(ei));

	// set material
	g_params.currentMaterialModel = MATERIAL_LIT;
	// set parameters
	MaterialInstance* materialInstance = updateInstances(g_params, *engine);
	auto& rcm = engine->getRenderableManager();
	auto instance = rcm.getInstance(renderable);
	rcm.setCastShadows(instance, g_params.castShadows);
	rcm.setMaterialInstanceAt(instance, 0, materialInstance);
	scene->addEntity(renderable);

	// add light
	scene->addEntity(g_params.light);

	return;
}

static void preRender(filament::Engine*, filament::View* view, filament::Scene*, filament::Renderer*) {
	view->setAntiAliasing(g_params.fxaa ? View::AntiAliasing::FXAA : View::AntiAliasing::NONE);
	view->setDithering(g_params.dithering ? View::Dithering::TEMPORAL : View::Dithering::NONE);
	view->setSampleCount((uint8_t)(g_params.msaa ? 4 : 1));
}

int main(int argc, char *argv[]) {

	g_config.iblDirectory = "../envs/pillars";
	utils::Path filename = "../assets/models/sphere2.obj";
	if (!filename.exists()) {
		std::cerr << "file " << filename << " not found!" << std::endl;
		return 1;
	}
	g_config.objFilename = filename;
	g_config.scale = 1.0f;
	g_config.title = "Material Sandbox";

	FilamentApp& filamentApp = FilamentApp::get();

	filamentApp.animate([](Engine* engine, View* view, double now) {
		//auto& tcm = engine->getTransformManager();
		//tcm.setTransform(tcm.getInstance(renderable),
		//	filament::math::mat4f::rotation(now, filament::math::float3{ 0, 0, 1 }));
		//tcm.setTransform(tcm.getInstance(renderable),
		//	filament::math::mat4f::scaling(3));
	});

	filamentApp.run(g_config, setup, cleanup);

	return 0;
}