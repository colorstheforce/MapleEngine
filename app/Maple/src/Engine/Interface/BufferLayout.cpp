
#include "BufferLayout.h"



namespace Maple
{
	
	BufferLayout::BufferLayout()
		: size(0)
	{
	}

	auto BufferLayout::push(const std::string& name, Format format, uint32_t size, bool normalized) -> void
	{
		layout.push_back({ name, format, this->size, normalized });
		this->size += size;
	}

	template<>
	auto BufferLayout::push<uint32_t>(const std::string& name, bool normalized) -> void
	{
		push(name, Format::R32_UINT, sizeof(uint32_t), normalized);
	}

	template<>
	auto BufferLayout::push<uint8_t>(const std::string& name, bool normalized) -> void
	{
		push(name, Format::R8_UINT, sizeof(uint8_t), normalized);
	}

	template<>
	auto BufferLayout::push<float>(const std::string& name, bool normalized) -> void
	{
		push(name, Format::R32_FLOAT, sizeof(float), normalized);
	}

	template<>
	auto BufferLayout::push<glm::vec2>(const std::string& name, bool normalized) -> void
	{
		push(name, Format::R32G32_FLOAT, sizeof(glm::vec2), normalized);
	}

	template<>
	auto BufferLayout::push<glm::vec3>(const std::string& name, bool normalized) -> void
	{
		push(name, Format::R32G32B32_FLOAT, sizeof(glm::vec3), normalized);
	}

	template<>
	auto BufferLayout::push<glm::vec4>(const std::string& name, bool normalized)  -> void
	{
		push(name, Format::R32G32B32A32_FLOAT, sizeof(glm::vec4), normalized);
	}


}
