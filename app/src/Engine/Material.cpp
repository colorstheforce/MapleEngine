

#include "Material.h"
#include "Engine/Interface/Texture.h"
#include "Engine/Vulkan/VulkanDescriptorSet.h"
#include "Engine/Vulkan/VulkanPipeline.h"
#include "Engine/Vulkan/VulkanUniformBuffer.h"
#include "Engine/Vulkan/VulkanShader.h"

namespace Maple
{

	Material::Material(const std::shared_ptr<Shader>& shader, const MaterialProperties& properties, const PBRMataterialTextures& textures)
		: pbrMaterialTextures(textures), shader(shader)
	{
		setRenderFlag(RenderFlags::DEFERREDRENDER);
		descriptorSet = nullptr;
		setMaterialProperites(properties);
	}

	Material::Material()
	{
		setRenderFlag(RenderFlags::DEFERREDRENDER);
		descriptorSet = nullptr;
	}

	Material::~Material()
	{
	}

	auto Material::loadPBRMaterial(const std::string& name, const std::string& path, const std::string& extension) -> void
	{
	}

	auto Material::loadMaterial(const std::string& name, const std::string& path) -> void
	{
		this->name = name;
		pbrMaterialTextures.albedo = Texture2D::create(name, path);
		pbrMaterialTextures.normal = nullptr;
		pbrMaterialTextures.roughness = nullptr;
		pbrMaterialTextures.metallic = nullptr;
		pbrMaterialTextures.ao = nullptr;
		pbrMaterialTextures.emissive = nullptr;
	}

	auto Material::createDescriptorSet(Pipeline* pipeline, int32_t layoutID, bool pbr) -> void
	{
		if (descriptorSet)
			descriptorSet.reset();

		this->pipeline = pipeline;

		DescriptorInfo info;
		info.pipeline = pipeline;
		info.layoutIndex = layoutID;
		info.shader = pipeline->getShader();

		if (materialPropertiesBuffer == nullptr && pbr)
		{
			materialPropertiesBuffer = std::make_shared<VulkanUniformBuffer>(sizeof(MaterialProperties),nullptr);
		}

		descriptorSet = std::make_shared<VulkanDescriptorSet>(info);

		std::vector<ImageInfo> imageInfos;
		std::vector<BufferInfo> bufferInfos;

		auto getImageInfo = [&](std::shared_ptr<Texture2D> texture,const std::string & name,float & value,int32_t binding) {
			ImageInfo imageInfo1 = {};
			if (texture != nullptr) 
			{
				imageInfo1.textures = { texture };
			}
			else 
			{
				imageInfo1.textures = { Texture2D::getDefaultTexture() };
				value = 0.f;
			}
			imageInfo1.binding = binding;
			imageInfo1.name = name;
			imageInfos.push_back(imageInfo1);
		};

		getImageInfo(pbrMaterialTextures.albedo, "albedoMap", materialProperties.usingAlbedoMap, 0);


		if (pbr)
		{
			getImageInfo(pbrMaterialTextures.metallic, "metallicMap", materialProperties.usingMetallicMap, 1);
			getImageInfo(pbrMaterialTextures.roughness, "roughnessMap", materialProperties.usingRoughnessMap, 2);
			getImageInfo(pbrMaterialTextures.normal, "normalMap", materialProperties.usingNormalMap, 3);
			getImageInfo(pbrMaterialTextures.ao, "aoMap", materialProperties.usingAOMap, 4);
			getImageInfo(pbrMaterialTextures.emissive, "emissiveMap", materialProperties.usingAOMap, 5);


			BufferInfo bufferInfo = {};
			bufferInfo.buffer = materialPropertiesBuffer;
			bufferInfo.offset = 0;
			bufferInfo.size = sizeof(MaterialProperties);
			bufferInfo.type = DescriptorType::UNIFORM_BUFFER;
			bufferInfo.binding = 6;
			bufferInfo.shaderType = ShaderType::FRAGMENT_SHADER;
			bufferInfo.name = "UniformMaterialData";
			bufferInfos.push_back(bufferInfo);
			materialPropertiesBuffer->setData(sizeof(MaterialProperties), &materialProperties);
		}

		descriptorSet->update(imageInfos, bufferInfos);
	}

	auto Material::setTextures(const PBRMataterialTextures& textures) -> void
	{
		pbrMaterialTextures = textures;
	}

	auto Material::setMaterialProperites(const MaterialProperties& properties) -> void
	{
		materialProperties = properties;
		if (materialPropertiesBuffer) {
			materialPropertiesBuffer->setData(sizeof(MaterialProperties), &materialProperties);
		}
	}


	auto Material::setAlbedoTexture(const std::string& path) -> void
	{
		auto tex = Texture2D::create(path, path);
		if (tex)
		{
			pbrMaterialTextures.albedo = tex;
			texturesUpdated = true;
		}
	}

	auto Material::setNormalTexture(const std::string& path) -> void
	{
		auto tex = Texture2D::create(path, path);
		if (tex)
		{
			pbrMaterialTextures.normal = tex;
			texturesUpdated = true;
		}
	}

	auto Material::setRoughnessTexture(const std::string& path) -> void
	{
		auto tex = Texture2D::create(path, path);
		if (tex)
		{
			pbrMaterialTextures.roughness = tex;
			texturesUpdated = true;
		}
	}

	auto Material::setMetallicTexture(const std::string& path) -> void
	{
		auto tex = Texture2D::create(path, path);
		if (tex)
		{
			pbrMaterialTextures.metallic = tex;
			texturesUpdated = true;
		}
	}

	auto Material::setAOTexture(const std::string& path) -> void
	{
		auto tex = Texture2D::create(path, path);
		if (tex)
		{
			pbrMaterialTextures.ao = tex;
			texturesUpdated = true;
		}
	}

	auto Material::setEmissiveTexture(const std::string& path) -> void
	{
		auto tex = Texture2D::create(path, path);
		if (tex)
		{
			pbrMaterialTextures.emissive = tex;
			texturesUpdated = true;
		}
	}

	auto Material::setShader(const std::string& path) -> void
	{
		shader = Shader::create(path);
	}

	auto Material::getShaderPath() const->std::string
	{
		return shader ? shader->getFilePath() : "";
	}

};
