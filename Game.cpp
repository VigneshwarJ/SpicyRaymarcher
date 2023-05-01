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
size(5.0f),
lightPos{ 0.0, -10.0 , 0.0 },
position { 0.0, 0.0 , 7.0 }
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


	camera = std::make_shared<Camera>(
		0.0f, 0.0f, -12.0f,	// Position
		3.0f,		// Move speed
		1.0f,		// Mouse look
		this->width / (float)this->height); // Aspect ratio

	sdfEntities = std::make_shared<std::vector<SDFEntity>>();
	InitSDFRenderer();

	// I put this in the sdfrenderer init method for now
	//// Ensure the command list is closed going into Draw for the first time
	//commandList->Close();

}

void Game::InitSDFRenderer()
{
	//sdfEntities->
	//CreateSDFEntity();
	sdfRenderer = renderer; //this is lazy and bad im sorry
	sdfRenderer->Init(vsync, camera, sdfEntities);//, *this);
}

void Game::CreateSDFEntity()
{
	sdfEntities->push_back(SDFEntity(sdfEntities->size()));
	selectedEntityIndex = sdfEntities->size()-1;
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

void Game::UpdateGUI()
{

	//io.WantCaptureMouse = true;
	static float f = 0.0f;


	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Settings", NULL, ImGuiWindowFlags_MenuBar);  // Create a window with a name and append into it.
	//SDFEntity::GetSDFEntity()->DisplaySDFSettings();
	SDFMainGUI();




	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::End();


}

void Game::SDFMainGUI()
{

	if (ImGui::Button("AddEntity"))
	{
		CreateSDFEntity();
	}
	if (ImGui::BeginListBox("Entities"))
	{
		for (int i = 0; i < sdfEntities->size(); i++)
		{
			const bool is_selected = (selectedEntityIndex == i);
			const char* name = sdfEntities->at(i).GetName()->c_str();
			if (ImGui::Selectable(name, is_selected))
				selectedEntityIndex = i;

			// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
			if (is_selected)
				ImGui::SetItemDefaultFocus();
		}
		ImGui::EndListBox();

	}

	ImGui::Separator();

	if (sdfEntities->size() > selectedEntityIndex)
	{
		sdfEntities->at(selectedEntityIndex).UpdateGUI();
	}
}

// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	camera->Update(deltaTime);

	//for (int i = 0; i < entities.size(); i++)
	//{
	//	entities[i]->GetTransform()->Rotate(0.0f, 0.3f * deltaTime, 0.0f);
	//}


	UpdateGUI();
	//UIManager::UIUpdate();

	// Example input checking: Quit if the escape key is pressed
	if (Input::GetInstance().KeyDown(VK_ESCAPE))
		Quit();
}

// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	// Should create a new entity for sdf structures.
	sdfRenderer->Render();
	//sdfRenderer->RenderEntity(sdfEntities);
	DX12Helper& dx12HelperInst = DX12Helper::GetInstance();

}