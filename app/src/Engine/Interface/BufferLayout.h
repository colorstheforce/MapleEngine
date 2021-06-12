//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////

#pragma once

#include <glm/glm.hpp>
#include <string>
#include "Engine/Vulkan/VulkanDescriptorSet.h"

namespace Maple
{
	
	struct BufferElement
	{
		std::string name;
		Format format;
		uint32_t offset = 0;
		bool normalized = false;
	};

	class BufferLayout
	{
	private:
		uint32_t size;
		std::vector<BufferElement> layout;

	public:
		BufferLayout();

		template<typename T>
		auto push(const std::string& name, bool normalized = false) -> void
		{
			
		}
		inline const auto& getLayout() const
		{
			return layout;
		}
		inline auto getStride() const
		{
			return size;
		}

	private:
		auto push(const std::string& name, Format format, uint32_t size, bool normalized) -> void;
	};

	template<>
	auto BufferLayout::push<float>(const std::string& name, bool normalized) -> void;
	template<>
	auto BufferLayout::push<uint32_t>(const std::string& name, bool normalized)-> void;
	template<>
	auto BufferLayout::push<uint8_t>(const std::string& name, bool normalized)-> void;
	template<>
	auto BufferLayout::push<glm::vec2>(const std::string& name, bool normalized)-> void;
	template<>
	auto BufferLayout::push<glm::vec3>(const std::string& name, bool normalized)-> void;
	template<>
	auto BufferLayout::push<glm::vec4>(const std::string& name, bool normalized)-> void;
	

}
