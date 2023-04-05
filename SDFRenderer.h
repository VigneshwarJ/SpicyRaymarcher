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
#include "Mesh.h"
#include "Material.h"
#include "BufferStructs.h"
#include "Lights.h"
class SDFRenderer //for now, this class is being declared as a friend of DXCore so that we can access the private variables. In the future I want to completely remove the need for Game to inherit from DXCore because this is tiresome
	: public RenderCore //i dont know if this is a good idea yet but sure lets go with this for now
{ //Probably *not* going to work to inherit from dxcore. remember how the asset manager was a pain this way? Probably going to have to do something like that with shaders being initialized in game and then sent over here. ugh
	//other option is to pull out stuff that this needs into its own singleton? that sounds like it could break a lot of stuff though

	/*
	* Need to:
	*	load shaders
	*	--i think just load the shaders in game and pass them here tbh.
	*	set up input layout, the variables that the sdf shaders will need
	*	
	*	set up root signature - which describes the pipeline (buffers, etc)
	*	
	*	set up pipeline state!!!
	* 
	*	set up PostResize here?
	* 
	* I think it makes sense to have the shaders load here, at least for now until we decide it makes sense to implement an asset manager class (a big task...)
	*	
	*/

public:
	
	void Init(
		bool vsync,
		std::shared_ptr<Camera> camera
	);

	~SDFRenderer();

	//going to have this take in some variables Just for Now, because the need to do this will probably change as the project evolves
	void Render(
		std::vector<std::shared_ptr<GameEntity>> entities, 
		float color[4], 
		float sphereSize,
		float lightPos[3],
		float spherePos[3]
		);

	Microsoft::WRL::ComPtr<ID3D12PipelineState> GetPipeState() { return pipelineState; };

private:
	//Microsoft::WRL::ComPtr<ID3D12Device> device;

#pragma region Stuff from DXCore/Game

	//from game, not in DXCore at all
	bool vsync;
	std::shared_ptr<Camera> camera;
	
	//definitely would not use traditional entities, but maybe would add another entity type later
	//same with materials, lights, etc

#pragma endregion

	Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderByteCode;
	Microsoft::WRL::ComPtr<ID3DBlob> pixelShaderByteCode;

	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState;
	D3D12_VERTEX_BUFFER_VIEW vbView;
	D3D12_INDEX_BUFFER_VIEW ibView;


	void CreateRootSigAndPipelineState();
	//I would like to split up the above function into these somehow, but there may not be a way to do that without it being a pain. Let's just get this up and running for now
	//void LoadShaders();
	//D3D12_INPUT_ELEMENT_DESC DefineInputLayout();
	//void CreateRootSignature();
	//void CreatePipelineState();

};

