
#include "Sprite.h"
#include "Engine/Interface/Texture.h"
#include "Application.h"

namespace Maple
{

	Sprite::Sprite(const std::string& uniqueName, const std::vector<uint8_t>& data, uint32_t width, uint32_t height)
	{
		if (auto q = app->getTexturePool()->addSprite(uniqueName, data, width, height)) {
			quad = *q;
		}
	}

	Sprite::Sprite()
	{

	}

	Sprite::~Sprite()
    {
    }
    
    auto Sprite::setTextureFromFile(const std::string& filePath) -> void
    {
   //    quad->setTexture(Texture2D::create(filePath, filePath));
    }


	auto Sprite::getTexturePath() const -> const std::string&
	{
        static std::string nullstr = "";
        return /*quad.getTexture() ? quad.getTexture()->getFilePath() :*/ nullstr;
	}

	auto Sprite::loadQuad(const std::string& path) -> void
	{
		if (auto q = app->getTexturePool()->addSprite(path)) {
			quad = *q;
		}
	}


    AnimatedSprite::AnimatedSprite()
    {

    }

	auto AnimatedSprite::addFrame(const std::vector<uint8_t>& data, uint32_t width, uint32_t height, float delay, const std::string& uniqueKey, float xOffset, float yOffset, uint32_t color) -> void
	{
		auto quad = app->getTexturePool()->addSprite(uniqueKey, data, width, height);
		if (quad != nullptr)
		{
			auto& back = animationFrames.emplace_back();
			back.width = width;
			back.height = height;
			back.delay = delay; 
			back.uniqueKey = uniqueKey;
			back.quad = *quad;
		
			struct color8888
			{
				uint8_t r : 8;
				uint8_t g : 8;
				uint8_t b : 8;
				uint8_t a : 8;
			};

			color8888* c = reinterpret_cast<color8888*>(&color);
			back.quad.setColor({ c->r / 255.f,c->g / 255.f,c->b / 255.f,c->a / 255.f });
			back.quad.setOffset({ xOffset,-yOffset - height });
		}
	}

	auto AnimatedSprite::onUpdate(float dt) -> void
	{
		frameTimer += dt;
		if (currentFrame < animationFrames.size()) {
			auto& frame = animationFrames[currentFrame];
			if (frameTimer >= frame.delay) {
				frameTimer = 0;
				currentFrame++;
				if (currentFrame >= animationFrames.size()) {
					currentFrame = loop ? 0 : animationFrames.size() - 1;
				}
			}
		}
    }

	auto AnimatedSprite::getAnimatedUVs() -> const std::array<glm::vec2, 4>&
	{
		if (currentFrame < animationFrames.size()) {
			auto& frame = animationFrames[currentFrame];
			return frame.quad.getTexCoords();
		}
		return Quad2D::getTexCoords({ 0,0 }, {0, 0});
	}


	auto AnimatedSprite::getQuad() -> const Quad2D&
	{
		if (currentFrame < animationFrames.size()) {
			auto& frame = animationFrames[currentFrame];
			return frame.quad;
		}
		return Quad2D::nullQuad;
	}
}





