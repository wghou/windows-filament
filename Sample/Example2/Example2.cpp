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
static SandboxParameters g_params2;
static Config g_config;
static bool g_shadowPlane = false;

static OBJLoader m_loader;
static OBJLoader m_loader2;
static utils::Entity renderable;
static utils::Entity renderable2;

VertexBuffer* vb = nullptr;
IndexBuffer* ib = nullptr;
VertexBuffer* vb2 = nullptr;
IndexBuffer* ib2 = nullptr;


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

	// g_params2
	for (auto& i : g_params2.materialInstance) {
		engine->destroy(i);
	}

	for (auto& i : g_params2.material) {
		engine->destroy(i);
	}

	engine->destroy(g_params2.light);
	EntityManager& em2 = EntityManager::get();
	em2.destroy(g_params2.light);
}

static void setup(Engine* engine, View* view, Scene* scene) {

	g_scene = scene;
	view->setClearColor({ 0.1, 0.125, 0.25, 1.0 });
	//view->setPostProcessingEnabled(false);
	//view->setDepthPrepass(filament::View::DepthPrepass::DISABLED);

	mat4f objTrans = mat4f{ mat3f(0.5f), float3(0.0f, -0.5f, 0.0f) }*
		mat4f::rotation(0, float3(1.0f, 0.0f, 0.0f))*
		mat4f::rotation(0, float3(0.0f, 1.0f, 0.0f))*
		mat4f::rotation(0, float3(0.0f, 0.0f, 1.0f));

	m_loader.loadObj("../assets/models/livers/liver_S1.obj", false);
	g_params.color = { 0.25f, 0.08f, 0.1f };

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
	tcm.setTransform(ei, objTrans *
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


	//////////////////////////////////////////////////////////////
	// obj 2
	g_params2.color = { 0.5f, 0.1f, 0.1f };
	m_loader2.loadObj("../assets/models/livers/liver_S2.obj", false);

	//// create material and light
	createInstances(g_params2, *engine);

	// vertex buffer
	VertexBuffer* vb2 = VertexBuffer::Builder()
		.vertexCount(m_loader2.getNumVertices())
		.bufferCount(2)
		.attribute(VertexAttribute::POSITION, 0, VertexBuffer::AttributeType::FLOAT3)
		.attribute(VertexAttribute::TANGENTS, 1, VertexBuffer::AttributeType::FLOAT4)
		.normalized(VertexAttribute::TANGENTS)
		.build(*engine);
	vb2->setBufferAt(*engine, 0,
		VertexBuffer::BufferDescriptor(m_loader2.getVertices().data(), m_loader2.getVertices().size() * sizeof(float), nullptr));
	vb2->setBufferAt(*engine, 1,
		VertexBuffer::BufferDescriptor(m_loader2.getTangents().data(), m_loader2.getTangents().size() * sizeof(float), nullptr));


	// index buffer
	IndexBuffer* ib2 = IndexBuffer::Builder()
		.indexCount(m_loader2.getFaces().size())
		.bufferType(IndexBuffer::IndexType::USHORT)
		.build(*engine);
	ib2->setBuffer(*engine,
		IndexBuffer::BufferDescriptor(m_loader2.getFaces().data(), m_loader2.getFaces().size() * sizeof(uint16_t), nullptr));

	// create renderable entity
	EntityManager::get().create(1, &renderable2);

	RenderableManager::Builder(1)
		.boundingBox({ {-1.f, -1.f, -1.f}, {1.f, 1.f, 1.f} })
		.geometry(0, RenderableManager::PrimitiveType::TRIANGLES, vb2, ib2, 0, m_loader2.getFaces().size())
		.build(*engine, renderable2);

	// set transform
	auto& tcm2 = engine->getTransformManager();
	auto ei2 = tcm2.getInstance(renderable2);
	tcm2.setTransform(ei2, objTrans *
		tcm2.getWorldTransform(ei2));

	// set material
	g_params2.currentMaterialModel = MATERIAL_LIT;
	// set parameters
	MaterialInstance* materialInstance2 = updateInstances(g_params2, *engine);
	auto& rcm2 = engine->getRenderableManager();
	auto instance2 = rcm2.getInstance(renderable2);
	rcm2.setCastShadows(instance2, g_params2.castShadows);
	rcm2.setMaterialInstanceAt(instance2, 0, materialInstance2);
	scene->addEntity(renderable2);

	// add light
	scene->addEntity(g_params2.light);

	return;
}

static void preRender(filament::Engine*, filament::View* view, filament::Scene*, filament::Renderer*) {
	view->setAntiAliasing(g_params.fxaa ? View::AntiAliasing::FXAA : View::AntiAliasing::NONE);
	view->setDithering(g_params.dithering ? View::Dithering::TEMPORAL : View::Dithering::NONE);
	view->setSampleCount((uint8_t)(g_params.msaa ? 4 : 1));
}

int main(int argc, char *argv[]) {

	g_config.iblDirectory = "../envs/pillars";
	utils::Path filename = "../assets/models/livers/liver_L1.obj";
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