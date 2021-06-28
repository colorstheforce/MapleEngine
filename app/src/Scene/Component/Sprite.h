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
		Sprite(const std::shared_ptr<Texture2D>& texture = nullptr,const glm::vec2& position = glm::vec2(0.0f, 0.0f), const glm::vec2& scale = glm::vec2(1.0f, 1.0f), const glm::vec4& color = glm::vec4(1.0f));
		virtual ~Sprite();

		auto setSpriteSheet(const std::shared_ptr<Texture2D>& texture, const glm::vec2& index, const glm::vec2& cellSize, const glm::vec2& spriteSize) -> void;
		auto setTextureFromFile(const std::string& filePath) -> void;

		inline auto& getQuad() { return quad; }

		template <typename Archive>
		auto save(Archive& archive) const -> void
		{
			std::string newPath = "";

			archive(
				cereal::make_nvp("TexturePath", getTexturePath()),
				cereal::make_nvp("Position",  position),
				cereal::make_nvp("Scale",     scale),
				cereal::make_nvp("Color",     color));
		}

		template <typename Archive>
		auto load(Archive& archive) -> void
		{
			std::string textureFilePath;
			archive(
				cereal::make_nvp("TexturePath", textureFilePath),
				cereal::make_nvp("Position", position),
				cereal::make_nvp("Scale", scale),
				cereal::make_nvp("Color", color));

			if (!textureFilePath.empty())
				quad.setTexture(Texture2D::create(textureFilePath, textureFilePath));
		}

		auto getTexturePath() const -> const std::string&;

	private:
		Quad2D quad;
	};
}
