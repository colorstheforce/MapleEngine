//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////
#include "VulkanPipeline.h"
#include "VulkanRenderPass.h"
#include "VulkanDevice.h"
#include "VulkanContext.h"
#include "VulkanSwapChain.h"
#include "VulkanShader.h"
#include "VulkanCommandBuffer.h"
#include "VulkanDescriptorSet.h"
#include "VulkanShader.h"

#include "Engine/Vertex.h"
#include "Others/Console.h"

#include <memory>

namespace Maple
{
	VulkanPipeline::VulkanPipeline(const PipelineInfo& info)
	{
		init(info);
	}

	VulkanPipeline::~VulkanPipeline()
	{
		unload();
	}

	/**
	 * destory the pipeline 
	 */
	auto VulkanPipeline::unload() const -> void
	{
		vkDestroyDescriptorPool(*VulkanDevice::get(), descriptorPool, nullptr);
		vkDestroyPipelineLayout(*VulkanDevice::get(), pipeLayout, nullptr);
		for (auto a : descriptorSetLayouts)
		{
			vkDestroyDescriptorSetLayout(*VulkanDevice::get(),a, nullptr);
		}
		vkDestroyPipeline(*VulkanDevice::get(), graphicsPipeline, nullptr);
	}


	auto VulkanPipeline::init(const PipelineInfo& info) -> bool
	{
		shader = std::static_pointer_cast<VulkanShader>(info.shader);

		createPipelineLayout();
		createDescriptorPool();
		createDescriptorSet();
	

		std::vector<VkDynamicState> dynamicStateDescriptors;
		std::vector<VkPipelineColorBlendAttachmentState> blendAttachState;


		VkPipelineVertexInputStateCreateInfo vi{};
		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		VkPipelineDynamicStateCreateInfo dynamicStateCI{};
		VkPipelineViewportStateCreateInfo vp{};
		VkPipelineDepthStencilStateCreateInfo ds{};
		VkPipelineMultisampleStateCreateInfo ms{};
		VkPipelineRasterizationStateCreateInfo rs{};
		VkPipelineColorBlendStateCreateInfo cb{};
		
		
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.topology = VkConverter::drawTypeToTopology(info.drawType);
		inputAssembly.primitiveRestartEnable = VK_FALSE;


		createVertexLayout(vi);
		createRasterization(rs, info);
		createColorBlend(cb, blendAttachState, info);
		createViewport(vp, dynamicStateDescriptors);
		if (info.depthBiasEnabled)
			dynamicStateDescriptors.emplace_back(VK_DYNAMIC_STATE_DEPTH_BIAS);
		createDepthStencil(ds);
		createMultisample(ms);


		dynamicStateCI.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicStateCI.pNext = NULL;
		dynamicStateCI.dynamicStateCount = uint32_t(dynamicStateDescriptors.size());
		dynamicStateCI.pDynamicStates = dynamicStateDescriptors.data();


		VkGraphicsPipelineCreateInfo pipelineInfo{};

		pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineInfo.pNext = NULL;
		pipelineInfo.layout = pipeLayout;
		pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
		pipelineInfo.basePipelineIndex = -1;
		pipelineInfo.pVertexInputState = &vi;
		pipelineInfo.pInputAssemblyState = &inputAssembly;
		pipelineInfo.pRasterizationState = &rs;
		pipelineInfo.pColorBlendState = &cb;
		pipelineInfo.pTessellationState = VK_NULL_HANDLE;
		pipelineInfo.pMultisampleState = &ms;
		pipelineInfo.pDynamicState = &dynamicStateCI;
		pipelineInfo.pViewportState = &vp;
		pipelineInfo.pDepthStencilState = &ds;
		auto vkShader = std::static_pointer_cast<VulkanShader>(shader);

		pipelineInfo.pStages =		vkShader->getStageInfos().data();
		pipelineInfo.stageCount =	vkShader->getStageInfos().size();

		pipelineInfo.renderPass = *std::static_pointer_cast<VulkanRenderPass>(info.renderPass);
		pipelineInfo.subpass = 0;

		VK_CHECK_RESULT(vkCreateGraphicsPipelines(*VulkanDevice::get(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline));

		return true;
	}


	auto VulkanPipeline::createDepthStencil(VkPipelineDepthStencilStateCreateInfo& ds) -> void
	{
		ds.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		ds.pNext = NULL;
		ds.depthTestEnable = VK_TRUE;
		ds.depthWriteEnable = VK_TRUE;
		ds.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
		ds.depthBoundsTestEnable = VK_FALSE;
		ds.stencilTestEnable = VK_FALSE;
		ds.back.failOp = VK_STENCIL_OP_KEEP;
		ds.back.passOp = VK_STENCIL_OP_KEEP;
		ds.back.compareOp = VK_COMPARE_OP_ALWAYS;
		ds.back.compareMask = 0;
		ds.back.reference = 0;
		ds.back.depthFailOp = VK_STENCIL_OP_KEEP;
		ds.back.writeMask = 0;
		ds.minDepthBounds = 0;
		ds.maxDepthBounds = 0;
		ds.front = ds.back;
	}

	auto VulkanPipeline::createMultisample(VkPipelineMultisampleStateCreateInfo& ms) -> void
	{
		ms.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		ms.pNext = NULL;
		ms.pSampleMask = NULL;
		ms.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
		ms.sampleShadingEnable = VK_FALSE;
		ms.alphaToCoverageEnable = VK_FALSE;
		ms.alphaToOneEnable = VK_FALSE;
		ms.minSampleShading = 0.0;
	}

	auto VulkanPipeline::createVertexLayout(VkPipelineVertexInputStateCreateInfo& vi) -> void
	{
		auto vkShader = std::static_pointer_cast<VulkanShader>(shader);
		vertexBindingDescription.binding = 0;
		vertexBindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		vertexBindingDescription.stride = vkShader->getVertexInputStride();
		auto & vertexInputAttributeDescription = vkShader->getVertexInputAttributeDescription();

		vi.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vi.pNext = NULL;
		vi.vertexBindingDescriptionCount = 1;
		vi.pVertexBindingDescriptions = &vertexBindingDescription;
		vi.vertexAttributeDescriptionCount = uint32_t(vertexInputAttributeDescription.size());
		vi.pVertexAttributeDescriptions = vertexInputAttributeDescription.data();

	}

	auto VulkanPipeline::createRasterization(VkPipelineRasterizationStateCreateInfo& rs, const PipelineInfo& info) -> void
	{
		rs.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rs.polygonMode = VkConverter::polygonModeToVk(info.polygonMode);
		rs.cullMode = VkConverter::cullModeToVk(info.cullMode);
		
		//TODO there is a bug here.
		//because vulkan Y axis is up to down and different with opengl
		//http://anki3d.org/vulkan-coordinate-system/
		rs.frontFace = VK_FRONT_FACE_CLOCKWISE;

		rs.depthClampEnable = VK_FALSE;
		rs.rasterizerDiscardEnable = VK_FALSE;
		rs.depthBiasEnable = (info.depthBiasEnabled ? VK_TRUE : VK_FALSE);
		rs.depthBiasConstantFactor = 0;
		rs.depthBiasClamp = 0;
		rs.depthBiasSlopeFactor = 0;
		rs.lineWidth = 1.0f;
		rs.pNext = NULL;
	}

	auto VulkanPipeline::createColorBlend(VkPipelineColorBlendStateCreateInfo& cb, std::vector<VkPipelineColorBlendAttachmentState> & blendAttachState, const PipelineInfo& info) -> void
	{
		cb.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		cb.pNext = NULL;
		cb.flags = 0;

		auto vkRenderPass = std::static_pointer_cast<VulkanRenderPass>(info.renderPass);

		for (auto i = 0; i < vkRenderPass->getAttachmentCount(); i++)
		{
			//TODO CubeMap and smaplearray?
			if (vkRenderPass->getAttachmentType(i) == TextureType::COLOR) 
			{
				auto& blend = blendAttachState.emplace_back();
				blend.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
				blend.alphaBlendOp = VK_BLEND_OP_ADD;
				blend.colorBlendOp = VK_BLEND_OP_ADD;

				if (info.transparencyEnabled)
				{
					blend.blendEnable = VK_TRUE;
					blend.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
					blend.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
					blend.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
					blend.dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
				}
				else
				{
					blend.blendEnable = VK_FALSE;
					blend.srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
					blend.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
					blend.srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
					blend.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
				}
			}
		}

		cb.attachmentCount = static_cast<uint32_t>(blendAttachState.size());
		cb.pAttachments = blendAttachState.data();
		cb.logicOpEnable = VK_FALSE;
		cb.logicOp = VK_LOGIC_OP_NO_OP;
		cb.blendConstants[0] = 1.0f;
		cb.blendConstants[1] = 1.0f;
		cb.blendConstants[2] = 1.0f;
		cb.blendConstants[3] = 1.0f;
	}

	auto VulkanPipeline::createViewport(VkPipelineViewportStateCreateInfo& vp, std::vector<VkDynamicState>& dynamicState) -> void
	{
		vp.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		vp.pNext = NULL;
		vp.viewportCount = 1;
		vp.scissorCount = 1;
		vp.pScissors = NULL;
		vp.pViewports = NULL;
		dynamicState.emplace_back(VK_DYNAMIC_STATE_VIEWPORT);
		dynamicState.emplace_back(VK_DYNAMIC_STATE_SCISSOR);
	}

	auto VulkanPipeline::createPipelineLayout() -> void
	{
		auto vkShader = std::static_pointer_cast<VulkanShader>(shader);
		std::vector<std::vector<DescriptorLayoutInfo>> layouts;
		for (auto& descriptorLayout : vkShader->getDescriptorLayoutInfo())
		{
			if (layouts.size() < descriptorLayout.setId + 1)
			{
				layouts.emplace_back();
			}
			layouts[descriptorLayout.setId].emplace_back(descriptorLayout);
		}

		for (auto& l : layouts)
		{
			std::vector<VkDescriptorSetLayoutBinding> bindings;
			bindings.reserve(l.size());

			for (auto i = 0; i < l.size(); i++)
			{
				auto& info = l[i];
				auto& setLayoutBinding = bindings.emplace_back();;
				setLayoutBinding.descriptorType = VkConverter::descriptorTypeToVK(info.type);
				setLayoutBinding.stageFlags = VkConverter::shaderTypeToVK(info.stage);
				setLayoutBinding.binding = info.binding;
				setLayoutBinding.descriptorCount = info.count;
			}

			// Pipeline layout
			VkDescriptorSetLayoutCreateInfo info{};
			info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			info.bindingCount = static_cast<uint32_t>(bindings.size());
			info.pBindings = bindings.data();
			auto& layout = descriptorSetLayouts.emplace_back();
			vkCreateDescriptorSetLayout(*VulkanDevice::get(), &info, VK_NULL_HANDLE, &layout);
		}

		auto& pushConsts = shader->getPushConstants();
		std::vector<VkPushConstantRange> pushConstantRanges;
		for (auto& pushConst : pushConsts)
		{
			pushConstantRanges.emplace_back(VulkanHelper::pushConstantRange(VkConverter::shaderTypeToVK(pushConst.shaderStage), pushConst.size, pushConst.offset));
		}

		VkPipelineLayoutCreateInfo pipelineLayoutCreateInfo{};
		pipelineLayoutCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipelineLayoutCreateInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
		pipelineLayoutCreateInfo.pSetLayouts = descriptorSetLayouts.data();
		pipelineLayoutCreateInfo.pushConstantRangeCount = uint32_t(pushConstantRanges.size());
		pipelineLayoutCreateInfo.pPushConstantRanges = pushConstantRanges.data();

		VK_CHECK_RESULT(vkCreatePipelineLayout(*VulkanDevice::get(), &pipelineLayoutCreateInfo, VK_NULL_HANDLE, &pipeLayout));
	}

	auto VulkanPipeline::createDescriptorPool() -> void
	{
		constexpr std::array<VkDescriptorPoolSize, 6> poolSizes =
		{
			VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_SAMPLER,                   200 },
			VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,    200 },
			VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE,             200 },
			VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,             200 },
			VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,            200 },
			VkDescriptorPoolSize{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,    200 }
		};
		VkDescriptorPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.flags = 0;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = MAX_DESCRIPTOR_SET;
		VK_CHECK_RESULT(vkCreateDescriptorPool(*VulkanDevice::get(), &poolInfo, nullptr, &descriptorPool));
	}


	auto VulkanPipeline::createDescriptorSet() -> void
	{
		DescriptorInfo descripInfo;
		descripInfo.pipeline = this;
		descripInfo.layoutIndex = 0;
		descripInfo.shader = shader;
		descriptorSet = std::make_shared<VulkanDescriptorSet>(descripInfo);
	}

	auto VulkanPipeline::bind(CommandBuffer* buffer) -> void
	{
		vkCmdBindPipeline(*static_cast<VulkanCommandBuffer*>(buffer), VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
	}
};