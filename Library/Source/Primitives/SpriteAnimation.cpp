/******************************************************************************/
/*!
\file	SpriteAnimation.cpp
\author Lim Chian Song
\par	email:
\brief
Sprite Animation
*/
/******************************************************************************/
#include "SpriteAnimation.h"
#include "GL\glew.h"

/**
 @brief Constructor This constructor has protected access modifier as this class will be a Singleton
 */
CSpriteAnimation::CSpriteAnimation(int row, int col)
	: CMesh()
	, row(row)
	, col(col)
	, currentTime(0)
	, currentFrame(0)
	, playCount(0)
	, currentAnimation("")
{
}

/**
 @brief Destructor This destructor has protected access modifier as this class will be a Singleton
 */
CSpriteAnimation::~CSpriteAnimation()
{
	//Loop through the animation list
	for (auto iter = animationList.begin(); iter != animationList.end(); ++iter)
	{
		if (iter->second != NULL)
		{
			//delete the animation
			delete iter->second;
		}
	}
	//Remove everything
	animationList.clear();

	glDisableVertexAttribArray(3);
	glDisableVertexAttribArray(2);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);
}

/**
 @brief Init this class instance by configuring the shaders
 */
void CSpriteAnimation::Init(void)
{
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glEnableVertexAttribArray(3);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)sizeof(glm::vec3));
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(glm::vec3) + sizeof(glm::vec3)));
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(glm::vec3) + sizeof(glm::vec3) + sizeof(glm::vec4)));

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
}

/**
 @brief Update this instance
 @param dElapsedTime A const double variable contains the time since the last frame
 @return A bool variable to indicate this method successfully completed its tasks
 */
bool CSpriteAnimation::Update(const double dElapsedTime)
{
	//Check if the current animation is active
	if (animationList[currentAnimation]->animActive)
	{
		//Add the delta time
		currentTime += static_cast<float>(dElapsedTime);

		//Get the number of frame to get the frame per second
		int numFrame = animationList[currentAnimation]->frames.size();
		float frameTime = animationList[currentAnimation]->animTime / numFrame;

		//Set the current frame based on the current time
		currentFrame = animationList[currentAnimation]->frames[(int)fmin((int)animationList[currentAnimation]->frames.size() - 1 , static_cast<int>(currentTime / frameTime))];

		//check if the current time is more than the total animated time
		if (currentTime >= animationList[currentAnimation]->animTime)
		{
			//if it is less than the repeat count, increase the count and repeat
			if (playCount < animationList[currentAnimation]->repeatCount)
			{
				++playCount;
				currentTime = 0;
				currentFrame = animationList[currentAnimation]->frames[0];
			}
			//if we repeat count is 0 or we have reach same number of play count
			else
			{
				animationList[currentAnimation]->animActive = false;
				animationList[currentAnimation]->ended = true;
			}

			//If the animaton is infinite
			if (animationList[currentAnimation]->repeatCount == -1)
			{
				currentTime = 0.f;
				currentFrame = animationList[currentAnimation]->frames[0];
				animationList[currentAnimation]->animActive = true;
				animationList[currentAnimation]->ended = false;
			}
		}
	}

	return true;
}

/**
 @brief Render this instance
 */
void CSpriteAnimation::Render(void)
{
	//glEnableVertexAttribArray(0);
	//glEnableVertexAttribArray(1);
	//glEnableVertexAttribArray(2);
	//glEnableVertexAttribArray(3);

	//glBindBuffer(GL_ARRAY_BUFFER, VBO);
	//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
	//glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)sizeof(glm::vec3));
	//glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(glm::vec3) + sizeof(glm::vec3)));
	//glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(sizeof(glm::vec3) + sizeof(glm::vec3) + sizeof(glm::vec4)));

	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);

	//Draw based on the current frame
	if (mode == DRAW_LINES)
		glDrawElements(GL_LINES, 6, GL_UNSIGNED_INT, (void*)(currentFrame * 6 * sizeof(GLuint)));
	else if (mode == DRAW_TRIANGLE_STRIP)
		glDrawElements(GL_TRIANGLE_STRIP, 6, GL_UNSIGNED_INT, (void*)(currentFrame * 6 * sizeof(GLuint)));
	else
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, (void*)(currentFrame * 6 * sizeof(GLuint)));

	//glDisableVertexAttribArray(3);
	//glDisableVertexAttribArray(2);
	//glDisableVertexAttribArray(1);
	//glDisableVertexAttribArray(0);
}

/**
 @brief Add an animation to the Sprite Animation by defining the start and the end
 @param anim_name A const int variable which contains the name of the animation
 @param start A const int variable which contains the starting frame based on the sprite sheet
 @param end A const int variable which contains the ending frame based on the sprite sheet
 */
void CSpriteAnimation::AddAnimation(std::string anim_name, int start, int end)
{
	CAnimation* anim = new CAnimation();

	//Check if start is more than end
	//Swap over if it is
	if(start > end)
		std::swap(start, end);

	//Add in all the frames in the range
	for (int i = start; i <= end; ++i) // <= end to include last sprite
	{
		anim->AddFrame(i);
	}

	//Link the animation to the animation list
	animationList[anim_name] = anim;
	//Set the current animation if it does not exisit
	if (currentAnimation == "")
	{
		currentAnimation = anim_name;
	}
	animationList[anim_name]->animActive = false;
}

/**
 @brief Add an animation to the Sprite Animation by defining the frame values
 @param anim_name The name of the animation
 @param count An int variable which contains the number of frame in the animation
 @param ... - the frames
 */
void CSpriteAnimation::AddSequeneAnimation(std::string anim_name, int count ...)
{
	CAnimation* anim = new CAnimation();

	va_list args;
	va_start(args, count);
	//Add the frames based on the input
	for (int i = 0; i < count; ++i)
	{
		int value = va_arg(args, int);
		anim->AddFrame(value);
	}
	va_end(args);

	//Link the animation to the animation list
	animationList[anim_name] = anim;
	//Set the current animation if it does not exisit
	if (currentAnimation == "")
	{
		currentAnimation = anim_name;
	}
	animationList[anim_name]->animActive = false;
}

/**
 @brief Play the animation based on the given name
 @param anim_name The name of the animation
 @param repeat A const int variable which contains the number of repeats (-1 for infinite looping)
 @param time A const int variable which contains the total time of the animation
 */
void CSpriteAnimation::PlayAnimation(std::string anim_name, const int repeat, const float time)
{
	//Check if the anim name exist 
	if (animationList[anim_name] != nullptr)
	{
		currentAnimation = anim_name;
		animationList[anim_name]->Set(repeat, time, true);
	}
}

/**
 @brief Resume the current animation if it is Paused
 */
void CSpriteAnimation::Resume(void)
{
	animationList[currentAnimation]->animActive = true;
}

/**
 @brief Pause the current animation
 */
void CSpriteAnimation::Pause(void)
{
	animationList[currentAnimation]->animActive = false;
}

/**
 @brief Reset the current animation
 */
void CSpriteAnimation::Reset(void)
{
	currentFrame = animationList[currentAnimation]->frames[0];
	playCount = 0;
}

std::string CSpriteAnimation::GetCurrentAnimationName()
{
	return currentAnimation;
}
