
//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////
#pragma once
#include <glm/glm.hpp>
#include <string>
#include <memory>
#include <unordered_map>
#include "Engine/Core.h"
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
		glm::vec4 albedoColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		glm::vec4 roughnessColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		glm::vec4 metallicColor = glm::vec4(1.0f, 1.0f, 1.0f, 1.0f);
		glm::vec4 emissiveColor = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
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
	class MAPLE_EXPORT Material
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
			return shader;
		}

		inline auto getDescriptorSet(Pipeline * pipeline) 
		{
			return descriptorSets[pipeline];
		}

		inline auto getRenderFlags() const
		{
			return renderFlags;
		}
		inline auto& GetName() const
		{
			return name;
		}

		inline const auto& getProperties() const
		{
			return materialProperties;
		}

		inline auto& getProperties() 
		{
			return materialProperties;
		}

		auto setShader(const std::string& path) -> void;

		template <typename Archive>
		void save(Archive& archive) const
		{
			archive(
				cereal::make_nvp("Albedo", pbrMaterialTextures.albedo ? pbrMaterialTextures.albedo->getFilePath() : ""),
				cereal::make_nvp("Normal", pbrMaterialTextures.normal ? pbrMaterialTextures.normal->getFilePath() : ""),
				cereal::make_nvp("Metallic", pbrMaterialTextures.metallic ? pbrMaterialTextures.metallic->getFilePath() : ""),
				cereal::make_nvp("Roughness", pbrMaterialTextures.roughness ? pbrMaterialTextures.roughness->getFilePath() : ""),
				cereal::make_nvp("AO", pbrMaterialTextures.ao ? pbrMaterialTextures.ao->getFilePath() : ""),
				cereal::make_nvp("Emissive", pbrMaterialTextures.emissive ? pbrMaterialTextures.emissive->getFilePath() : ""),
				cereal::make_nvp("albedoColour", materialProperties.albedoColor),
				cereal::make_nvp("roughnessColour", materialProperties.roughnessColor),
				cereal::make_nvp("metallicColour", materialProperties.metallicColor),
				cereal::make_nvp("emissiveColour", materialProperties.emissiveColor),
				cereal::make_nvp("usingAlbedoMap", materialProperties.usingAlbedoMap),
				cereal::make_nvp("usingMetallicMap", materialProperties.usingMetallicMap),
				cereal::make_nvp("usingRoughnessMap", materialProperties.usingRoughnessMap),
				cereal::make_nvp("usingNormalMap", materialProperties.usingNormalMap),
				cereal::make_nvp("usingAOMap", materialProperties.usingAOMap),
				cereal::make_nvp("usingEmissiveMap", materialProperties.usingEmissiveMap),
				cereal::make_nvp("workflow", materialProperties.workflow),
				cereal::make_nvp("shader", getShaderPath()));
		}

		template <typename Archive>
		void load(Archive& archive)
		{
			std::string albedoFilePath;
			std::string normalFilePath;
			std::string roughnessFilePath;
			std::string metallicFilePath;
			std::string emissiveFilePath;
			std::string aoFilePath;
			std::string shaderFilePath;

			archive(cereal::make_nvp("Albedo", albedoFilePath),
				cereal::make_nvp("Normal", normalFilePath),
				cereal::make_nvp("Metallic", metallicFilePath),
				cereal::make_nvp("Roughness", roughnessFilePath),
				cereal::make_nvp("AO", aoFilePath),
				cereal::make_nvp("Emissive", emissiveFilePath),
				cereal::make_nvp("albedoColour", materialProperties.albedoColor),
				cereal::make_nvp("roughnessColour", materialProperties.roughnessColor),
				cereal::make_nvp("metallicColour", materialProperties.metallicColor),
				cereal::make_nvp("emissiveColour", materialProperties.emissiveColor),
				cereal::make_nvp("usingAlbedoMap", materialProperties.usingAlbedoMap),
				cereal::make_nvp("usingMetallicMap", materialProperties.usingMetallicMap),
				cereal::make_nvp("usingRoughnessMap", materialProperties.usingRoughnessMap),
				cereal::make_nvp("usingNormalMap", materialProperties.usingNormalMap),
				cereal::make_nvp("usingAOMap", materialProperties.usingAOMap),
				cereal::make_nvp("usingEmissiveMap", materialProperties.usingEmissiveMap),
				cereal::make_nvp("workflow", materialProperties.workflow),
				cereal::make_nvp("shader", shaderFilePath)
			);

			if (!shaderFilePath.empty())
				setShader(shaderFilePath);

			if (!albedoFilePath.empty())
				pbrMaterialTextures.albedo = Texture2D::create("albedo", albedoFilePath);
			if (!normalFilePath.empty())
				pbrMaterialTextures.normal = Texture2D::create("roughness", normalFilePath);
			if (!metallicFilePath.empty())
				pbrMaterialTextures.metallic = Texture2D::create("metallic", metallicFilePath);
			if (!roughnessFilePath.empty())
				pbrMaterialTextures.roughness = Texture2D::create("roughness", roughnessFilePath);
			if (!emissiveFilePath.empty())
				pbrMaterialTextures.emissive = Texture2D::create("emissive", emissiveFilePath);
			if (!aoFilePath.empty())
				pbrMaterialTextures.ao = Texture2D::create("ao", aoFilePath);
		}

		auto getShaderPath() const->std::string;

	private:
		PBRMataterialTextures pbrMaterialTextures;
		MaterialProperties materialProperties;

		std::shared_ptr	<Shader> shader;
		std::shared_ptr <UniformBuffer> materialPropertiesBuffer;
		//std::shared_ptr <DescriptorSet> descriptorSet;
		//Pipeline* pipeline = nullptr;
		std::string name;
		bool texturesUpdated = false;
		std::unordered_map<Pipeline*, std::shared_ptr <DescriptorSet>> descriptorSets;

	};


}


