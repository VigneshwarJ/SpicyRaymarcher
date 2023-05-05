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



SDFEntity::SDFEntity(int nth, RaymarchPSExternalData* data):
    masterPSData(data)
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
    newPrim.idx = masterPSData->sphereCount;
    masterPSData->spherePrims[newPrim.idx] = {}; //new struct with default values
    masterPSData->sphereCount++;
    sphereCount++;
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

    
    std::string name = "box" + std::to_string(boxCount);    
    PrimitiveData newPrim = {};
    newPrim.name = name;
    newPrim.type = SDFType::Box;
    newPrim.idx = masterPSData->boxCount;
    masterPSData->boxPrims[newPrim.idx] = {}; //new struct with default values
    masterPSData->boxCount++;
    //thisEntData.boxPrims[boxCount++] = {}; //new struct with default values
    primitives.push_back(newPrim);
    boxCount++;
    //primitivesNames->push_back(name);// std::to_string(primitiveCount));
    //nameToType->insert(std::pair<std::string, SDFType>(name, SDFType::Box));
}

void SDFEntity::AddTorus()
{
    if (!CanAddPrimitive(torusCount))
        return;


    std::string name = "torus" + std::to_string(torusCount);
    PrimitiveData newPrim = {};
    newPrim.name = name;
    newPrim.type = SDFType::Torus;
    newPrim.idx = masterPSData->torusCount;
    masterPSData->torusPrims[newPrim.idx] = {}; //new struct with default values
    masterPSData->torusCount++;
    //thisEntData.boxPrims[boxCount++] = {}; //new struct with default values
    primitives.push_back(newPrim);
    torusCount++;
}

void SDFEntity::UpdateGUI()
{
    if (ImGui::TreeNode("Primitives"))
    {
        if (ImGui::BeginListBox("Primitives"))
        {
            for (int i = 0; i < primitives.size(); i++)
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

        case SDFType::Torus:
            ShowTorusSettings(item_current_idx);
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
    if (ImGui::Button("CreateTorus"))
    {
        AddTorus();
    }

}

void SDFEntity::ShowSphereSettings(int selectedIndex)
{
    ImGui::SeparatorText("Sphere Settings");


	ImGui::SliderFloat3("Position", (float*)&masterPSData->spherePrims[primitives[selectedIndex].idx].Position, -50.0, 50.0);
	//ImGui::SliderFloat3("Position", (float*)&thisEntData.spherePrims[primitives[selectedIndex].idx].Position, -50.0, 50.0);


	ImGui::SliderFloat("Radius", &masterPSData->spherePrims[primitives[selectedIndex].idx].Radius, 0, 100);
	//ImGui::SliderFloat("Radius", &thisEntData.spherePrims[primitives[selectedIndex].idx].Size, 0, 100);

    ImGui::SeparatorText("looping Animation Settings");
    ImGui::SliderFloat3("Delta position", (float*)&masterPSData->spherePrims[primitives[selectedIndex].idx].DeltaPosition, 0, 10);
    ImGui::SliderFloat("speed", &masterPSData->spherePrims[primitives[selectedIndex].idx].speed, 0, 10);
    ImGui::SliderFloat("smooth step", &masterPSData->spherePrims[primitives[selectedIndex].idx].smoothStep, 0, 1);
    ImGui::SliderInt("Material type", &masterPSData->spherePrims[primitives[selectedIndex].idx].MaterialType, 0, masterPSData->materialCount);


}

void SDFEntity::ShowBoxSettings(int selectedIndex)
{
    ImGui::SeparatorText("Box Settings");

    ImGui::SliderFloat3("Position", (float*)&masterPSData->boxPrims[primitives[selectedIndex].idx].Position, -50.0, 50.0);
    //ImGui::SliderFloat3("Position", (float*)&thisEntData.boxPrims[primitives[selectedIndex].idx].Position, -50.0, 50.0);
    ImGui::SliderFloat3("Dimensions", (float*)&masterPSData->boxPrims[primitives[selectedIndex].idx].Dimensions, -100.0, 100.0);
    //ImGui::SliderFloat3("Dimensions", (float*)&thisEntData.boxPrims[primitives[selectedIndex].idx].Dimensions, -100.0, 100.0);
    ImGui::SeparatorText("looping Animation Settings");
    ImGui::SliderFloat3("Delta position", (float*)&masterPSData->boxPrims[primitives[selectedIndex].idx].DeltaPosition, 0, 10);
    ImGui::SliderFloat("rotation Radius", &masterPSData->boxPrims[primitives[selectedIndex].idx].RotationRadius, 0, 10);
    ImGui::SliderFloat("smooth step", &masterPSData->boxPrims[primitives[selectedIndex].idx].smoothStep, 0, 1);
    ImGui::SliderInt("Material type", &masterPSData->boxPrims[primitives[selectedIndex].idx].MaterialType, 0, masterPSData->materialCount);


}

void SDFEntity::ShowTorusSettings(int selectedIndex)
{
    ImGui::SeparatorText("Torus Settings");

    ImGui::SliderFloat3("Position", (float*)&masterPSData->torusPrims[primitives[selectedIndex].idx].Position, -50.0, 50.0);
    //ImGui::SliderFloat3("Position", (float*)&thisEntData.boxPrims[primitives[selectedIndex].idx].Position, -50.0, 50.0);
    ImGui::SliderFloat("Large Radius", &masterPSData->torusPrims[primitives[selectedIndex].idx].Radius, 0, 100);
    ImGui::SliderFloat("Small Radius", &masterPSData->torusPrims[primitives[selectedIndex].idx].SmallRadius, 0, 10);
    
    ImGui::SeparatorText("looping Animation Settings");
    ImGui::SliderFloat3("Delta position", (float*)&masterPSData->torusPrims[primitives[selectedIndex].idx].DeltaPosition, 0, 10);
    ImGui::SliderFloat("rotation Radius", &masterPSData->torusPrims[primitives[selectedIndex].idx].RotationRadius, 0, 10);
    ImGui::SliderFloat("smooth step", &masterPSData->torusPrims[primitives[selectedIndex].idx].smoothStep, 0, 1);
    ImGui::SliderInt("Material type", &masterPSData->torusPrims[primitives[selectedIndex].idx].MaterialType, 0, masterPSData->materialCount);
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
    return nullptr;
    //masterPSDatalightPosition = DirectX::XMFLOAT3A(lightPos);
    //masterPSData.sphereCount = sphereCount;
    //masterPSData.boxCount = boxCount;
    //masterPSData.torusCount = torusCount;
    //return &masterPSData;
}