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
    //primitivesNames->push_back("hello");// std::to_string(primitiveCount));
}

void SDFEntity::AddBox()
{
    if (!CanAddPrimitive(boxCount))
        return;

    psData.primitives[MAX_PRIMITIVES+  boxCount].MaterialType = uiSettings.materialType;
    psData.primitives[MAX_PRIMITIVES + boxCount].Position = DirectX::XMFLOAT3(uiSettings.position);
    psData.primitives[MAX_PRIMITIVES + boxCount].Dimensions = DirectX::XMFLOAT3(uiSettings.size, uiSettings.size, uiSettings.size);

    boxCount++;
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

void SDFEntity::ShowSphereSettings()
{
	//ImGui::Text("hello");
    //ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
    //if (ImGui::BeginTable("split", 2, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_Resizable))
    if (ImGui::TreeNode("Primitives"))
    {
        ImGui::Text("uuuuuh list box here soon");
    //    if (ImGui::BeginListBox("Spheres"))
    //    {
    //        for (int i = 0; i < primitiveCount; i++)
    //        {
				//const bool is_selected = (item_current_idx == i);
    //            const char* name = primitivesNames->at(i).c_str(); //idk why it didnt work with [i] but why would strings in c++ ever miss out on a chance to make me miserable and confused
    //            if (ImGui::Selectable(name, is_selected))
    //                item_current_idx = i;

    //            // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
    //            if (is_selected)
    //                ImGui::SetItemDefaultFocus();
    //        }
    //        ImGui::EndListBox();

    //    }
    // 
 //------------------------------------------------------------------------------------------
        //ImGui::ListBox("prims", &item_current_idx, psData.primitives, primitiveCount, 4);
		//if (ImGui::BeginListBox("Spheres"))
		//{
		//	// Iterate placeholder objects (all the same data)
		//	for (int i = 0; i < primitiveCount; i++)
		//	{
  //              const bool is_selected = (item_current_idx == i);
  //              //char i[2] = { n + '0','\0' };
  //              if (ImGui::Selectable(i, is_selected))
  //              {
  //                  item_current_idx = n;
  //                  continuos_update = false;
  //                  uiSettings.sphereSize = psData.primitives[item_current_idx].Radius;
  //                  uiSettings.color[0] = psData.primitives[item_current_idx].Color.x;
  //                  uiSettings.color[1] = psData.primitives[item_current_idx].Color.y;
  //                  uiSettings.color[2] = psData.primitives[item_current_idx].Color.z;
  //                  uiSettings.color[3] = psData.primitives[item_current_idx].Color.w;
  //                  uiSettings.spherePos[0] = psData.primitives[item_current_idx].Position.x;
  //                  uiSettings.spherePos[1] = psData.primitives[item_current_idx].Position.y;
  //                  uiSettings.spherePos[2] = psData.primitives[item_current_idx].Position.z;
  //              }

  //              // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
  //              if (is_selected)
  //                  ImGui::SetItemDefaultFocus();
		//		//std::cout << primitiveCount;
		//		//ShowPlaceholderObject("Object", obj_i);
		//		//ImGui::SeparatorText("hello");
		//	}
		//	//ImGui::EndTable();

		//}
        ImGui::TreePop();
    }
    //ImGui::PopStyleVar();



    ImGui::SliderFloat("Sphere size", &uiSettings.size, 0, 100);

    ImGui::SliderFloat3("position", uiSettings.position, -100.0, 100.0);
    ImGui::ColorEdit3("Material color", uiSettings.color);
    ImGui::Checkbox("continuous update", &continuos_update);
    if (ImGui::Button("CreateSphere"))
    {
        AddSphere();
    }
    if (ImGui::Button("CreateBox"))
    {
        AddBox();
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
                uiSettings.size = psData.primitives[item_current_idx].Size;
                uiSettings.position[0] = psData.primitives[item_current_idx].Position.x;
                uiSettings.position[1] = psData.primitives[item_current_idx].Position.y;
                uiSettings.position[2] = psData.primitives[item_current_idx].Position.z;
                uiSettings.materialType = psData.primitives[item_current_idx].MaterialType;
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
    //Add one for box or put code for box in this
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
        ShowSphereSettings();
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
    psData.boxCount = START_BOXES + boxCount;
    return &psData;
}