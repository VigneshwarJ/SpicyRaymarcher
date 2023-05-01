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



SDFEntity::SDFEntity(int nth)
{
    AddSphere();
    name = "SDF number " + std::to_string(nth);
}

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

    //psData.primitives[sphereCount].Size = uiSettings.size;
    //psData.primitives[sphereCount].MaterialType = uiSettings.materialType;
    //psData.primitives[sphereCount].Position = DirectX::XMFLOAT3(uiSettings.position);
    //psData.primitives[sphereCount].MaterialType = SDF_TYPE_SPHERE;


    

    //SDFPrimRenderData renderData = {};
    

    std::string name = "sphere" + std::to_string(sphereCount);
    PrimitiveData newPrim = {};
    newPrim.name = name;
    newPrim.type = SDFType::Sphere;
    newPrim.idx = sphereCount;
    psData.spherePrims[sphereCount++] = {}; //new struct with default values
    //thisEntData.spherePrims[sphereCount++] = {}; //new struct with default values
    //newPrim.renderData
    //primitivesNames->push_back(name);// std::to_string(primitiveCount));
    //nameToType->insert(std::pair<std::string, SDFType>(name, SDFType::Sphere));
    primitives.push_back(newPrim);
}

void SDFEntity::AddBox()
{
    if (!CanAddPrimitive(boxCount))
        return;

   /* psData.primitives[MAX_PRIMITIVES+  boxCount].MaterialType = uiSettings.materialType;
    psData.primitives[MAX_PRIMITIVES + boxCount].Position = DirectX::XMFLOAT3(uiSettings.position);
    psData.primitives[MAX_PRIMITIVES + boxCount].Dimensions = DirectX::XMFLOAT3(uiSettings.size, uiSettings.size, uiSettings.size);*/

    
    std::string name = "box" + std::to_string(boxCount);    PrimitiveData newPrim = {};
    newPrim.name = name;
    newPrim.type = SDFType::Box;
    newPrim.idx = boxCount;
    psData.boxPrims[boxCount++] = {}; //new struct with default values
    //thisEntData.boxPrims[boxCount++] = {}; //new struct with default values
    primitives.push_back(newPrim);
    //primitivesNames->push_back(name);// std::to_string(primitiveCount));
    //nameToType->insert(std::pair<std::string, SDFType>(name, SDFType::Box));
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
                const char* name = primitives.at(i).name.c_str(); //idk why it didnt work with [i] but why would strings in c++ ever miss out on a chance to make me miserable and confused
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
    if (primitives.size() > 0 && item_current_idx < primitives.size())
    {
        switch (primitives.at(item_current_idx).type)
        {
        case SDFType::Sphere:
            ShowSphereSettings(item_current_idx);
            break;

        case SDFType::Box:
            ShowBoxSettings(item_current_idx);
            break;

        default:
            break;
        }
    }
    ImGui::Separator();
    //ImGui::ColorEdit3("Material color", uiSettings.color);

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


	ImGui::SliderFloat3("Position", (float*)&psData.spherePrims[primitives[selectedIndex].idx].Position, -50.0, 50.0);
	//ImGui::SliderFloat3("Position", (float*)&thisEntData.spherePrims[primitives[selectedIndex].idx].Position, -50.0, 50.0);


	ImGui::SliderFloat("Radius", &psData.spherePrims[primitives[selectedIndex].idx].Size, 0, 100);
	//ImGui::SliderFloat("Radius", &thisEntData.spherePrims[primitives[selectedIndex].idx].Size, 0, 100);


}

void SDFEntity::ShowBoxSettings(int selectedIndex)
{
    ImGui::SeparatorText("Box Settings");

    ImGui::SliderFloat3("Position", (float*)&psData.boxPrims[primitives[selectedIndex].idx].Position, -50.0, 50.0);
    //ImGui::SliderFloat3("Position", (float*)&thisEntData.boxPrims[primitives[selectedIndex].idx].Position, -50.0, 50.0);
    ImGui::SliderFloat3("Dimensions", (float*)&psData.boxPrims[primitives[selectedIndex].idx].Dimensions, -100.0, 100.0);
    //ImGui::SliderFloat3("Dimensions", (float*)&thisEntData.boxPrims[primitives[selectedIndex].idx].Dimensions, -100.0, 100.0);

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
    psData.lightPosition = DirectX::XMFLOAT3A(lightPos);
    psData.sphereCount = sphereCount;
    psData.boxCount = boxCount;
    return &psData;
}