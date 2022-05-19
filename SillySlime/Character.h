#pragma once

#include "GameObject.h"
#include "Weapon.h"

enum CHARACTER_STATE
{
	STATE_IDLE,
	STATE_JUMP,
	STATE_WALK,
	STATE_ATTACK,
	STATE_DIE
};

class Character : public GameObject
{
	protected:
		float maxHP;
		float currentHP;
		float hitCooldown;
		float currentHitCooldown;
		float AGI;
		bool jumping;
		Weapon* weapon;
		CHARACTER_STATE characterState;
		glm::vec3 frontVec;

	public:
		Character();
		virtual ~Character();

		// Getter
		float getMaxHP();
		float getCurrentHP();
		float getHitCooldown();
		float getCurrentHitCooldown();
		float getAGI();
		bool isJumping();
		Weapon* getWeapon() const;
		int getCharacterState();
		glm::vec3 getFrontVec();

		// Setter
		void setMaxHP(float hp);
		void setCurrentHP(float hp);
		void setCurrentHitCooldown(float cooldown);
		void setAGI(float agi);
		void setJumping(bool jumping);
		void setWeapon(Weapon* weapon);
		void setCharacterState(CHARACTER_STATE state);
		void setFrontVector(glm::vec3 vec);
		// Override (GameObject)
		void setScaleX(const float& scale_x) override;


		// Take Actions
		virtual void decreaseHP(float hp);
		void decreaseHitCooldown(float dt);
};