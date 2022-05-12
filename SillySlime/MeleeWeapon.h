#pragma once

#include "Weapon.h"

class MeleeWeapon : public Weapon
{
	public:
		MeleeWeapon();
		void Attack() override;
};