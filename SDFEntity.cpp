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
bool SDFEntity::CanAddPrimitive()
{
    if (primitiveCount >= MAX_SDF_COUNT)
    {
        std::cerr << "Max sdf primitives count reached\n";
        return false;
    }
    return true;
}

void  SDFEntity::AddSphere()
{
    if (!CanAddPrimitive())
        return;

    psData.primitives[primitiveCount].Radius = uiSettings.sphereSize;
    psData.primitives[primitiveCount].Color = DirectX::XMFLOAT4(uiSettings.color);
    psData.primitives[primitiveCount].Position = DirectX::XMFLOAT3(uiSettings.spherePos);
    psData.primitives[primitiveCount].Type = SDF_TYPE_SPHERE;


    primitiveCount++;
    //primitivesNames->push_back("hello");// std::to_string(primitiveCount));
}

void SDFEntity::AddBox()
{
    if (!CanAddPrimitive())
        return;

    psData.primitives[primitiveCount].Color = DirectX::XMFLOAT4(uiSettings.color);
    psData.primitives[primitiveCount].Position = DirectX::XMFLOAT3(uiSettings.spherePos);
    psData.primitives[primitiveCount].Dimensions = DirectX::XMFLOAT3(1.0f, 1.0f, 1.0f);
    psData.primitives[primitiveCount].Type = SDF_TYPE_BOX;



    primitiveCount++;
}

void SDFEntity::ChangeSphereSettings(int no)
{
    if (no >= MAX_SDF_COUNT)
    {
        std::cerr << "Max sdf spheres count reached\n";
        return;
    }

    psData.primitives[no].Radius = uiSettings.sphereSize;
    psData.primitives[no].Color = DirectX::XMFLOAT4(uiSettings.color);
    psData.primitives[no].Position = DirectX::XMFLOAT3(uiSettings.spherePos);
    
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



    ImGui::SliderFloat("Sphere size", &uiSettings.sphereSize, 0, 100);

    ImGui::SliderFloat3("position", uiSettings.spherePos, -100.0, 100.0);
    ImGui::ColorEdit3("color", uiSettings.color);
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
        for (int n = 0; n < GetSDFEntity()->primitiveCount; n++)
        {
            const bool is_selected = (item_current_idx == n);
            char i[2] = { n + '0','\0' };
            if (ImGui::Selectable(i, is_selected))
            {
                item_current_idx = n;
                continuos_update = false;
                uiSettings.sphereSize = psData.primitives[item_current_idx].Radius;
                uiSettings.color[0] = psData.primitives[item_current_idx].Color.x;
                uiSettings.color[1] = psData.primitives[item_current_idx].Color.y;
                uiSettings.color[2] = psData.primitives[item_current_idx].Color.z;
                uiSettings.color[3] = psData.primitives[item_current_idx].Color.w ;
                uiSettings.spherePos[0] = psData.primitives[item_current_idx].Position.x;
                uiSettings.spherePos[1] = psData.primitives[item_current_idx].Position.y;
                uiSettings.spherePos[2] = psData.primitives[item_current_idx].Position.z;
            }

            // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
            if (is_selected)
                ImGui::SetItemDefaultFocus();
        }
        ImGui::EndListBox();
    }
    if (GetSDFEntity()->primitiveCount > 0)
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
    psData.primitiveCount = primitiveCount;
    return &psData;
}