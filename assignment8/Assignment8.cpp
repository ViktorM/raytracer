#include "assignment8/Assignment8.h"
#include "common/core.h"

std::shared_ptr<Camera> Assignment8::CreateCamera() const
{
    const glm::vec2 resolution = GetImageOutputResolution();
    std::shared_ptr<Camera> camera = std::make_shared<PerspectiveCamera>(resolution.x / resolution.y, 26.6f);
    camera->SetPosition(glm::vec3(0.f, -4.1469f, 0.73693f));
    camera->Rotate(glm::vec3(1.f, 0.f, 0.f), PI / 2.f);
    return camera;
}

std::shared_ptr<Scene> Assignment8::CreateScene2() const
{
	std::shared_ptr<Scene> newScene = std::make_shared<Scene>();

	// Material
	std::shared_ptr<BlinnPhongMaterial> cubeMaterial = std::make_shared<BlinnPhongMaterial>();
	cubeMaterial->SetDiffuse(glm::vec3(1.f, 1.f, 1.f));
	cubeMaterial->SetSpecular(glm::vec3(0.6f, 0.6f, 0.6f), 40.f);

	// Objects
	std::vector<std::shared_ptr<aiMaterial>> loadedMaterials;
	std::vector<std::shared_ptr<MeshObject>> cubeObjects = MeshLoader::LoadMesh("CornellBox/CornellBox-Photon.obj", &loadedMaterials);
	for (size_t i = 0; i < cubeObjects.size(); ++i)
	{
		std::shared_ptr<Material> materialCopy = cubeMaterial->Clone();
		materialCopy->LoadMaterialFromAssimp(loadedMaterials[i]);
//		materialCopy->SetAmbient(glm::vec3(0.f, 0.f, 0.f));
		cubeObjects[i]->SetMaterial(materialCopy);
	}

	std::shared_ptr<SceneObject> cubeSceneObject = std::make_shared<SceneObject>();
	cubeSceneObject->AddMeshObject(cubeObjects);
	cubeSceneObject->Rotate(glm::vec3(1.f, 0.f, 0.f), PI / 2.f);
	cubeSceneObject->CreateAccelerationData(AccelerationTypes::BVH);
	newScene->AddSceneObject(cubeSceneObject);

	// Lights
	std::shared_ptr<Light> pointLight = std::make_shared<PointLight>();
	pointLight->SetPosition(glm::vec3(0.01909f, 0.0101f, 1.77640f));
	pointLight->SetLightColor(glm::vec3(1.f, 1.f, 1.f));
	newScene->AddLight(pointLight);

	return newScene;
}

std::shared_ptr<Scene> Assignment8::CreateScene3() const // main
{
    std::shared_ptr<Scene> newScene = std::make_shared<Scene>();

    // Material
    std::shared_ptr<BlinnPhongMaterial> cubeMaterial = std::make_shared<BlinnPhongMaterial>();
    cubeMaterial->SetDiffuse(glm::vec3(1.f, 1.f, 1.f));
    cubeMaterial->SetSpecular(glm::vec3(0.6f, 0.6f, 0.6f), 40.f);

    // Objects
    std::vector<std::shared_ptr<aiMaterial>> loadedMaterials;
    std::vector<std::shared_ptr<MeshObject>> cubeObjects = MeshLoader::LoadMesh("CornellBox/CornellBox-Assignment8.obj", &loadedMaterials);
    for (size_t i = 0; i < cubeObjects.size(); ++i) 
	{
        std::shared_ptr<Material> materialCopy = cubeMaterial->Clone();
        materialCopy->LoadMaterialFromAssimp(loadedMaterials[i]);
		if (i == -1)
		{
			materialCopy->SetTransmittance(0.8f);
			materialCopy->SetIOR(1.5f);
		}
        cubeObjects[i]->SetMaterial(materialCopy);
    }

    std::shared_ptr<SceneObject> cubeSceneObject = std::make_shared<SceneObject>();
    cubeSceneObject->AddMeshObject(cubeObjects);
    cubeSceneObject->Rotate(glm::vec3(1.f, 0.f, 0.f), PI / 2.f);
    cubeSceneObject->CreateAccelerationData(AccelerationTypes::BVH);
    newScene->AddSceneObject(cubeSceneObject);

    // Lights
    std::shared_ptr<PointLight> pointLight = std::make_shared<PointLight>();
    pointLight->SetPosition(glm::vec3(0.01909f, 0.0101f, 1.97028f));
    pointLight->SetLightColor(glm::vec3(1.f, 1.f, 1.f));
    newScene->AddLight(pointLight);

    return newScene;

}

std::shared_ptr<Scene> Assignment8::CreateScene() const
{
	std::shared_ptr<Scene> newScene = std::make_shared<Scene>();

	// Material
	std::shared_ptr<BlinnPhongMaterial> cubeMaterial = std::make_shared<BlinnPhongMaterial>();
	cubeMaterial->SetDiffuse(glm::vec3(1.f, 1.f, 1.f));
	cubeMaterial->SetSpecular(glm::vec3(0.6f, 0.6f, 0.6f), 40.f);
//	cubeMaterial->SetReflectivity(0.3f);

	// Objects
	std::vector<std::shared_ptr<aiMaterial>> loadedMaterials;
	std::vector<std::shared_ptr<MeshObject>> cubeObjects = MeshLoader::LoadMesh("CornellBox/CornellBox-Sphere.obj", &loadedMaterials);
	for (size_t i = 0; i < cubeObjects.size(); ++i) 
	{
		std::shared_ptr<Material> materialCopy = cubeMaterial->Clone();
		materialCopy->LoadMaterialFromAssimp(loadedMaterials[i]);
		materialCopy->SetAmbient(glm::vec3(0.0f, 0.0f, 0.0f));

		if (i == 0)
		{
			materialCopy->SetReflectivity(0.8f);
		}

		if (i == 1)
		{
			materialCopy->SetTransmittance(0.95f);
			materialCopy->SetIOR(2.f);
		}

		cubeObjects[i]->SetMaterial(materialCopy);
	}

	std::shared_ptr<SceneObject> cubeSceneObject = std::make_shared<SceneObject>();
	cubeSceneObject->AddMeshObject(cubeObjects);
	cubeSceneObject->Rotate(glm::vec3(1.f, 0.f, 0.f), PI / 2.f);
	cubeSceneObject->CreateAccelerationData(AccelerationTypes::BVH);
	newScene->AddSceneObject(cubeSceneObject);

	// Lights
	std::shared_ptr<PointLight> pointLight = std::make_shared<PointLight>();
	pointLight->SetPosition(glm::vec3(0.05909f, 0.0501f, 1.5628f)); // glm::vec3(0.01909f, 0.0101f, 1.5328f)
	pointLight->SetLightColor(glm::vec3(0.1f, 0.3f, 0.1f)); 
	newScene->AddLight(pointLight);

	std::shared_ptr<PointLight> pointLight2 = std::make_shared<PointLight>();
	pointLight2->SetPosition(glm::vec3(-0.005f, -0.01f, 1.5328f));
	pointLight2->SetLightColor(glm::vec3(1.f, 1.f, 1.f));
	newScene->AddLight(pointLight2);

	return newScene;
}

std::shared_ptr<Scene> Assignment8::CreateScene4() const
{
	std::shared_ptr<Scene> newScene = std::make_shared<Scene>();

	// Material
	std::shared_ptr<BlinnPhongMaterial> cubeMaterial = std::make_shared<BlinnPhongMaterial>();
	cubeMaterial->SetDiffuse(glm::vec3(1.f, 1.f, 1.f));
	cubeMaterial->SetSpecular(glm::vec3(0.6f, 0.6f, 0.6f), 40.f);

	// Objects
	std::vector<std::shared_ptr<aiMaterial>> loadedMaterials;
	std::vector<std::shared_ptr<MeshObject>> cubeObjects = MeshLoader::LoadMesh("CornellBox/CornellBox-Water.obj", &loadedMaterials);
	for (size_t i = 0; i < cubeObjects.size(); ++i) 
	{
		std::shared_ptr<Material> materialCopy = cubeMaterial->Clone();
		materialCopy->LoadMaterialFromAssimp(loadedMaterials[i]);
		cubeObjects[i]->SetMaterial(materialCopy);
	}

	std::shared_ptr<SceneObject> cubeSceneObject = std::make_shared<SceneObject>();
	cubeSceneObject->AddMeshObject(cubeObjects);
	cubeSceneObject->Rotate(glm::vec3(1.f, 0.f, 0.f), PI / 2.f);
	cubeSceneObject->CreateAccelerationData(AccelerationTypes::BVH);
	newScene->AddSceneObject(cubeSceneObject);

	// Lights
	std::shared_ptr<PointLight> pointLight = std::make_shared<PointLight>();
	pointLight->SetPosition(glm::vec3(0.01909f, 0.0101f, 1.97028f));
	pointLight->SetLightColor(glm::vec3(1.f, 1.f, 1.f));
	newScene->AddLight(pointLight);

	return newScene;
}

std::shared_ptr<Scene> Assignment8::CreateScene5() const
{
	std::shared_ptr<Scene> newScene = std::make_shared<Scene>();

	// Material
	std::shared_ptr<BlinnPhongMaterial> cubeMaterial = std::make_shared<BlinnPhongMaterial>();
	cubeMaterial->SetDiffuse(glm::vec3(1.f, 1.f, 1.f));
	cubeMaterial->SetSpecular(glm::vec3(0.6f, 0.6f, 0.6f), 40.f);
	cubeMaterial->SetReflectivity(0.3f);

	// Objects
	std::vector<std::shared_ptr<aiMaterial>> loadedMaterials;
	std::vector<std::shared_ptr<MeshObject>> cubeObjects = MeshLoader::LoadMesh("CornellBox/CornellBox-Assignment6-Alt.obj", &loadedMaterials);
	for (size_t i = 0; i < cubeObjects.size(); ++i) {
		std::shared_ptr<Material> materialCopy = cubeMaterial->Clone();
		materialCopy->LoadMaterialFromAssimp(loadedMaterials[i]);
		materialCopy->SetAmbient(glm::vec3(0.f, 0.f, 0.f));
		cubeObjects[i]->SetMaterial(materialCopy);

		std::shared_ptr<SceneObject> cubeSceneObject = std::make_shared<SceneObject>();
		cubeSceneObject->AddMeshObject(cubeObjects[i]);
		cubeSceneObject->Rotate(glm::vec3(1.f, 0.f, 0.f), PI / 2.f);
		cubeSceneObject->CreateAccelerationData(AccelerationTypes::BVH);

		cubeSceneObject->ConfigureAccelerationStructure([](AccelerationStructure* genericAccelerator) {
			BVHAcceleration* accelerator = dynamic_cast<BVHAcceleration*>(genericAccelerator);
			accelerator->SetMaximumChildren(2);
			accelerator->SetNodesOnLeaves(2);
		});

		cubeSceneObject->ConfigureChildMeshAccelerationStructure([](AccelerationStructure* genericAccelerator) {
			BVHAcceleration* accelerator = dynamic_cast<BVHAcceleration*>(genericAccelerator);
			accelerator->SetMaximumChildren(2);
			accelerator->SetNodesOnLeaves(2);
		});

		newScene->AddSceneObject(cubeSceneObject);
	}

	// Lights
	std::shared_ptr<Light> pointLight = std::make_shared<PointLight>();
	pointLight->SetPosition(glm::vec3(0.01909f, 0.0101f, 1.97028f));
	pointLight->SetLightColor(glm::vec3(1.f, 1.f, 1.f));

	AccelerationTypes accType = GetAcceleratingStructureType();
	AccelerationStructure* accStructure = newScene->GenerateAccelerationData(accType);
	if (accType == AccelerationTypes::UNIFORM_GRID)
	{
		UniformGridAcceleration* accelerator = dynamic_cast<UniformGridAcceleration*>(accStructure);
		assert(accelerator);

		// To do: implement setting uniform grid size from application
		accelerator->SetSuggestedGridSize(glm::ivec3(8, 8, 8));
	}

	newScene->AddLight(pointLight);

	return newScene;
}

std::shared_ptr<ColorSampler> Assignment8::CreateSampler() const
{
    std::shared_ptr<JitterColorSampler> jitter = std::make_shared<JitterColorSampler>();
    jitter->SetGridSize(GetGridSize());
    return jitter;
}

std::shared_ptr<class Renderer> Assignment8::CreateRenderer(std::shared_ptr<Scene> scene, std::shared_ptr<ColorSampler> sampler) const
{
//	return std::make_shared<BackwardRenderer>(scene, sampler);
	return std::make_shared<PhotonMappingRenderer>(scene, sampler);
}

bool Assignment8::NotifyNewPixelSample(glm::vec3 inputSampleColor, int sampleIndex)
{
    return true;
}