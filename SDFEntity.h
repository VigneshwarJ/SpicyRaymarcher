#pragma once
#include <memory>
#include <vector>
#include <map>
#include "BufferStructs.h"
#include <string>

struct UISettings
{
	UISettings()
		//:spheres(sphere),
		:sphereSize(5.0f)
	{}
	float color[4] = { 1.0f,1.0f,1.0f,1.0f };;
	float sphereSize;
	float spherePos[3] = { 0.0, 0.0 , 7.0 };
	//SDFSphere* spheres = nullptr;
};

//multiple buttons for particular shapes
//enum for shape types

class SDFEntity
{
public:
	SDFEntity():uiSettings(){
		
	}

	bool CanAddPrimitive();

	void AddSphere();
	void AddBox();
	void ChangeSphereSettings(int number);
	void ShowSphereSettings();

	void DisplaySDFSettings();

	static SDFEntity* GetSDFEntity() {
		if (!s_Entity)
		{
			s_Entity = std::make_unique<SDFEntity>();
		}
		return s_Entity.get();
	}

	float* getLightPos()
	{
		return lightPos;
	}
	RaymarchPSExternalData* GetRayMarchPSData();


private:
	int primitiveCount = 0;
	float lightPos[3] = { 0.0, -10.0 , 0.0 };
	UISettings uiSettings;
	static std::unique_ptr<SDFEntity> s_Entity;

	//I dont want to have the primitives hold the names themselves. it doesnt 
	// work well with list boxes, and its unnecessary to send to the shader
	//std::unique_ptr<std::map<std::string, SDFPrimitive>> primitives;
	//std::unique_ptr<std::vector<SDFPrimitive>> primitives;
	std::unique_ptr<std::vector<std::string>> primitivesNames = std::make_unique<std::vector<std::string>>();

	RaymarchPSExternalData psData = {};
};


