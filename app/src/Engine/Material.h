
//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////
#pragma once
#include <glm/glm.hpp>
#include <string>
#include <memory>

namespace Maple
{
	class Texture2D;
	class Shader;
	class Pipeline;
	class UniformBuffer;
	class DescriptorSet;

	const float PBR_WORKFLOW_SEPARATE_TEXTURES = 0.0f;
	const float PBR_WORKFLOW_METALLIC_ROUGHNESS = 1.0f;
	const float PBR_WORKFLOW_SPECULAR_GLOSINESS = 2.0f;

	struct MaterialProperties
	{
		glm::vec4 albedoColour = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		glm::vec4 roughnessColour = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		glm::vec4 metallicColour = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		glm::vec4 emissiveColour = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
		float usingAlbedoMap = 1.0f;
		float usingMetallicMap = 1.0f;
		float usingRoughnessMap = 1.0f;
		float usingNormalMap = 1.0f;
		float usingAOMap = 1.0f;
		float usingEmissiveMap = 1.0f;
		float workflow = PBR_WORKFLOW_SEPARATE_TEXTURES;

		//padding in vulkan
		float padding = 0.0f;
	};


	struct PBRMataterialTextures
	{
		std::shared_ptr<Texture2D> albedo;
		std::shared_ptr<Texture2D> normal;
		std::shared_ptr<Texture2D> metallic;
		std::shared_ptr<Texture2D> roughness;
		std::shared_ptr<Texture2D> ao;
		std::shared_ptr<Texture2D> emissive;
	};
#define BIT(x) (1 << x)
	class Material
	{
	public:
		enum class RenderFlags
		{
			NONE = 0,
			DISABLE_DEPTH_TEST = BIT(0),
			WIREFRAME = BIT(1),
			FORWARDRENDER = BIT(2),
			DEFERREDRENDER = BIT(3),
			NOSHADOW = BIT(4),
		};

	protected:
		int32_t renderFlags = 0;

	public:
		Material(const std::shared_ptr<Shader>& shader, const MaterialProperties& properties = MaterialProperties(), const PBRMataterialTextures& textures = PBRMataterialTextures());
		Material();

		~Material();

		inline auto setRenderFlags(int32_t flags)
		{
			renderFlags = flags;
		}
		inline auto setRenderFlag(Material::RenderFlags flag)
		{
			renderFlags |= static_cast<int32_t>(flag);
		}
		auto loadPBRMaterial(const std::string& name, const std::string& path, const std::string& extension = ".png") -> void;
		auto loadMaterial(const std::string& name, const std::string& path)-> void;
		auto createDescriptorSet(Pipeline * pipeline, int32_t layoutID, bool pbr = true)-> void;
		auto setTextures(const PBRMataterialTextures& textures)-> void;
		auto setMaterialProperites(const MaterialProperties& properties)-> void;
		auto setAlbedoTexture(const std::string& path)-> void;
		auto setNormalTexture(const std::string& path)-> void;
		auto setRoughnessTexture(const std::string& path)-> void;
		auto setMetallicTexture(const std::string& path)-> void;
		auto setAOTexture(const std::string& path)-> void;
		auto setEmissiveTexture(const std::string& path)-> void;

		inline auto& getTexturesUpdated()
		{
			return texturesUpdated;
		}
		inline auto& getTexturesUpdated()const
		{
			return texturesUpdated;
		}


		inline auto setTexturesUpdated(bool updated)
		{
			texturesUpdated = updated;
		}

		inline auto& getTextures()
		{
			return pbrMaterialTextures;
		}
		inline const auto& getTextures() const
		{
			return pbrMaterialTextures;
		}

		inline auto getShader() const
		{
			return shader.get();
		}
		inline auto getDescriptorSet() const
		{
			return descriptorSet;
		}
		inline auto getPipeline() const
		{
			return pipeline;
		}
		inline auto getRenderFlags() const
		{
			return renderFlags;
		}
		inline auto& GetName() const
		{
			return name;
		}

		inline auto getProperties() const
		{
			return materialProperties;
		}

	private:
		PBRMataterialTextures pbrMaterialTextures;
		MaterialProperties materialProperties;

		std::shared_ptr	<Shader> shader;
		std::shared_ptr <UniformBuffer> materialPropertiesBuffer;
		std::shared_ptr <DescriptorSet> descriptorSet;
		Pipeline* pipeline = nullptr;
		std::string name;
		bool texturesUpdated = false;
	};


}


