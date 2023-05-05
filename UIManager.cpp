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

	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::End();
}

void UIManager::DisplaySDFSettings()
{
    ////if (!ImGui::Begin("Dear ImGui Demo", NULL, ImGuiWindowFlags_MenuBar))
    ////{
    ////    ImGui::End();
    ////    return nullptr;
    ////}
    //if (ImGui::BeginMenuBar())
    //{

    //    if (ImGui::BeginMenu("Shapes"))
    //    {
    //        ImGui::MenuItem("Sphere", NULL, &create_sphere);
    //        ImGui::MenuItem("Cube", NULL, &create_cube);

    //        ImGui::EndMenu();
    //    }
    //    //if (ImGui::MenuItem("MenuItem")) {} // You can also use MenuItem() inside a menu bar!
    //    if (ImGui::BeginMenu("Tools"))
    //    {

    //        ImGui::EndMenu();
    //    }
    //    ImGui::EndMenuBar();
    //}
    //if (create_sphere)
    //{
    //    //ShowSphereSettings();
    //    UpdateGUI();
    //}
    //else if (create_cube)
    //{

    //}
    //ImGui::SliderFloat3("light position", lightPos, -100.0, 100.0);



    //return;
}
