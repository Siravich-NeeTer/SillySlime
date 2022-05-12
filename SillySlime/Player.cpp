#include "Player.h"

Player::Player()
{
	setCollision(true);
}

void Player::getInput()
{
	//+ Moving the Player
		//	- W:	jumping
		//	- AD:	go left, go right
	if ((glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) && (isJumping() == false))
	{
		this->setVelocityY(JUMP_VELOCITY);
		this->setJumping(true);
		this->setAnimation(false);
	}
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
		this->setVelocityX(-MOVE_VELOCITY_PLAYER);
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
		this->setVelocityX(MOVE_VELOCITY_PLAYER);
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