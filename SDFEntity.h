#pragma once
#include <memory>
#include <vector>
#include "BufferStructs.h"

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

	void AddSphere();
	void ChangeSphereSettings(int number);
	void TweakSphereSettings();

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
	int sphereCount = 0;
	float lightPos[3] = { 0.0, -10.0 , 0.0 };
	UISettings uiSettings;
	static std::unique_ptr<SDFEntity> s_Entity;

	RaymarchPSExternalData psData = {};
};


