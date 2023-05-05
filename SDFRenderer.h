#pragma once

#include "DXCore.h"
#include "Game.h"
#include "DX12Helper.h"
#include <DirectXMath.h>
#include <wrl/client.h> // Used for ComPtr - a smart pointer for COM objects
#include <memory>

#include "Utils.h"
#include "RenderCore.h"

#include "Camera.h"
#include "GameEntity.h"
#include "SDFEntity.h"
#include "BufferStructs.h"

class Game;

class SDFRenderer 
	: public RenderCore 
{
	/*
	* Need to:
	*	load shaders
	*	--i think just load the shaders in game and pass them here tbh.
	*	set up input layout, the variables that the sdf shaders will need
	* 
	*	set up PostResize here?
	* 
	* I think it makes sense to have the shaders load here, at least for now until we decide it makes sense to implement an asset manager class (a big task...)
	*	
	*/

public:
	
	void Init(
		bool vsync,
		std::shared_ptr<Camera> camera, 
		std::shared_ptr<std::vector<SDFEntity>> entitiesRef
	);

	~SDFRenderer();

	void Render() override;
	SDFMaterial* GetMaterialBuffer() {
			return masterPSData.material;
	}
	RaymarchPSExternalData* GetPSData() {
		return &masterPSData;
	}

	Microsoft::WRL::ComPtr<ID3D12PipelineState> GetPipeState() { return pipelineState; };

private:

	void createTriangleForScreenQuad();

#pragma region Stuff from DXCore/Game

	//from game, not in DXCore at all
	bool vsync;
	std::shared_ptr<Camera> camera;
	//Game* game;
	//definitely would not use traditional entities, but maybe would add another entity type later
	//same with materials, lights, etc

#pragma endregion

	Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderByteCode;
	Microsoft::WRL::ComPtr<ID3DBlob> pixelShaderByteCode;

	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState;

	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW vbView;


	std::shared_ptr<std::vector<SDFEntity>> entities;

	void CreateRootSigAndPipelineState();
	RaymarchPSExternalData masterPSData = {};
	//I would like to split up the above function into these somehow, but there may not be a way to do that without it being a pain. Let's just get this up and running for now
	//void LoadShaders();
	//D3D12_INPUT_ELEMENT_DESC DefineInputLayout();
	//void CreateRootSignature();
	//void CreatePipelineState();

};

