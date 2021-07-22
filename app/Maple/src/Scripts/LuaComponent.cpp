//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////
#include <imgui.h>
#include <imgui_internal.h>
#include <fstream>
#include <filesystem>

#include "IconsMaterialDesignIcons.h"
#include "LuaComponent.h"
#include "LuaVirtualMachine.h"
#include "ImGui/ImGuiHelpers.h"
#include "Scene/Entity/Entity.h"
#include "Others/StringUtils.h"
#include "Others/Console.h"

#include "Engine/Camera.h"

#include "Scene/Component/Transform.h"
#include "Scene/Component/Component.h"
#include "Application.h"

namespace Maple 
{
	LuaComponent::LuaComponent( const std::string& file, Scene* initScene)
		:scene(initScene),file(file)
	{
		table = nullptr;
		init();
	}


	auto LuaComponent::saveNewFile(const std::string& clazzName) -> void
	{
		std::string newPath = "scripts/";

		std::string defaultScript =
			R"(

#name = {}

function #name:OnInit()
end

function #name:OnUpdate(dt)
end

return #name

)";

		StringUtils::replace(defaultScript, "#name", clazzName);

		std::ofstream out(newPath + clazzName + ".lua");
		out << defaultScript;
		out.flush();
		out.close();
		className = clazzName;

		setFilePath(newPath + clazzName + ".lua");
		reload();
	}


	auto LuaComponent::setFilePath(const std::string& fileName) -> void
	{
		file = fileName;
		className = StringUtils::removeExtension(StringUtils::getFileName(file));
	}


	auto LuaComponent::init() -> void
	{
		className = StringUtils::removeExtension(StringUtils::getFileName(file));
		loadScript();
	}

	LuaComponent::LuaComponent()
	{

	}

	LuaComponent::~LuaComponent()
	{
	
	}

	auto LuaComponent::onInit() -> void
	{
		if (table) 
		{
			(*table)["entity"] = getEntity();
		}

		//load prev values
		if (onInitFunc && onInitFunc->isFunction())
		{
			try
			{
				(*onInitFunc)(*table);
			}
			catch (const std::exception& e)
			{
				LOGE("{0}",e.what());
			}
			metaFile.load(this, file + ".meta", scene);
		}
	}

	auto LuaComponent::onUpdate(float dt) -> void
	{
		if (onUpdateFunc && onUpdateFunc->isFunction())
		{
			try
			{
				(*onUpdateFunc)(*table, dt);
			}
			catch (const std::exception& e) 
			{
				LOGE("{0}", e.what());
			}
		}
	}

	auto LuaComponent::reload()  -> void
	{	
		loadScript();
	}

	auto LuaComponent::loadScript() -> void
	{
		auto & vm = app->getLuaVirtualMachine();
		luaL_dofile(vm->getState(), file.c_str());
		try
		{
			table = std::make_shared<luabridge::LuaRef>(luabridge::LuaRef::fromStack(vm->getState()));
			
			table = std::make_shared<luabridge::LuaRef>((*table)["new"]());
			
			onInitFunc = std::make_shared<luabridge::LuaRef>((*table)["OnInit"]);
			onUpdateFunc = std::make_shared<luabridge::LuaRef>((*table)["OnUpdate"]);
			(*table)["entity"] = getEntity();
		}
		catch (const std::exception& e)
		{
			LOGE("{0}", e.what());
		}
		//OnInit();
	}

	void LuaComponent::onImGui()
	{
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2, 2));
		ImGui::Columns(2);
		ImGui::Separator();
		static char modelName[255] = {};
		if (table && table->isTable()) 
		{
			for (auto&& pair : luabridge::pairs(*table))
			{
				auto name = pair.first.tostring();

				if (name == "__cname" || name == "__index") {
					continue;
				}

				if (pair.second.isNumber()) 
				{
					float number = pair.second;
					if (ImGuiHelper::property(pair.first.tostring(), number))
					{
						(*table)[pair.first.tostring()] = number;
					}
				}
				else if (pair.second.isString()) 
				{
					std::string str = pair.second;
					if (ImGuiHelper::property(pair.first.tostring(), str))
					{
						(*table)[pair.first.tostring()] = str;
					}
				}
				else if (pair.second.isFunction())
				{
					ImGui::Columns(1);
					ImGui::TextUnformatted(name.c_str());
				}
				else if (pair.second.isBool())
				{
					bool value = pair.second;
					if (ImGuiHelper::property(pair.first.tostring(), value))
					{
						(*table)[pair.first.tostring()] = value;
					}
				}
				else if (pair.second.isTable()) 
				{
					ImGui::TextUnformatted(name.c_str());
					ImGui::NextColumn();
					ImGui::PushItemWidth(-1);

					std::string id =  name + " not support now";

					try
					{
						std::string cname = pair.second["__cname"];
						memcpy(modelName, cname.c_str(), cname.size() + 1);
					}
					catch (...)
					{
						
					}
					
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(ImColor(200, 200, 200)));
					ImGui::InputText(id.c_str(), modelName, 255, ImGuiInputTextFlags_ReadOnly);
					ImGui::PopStyleColor();

					ImGui::PopItemWidth();
					ImGui::NextColumn();

				}
				else if (pair.second.isUserdata())
				{
					if (pair.second.isInstance<glm::vec2>()) 
					{
						glm::vec2* v = pair.second;

						if (ImGuiHelper::property(pair.first.tostring(), *v))
						{
						}
					}
					else if (pair.second.isInstance<glm::vec3>())
					{
						glm::vec3* v = pair.second;

						if (ImGuiHelper::property(pair.first.tostring(), *v))
						{
						}
					}
					else if (pair.second.isInstance<glm::vec4>())
					{
						glm::vec4* v = pair.second;

						if (ImGuiHelper::property(pair.first.tostring(), *v))
						{
						}
					}
					else if ((pair.second.isInstance<Entity>() && name != "entity"))
					{
						Entity* v = pair.second;
						if (v != nullptr && v->getHandle() != entt::null)
						{
							auto icon = 
								v->hasComponent<Camera>() ?
								ICON_MDI_CAMERA : 
								v->hasComponent<LuaComponent>() ?
								ICON_MDI_SCRIPT : ICON_MDI_CUBE
								;

							auto entityName = icon + v->getComponent<NameComponent>().name;
							memcpy(modelName, entityName.c_str(), entityName.length() + 1);
						}

			
						ImGui::TextUnformatted(name.c_str());
						ImGui::NextColumn();
						ImGui::PushItemWidth(-1);

						std::string id = "##" + name;
			
						ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(ImColor(200, 200, 200)));
						ImGui::InputText(id.c_str(), modelName, 255, ImGuiInputTextFlags_ReadOnly);
						ImGui::PopStyleColor();

						ImGui::PopItemWidth();
						ImGui::NextColumn();
					
						const ImGuiPayload* payload = ImGui::GetDragDropPayload();
						if (payload != NULL && payload->IsDataType("Drag_Entity"))
						{
							auto& entity = *reinterpret_cast<entt::entity*>(payload->Data);

							if (ImGui::BeginDragDropTarget())
							{
								if (ImGui::AcceptDragDropPayload("Drag_Entity"))
								{
									v->setHandle(entity);
									v->setScene(app->getSceneManager()->getCurrentScene());
								}
								ImGui::EndDragDropTarget();
							}
						}
					}
				}
			}
		}

		ImGui::Columns(1);
		ImGui::Separator();
		ImGui::PopStyleVar();

	}

	auto LuaComponent::isLoaded() -> bool
	{
		return table != nullptr && !table->isNil();
	}

};