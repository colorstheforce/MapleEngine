//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              // 
// Copyright ?2020-2022 Tian Zeng                                           // 

////////////////////////////////////////////////////////////////////////////// 

#pragma once
#include <memory>
#include <cstdint>
#include <vector>

namespace Maple
{
	class Texture;
	class CommandBuffer;

	class SwapChain
	{
	public:
		virtual auto init() -> void = 0;
		static auto create()->std::shared_ptr<SwapChain>;

		inline auto& getSwapChainBuffers() { return swapChainBuffers; }
		inline auto& getTexture(int32_t i) { return swapChainBuffers[i]; }
		inline auto getCurrentBuffer() const { return currentBuffer; }

		virtual auto getCurrentCommandBuffer() -> CommandBuffer* { return nullptr; }

	protected:
		std::vector<std::shared_ptr<Texture>> swapChainBuffers;
		uint32_t currentBuffer = 0;

	public:
		virtual auto resize(uint32_t width, uint32_t height) -> void;
	};
};