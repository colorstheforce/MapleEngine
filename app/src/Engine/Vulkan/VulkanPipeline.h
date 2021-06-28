//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////
#pragma once
#include "Engine/Interface/BufferLayout.h"
#include "Engine/Interface/Pipeline.h"
#include "Engine/Renderer/RenderParam.h"
#include "VulkanHelper.h"
#include <memory>
#include <functional>

namespace Maple
{

	class Shader;
	class VulkanShader;
	class RenderPass;
	class CommandBuffer;



	class VulkanPipeline : public Pipeline
	{
	public:
		constexpr static uint32_t MAX_DESCRIPTOR_SET = 1500;

		VulkanPipeline(const PipelineInfo& info);
		virtual ~VulkanPipeline();

		auto unload() const -> void;
		auto bind(CommandBuffer* buffer) -> void override;
		auto init(const PipelineInfo& info) -> bool;
		//bind to layoutId = 0;
		auto getDescriptorSet() -> std::shared_ptr<DescriptorSet> override { return descriptorSet; };


		inline const auto& getPipelineLayout() const { return pipeLayout; };
		inline const auto& getDescriptorPool() const { return descriptorPool; };
		inline auto& getDescriptorPool() { return descriptorPool; };

		inline const auto& getGraphicsPipeline() const { return graphicsPipeline; };

		inline const auto getDescriptorLayout(uint32_t layoutIndex) const {
			return &descriptorSetLayouts[layoutIndex];
		}
		
		inline auto& getDescriptorLayouts()  {
			return descriptorSetLayouts;
		}


		auto getShader() -> std::shared_ptr<Shader> override { return shader; }

	private:
		auto createDepthStencil(VkPipelineDepthStencilStateCreateInfo& ds) -> void;
		auto createMultisample(VkPipelineMultisampleStateCreateInfo& ms) -> void;
		auto createVertexLayout(VkPipelineVertexInputStateCreateInfo& vi) -> void;
		auto createRasterization(VkPipelineRasterizationStateCreateInfo & vi,const PipelineInfo& info) -> void;
		auto createColorBlend(VkPipelineColorBlendStateCreateInfo& cb, std::vector<VkPipelineColorBlendAttachmentState>& blendAttachState, const PipelineInfo& info) -> void;
		auto createViewport(VkPipelineViewportStateCreateInfo& cb, std::vector<VkDynamicState> & dynamicState) -> void;
		auto createPipelineLayout() -> void;
		auto createDescriptorPool() -> void;
		auto createDescriptorSet() -> void;

		VkPipelineLayout pipeLayout = nullptr;
		VkPipeline graphicsPipeline = nullptr;

		VkDescriptorPool descriptorPool;
		std::shared_ptr<Shader> shader;
		std::shared_ptr<DescriptorSet> descriptorSet;
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
		VkVertexInputBindingDescription vertexBindingDescription;
	};
};