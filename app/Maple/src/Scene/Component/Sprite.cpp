
#include "Sprite.h"
#include "Engine/Interface/Texture.h"
#include "Application.h"

namespace Maple
{

	Sprite::Sprite(const std::string& uniqueName, const std::vector<uint8_t>& data, uint32_t width, uint32_t height)
	{
		quad = app->getTexturePool()->addSprite(uniqueName, data, width, height);
		
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

/*

    AnimatedSprite::AnimatedSprite()
    {

    }


    AnimatedSprite::AnimatedSprite(const std::shared_ptr<Texture2D>& texture, const glm::vec2& position, const glm::vec2& scale, const std::vector<glm::vec2>& frames, float frameDuration, const std::string& stateName)
        :Sprite(texture,position,scale), state(stateName)
    {
		AnimationState state;
		state.frames = frames;
		state.frameDuration = frameDuration;
		state.mode = PlayMode::Loop;

		animationStates[stateName] = state;

		currentFrame = 0;
    }


    auto AnimatedSprite::onUpdate(float dt) -> void
    {
		if (animationStates.find(state) == animationStates.end())
			return;

		frameTimer += dt;

		auto& currentState = animationStates[state];
		if (frameTimer >= currentState.frameDuration)
		{
			frameTimer = 0.0f;

			if (forward)
			{

				if (currentFrame == currentState.frames.size() - 1)
				{
					if (currentState.mode == PlayMode::PingPong)
					{
						currentFrame--;
						forward = false;
					}
					else
					{
						currentFrame = 0;
					}
				}
				else
					currentFrame++;
			}
			else
			{
				if (currentFrame == 0)
				{
					if (currentState.mode == PlayMode::PingPong)
					{
						currentFrame = 1;
						forward = true;
					}
					else
					{
						currentFrame = 1;
						forward = true;///???
					}
				}
				else
					currentFrame--;
			}

			getAnimatedUVs();
		}
    }


    auto AnimatedSprite::addState(const std::vector<glm::vec2>& frames, float frameDuration, const std::string& stateName) -> void
    {
		auto& state = animationStates[stateName];
		state.frames = frames;
		state.frameDuration = frameDuration;
		state.mode = PlayMode::Loop;
    }

	auto AnimatedSprite::setState(const std::string& stateName) -> void
	{
		state = stateName;
		currentFrame = 0;
		frameTimer = 0.0f;
		getAnimatedUVs();
	}

	auto AnimatedSprite::getAnimatedUVs() -> const std::array<glm::vec2, 4>&
	{
		auto& currentState = animationStates[state];
        if (animationStates.find(state) == animationStates.end() || currentState.frames.empty())
            return quad.getDefaultTexCoords();

		auto min = currentState.frames[currentFrame];
		auto max = currentState.frames[currentFrame] + quad.getScale();

		min.x /= quad.getTexture()->getWidth();
		min.y /= quad.getTexture()->getHeight();

		max.x /= quad.getTexture()->getWidth();
		max.y /= quad.getTexture()->getHeight();
       

		return quad.getTexCoords(min, max);
	}
*/

}





