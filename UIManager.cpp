#include "UIManager.h"

#include <imgui.h>
#include <imgui_impl_win32.h>
#include "imgui_impl_dx12.h"

#include "SDFEntity.h"
void UIManager::UIUpdate()
{
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Settings", NULL, ImGuiWindowFlags_MenuBar);                          // Create a window called "Hello, world!" and append into it.
	SDFEntity::GetSDFEntity()->DisplaySDFSettings();





	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::End();
}
