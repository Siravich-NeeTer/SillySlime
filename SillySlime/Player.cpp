#include "Player.h"

Player::Player()
{
	setCollision(true);
	this->fly = false;
}

void Player::getInput()
{
	//+ Moving the Player
		//	- W:	jumping
		//	- AD:	go left, go right
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
	{
		if (this->getScale().x > 0.0f)
		{
			this->setScaleX(this->getScale().x * -1.0f);
		}

		// Set Range Weapon Facing Direction
		if (this->getWeapon()->getWeaponType() == WEAPON_TYPE_RANGED)
		{
			if (this->getWeapon()->getOrientation() > -90 * DegToRad && this->getWeapon()->getOrientation() < 90 * DegToRad)
			{
				this->getWeapon()->setOrientation(180.0f * DegToRad);
			}
		}
		if (this->getWeapon()->getType() == GAMEOBJ_TYPE::TYPE_WEAPON_BOW)
		{
			this->setVelocityX(-MOVE_VELOCITY_PLAYER * 1.25f);
		}
		else
		{
			this->setVelocityX(-MOVE_VELOCITY_PLAYER);
		}
		this->setAnimation((isJumping() == false ? true : false));
	}
	else if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
	{
		if (this->getScale().x < 0.0f)
		{
			this->setScaleX(this->getScale().x * -1.0f);
		}

		// Set Range Weapon Facing Direction
		if (this->getWeapon()->getWeaponType() == WEAPON_TYPE_RANGED)
		{
			if (!(this->getWeapon()->getOrientation() > -90 * DegToRad && this->getWeapon()->getOrientation() < 90 * DegToRad))
			{
				this->getWeapon()->setOrientation(0.0f * DegToRad);
			}
		}
		if (this->getWeapon()->getType() == GAMEOBJ_TYPE::TYPE_WEAPON_BOW)
		{
			this->setVelocityX(MOVE_VELOCITY_PLAYER * 1.25f);
		}
		else
		{
			this->setVelocityX(MOVE_VELOCITY_PLAYER);
		}
		this->setAnimation((isJumping() == false ? true : false));
	}
	else
	{
		this->setVelocityX(0.0f);
		this->setAnimation(false);
		this->setOffsetX(0);
	}
	// Play jump animation
	if (isJumping() == true)
	{
		this->setOffsetX(0.75f);	// the 4th sprite of the sheet of 4
	}
}

void Player::decreaseHP(float hp)
{
	if (this->getWeapon()->getType() == GAMEOBJ_TYPE::TYPE_WEAPON_SWORD)
	{
		Character::decreaseHP(hp * 0.5f);
	}
	else
	{
		Character::decreaseHP(hp);
	}
}

void Player::setFlying(const bool& fly) { this->fly = fly; }
bool Player::isFlying() const { return this->fly; }