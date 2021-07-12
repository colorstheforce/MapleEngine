
#include "Sprite.h"

#include "Engine/Interface/Texture.h"


namespace Maple
{
    Sprite::Sprite(const std::shared_ptr<Texture2D>& texture,const glm::vec2& position, const glm::vec2& scale, const glm::vec4& color)
    {
		this->quad.position = position;
		this->quad.scale = scale;
		this->quad.color = color;
		this->quad.texCoords = Quad2D::getDefaultTexCoords();
        this->quad.texture = texture;
    }


	Sprite::~Sprite()
    {
    }

    auto Sprite::setSpriteSheet(const std::shared_ptr<Texture2D>& texture, const glm::vec2& index, const glm::vec2& cellSize, const glm::vec2& spriteSize) -> void
    {
        quad.setTexture(texture);
        glm::vec2 min = { 
            (index.x * cellSize.x) / texture->getWidth(), 
            (index.y * cellSize.y) / texture->getHeight() 
        };
        glm::vec2 max = {
            ((index.x + spriteSize.x) * cellSize.x) / texture->getWidth(), 
            ((index.y + spriteSize.y) * cellSize.y) / texture->getHeight()
        };

        quad.texCoords = Quad2D::getTexCoords(min, max);
    }

    auto Sprite::setTextureFromFile(const std::string& filePath) -> void
    {
       quad.setTexture(Texture2D::create(filePath, filePath));
    }


	auto Sprite::getTexturePath() const -> const std::string&
	{
        static std::string nullstr = "";
        return quad.getTexture() ? quad.getTexture()->getFilePath() : nullstr;
	}
}





