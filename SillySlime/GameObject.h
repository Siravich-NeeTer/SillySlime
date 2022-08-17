#pragma once

#include "CDT.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/random.hpp>
#include <cmath>
#include <iostream>

enum GAMEOBJ_TYPE
{
	// list of game object types
	TYPE_BG = 0,
	TYPE_EXIT,
	TYPE_PINK_SLIME,
	TYPE_PLAYER,

	TYPE_ENEMY,
	TYPE_ENEMY_WARRIOR,
	TYPE_ENEMY_ARCHER,
	TYPE_ENEMY_MAGE,
	TYPE_ENEMY_END,

	TYPE_WEAPON_SWORD,
	TYPE_WEAPON_BOW,
	TYPE_WEAPON_FIRE_WAND,
	TYPE_WEAPON_SLIME,

	TYPE_WEAPON_BULLET,
	TYPE_WEAPON_BOW_ARROW,
	TYPE_WEAPON_FIRE_SPELL,
	TYPE_WEAPON_SLIME_BALL,

	TYPE_UI,
	TYPE_UI_BAR,
	TYPE_UI_HEALTH,
	TYPE_UI_HEALTH_BAR,
	TYPE_UI_WALK,
	TYPE_UI_JUMP,
	TYPE_UI_LEFT_CLICK,
	TYPE_UI_RIGHT_CLICK,
	TYPE_UI_MISSION_COMPLETE,
	TYPE_UI_TYPE,
	TYPE_UI_TYPE_NORMAL,
	TYPE_UI_TYPE_WARRIOR,
	TYPE_UI_TYPE_ARCHER,
	TYPE_UI_TYPE_MAGE
};

class GameObject
{
	public:
		CDTMesh*		mesh;
		CDTTex*			tex;

	private:
		int				type;				// enum GAMEOBJ_TYPE
		int				flag;				// 0 - inactive, 1 - active
		glm::vec3		position;			// usually we will use only x and y
		glm::vec3		velocity;			// usually we will use only x and y
		glm::vec3		scale;				// usually we will use only x and y
		float			orientation;		// 0 radians is 3 o'clock, PI/2 radian is 12 o'clock
		glm::mat4		modelMatrix;
		bool			animation;
		int				totalFrame;
		int				currentFrame;
		float			offset;				// offset value of each frame
		float			offsetX;			// assume single row sprite sheet
		float			offsetY;			// will be set to 0 for this single row implementation
		bool			collision;			// Used for check collision
		bool			cull;
		int			hurt;				// If Object hurt -> Turn Red
		
	public:
		virtual ~GameObject();

		// Function
		float getLeftBound() const;
		float getRightBound() const;
		float getTopBound() const;
		float getBottomBound() const;

		/* GameObject */
		// Getter
		CDTMesh* getMesh() const;
		CDTTex* getTexture() const;
		int getType() const;
		int getFlag() const;
		glm::vec3 getPosition() const;
		glm::vec3 getVelocity() const;
		glm::vec3 getScale() const;
		float getOrientation() const;
		glm::mat4 getModelMatrix() const;
		int getTotalFrame() const;
		int getCurrentFrame() const;
		bool isAnimation() const;
		float getOffset() const;
		float getOffsetX() const;
		float getOffsetY() const;
		bool hasCollision() const;
		bool isCulling() const;
		int isHurt() const;

		//Setter
		void setMesh(CDTMesh* mesh);
		void setTexture(CDTTex* texture);
		void setType(const int& type);
		void setFlag(const int& flag);
		void setPosition(const glm::vec3& position);
		void setPositionX(const float& position_x);
		void setPositionY(const float& position_y);
		// virtual for Bullet class
		virtual void updatePosition(const glm::vec3& position);

		void setVelocity(const glm::vec3& velocity);
		void setVelocityX(const float& velocity_x);
		void setVelocityY(const float& velocity_y);
		void increaseVelocityX(const float& velocity_x);
		void increaseVelocityY(const float& velocity_y);

		void setScale(const glm::vec3& scale);
		// virtual for Character class
		virtual void setScaleX(const float& scale_x);
		void setScaleY(const float& scale_y);
		void setOrientation(const float& orientation);
		void setModelMatrix(const glm::mat4& modelMatrix);

		void setTotalFrame(const int& totalFrame);
		void setCurrentFrame(const int& frame);
		void setAnimation(const bool& anim);
		
		void setOffset(const float& offset);
		void setOffsetX(const float& x);
		void setOffsetY(const float& y);
		
		void setCollision(const bool& collision);
		void setCulling(const bool& cull);
		void setHurt(const int& hurt);
};