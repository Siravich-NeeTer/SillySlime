#pragma once

#include "Weapon.h"

class SpellWeapon : public Weapon
{
	private:
		int bulletType;
	public:
		SpellWeapon();
		void Attack() override;

		// Getter
		int getBulletType() const;

		// Setter
		void setBulletType(const GAMEOBJ_TYPE& type);
};