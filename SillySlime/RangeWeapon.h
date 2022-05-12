#pragma once

#include "Weapon.h"
#include <vector>

class RangeWeapon : public Weapon
{
	private:
		int bulletType;
	public:
		RangeWeapon();
		void Attack() override;

		// Getter
		int getBulletType() const;

		// Setter
		void setBulletType(const GAMEOBJ_TYPE& type);
};