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



SDFEntity::SDFEntity(unsigned int nth, RaymarchPSExternalData* data):
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

    std::string name = "sphere" + std::to_string(sphereCount);
    PrimitiveData newPrim = {};
    newPrim.name = name;
    newPrim.type = SDFType::Sphere;
    newPrim.idx = masterPSData->sphereCount;
    masterPSData->spherePrims[newPrim.idx] = {}; //new struct with default values
    masterPSData->sphereCount++;
    sphereCount++;

    primitives.push_back(newPrim);
}

void SDFEntity::AddBox()
{
    if (!CanAddPrimitive(boxCount))
        return;

    std::string name = "box" + std::to_string(boxCount);    
    PrimitiveData newPrim = {};
    newPrim.name = name;
    newPrim.type = SDFType::Box;
    newPrim.idx = masterPSData->boxCount;
    masterPSData->boxPrims[newPrim.idx] = {}; //new struct with default values
    masterPSData->boxCount++;
    boxCount++;
    primitives.push_back(newPrim);

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
    masterPSData->torusPrims[newPrim.idx] = {}; 
    masterPSData->torusCount++;
    torusCount++;
    primitives.push_back(newPrim);
}

void SDFEntity::AddRoundBox()
{
    if (!CanAddPrimitive(roundBoxCount))
        return;


    std::string name = "roundBox" + std::to_string(roundBoxCount);
    PrimitiveData newPrim = {};
    newPrim.name = name;
    newPrim.type = SDFType::RoundBox;
    newPrim.idx = masterPSData->roundBoxCount;
    masterPSData->roundBoxPrims[newPrim.idx] = {};
    masterPSData->roundBoxCount++;
    roundBoxCount++;
    primitives.push_back(newPrim);
}
//
//void SDFEntity::AddCylinder()
//{
//    if (!CanAddPrimitive(cylinderCount))
//        return;
//
//
//    std::string name = "cylinder" + std::to_string(cylinderCount);
//    PrimitiveData newPrim = {};
//    newPrim.name = name;
//    newPrim.type = SDFType::RoundBox;
//    newPrim.idx = masterPSData->cylinderCount;
//    masterPSData->cylinderPrims[newPrim.idx] = {};
//    masterPSData->cylinderCount++;
//    cylinderCount++;
//    primitives.push_back(newPrim);
//}

void SDFEntity::UpdateGUI()
{

    ImGui::SeparatorText("EditEntity");
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

			case SDFType::RoundBox:
				ShowRoundBoxSettings(item_current_idx);
				break;

				//case SDFType::Cylinder:
				//    ShowCylinderSettings(item_current_idx);
				//    break;

			default:
				break;
			}
		}


		ImGui::SeparatorText("Add Primitives");
		if (ImGui::Button("Sphere"))
		{
			AddSphere();
		}
		if (ImGui::Button("Box"))
		{
			AddBox();
		}
		if (ImGui::Button("Torus"))
		{
			AddTorus();
		}
		if (ImGui::Button("RoundBox"))
		{
			AddRoundBox();
		}

		ImGui::TreePop();
	}

}

void SDFEntity::ShowSphereSettings(int selectedIndex)
{
    ImGui::SeparatorText("Sphere Settings");


	ImGui::SliderFloat3("Position", (float*)&masterPSData->spherePrims[primitives[selectedIndex].idx].Position, -50.0, 50.0);
	ImGui::SliderFloat("Radius", &masterPSData->spherePrims[primitives[selectedIndex].idx].Radius, 0, 100);
    //ImGui::SliderInt("Material type", &masterPSData->spherePrims[primitives[selectedIndex].idx].MaterialType, 0, masterPSData->materialCount);

    //ImGui::SeparatorText("looping Animation Settings");
    //ImGui::SliderFloat3("Delta position", (float*)&masterPSData->spherePrims[primitives[selectedIndex].idx].DeltaPosition, 0, 10);
    //ImGui::SliderFloat("speed", &masterPSData->spherePrims[primitives[selectedIndex].idx].speed, 0, 10);
    //ImGui::SliderFloat("smooth step", &masterPSData->spherePrims[primitives[selectedIndex].idx].smoothStep, 0, 1);

    ShowMoreSettings(&masterPSData->spherePrims[primitives[selectedIndex].idx]);
}

void SDFEntity::ShowBoxSettings(int selectedIndex)
{
    ImGui::SeparatorText("Box Settings");

    ImGui::SliderFloat3("Position", (float*)&masterPSData->boxPrims[primitives[selectedIndex].idx].Position, -50.0, 50.0);
    ImGui::SliderFloat3("Dimensions", (float*)&masterPSData->boxPrims[primitives[selectedIndex].idx].Dimensions, -100.0, 100.0);
    //ImGui::SliderInt("Material type", &masterPSData->boxPrims[primitives[selectedIndex].idx].MaterialType, 0, masterPSData->materialCount);

    //ImGui::SeparatorText("looping Animation Settings");
    //ImGui::SliderFloat3("Delta position", (float*)&masterPSData->boxPrims[primitives[selectedIndex].idx].DeltaPosition, 0, 10);
    //ImGui::SliderFloat("rotation Radius", &masterPSData->boxPrims[primitives[selectedIndex].idx].RotationRadius, 0, 10);
    //ImGui::SliderFloat("smooth step", &masterPSData->boxPrims[primitives[selectedIndex].idx].smoothStep, 0, 1);


    ShowMoreSettings(&masterPSData->boxPrims[primitives[selectedIndex].idx]);
}

void SDFEntity::ShowTorusSettings(int selectedIndex)
{
    ImGui::SeparatorText("Torus Settings");

    ImGui::SliderFloat3("Position", (float*)&masterPSData->torusPrims[primitives[selectedIndex].idx].Position, -50.0, 50.0);
    ImGui::SliderFloat("Large Radius", &masterPSData->torusPrims[primitives[selectedIndex].idx].Radius, 0, 100);
    ImGui::SliderFloat("Small Radius", &masterPSData->torusPrims[primitives[selectedIndex].idx].SmallRadius, 0, 10);
    //ImGui::SliderInt("Material type", &masterPSData->torusPrims[primitives[selectedIndex].idx].MaterialType, 0, masterPSData->materialCount);
    //
    //ImGui::SeparatorText("looping Animation Settings");
    //ImGui::SliderFloat3("Delta position", (float*)&masterPSData->torusPrims[primitives[selectedIndex].idx].DeltaPosition, 0, 10);
    //ImGui::SliderFloat("rotation Radius", &masterPSData->torusPrims[primitives[selectedIndex].idx].RotationRadius, 0, 10);
    //ImGui::SliderFloat("smooth step", &masterPSData->torusPrims[primitives[selectedIndex].idx].smoothStep, 0, 1);

    ShowMoreSettings(&masterPSData->torusPrims[primitives[selectedIndex].idx]);

}

void SDFEntity::ShowRoundBoxSettings(int selectedIndex)
{
    ImGui::SeparatorText("Round Box Settings");

    ImGui::SliderFloat3("Position", (float*)&masterPSData->roundBoxPrims[primitives[selectedIndex].idx].Position, -50.0, 50.0);
    ImGui::SliderFloat3("Dimensions", (float*)&masterPSData->roundBoxPrims[primitives[selectedIndex].idx].Dimensions, 0, 100);
    ImGui::SliderFloat("Edge Radius", &masterPSData->roundBoxPrims[primitives[selectedIndex].idx].Radius, 0, 100);
    //ImGui::SliderInt("Material type", &masterPSData->roundBoxPrims[primitives[selectedIndex].idx].MaterialType, 0, masterPSData->materialCount);
    //
    //ImGui::SeparatorText("looping Animation Settings");
    //ImGui::SliderFloat3("Delta position", (float*)&masterPSData->roundBoxPrims[primitives[selectedIndex].idx].DeltaPosition, 0, 10);
    //ImGui::SliderFloat("rotation Radius", &masterPSData->roundBoxPrims[primitives[selectedIndex].idx].RotationRadius, 0, 10);
    //ImGui::SliderFloat("smooth step", &masterPSData->roundBoxPrims[primitives[selectedIndex].idx].smoothStep, 0, 1);

    ShowMoreSettings(&masterPSData->roundBoxPrims[primitives[selectedIndex].idx]);
}
//
//void SDFEntity::ShowCylinderSettings(int selectedIndex)
//{
//    ImGui::SeparatorText("Cylinder Settings");
//
//    ImGui::SliderFloat3("Position", (float*)&masterPSData->cylinderPrims[primitives[selectedIndex].idx].Position, -50.0, 50.0);
//    ImGui::SliderFloat3("Dimensions", (float*)&masterPSData->cylinderPrims[primitives[selectedIndex].idx].Dimensions, 0, 100);
//    ImGui::SliderFloat("Edge Radius", &masterPSData->cylinderPrims[primitives[selectedIndex].idx].Radius, 0, 100);
//    //ImGui::SliderInt("Material type", &masterPSData->roundBoxPrims[primitives[selectedIndex].idx].MaterialType, 0, masterPSData->materialCount);
//    //
//    //ImGui::SeparatorText("looping Animation Settings");
//    //ImGui::SliderFloat3("Delta position", (float*)&masterPSData->roundBoxPrims[primitives[selectedIndex].idx].DeltaPosition, 0, 10);
//    //ImGui::SliderFloat("rotation Radius", &masterPSData->roundBoxPrims[primitives[selectedIndex].idx].RotationRadius, 0, 10);
//    //ImGui::SliderFloat("smooth step", &masterPSData->roundBoxPrims[primitives[selectedIndex].idx].smoothStep, 0, 1);
//
//    ShowMoreSettings(&masterPSData->cylinderPrims[primitives[selectedIndex].idx]);
//}

void SDFEntity::ShowMoreSettings(SDFPrimRenderData* thisData)
{

    ImGui::SliderInt("Material type", &thisData->MaterialType, 0, masterPSData->materialCount);

    ImGui::SeparatorText("looping Animation Settings");
    ImGui::SliderFloat3("Delta position", (float*)&thisData->DeltaPosition, 0, 10);
    ImGui::SliderFloat("speed", &thisData->speed, 0, 10);
    ImGui::SliderFloat("smooth step", &thisData->smoothStep, 0, 1);
}