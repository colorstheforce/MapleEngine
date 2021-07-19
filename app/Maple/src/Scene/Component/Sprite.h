#pragma once

#include <cereal/cereal.hpp>
#include "Engine/Quad2D.h"
#include "Component.h"
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

		inline auto getQuad() { return quad; }

		template <typename Archive>
		auto save(Archive& archive) const -> void
		{
			std::string newPath = "";

			archive(
				cereal::make_nvp("TexturePath", getTexturePath()),
				/*cereal::make_nvp("Position", quad.position),
				cereal::make_nvp("Scale", quad.scale),
				cereal::make_nvp("Color", quad.color)*/);
		}

		template <typename Archive>
		auto load(Archive& archive) -> void
		{
			std::string textureFilePath;
			archive(
				cereal::make_nvp("TexturePath", textureFilePath)
			/*	cereal::make_nvp("Position", quad.position),
				cereal::make_nvp("Scale", quad.scale),
				cereal::make_nvp("Color", quad.color)*/);

			if (!textureFilePath.empty())
				quad.setTexture(Texture2D::create(textureFilePath, textureFilePath));
		}

		auto getTexturePath() const -> const std::string&;

	protected:
		Quad2D * quad = nullptr;
	};


/*
	class AnimatedSprite : public Sprite
	{
	public:
		enum class PlayMode
		{
			Loop = 0,
			PingPong
		};

		struct AnimationState
		{
			PlayMode mode;
			std::vector<glm::vec2> frames;
			float frameDuration = 1.0f;

			template <typename Archive>
			auto serialize(Archive& archive) -> void
			{
				archive(cereal::make_nvp("PlayMode", mode),
					cereal::make_nvp("Frames", frames),
					cereal::make_nvp("FrameDuration", frameDuration));
			}
		};

		AnimatedSprite();
		AnimatedSprite(const std::shared_ptr<Texture2D>& texture, 
			const glm::vec2& position, 
			const glm::vec2& scale, const std::vector<glm::vec2>& frames, float frameDuration, const std::string& stateName);

		virtual ~AnimatedSprite() = default;

		auto onUpdate(float dt) -> void;
		auto addState(const std::vector<glm::vec2>& frames, float frameDuration, const std::string& stateName) -> void;
		auto setState(const std::string& state) -> void;

		inline auto& getState() const { return state; }
		inline auto& getAnimationStates() { return animationStates; }
		inline auto getAnimatedUVs() -> const std::array<glm::vec2, 4>&;


		template <typename Archive>
		auto save(Archive& archive) const -> void
		{
			archive(cereal::make_nvp("TexturePath", getTexturePath()),
				cereal::make_nvp("Position", quad.position),
				cereal::make_nvp("Scale", quad.scale),
				cereal::make_nvp("Color", quad.color),
				cereal::make_nvp("AnimationFrames", animationStates),
				cereal::make_nvp("State", state));
		}

		template <typename Archive>
		auto load(Archive& archive) -> void
		{
			std::string textureFilePath;
			archive(cereal::make_nvp("TexturePath", textureFilePath),
				cereal::make_nvp("Position", quad.position),
				cereal::make_nvp("Scale", quad.scale),
				cereal::make_nvp("Color", quad.color),
				cereal::make_nvp("AnimationFrames", animationStates),
				cereal::make_nvp("State", state));

			if (!textureFilePath.empty())
				quad.setTexture(Texture2D::create("Animation", textureFilePath));

			setState(state);
		}

		std::unordered_map<std::string, AnimationState> animationStates;
		uint32_t currentFrame = 0;
		float frameTimer = 0.0f;
		std::string state;
		bool forward = true;
	};*/

}
