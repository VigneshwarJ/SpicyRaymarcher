#include "Game.h"
#include "Vertex.h"
#include "Input.h"
#include "WICTextureLoader.h"

#include "RaytracingHelper.h"

// Needed for a helper function to read compiled shader files from the hard drive
#pragma comment(lib, "d3dcompiler.lib")
#include <d3dcompiler.h>

// For the DirectX Math library
using namespace DirectX;

// Helper macro for getting a float between min and max
#define RandomRange(min, max) (float)rand() / RAND_MAX * (max - min) + min

// Helper macros for making texture and shader loading code more succinct
//#define LoadTexture(file, srv) CreateWICTextureFromFile(device.Get(), context.Get(), (assetsFolderPath_Wide + file).c_str(), 0, srv.GetAddressOf())
//#define LoadShader(type, file) new type(device.Get(), context.Get(), filepath.c_str())


// --------------------------------------------------------
// Constructor
//
// DXCore (base class) constructor will set up underlying fields.
// DirectX itself, and our window, are not ready yet!
//
// hInstance - the application's OS-level handle (unique ID)
// --------------------------------------------------------
Game::Game(HINSTANCE hInstance)
	: DXCore(
		hInstance,		   // The application's handle
		"DirectX Game",	   // Text for the window's title bar
		1280,			   // Width of the window's client area
		720,			   // Height of the window's client area
		true),			   // Show extra stats (fps) in title bar?
	vsync(false)
{
#if defined(DEBUG) || defined(_DEBUG)
	// Do we want a console window?  Probably only in debug mode
	CreateConsoleWindow(500, 120, 32, 120);
	printf("Console window created successfully.  Feel free to printf() here.\n");
#endif
}

// --------------------------------------------------------
// Destructor - Clean up anything our game has created:
//  - Release all DirectX objects created here
//  - Delete any objects to prevent memory leaks
// --------------------------------------------------------
Game::~Game()
{
	// Note: Since we're using smart pointers (ComPtr),
	// we don't need to explicitly clean up those DirectX objects
	// - If we weren't using smart pointers, we'd need
	//   to call Release() on each DirectX object created in Game

	//dx12 - We need to wait here until the GPU is actually done with its work
	DX12Helper::GetInstance().WaitForGPU();
}

// --------------------------------------------------------
// Called once per program, after DirectX and the window
// are initialized but before the game loop.
// --------------------------------------------------------
void Game::Init()
{
	// Attempt to initialize DXR
	RaytracingHelper::GetInstance().Initialize(
		width,
		height,
		device,
		commandQueue,
		commandList,
		GetFullPathTo_Wide(L"Raytracing.cso"));


	// Seed random
	srand((unsigned int)time(0));

	ibView = {};
	vbView = {};
	// Helper methods for loading shaders, creating some basic
	// geometry to draw and some simple camera matrices.
	//  - You'll be expanding and/or replacing these later
	CreateRootSigAndPipelineState();
	CreateBasicGeometry();

	CreateMaterials();

	CreateEntities();
	PlaceEntities();

	// Last step in raytracing setup is to create the accel structures,
	// which requires mesh data.  Currently just a single mesh is handled!
	CreateAccelerationStructures();

	//RandomLighting();
	Light point = {};
	point.Type = LIGHT_TYPE_POINT;
	point.Position = XMFLOAT3(-5.0f, -5.0f, -5.0f);
	point.Color = XMFLOAT3(0.5f, 0.5f, 0.0f);
	point.Range = 100.0f;
	point.Intensity = 1.0f;

	Light directional = {};
	directional.Type = LIGHT_TYPE_DIRECTIONAL;
	directional.Direction = XMFLOAT3(3.0f, 3.0f, 3.0f);
	directional.Color = XMFLOAT3(1.0f, 1.0f, 1.0f);
	directional.Range = 10.0f;
	directional.Intensity = 2.0f;

	lights.push_back(point);
	lights.push_back(directional);
	lightCount = 2;


	camera = std::make_shared<Camera>(
		0.0f, 0.0f, -5.0f,	// Position
		3.0f,		// Move speed
		1.0f,		// Mouse look
		this->width / (float)this->height); // Aspect ratio

		// Ensure the command list is closed going into Draw for the first time
	commandList->Close();

	raytracing = true;
}

// --------------------------------------------------------
// Loads the two basic shaders, then creates the root signature 
// and pipeline state object for our very basic demo.
// --------------------------------------------------------
void Game::CreateRootSigAndPipelineState()
{
	// Blobs to hold raw shader byte code used in several steps below
	Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderByteCode;
	Microsoft::WRL::ComPtr<ID3DBlob> pixelShaderByteCode;
	// Load shaders
	{
		// Read our compiled vertex shader code into a blob
		// - Essentially just "open the file and plop its contents here"
		D3DReadFileToBlob(GetFullPathTo_Wide(L"VertexShader.cso").c_str(), vertexShaderByteCode.GetAddressOf());
		D3DReadFileToBlob(GetFullPathTo_Wide(L"PixelShader.cso").c_str(), pixelShaderByteCode.GetAddressOf());
	}
	// Input layout
	const unsigned int inputElementCount = 4;
	D3D12_INPUT_ELEMENT_DESC inputElements[inputElementCount] = {};
	{
		inputElements[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
		inputElements[0].Format = DXGI_FORMAT_R32G32B32_FLOAT; // R32 G32 B32 = float3
		inputElements[0].SemanticName = "POSITION"; // Name must match semantic in shader
		inputElements[0].SemanticIndex = 0; // This is the first POSITION semantic
		inputElements[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
		inputElements[1].Format = DXGI_FORMAT_R32G32_FLOAT; // R32 G32 = float2
		inputElements[1].SemanticName = "TEXCOORD";
		inputElements[1].SemanticIndex = 0; // This is the first TEXCOORD semantic
		inputElements[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
		inputElements[2].Format = DXGI_FORMAT_R32G32B32_FLOAT; // R32 G32 B32 = float3
		inputElements[2].SemanticName = "NORMAL";
		inputElements[2].SemanticIndex = 0; // This is the first NORMAL semantic
		inputElements[3].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
		inputElements[3].Format = DXGI_FORMAT_R32G32B32_FLOAT; // R32 G32 B32 = float3
		inputElements[3].SemanticName = "TANGENT";
		inputElements[3].SemanticIndex = 0; // This is the first TANGENT semantic
	}
	// Root Signature
	{
		// Describe the range of CBVs needed for the vertex shader
		D3D12_DESCRIPTOR_RANGE cbvRangeVS = {};
		cbvRangeVS.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
		cbvRangeVS.NumDescriptors = 1;
		cbvRangeVS.BaseShaderRegister = 0;
		cbvRangeVS.RegisterSpace = 0;
		cbvRangeVS.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		// Describe the range of CBVs needed for the pixel shader
		D3D12_DESCRIPTOR_RANGE cbvRangePS = {};
		cbvRangePS.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
		cbvRangePS.NumDescriptors = 1;
		cbvRangePS.BaseShaderRegister = 0;
		cbvRangePS.RegisterSpace = 0;
		cbvRangePS.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		// Create a range of SRV's for textures
		D3D12_DESCRIPTOR_RANGE srvRange = {};
		srvRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		srvRange.NumDescriptors = 4;		// Set to max number of textures at once (match pixel shader!)
		srvRange.BaseShaderRegister = 0;	// Starts at s0 (match pixel shader!)
		srvRange.RegisterSpace = 0;
		srvRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

		// Create the root parameters
		D3D12_ROOT_PARAMETER rootParams[3] = {};

		// CBV table param for vertex shader
		rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
		rootParams[0].DescriptorTable.NumDescriptorRanges = 1;
		rootParams[0].DescriptorTable.pDescriptorRanges = &cbvRangeVS;

		// CBV table param for vertex shader
		rootParams[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParams[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		rootParams[1].DescriptorTable.NumDescriptorRanges = 1;
		rootParams[1].DescriptorTable.pDescriptorRanges = &cbvRangePS;

		// SRV table param
		rootParams[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParams[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		rootParams[2].DescriptorTable.NumDescriptorRanges = 1;
		rootParams[2].DescriptorTable.pDescriptorRanges = &srvRange;

		// Create a single static sampler (available to all pixel shaders at the same slot)
		// Note: This is in lieu of having materials have their own samplers for this demo
		D3D12_STATIC_SAMPLER_DESC anisoWrap = {};
		anisoWrap.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		anisoWrap.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		anisoWrap.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		anisoWrap.Filter = D3D12_FILTER_ANISOTROPIC;
		anisoWrap.MaxAnisotropy = 16;
		anisoWrap.MaxLOD = D3D12_FLOAT32_MAX;
		anisoWrap.ShaderRegister = 0;  // register(s0)
		anisoWrap.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

		D3D12_STATIC_SAMPLER_DESC samplers[] = { anisoWrap };

		// Describe and serialize the root signature
		D3D12_ROOT_SIGNATURE_DESC rootSig = {};
		rootSig.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
		rootSig.NumParameters = ARRAYSIZE(rootParams);
		rootSig.pParameters = rootParams;
		rootSig.NumStaticSamplers = ARRAYSIZE(samplers);
		rootSig.pStaticSamplers = samplers;

		ID3DBlob* serializedRootSig = 0;
		ID3DBlob* errors = 0;

		D3D12SerializeRootSignature(
			&rootSig,
			D3D_ROOT_SIGNATURE_VERSION_1,
			&serializedRootSig,
			&errors);

		// Check for errors during serialization
		if (errors != 0)
		{
			OutputDebugString((char*)errors->GetBufferPointer());
		}

		// Actually create the root sig
		device->CreateRootSignature(
			0,
			serializedRootSig->GetBufferPointer(),
			serializedRootSig->GetBufferSize(),
			IID_PPV_ARGS(rootSignature.GetAddressOf()));
	}
	// Pipeline state
	{
		// Describe the pipeline state
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		// -- Input assembler related ---
		psoDesc.InputLayout.NumElements = inputElementCount;
		psoDesc.InputLayout.pInputElementDescs = inputElements;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		// Root sig
		psoDesc.pRootSignature = rootSignature.Get();
		// -- Shaders (VS/PS) ---
		psoDesc.VS.pShaderBytecode = vertexShaderByteCode->GetBufferPointer();
		psoDesc.VS.BytecodeLength = vertexShaderByteCode->GetBufferSize();
		psoDesc.PS.pShaderBytecode = pixelShaderByteCode->GetBufferPointer();
		psoDesc.PS.BytecodeLength = pixelShaderByteCode->GetBufferSize();
		// -- Render targets ---
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
		psoDesc.SampleDesc.Count = 1;
		psoDesc.SampleDesc.Quality = 0;
		// -- States ---
		psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
		psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
		psoDesc.RasterizerState.DepthClipEnable = true;
		psoDesc.DepthStencilState.DepthEnable = true;
		psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
		psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
		psoDesc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
		psoDesc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
		psoDesc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
		// -- Misc ---
		psoDesc.SampleMask = 0xffffffff;
		// Create the pipe state object
		device->CreateGraphicsPipelineState(&psoDesc,
			IID_PPV_ARGS(pipelineState.GetAddressOf()));

	}
}




// --------------------------------------------------------
// Creates the geometry we're going to draw - a single triangle for now
// --------------------------------------------------------
void Game::CreateBasicGeometry()
{
	// Create some temporary variables to represent colors
	// - Not necessary, just makes things more readable
	XMFLOAT4 red = XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4 green = XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f);
	XMFLOAT4 blue = XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f);

	// Set up the vertices of the triangle we would like to draw
	// - We're going to copy this array, exactly as it exists in memory
	//    over to a DirectX-controlled data structure (the vertex buffer)
	// - Note: Since we don't have a camera or really any concept of
	//    a "3d world" yet, we're simply describing positions within the
	//    bounds of how the rasterizer sees our screen: [-1 to +1] on X and Y
	// - This means (0,0) is at the very center of the screen.
	// - These are known as "Normalized Device Coordinates" or "Homogeneous 
	//    Screen Coords", which are ways to describe a position without
	//    knowing the exact size (in pixels) of the image/window/etc.  
	// - Long story short: Resizing the window also resizes the triangle,
	//    since we're describing the triangle in terms of the window itself
	//Vertex vertices[] =
	//{
	//	{ XMFLOAT3(+0.0f, +0.5f, +0.0f), red },
	//	{ XMFLOAT3(+0.5f, -0.5f, +0.0f), blue },
	//	{ XMFLOAT3(-0.5f, -0.5f, +0.0f), green },
	//};

	//// Set up the indices, which tell us which vertices to use and in which order
	//// - This is somewhat redundant for just 3 vertices (it's a simple example)
	//// - Indices are technically not required if the vertices are in the buffer 
	////    in the correct order and each one will be used exactly once
	//// - But just to see how it's done...
	//unsigned int indices[] = { 0, 1, 2 };


	//// Create the two buffers
	//DX12Helper& dx12Helper = DX12Helper::GetInstance();
	//vertexBuffer = dx12Helper.CreateStaticBuffer(sizeof(Vertex), ARRAYSIZE(vertices), vertices);
	//indexBuffer = dx12Helper.CreateStaticBuffer(sizeof(unsigned int), ARRAYSIZE(indices), indices);
	//// Set up the views
	//vbView.StrideInBytes = sizeof(Vertex);
	//vbView.SizeInBytes = sizeof(Vertex) * ARRAYSIZE(vertices);
	//vbView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	//ibView.Format = DXGI_FORMAT_R32_UINT;
	//ibView.SizeInBytes = sizeof(unsigned int) * ARRAYSIZE(indices);
	//ibView.BufferLocation = indexBuffer->GetGPUVirtualAddress();




}

void Game::CreateEntities()
{
	CreateMeshes();
	entities.push_back(std::make_shared<GameEntity>(meshes[0], materials[2], Transform()));
	entities.push_back(std::make_shared<GameEntity>(meshes[1], materials[1], Transform()));
	entities.push_back(std::make_shared<GameEntity>(meshes[2], materials[0], Transform()));
}

void Game::CreateMeshes()
{
	meshes.push_back(std::make_shared<Mesh>(GetFullPathTo("../../Assets/Models/sphere.obj").c_str(), device));
	meshes.push_back(std::make_shared<Mesh>(GetFullPathTo("../../Assets/Models/cube.obj").c_str(), device));
	meshes.push_back(std::make_shared<Mesh>(GetFullPathTo("../../Assets/Models/helix.obj").c_str(), device));
}

void Game::PlaceEntities()
{
	entities[0]->GetTransform()->SetPosition(2.0f, 0.0f, 0.0f);
	entities[1]->GetTransform()->SetPosition(0.0f, 0.0f, 0.0f);
	entities[2]->GetTransform()->SetPosition(-2.0f, 0.0f, 0.0f);
	//entities[0]->GetTransform()->SetScale(10.0f, 10.0f, 10.0f);
	//entities[1]->GetTransform()->SetScale(10.0f, 10.0f, 10.0f);
	//entities[2]->GetTransform()->SetScale(10.0f, 10.0f, 10.0f);
}

void Game::LoadTextures()
{
	
}

void Game::CreateMaterials()
{
	std::wstring assets = GetFullPathTo_Wide(L"../../Assets");
	std::wstring textures = assets + L"\\Textures";
	D3D12_CPU_DESCRIPTOR_HANDLE albedo =    DX12Helper::GetInstance().LoadTexture((textures + L"\\bronze_albedo.png").c_str());
	D3D12_CPU_DESCRIPTOR_HANDLE metal =     DX12Helper::GetInstance().LoadTexture((textures + L"\\bronze_metal.png").c_str());
	D3D12_CPU_DESCRIPTOR_HANDLE normals =   DX12Helper::GetInstance().LoadTexture((textures + L"\\bronze_normals.png").c_str());
	D3D12_CPU_DESCRIPTOR_HANDLE roughness = DX12Helper::GetInstance().LoadTexture((textures + L"\\bronze_roughness.png").c_str());
	
	//D3D12_CPU_DESCRIPTOR_HANDLE albedo =    DX12Helper::GetInstance().LoadTexture((textures + L"\\cobblestone_albedo.png").c_str());
	//D3D12_CPU_DESCRIPTOR_HANDLE metal =     DX12Helper::GetInstance().LoadTexture((textures + L"\\cobblestone_metal.png").c_str());
	//D3D12_CPU_DESCRIPTOR_HANDLE normals =   DX12Helper::GetInstance().LoadTexture((textures + L"\\cobblestone_normals.png").c_str());
	//D3D12_CPU_DESCRIPTOR_HANDLE roughness = DX12Helper::GetInstance().LoadTexture((textures + L"\\cobblestone_roughness.png").c_str());

	std::shared_ptr<Material> material = std::make_shared<Material>(
		pipelineState,
		DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		DirectX::XMFLOAT2(1.0f, 1.0f),
		DirectX::XMFLOAT2(1.0f, 1.0f)
		);

	material->AddTexture(albedo, 0);
	material->AddTexture(metal, 1);
	material->AddTexture(normals, 2);
	material->AddTexture(roughness, 3);

	material->FinalizeMaterial();

	materials.push_back(material);

	albedo =	DX12Helper::GetInstance().LoadTexture((textures + L"\\cobblestone_albedo.png").c_str());
	metal =		DX12Helper::GetInstance().LoadTexture((textures + L"\\cobblestone_metal.png").c_str());
	normals =	DX12Helper::GetInstance().LoadTexture((textures + L"\\cobblestone_normals.png").c_str());
	roughness = DX12Helper::GetInstance().LoadTexture((textures + L"\\cobblestone_roughness.png").c_str());

	//D3D12_CPU_DESCRIPTOR_HANDLE albedo =    DX12Helper::GetInstance().LoadTexture((textures + L"\\cobblestone_albedo.png").c_str());
	//D3D12_CPU_DESCRIPTOR_HANDLE metal =     DX12Helper::GetInstance().LoadTexture((textures + L"\\cobblestone_metal.png").c_str());
	//D3D12_CPU_DESCRIPTOR_HANDLE normals =   DX12Helper::GetInstance().LoadTexture((textures + L"\\cobblestone_normals.png").c_str());
	//D3D12_CPU_DESCRIPTOR_HANDLE roughness = DX12Helper::GetInstance().LoadTexture((textures + L"\\cobblestone_roughness.png").c_str());

	material = std::make_shared<Material>(
		pipelineState,
		DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		DirectX::XMFLOAT2(1.0f, 1.0f),
		DirectX::XMFLOAT2(1.0f, 1.0f)
		);

	material->AddTexture(albedo, 0);
	material->AddTexture(metal, 1);
	material->AddTexture(normals, 2);
	material->AddTexture(roughness, 3);

	material->FinalizeMaterial();

	materials.push_back(material);

	albedo =	DX12Helper::GetInstance().LoadTexture((textures + L"\\Planet1\\planet1_albedo.tif").c_str());
	metal =		DX12Helper::GetInstance().LoadTexture((textures + L"\\Planet1\\planet1_metalic.tif").c_str());
	normals =	DX12Helper::GetInstance().LoadTexture((textures + L"\\Planet1\\planet1_normal.tif").c_str());
	roughness = DX12Helper::GetInstance().LoadTexture((textures + L"\\Planet1\\planet1_roughness.tif").c_str());

	//D3D12_CPU_DESCRIPTOR_HANDLE albedo =    DX12Helper::GetInstance().LoadTexture((textures + L"\\cobblestone_albedo.png").c_str());
	//D3D12_CPU_DESCRIPTOR_HANDLE metal =     DX12Helper::GetInstance().LoadTexture((textures + L"\\cobblestone_metal.png").c_str());
	//D3D12_CPU_DESCRIPTOR_HANDLE normals =   DX12Helper::GetInstance().LoadTexture((textures + L"\\cobblestone_normals.png").c_str());
	//D3D12_CPU_DESCRIPTOR_HANDLE roughness = DX12Helper::GetInstance().LoadTexture((textures + L"\\cobblestone_roughness.png").c_str());

	material = std::make_shared<Material>(
		pipelineState,
		DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
		DirectX::XMFLOAT2(1.0f, 1.0f),
		DirectX::XMFLOAT2(1.0f, 1.0f)
		);

	material->AddTexture(albedo, 0);
	material->AddTexture(metal, 1);
	material->AddTexture(normals, 2);
	material->AddTexture(roughness, 3);

	material->FinalizeMaterial();

	materials.push_back(material);

}

void Game::CreateAccelerationStructures()
{
	//RaytracingHelper::GetInstance().CreateAccelerationStructures(meshes[0]);
	//RaytracingHelper::GetInstance().CreateAccelerationStructures(meshes[1]);
	RaytracingHelper::GetInstance().CreateAccelerationStructures(meshes[2]);
}

void Game::RandomLighting()
{
	// Reset
	lights.clear();
	//for (int i = 0; i < MAX_LIGHTS; i++)
	for (int i = 0; i < 10; i++)
	{
		Light light = RandomPointLight(
			5.0f,
			10.0f,
			2.0f,
			3.0f
		);
		lights.push_back(light);
		lightCount = i + 1;
	}

	// Make sure we're exactly MAX_LIGHTS big
	//lights.resize(MAX_LIGHTS);
}

Light Game::RandomPointLight(float minRange, float maxRange, float minIntensity, float maxIntensity)
{
	Light point = {};
	point.Type = LIGHT_TYPE_POINT;
	point.Position = XMFLOAT3(RandomRange(-10.0f, 10.0f), RandomRange(-5.0f, 5.0f), RandomRange(-10.0f, 10.0f));
	point.Color = XMFLOAT3(RandomRange(0, 1), RandomRange(0, 1), RandomRange(0, 1));
	point.Range = RandomRange(minRange, maxRange);
	point.Intensity = RandomRange(minIntensity, maxIntensity);

	return point;
}

// --------------------------------------------------------
// Handle resizing DirectX "stuff" to match the new window size.
// For instance, updating our projection matrix's aspect ratio.
// --------------------------------------------------------
void Game::OnResize()
{
	// Handle base-level DX resize stuff
	DXCore::OnResize();
}

// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	camera->Update(deltaTime);
	
	for (int i = 0; i < entities.size(); i++)
	{
		entities[i]->GetTransform()->Rotate(0.0f, 0.3f * deltaTime, 0.0f);
	}

	if (Input::GetInstance().KeyDown('R')) { 
		raytracing != raytracing; 
	}
	// Example input checking: Quit if the escape key is pressed
	if (Input::GetInstance().KeyDown(VK_ESCAPE))
		Quit();
}

// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	DX12Helper& dx12HelperInst = DX12Helper::GetInstance();

	//reset allocator for THIS buffer and set up the command list to use THIS allocator for THIS buffer
	commandAllocators[currentSwapBuffer]->Reset();
	commandList->Reset(commandAllocators[currentSwapBuffer].Get(), 0);

	// Grab the current back buffer for this frame
	Microsoft::WRL::ComPtr<ID3D12Resource> currentBackBuffer = backBuffers[currentSwapBuffer];

	if (!raytracing)
	{
		// Clearing the render target
		{
			// Transition the back buffer from present to render target
			D3D12_RESOURCE_BARRIER rb = {};
			rb.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
			rb.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
			rb.Transition.pResource = currentBackBuffer.Get();
			rb.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
			rb.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
			rb.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
			commandList->ResourceBarrier(1, &rb);
			// Background color (Cornflower Blue in this case) for clearing
			float color[] = { 0.4f, 0.6f, 0.75f, 1.0f };
			// Clear the RTV
			commandList->ClearRenderTargetView(
				rtvHandles[currentSwapBuffer],
				color,
				0, 0); // No scissor rectangles
				// Clear the depth buffer, too
			commandList->ClearDepthStencilView(
				dsvHandle,
				D3D12_CLEAR_FLAG_DEPTH,
				1.0f, // Max depth = 1.0f
				0, // Not clearing stencil, but need a value
				0, 0); // No scissor rects
		}

		// Rendering here!
		{
			// Set overall pipeline state
			commandList->SetPipelineState(pipelineState.Get());
			// Root sig (must happen before root descriptor table)
			commandList->SetGraphicsRootSignature(rootSignature.Get());
			// Set up other commands for rendering
			commandList->OMSetRenderTargets(1, &rtvHandles[currentSwapBuffer], true, &dsvHandle);
			commandList->RSSetViewports(1, &viewport);
			commandList->RSSetScissorRects(1, &scissorRect);
			commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

			Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descriptorHeap =
				dx12HelperInst.GetCBVSRVDescriptorHeap();
			commandList->SetDescriptorHeaps(1, descriptorHeap.GetAddressOf());

			// Draw
			for (int i = 0; i < entities.size(); i++)
			{
				std::shared_ptr<GameEntity> thisEntity = entities[i];
				std::shared_ptr<Material> mat = thisEntity->GetMaterial();

				commandList->SetPipelineState(mat->GetPipelineState().Get());
				//vert shader setup
				{
					VertexShaderExternalData externalData = {};
					externalData.world = thisEntity->GetTransform()->GetWorldMatrix();
					externalData.worldInverseTranspose = thisEntity->GetTransform()->GetWorldInverseTransposeMatrix();
					externalData.view = camera->GetView();
					externalData.projection = camera->GetProjection();

					//send to a chunk of a constant buffer heap, and grab the GPU handle we need to draw
					D3D12_GPU_DESCRIPTOR_HANDLE handleVS = dx12HelperInst.FillNextConstantBufferAndGetGPUDescriptorHandle((void*)(&externalData), sizeof(externalData));

					commandList->SetGraphicsRootDescriptorTable(0, handleVS);
				}
				// Pixel shader data and cbuffer setup
				{
					PixelShaderExternalData psData = {};
					psData.colorTint = mat->GetColorTint();
					psData.uvScale = mat->GetUVScale();
					psData.uvOffset = mat->GetUVOffset();
					psData.cameraPosition = camera->GetTransform()->GetPosition();
					psData.lightCount = lightCount;
					memcpy(psData.lights, &lights[0], sizeof(Light) * MAX_LIGHTS);
					// Send this to a chunk of the constant buffer heap
					// and grab the GPU handle for it so we can set it for this draw
					D3D12_GPU_DESCRIPTOR_HANDLE cbHandlePS =
						dx12HelperInst.FillNextConstantBufferAndGetGPUDescriptorHandle(
							(void*)(&psData), sizeof(PixelShaderExternalData));
					// Set this constant buffer handle
					// Note: This assumes that descriptor table 1 is the
					// place to put this particular descriptor. This
					// is based on how we set up our root signature.
					commandList->SetGraphicsRootDescriptorTable(1, cbHandlePS);
				}

				// Set the SRV descriptor handle for this material's textures
				// Note: This assumes that descriptor table 2 is for textures (as per our root sig)
				commandList->SetGraphicsRootDescriptorTable(2, mat->GetFinalGPUHandleForTextures());

				// Grab the vertex buffer view and index buffer view from this entity�s mesh
				vbView = thisEntity->GetMesh()->GetVertBufferView();
				ibView = thisEntity->GetMesh()->GetIndexBufferView();

				//Set them using IASetVertexBuffers() and IASetIndexBuffer()
				commandList->IASetVertexBuffers(0, 1, &vbView);
				commandList->IASetIndexBuffer(&ibView);

				// Call DrawIndexedInstanced() using the index count of this entity�s mesh
				commandList->DrawIndexedInstanced(thisEntity->GetMesh()->GetIndexCount(), 1, 0, 0, 0); //first is the PER INSTANCE index count. second is HOW MANY of the INSTANCES themselves
			}



			//Present
			{
				// Transition back to present
				D3D12_RESOURCE_BARRIER rb = {};
				rb.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
				rb.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
				rb.Transition.pResource = currentBackBuffer.Get();
				rb.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
				rb.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
				rb.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
				commandList->ResourceBarrier(1, &rb);
				// Must occur BEFORE present
				dx12HelperInst.CloseAndExecuteCommandList();
				// Present the current back buffer
				swapChain->Present(vsync ? 1 : 0, 0);
				//// Figure out which buffer is next
				//currentSwapBuffer++;
				//if (currentSwapBuffer >= numBackBuffers)
				//	currentSwapBuffer = 0;
				currentSwapBuffer = dx12HelperInst.SyncSwapChain(currentSwapBuffer);
			}
		}
	}
	else
	{

		// RAYTRACING HERE
		RaytracingHelper::GetInstance().Raytrace(camera, backBuffers[currentSwapBuffer], currentSwapBuffer);

		// Present the current back buffer
		swapChain->Present(vsync ? 1 : 0, 0);

		// Wait to proceed to the next frame until the associated buffer is ready
		currentSwapBuffer = dx12HelperInst.SyncSwapChain(currentSwapBuffer);
	}	

}