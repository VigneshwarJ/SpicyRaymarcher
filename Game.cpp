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
	vsync(false),
	color{ 1.0f,1.0f,1.0f,1.0f },
sphereSize(5.0f),
lightPos{ 0.0, -10.0 , 0.0 },
spherePos { 0.0, 0.0 , 7.0 }
{
#if defined(DEBUG) || defined(_DEBUG)
	// Do we want a console window?  Probably only in debug mode
	CreateConsoleWindow(500, 120, 32, 120);
	printf("Console window created successfully.  Feel free to printf() here.\n");
#endif
	renderer = std::make_shared<SDFRenderer>();
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





	// Seed random
	srand((unsigned int)time(0));

	ibView = {};
	vbView = {};

	CreateBasicGeometry();

	camera = std::make_shared<Camera>(
		0.0f, 0.0f, -12.0f,	// Position
		3.0f,		// Move speed
		1.0f,		// Mouse look
		this->width / (float)this->height); // Aspect ratio

	InitSDFRenderer();
	CreateMaterials();

	CreateEntities();

	// I put this in the sdfrenderer init method for now
	//// Ensure the command list is closed going into Draw for the first time
	//commandList->Close();

	raytracing = false;
}

void Game::InitSDFRenderer()
{
	sdfRenderer = renderer; //this is lazy and bad im sorry
	sdfRenderer->Init(vsync, camera);
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

}

void Game::CreateEntities()
{
	CreateMeshes();
	entities.push_back(std::make_shared<GameEntity>(meshes[0], materials[2], Transform()));
	//entities.push_back(std::make_shared<GameEntity>(meshes[1], materials[1], Transform()));
	//entities.push_back(std::make_shared<GameEntity>(meshes[2], materials[0], Transform()));
}

void Game::CreateMeshes()
{
	meshes.push_back(std::make_shared<Mesh>(Utils::GetFullPathTo("../../Assets/Models/sphere.obj").c_str(), device));
	meshes.push_back(std::make_shared<Mesh>(Utils::GetFullPathTo("../../Assets/Models/cube.obj").c_str(), device));
	meshes.push_back(std::make_shared<Mesh>(Utils::GetFullPathTo("../../Assets/Models/helix.obj").c_str(), device));
}


void Game::LoadTextures()
{

}

void Game::CreateMaterials()
{
	std::wstring assets = Utils::GetFullPathTo_Wide(L"../../Assets");
	std::wstring textures = assets + L"\\Textures";
	D3D12_CPU_DESCRIPTOR_HANDLE albedo = DX12Helper::GetInstance().LoadTexture((textures + L"\\bronze_albedo.png").c_str());
	D3D12_CPU_DESCRIPTOR_HANDLE metal = DX12Helper::GetInstance().LoadTexture((textures + L"\\bronze_metal.png").c_str());
	D3D12_CPU_DESCRIPTOR_HANDLE normals = DX12Helper::GetInstance().LoadTexture((textures + L"\\bronze_normals.png").c_str());
	D3D12_CPU_DESCRIPTOR_HANDLE roughness = DX12Helper::GetInstance().LoadTexture((textures + L"\\bronze_roughness.png").c_str());

	//D3D12_CPU_DESCRIPTOR_HANDLE albedo =    DX12Helper::GetInstance().LoadTexture((textures + L"\\cobblestone_albedo.png").c_str());
	//D3D12_CPU_DESCRIPTOR_HANDLE metal =     DX12Helper::GetInstance().LoadTexture((textures + L"\\cobblestone_metal.png").c_str());
	//D3D12_CPU_DESCRIPTOR_HANDLE normals =   DX12Helper::GetInstance().LoadTexture((textures + L"\\cobblestone_normals.png").c_str());
	//D3D12_CPU_DESCRIPTOR_HANDLE roughness = DX12Helper::GetInstance().LoadTexture((textures + L"\\cobblestone_roughness.png").c_str());

	std::shared_ptr<Material> material = std::make_shared<Material>(
		sdfRenderer->GetPipeState(),
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

	albedo = DX12Helper::GetInstance().LoadTexture((textures + L"\\cobblestone_albedo.png").c_str());
	metal = DX12Helper::GetInstance().LoadTexture((textures + L"\\cobblestone_metal.png").c_str());
	normals = DX12Helper::GetInstance().LoadTexture((textures + L"\\cobblestone_normals.png").c_str());
	roughness = DX12Helper::GetInstance().LoadTexture((textures + L"\\cobblestone_roughness.png").c_str());

	//D3D12_CPU_DESCRIPTOR_HANDLE albedo =    DX12Helper::GetInstance().LoadTexture((textures + L"\\cobblestone_albedo.png").c_str());
	//D3D12_CPU_DESCRIPTOR_HANDLE metal =     DX12Helper::GetInstance().LoadTexture((textures + L"\\cobblestone_metal.png").c_str());
	//D3D12_CPU_DESCRIPTOR_HANDLE normals =   DX12Helper::GetInstance().LoadTexture((textures + L"\\cobblestone_normals.png").c_str());
	//D3D12_CPU_DESCRIPTOR_HANDLE roughness = DX12Helper::GetInstance().LoadTexture((textures + L"\\cobblestone_roughness.png").c_str());

	material = std::make_shared<Material>(
		sdfRenderer->GetPipeState(),
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

	albedo = DX12Helper::GetInstance().LoadTexture((textures + L"\\Planet1\\planet1_albedo.tif").c_str());
	metal = DX12Helper::GetInstance().LoadTexture((textures + L"\\Planet1\\planet1_metalic.tif").c_str());
	normals = DX12Helper::GetInstance().LoadTexture((textures + L"\\Planet1\\planet1_normal.tif").c_str());
	roughness = DX12Helper::GetInstance().LoadTexture((textures + L"\\Planet1\\planet1_roughness.tif").c_str());

	//D3D12_CPU_DESCRIPTOR_HANDLE albedo =    DX12Helper::GetInstance().LoadTexture((textures + L"\\cobblestone_albedo.png").c_str());
	//D3D12_CPU_DESCRIPTOR_HANDLE metal =     DX12Helper::GetInstance().LoadTexture((textures + L"\\cobblestone_metal.png").c_str());
	//D3D12_CPU_DESCRIPTOR_HANDLE normals =   DX12Helper::GetInstance().LoadTexture((textures + L"\\cobblestone_normals.png").c_str());
	//D3D12_CPU_DESCRIPTOR_HANDLE roughness = DX12Helper::GetInstance().LoadTexture((textures + L"\\cobblestone_roughness.png").c_str());

	material = std::make_shared<Material>(
		sdfRenderer->GetPipeState(),
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


// --------------------------------------------------------
// Handle resizing DirectX "stuff" to match the new window size.
// For instance, updating our projection matrix's aspect ratio.
// --------------------------------------------------------
void Game::OnResize()
{
	// Handle base-level DX resize stuff
	//DXCore::OnResize();
	sdfRenderer->OnResize(width, height);
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

	sdfRenderer->Render(entities, color, sphereSize, lightPos, spherePos);
	DX12Helper& dx12HelperInst = DX12Helper::GetInstance();

}