#pragma once
#include <memory>
#include <vector>
#include "BufferStructs.h"

struct SphereSettings
{
	SphereSettings(SDFSphere* sphere)
		:spheres(sphere),
		sphereSize(5.0f)
	{}
	float color[4] = { 1.0f,1.0f,1.0f,1.0f };;
	float sphereSize;
	float spherePos[3] = { 0.0, 0.0 , 7.0 };
	SDFSphere* spheres = nullptr;
};


class SDFEntity
{
public:
	SDFEntity():sphereSetting(psData.spheres){
		
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
	SphereSettings sphereSetting;
	static std::unique_ptr<SDFEntity> s_Entity;

	RaymarchPSExternalData psData = {};
};


