#include "SDFEntity.h"
#include <iostream>
#include "imgui/imgui.h"

std::unique_ptr<SDFEntity> SDFEntity::s_Entity = nullptr;


static bool create_sphere = true;
static bool create_cube = false;
static bool create_cone = false;
// TODO: more shapes  

static int item_current_idx = 0;
static bool continuos_update = false;
void SDFEntity::AddSphere()
{
    if (sphereCount >= MAX_SDF_COUNT)
    {
        std::cerr << "Max sdf spheres count reached\n";
        return;
    }

    sphereSetting.spheres[sphereCount].sphereRadius = sphereSetting.sphereSize;
    sphereSetting.spheres[sphereCount].sphereColor = DirectX::XMFLOAT4(sphereSetting.color);
    sphereSetting.spheres[sphereCount].spherePosition = DirectX::XMFLOAT3(sphereSetting.spherePos);
    sphereCount++;
}

void SDFEntity::ChangeSphereSettings(int no)
{
    if (no >= MAX_SDF_COUNT)
    {
        std::cerr << "Max sdf spheres count reached\n";
        return;
    }

    sphereSetting.spheres[no].sphereRadius = sphereSetting.sphereSize;
    sphereSetting.spheres[no].sphereColor = DirectX::XMFLOAT4(sphereSetting.color);
    sphereSetting.spheres[no].spherePosition = DirectX::XMFLOAT3(sphereSetting.spherePos);
    
}

void SDFEntity::TweakSphereSettings()
{
    ImGui::SliderFloat("Sphere size", &sphereSetting.sphereSize, 0, 100);

    ImGui::SliderFloat3("sphere position", sphereSetting.spherePos, -100.0, 100.0);
    ImGui::ColorEdit3("color", sphereSetting.color);
    ImGui::Checkbox("continuous update", &continuos_update);
    if (ImGui::Button("CreateSphere"))
    {
        AddSphere();
    }

    if (ImGui::BeginListBox("Spheres"))
    {
        for (int n = 0; n < GetSDFEntity()->sphereCount; n++)
        {
            const bool is_selected = (item_current_idx == n);
            char i[2] = { n + '0','\0' };
            if (ImGui::Selectable(i, is_selected))
            {
                item_current_idx = n;
                continuos_update = false;
                sphereSetting.sphereSize = sphereSetting.spheres[item_current_idx].sphereRadius;
                sphereSetting.color[0] = sphereSetting.spheres[item_current_idx].sphereColor.x;
                sphereSetting.color[1] = sphereSetting.spheres[item_current_idx].sphereColor.y;
                sphereSetting.color[2] = sphereSetting.spheres[item_current_idx].sphereColor.z;
                sphereSetting.color[3] = sphereSetting.spheres[item_current_idx].sphereColor.w ;
                sphereSetting.spherePos[0] = sphereSetting.spheres[item_current_idx].spherePosition.x;
                sphereSetting.spherePos[1] = sphereSetting.spheres[item_current_idx].spherePosition.y;
                sphereSetting.spherePos[2] = sphereSetting.spheres[item_current_idx].spherePosition.z;
            }

            // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndListBox();
    }
    if (GetSDFEntity()->sphereCount > 0)
    {
        if (continuos_update)
        {
            GetSDFEntity()->ChangeSphereSettings(item_current_idx);
        }
    }
}

void SDFEntity::DisplaySDFSettings()
{
    //if (!ImGui::Begin("Dear ImGui Demo", NULL, ImGuiWindowFlags_MenuBar))
    //{
    //    ImGui::End();
    //    return nullptr;
    //}
    if (ImGui::BeginMenuBar())
    {

        if (ImGui::BeginMenu("Shapes"))
        {
            ImGui::MenuItem("Sphere", NULL, &create_sphere);
            ImGui::MenuItem("Cube", NULL, &create_cube);

            ImGui::EndMenu();
        }
        //if (ImGui::MenuItem("MenuItem")) {} // You can also use MenuItem() inside a menu bar!
        if (ImGui::BeginMenu("Tools"))
        {

            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
    if (create_sphere)
    {
        TweakSphereSettings();
    }
    else if (create_cube)
    {
        
    }
    ImGui::SliderFloat3("light position", lightPos, -100.0, 100.0);
        
  

   return;
}

RaymarchPSExternalData* SDFEntity::GetRayMarchPSData()
{
    psData.lightPosition = DirectX::XMFLOAT3A(lightPos);
    psData.sphereCount = sphereCount;
    return &psData;
}