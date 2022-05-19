#include "Weapon.h"

// Function Implement
Weapon::Weapon()
{
	this->currentCooldown = 0.0f;
	this->setCollision(false);
}
void Weapon::Attack()
{

}

void Weapon::updateWeaponDirection()
{
	if (holder == nullptr)
	{
		return;
	}

	/* Update Position to be same with holder*/
	if (holder->getScale().x >= 1.0f)
	{
		if (weaponType != WEAPON_TYPE::WEAPON_TYPE_RANGED)
		{
			this->setScaleX(this->getScale().x * (this->getScale().x < 0.0f ? -1.0f : 1.0f));
			this->setPositionX(holder->getRightBound());
		}
		else
		{
			this->setPositionX(holder->getPosition().x);
		}
	}
	else
	{
		if (weaponType != WEAPON_TYPE::WEAPON_TYPE_RANGED)
		{
			this->setScaleX(this->getScale().x * (this->getScale().x > 0.0f ? -1.0f : 1.0f));
			this->setPositionX(holder->getLeftBound());
		}
		else
		{
			this->setPositionX(holder->getPosition().x);
		}
	}
	this->setPositionY(holder->getPosition().y);
}

void Weapon::updateWeaponCooldown(double dt)
{
	if (this->currentCooldown <= 0.0f)
	{
		this->currentCooldown = 0.0f;
		this->setCollision(false);
	}
	else
	{
		this->currentCooldown -= dt;
	}
}

// Getter Implement
float Weapon::getATK() const { return this->ATK; }
float Weapon::getRange() const { return this->Range; }
float Weapon::getCurrentCooldown() const { return this->currentCooldown; }
float Weapon::getCooldown() const { return this->cooldown; }
GameObject* Weapon::getHolder() const { return this->holder; }
int Weapon::getWeaponType() const { return this->weaponType; }

// Setter Implement
void Weapon::setATK(const float& atk) { this->ATK = atk; }
void Weapon::setRange(const float& range) { this->Range = range; }
void Weapon::setCurrentCooldown(const float& cooldown) { this->currentCooldown = cooldown; }
void Weapon::decreaseCurrentCooldown(const float& cooldown) { this->currentCooldown -= cooldown; }
void Weapon::setCooldown(const float& cooldown) { this->cooldown = cooldown; }
void Weapon::setHolder(GameObject* holder) { this->holder = holder; }
void Weapon::setWeaponType(const WEAPON_TYPE& type) { this->weaponType = type; }