#pragma once
#include <memory>
#include <vector>
#include <map>
#include <string>
#include "BufferStructs.h"
#include "Transform.h"

#define MAX_PRIMITIVES 50
#define START_BOXES MAX_PRIMITIVES


//struct UISettings
//{
//	UISettings()
//		//:spheres(sphere),
//		:size(5.0f)
//	{}
//	float size;
//	float position[3] = { 0.0, 0.0 , 7.0 };
//	//SDFSphere* spheres = nullptr;
//	unsigned int materialType = 0;
//
//
//	//materials
//	float color[4] = { 0.0, 0.0 , 1.0,1.0 };
//};

//multiple buttons for particular shapes
//enum for shape types
enum class SDFType
{
	Sphere,
	Box,
	Torus
};

struct PrimitiveData
{
	SDFType type;
	std::string name;
	unsigned int idx;
};

//struct PerEntityData
//{
//	int boxCount;
//	int sphereCount;
//	SDFPrimRenderData spherePrims[MAX_SDF_COUNT];
//	SDFPrimRenderData boxPrims[MAX_SDF_COUNT];
//};

class SDFEntity
{
public:
	SDFEntity(int nth, RaymarchPSExternalData* data);

	//SDFEntity():uiSettings(){

	//	psData.color[0] = SDFMaterial{ {1,1,1,1} };
	//}

	bool CanAddPrimitive(int count);

	void AddSphere();
	void AddBox();
	void AddTorus();
	void ChangeSphereSettings(int number);
	void UpdateGUI();
	void ShowSphereSettings(int selectedIndex);
	void ShowBoxSettings(int selectedIndex);
	void ShowTorusSettings(int selectedIndex);

	void DisplaySDFSettings();

	//static SDFEntity* GetSDFEntity() {
	//	if (!s_Entity)
	//	{
	//		s_Entity = std::make_unique<SDFEntity>();
	//	}
	//	return s_Entity.get();
	//}

	float* getLightPos()
	{
		return lightPos;
	}
	//std::shared_ptr<RaymarchPSExternalData> GetRayMarchPSData();
	RaymarchPSExternalData* GetRayMarchPSData();
	//PerEntityData* GetPerEntityData() { return &thisEntData; }
	std::string* GetName() { return &name; };

private:
	std::string name;
	Transform transform;

	unsigned int sphereCount = 0;
	unsigned int boxCount = 0;
	unsigned int torusCount = 0;
	unsigned int roundBoxCount = 0;
	unsigned int cylinderCount = 0;
	unsigned int coneCount = 0;
	float lightPos[3] = { 5.0, 10.0 , 0.0 };
	//UISettings uiSettings;
	static std::unique_ptr<SDFEntity> s_Entity;

	//I dont want to have the primitives hold the names themselves. it doesnt 
	// work well with list boxes, and its unnecessary to send to the shader
	//std::unique_ptr<std::map<std::string, SDFPrimRenderData>> primitives;
	//std::unique_ptr<std::map<std::string, SDFType>> nameToType = std::make_unique<std::map<std::string, SDFType>>();
	////std::unique_ptr<std::vector<SDFPrimitive>> primitives;
	//std::unique_ptr<std::vector<std::string>> primitivesNames = std::make_unique<std::vector<std::string>>();

	std::vector<PrimitiveData> primitives;


	//std::shared_ptr<RaymarchPSExternalData> psData = {}; //better, but wait until the rest of this is fixed
	RaymarchPSExternalData* const masterPSData;

	//PerEntityData thisEntData = {};
};


