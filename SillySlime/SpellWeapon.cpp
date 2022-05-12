#include "SpellWeapon.h"

SpellWeapon::SpellWeapon()
{
	this->setCurrentCooldown(0.0f);
	this->setWeaponType(WEAPON_TYPE::WEAPON_TYPE_SPELL);
}

void SpellWeapon::Attack()
{
	this->setCurrentCooldown(this->getCooldown());
}

// Getter Implement
int SpellWeapon::getBulletType() const { return this->bulletType; }

// Setter Implement
void SpellWeapon::setBulletType(const GAMEOBJ_TYPE& type) { this->bulletType = type; }