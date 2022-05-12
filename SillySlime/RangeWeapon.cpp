#include "RangeWeapon.h"

RangeWeapon::RangeWeapon()
{
	this->setCurrentCooldown(0.0f);
	this->setWeaponType(WEAPON_TYPE::WEAPON_TYPE_RANGED);
}

void RangeWeapon::Attack()
{
	this->setCurrentCooldown(this->getCooldown());
}

// Getter Implement
int RangeWeapon::getBulletType() const { return this->bulletType; }

// Setter Implement
void RangeWeapon::setBulletType(const GAMEOBJ_TYPE& type) { this->bulletType = type; }