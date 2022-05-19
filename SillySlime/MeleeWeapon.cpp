#include "MeleeWeapon.h"

MeleeWeapon::MeleeWeapon()
{
	this->setCurrentCooldown(0.0f);
	this->setWeaponType(WEAPON_TYPE::WEAPON_TYPE_MELEE);
}

void MeleeWeapon::Attack()
{
	this->setCurrentCooldown(this->getCooldown());
}