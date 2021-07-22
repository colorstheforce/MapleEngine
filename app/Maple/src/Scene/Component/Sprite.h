#pragma once

#include <cereal/cereal.hpp>
#include "Engine/Quad2D.h"
#include "Component.h"
#include <glm/glm.hpp>

namespace Maple
{
	class Texture2D;

	class Sprite : public Component
	{
	public:
		Sprite();
		Sprite(const std::string & uniqueName,const std::vector<uint8_t> & data,uint32_t width,uint32_t height);
		virtual ~Sprite();

		auto setTextureFromFile(const std::string& filePath) -> void;

		virtual auto getQuad() -> const Quad2D& { return quad; }

		template <typename Archive>
		auto save(Archive& archive) const -> void
		{
			std::string newPath = "";
			archive(
				cereal::make_nvp("TexturePath", getTexturePath()),
			);
		}

		template <typename Archive>
		auto load(Archive& archive) -> void
		{
			std::string textureFilePath;
			archive(
				cereal::make_nvp("TexturePath", textureFilePath)
			);

			if (!textureFilePath.empty())
				loadQuad(textureFilePath);
		}

		inline auto getWidth() const { return quad.getWidth(); }
		inline auto getHeight() const { return quad.getHeight(); }

		auto getTexturePath() const -> const std::string&;
	protected:
		auto loadQuad(const std::string& path) -> void;
		Quad2D quad;
	};



	class AnimatedSprite : public Sprite
	{
	public:
		
		struct AnimationFrame
		{
			uint32_t width;
			uint32_t height;
			float delay;
			std::string uniqueKey;
			Quad2D quad;
		};

		AnimatedSprite();
		virtual ~AnimatedSprite() = default;

		auto addFrame(const std::vector<uint8_t>& data, uint32_t width, uint32_t height, float delay, const std::string& uniqueKey,float xOffset,float yOffset,uint32_t color = UINT32_MAX) -> void;
		auto onUpdate(float dt) -> void;
		auto getAnimatedUVs() -> const std::array<glm::vec2, 4>&;
		auto getQuad()->const Quad2D & override;
		inline auto getCurrentFrame() const -> const AnimationFrame* {
			if (currentFrame < animationFrames.size()) {
				return &animationFrames[currentFrame];
			} return nullptr;
		};
		inline auto setLoop(bool val) { loop = val; }


		inline auto getWidth() const { 
			auto frame = getCurrentFrame();
			return frame ? frame->width : 0;
		}
		inline auto getHeight() const { 
			auto frame = getCurrentFrame();
			return frame ? frame->height : 0;
		}

	private:
		uint32_t currentFrame = 0;
		float frameTimer = 0.0f;
		bool loop = true;
		std::vector<AnimationFrame> animationFrames;
	};

}
