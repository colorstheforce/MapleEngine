//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Game Engine			                    //
//////////////////////////////////////////////////////////////////////////////

#pragma once
#include <string>
#include <memory>
#include "Component.h"

namespace Maple
{
	class CameraController;

	class MAPLE_EXPORT CameraControllerComponent : public Component
	{
	public:
		enum class ControllerType : int32_t
		{
			FPS = 0,
			EditorCamera,
			Custom
		};

		static std::string typeToString(ControllerType type);
		static ControllerType stringToType(const std::string& type);

		CameraControllerComponent()
			: type(ControllerType::FPS)
		{
		}

		CameraControllerComponent(ControllerType type);
	
		void setControllerType(CameraControllerComponent::ControllerType type);

		inline auto & getController() {return cameraController;}

		template<typename Archive>
		void save(Archive& archive) const
		{
			archive(cereal::make_nvp("ControllerType", type), cereal::make_nvp("Id", entity));
		}

		template<typename Archive>
		void load(Archive& archive)
		{
			archive(cereal::make_nvp("ControllerType",type), cereal::make_nvp("Id", entity));
			setControllerType(type);
		}

		inline auto getType() const 
		{
			return type;
		}

	private:
		ControllerType type = ControllerType::FPS;
		std::shared_ptr<CameraController> cameraController;
	};
};