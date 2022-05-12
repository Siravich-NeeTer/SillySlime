#pragma once

#include "GameObject.h"

enum WEAPON_TYPE
{
	WEAPON_TYPE_MELEE,
	WEAPON_TYPE_RANGED,
	WEAPON_TYPE_SPELL
};

class Weapon : public GameObject
{
	private:
		float ATK;
		float Range;
		float cooldown;
		float currentCooldown;
		WEAPON_TYPE weaponType;
		GameObject* holder;
	public:
		// Function
		Weapon();
		void updateWeaponDirection();
		void updateWeaponCooldown(double dt);
		virtual void Attack();

		// Getter
		float getATK() const;
		float getRange() const;
		float getCurrentCooldown() const;
		float getCooldown() const;
		GameObject* getHolder() const;
		int getWeaponType() const;

		// Setter
		void setATK(const float& atk);
		void setRange(const float& range);
		void setCurrentCooldown(const float& cooldown);
		void decreaseCurrentCooldown(const float& cooldown);
		void setCooldown(const float& cooldown);
		void setHolder(GameObject* holder);
		void setWeaponType(const WEAPON_TYPE& type);

};