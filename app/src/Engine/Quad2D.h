//////////////////////////////////////////////////////////////////////////////
// This file is part of the Maple Engine                              //
// Copyright ?2020-2022 Tian Zeng                                           //
//////////////////////////////////////////////////////////////////////////////

#pragma once
#include <glm/glm.hpp>
#include <memory>
#include <array>

namespace Maple 
{
	class Texture2D;

	class Quad2D  
	{
	public:
		Quad2D();
		virtual ~Quad2D() = default;
		inline auto getTexture() const { return texture; }
		inline auto getPosition() const { return position; }
		inline auto getScale() const { return scale; }
		inline auto& getColor() const { return color; }
		inline auto getTexCoords() const { return texCoords; }
		
		static auto getDefaultTexCoords() -> const std::array<glm::vec2, 4>&;
		static auto getTexCoords(const glm::vec2& min, const glm::vec2& max)-> 	const std::array<glm::vec2, 4>&;

		inline auto setPosition(const glm::vec2& vector2) { position = vector2; };
		inline auto setColor(const glm::vec4& c) { color = c; }
		inline auto setScale(const glm::vec2& s) { scale = s; }
		inline auto setTexture(const std::shared_ptr<Texture2D>& texture) { this->texture = texture; }
		friend class Sprite;
	protected:
		std::shared_ptr<Texture2D> texture;
		glm::vec2 position = {};
		glm::vec2 scale = {};
		glm::vec4 color = {};
		std::array<glm::vec2, 4> texCoords = {};
	};



};