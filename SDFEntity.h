#pragma once
#include <memory>
#include <vector>
#include <map>
#include <string>
#include "BufferStructs.h"
#include "Transform.h"

#define MAX_PRIMITIVES 50
#define START_BOXES MAX_PRIMITIVES

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

class SDFEntity
{
public:
	SDFEntity(unsigned int nth, RaymarchPSExternalData* data);

	bool CanAddPrimitive(int count);

	void AddSphere();
	void AddBox();
	void AddTorus();
	void UpdateGUI();
	void ShowSphereSettings(int selectedIndex);
	void ShowBoxSettings(int selectedIndex);
	void ShowTorusSettings(int selectedIndex);

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
	
	std::vector<PrimitiveData> primitives;

	RaymarchPSExternalData* const masterPSData;

};


