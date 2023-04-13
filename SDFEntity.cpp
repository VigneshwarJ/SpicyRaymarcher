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



bool SDFEntity::CanAddPrimitive(int count)
{
    if (count >= MAX_PRIMITIVES)
    {
        std::cerr << "Max sdf primitives count reached\n";
        return false;
    }
    return true;
}

void  SDFEntity::AddSphere()
{
    if (!CanAddPrimitive(sphereCount))
        return;

    psData.primitives[sphereCount].Size = uiSettings.size;
    psData.primitives[sphereCount].MaterialType = uiSettings.materialType;
    psData.primitives[sphereCount].Position = DirectX::XMFLOAT3(uiSettings.position);
    psData.primitives[sphereCount].MaterialType = SDF_TYPE_SPHERE;


    sphereCount++;
    std::string name = "sphere" + std::to_string(sphereCount);
    primitivesNames->push_back(name);// std::to_string(primitiveCount));
    nameToType->insert(std::pair<std::string, SDFType>(name, SDFType::Sphere));
}

void SDFEntity::AddBox()
{
    if (!CanAddPrimitive(boxCount))
        return;

    psData.primitives[MAX_PRIMITIVES+  boxCount].MaterialType = uiSettings.materialType;
    psData.primitives[MAX_PRIMITIVES + boxCount].Position = DirectX::XMFLOAT3(uiSettings.position);
    psData.primitives[MAX_PRIMITIVES + boxCount].Dimensions = DirectX::XMFLOAT3(uiSettings.size, uiSettings.size, uiSettings.size);

    boxCount++;
    std::string name = "box" + std::to_string(boxCount);
    primitivesNames->push_back(name);// std::to_string(primitiveCount));
    nameToType->insert(std::pair<std::string, SDFType>(name, SDFType::Box));
}

void SDFEntity::ChangeSphereSettings(int no)
{
    if (no >= MAX_SDF_COUNT)
    {
        std::cerr << "Max sdf spheres count reached\n";
        return;
    }

    psData.primitives[no].Size = uiSettings.size;
    psData.primitives[no].MaterialType = uiSettings.materialType;
    psData.primitives[no].Position = DirectX::XMFLOAT3(uiSettings.position);
    
}

void SDFEntity::UpdateGUI()
{
    if (ImGui::TreeNode("Primitives"))
    {
        if (ImGui::BeginListBox("Primitives"))
        {
            for (int i = 0; i < sphereCount + boxCount; i++)
            {
                const bool is_selected = (item_current_idx == i);
                const char* name = primitivesNames->at(i).c_str(); //idk why it didnt work with [i] but why would strings in c++ ever miss out on a chance to make me miserable and confused
                if (ImGui::Selectable(name, is_selected))
                    item_current_idx = i;

                // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndListBox();

        }


        ImGui::TreePop();
    }
    if (primitivesNames->size() > 0)
    {
        switch (nameToType->at(primitivesNames->at(item_current_idx)))
        {
        case SDFType::Sphere:
            ShowSphereSettings(item_current_idx);
            break;

        case SDFType::Box:
            ShowBoxSettings(item_current_idx); //METHOD NOT IMPLEMENTED YET
            break;

        default:
            break;
        }
    }
    ImGui::Separator();
    ImGui::ColorEdit3("Material color", uiSettings.color);

    ImGui::Separator();
    if (ImGui::Button("CreateSphere"))
    {
        AddSphere();
    }
    if (ImGui::Button("CreateBox"))
    {
        AddBox();
    }

}

void SDFEntity::ShowSphereSettings(int selectedIndex)
{
    ImGui::SeparatorText("Sphere Settings");

    ImGui::SliderFloat3("Position", (float*)&psData.primitives[selectedIndex].Position, -100.0, 100.0);
	

    ImGui::SliderFloat("Radius", &psData.primitives[selectedIndex].Size, 0, 100);

    
}

void SDFEntity::ShowBoxSettings(int selectedIndex)
{
    ImGui::SeparatorText("Box Settings");
    ImGui::SliderFloat3("Position", (float*)&psData.primitives[MAX_PRIMITIVES + selectedIndex].Position, -100.0, 100.0);
    ImGui::SliderFloat3("Dimensions", (float*)&psData.primitives[MAX_PRIMITIVES + selectedIndex].Dimensions, -100.0, 100.0);

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
        //ShowSphereSettings();
        UpdateGUI();
    }
    else if (create_cube)
    {
        
    }
    ImGui::SliderFloat3("light position", lightPos, -100.0, 100.0);
        
  

   return;
}

//std::shared_ptr<RaymarchPSExternalData> SDFEntity::GetRayMarchPSData()
RaymarchPSExternalData* SDFEntity::GetRayMarchPSData()
{
    //psData->lightPosition = DirectX::XMFLOAT3A(lightPos);
    //psData->sphereCount = sphereCount;
    //psData->boxCount = START_BOXES + boxCount;
    //return psData;
    psData.lightPosition = DirectX::XMFLOAT3A(lightPos);
    psData.sphereCount = sphereCount;
    psData.boxCount = START_BOXES + boxCount;
    return &psData;
}