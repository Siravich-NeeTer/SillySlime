#include "Enemy.h"

Enemy::Enemy()
{
	setCollision(true);
	this->state = ENEMY_STATE::STATE_GOING_LEFT;
	this->innerState = ENEMY_INNER_STATE::INNER_STATE_ON_ENTER;
	this->counter = 0.0f;
}

void Enemy::UpdateEnemyState(int** sMapCollisionData, double dt)
{
	if (this->getCharacterState() == CHARACTER_STATE::STATE_DIE)
	{
		return;
	}

	// Enemy move Left
	if (this->state == ENEMY_STATE::STATE_GOING_LEFT)
	{
		if (this->innerState == ENEMY_INNER_STATE::INNER_STATE_ON_ENTER)
		{
			this->setVelocityX(-MOVE_VELOCITY_ENEMY);
			this->innerState = ENEMY_INNER_STATE::INNER_STATE_ON_UPDATE;
		}
		else if (this->innerState == ENEMY_INNER_STATE::INNER_STATE_ON_UPDATE)
		{
			this->setVelocityX(-MOVE_VELOCITY_ENEMY);
			int collision = GameState::CheckCharacterMapCollision(sMapCollisionData, this->getPosition().x, this->getPosition().y);
			if (collision & COLLISION_LEFT)
			{
				this->setPositionX((int)this->getPosition().x + 0.5f);
				this->setVelocityX(0.0f);
				this->innerState = ENEMY_INNER_STATE::INNER_STATE_ON_EXIT;
				this->counter = ENEMY_IDLE_TIME;
			}
			else
			{
				collision = GameState::CheckCharacterMapCollision(sMapCollisionData, this->getPosition().x - 0.5f, this->getPosition().y);
				if (!(collision & COLLISION_BOTTOM))
				{
					this->setPositionY((int)this->getPosition().y + 0.5f);
					this->setVelocityX(0.0f);
					this->innerState = ENEMY_INNER_STATE::INNER_STATE_ON_EXIT;
					this->counter = ENEMY_IDLE_TIME;
				}
			}
		}
		else if (this->innerState == ENEMY_INNER_STATE::INNER_STATE_ON_EXIT)
		{
			if (this->counter < 0)
			{
				this->state = ENEMY_STATE::STATE_GOING_RIGHT;
				this->innerState = ENEMY_INNER_STATE::INNER_STATE_ON_ENTER;
			}
			else
			{
				this->counter -= dt;
			}
		}
	}
	// Enemy move Right
	else if (this->state == ENEMY_STATE::STATE_GOING_RIGHT)
	{
		if (this->innerState == ENEMY_INNER_STATE::INNER_STATE_ON_ENTER)
		{
			this->setVelocityX(MOVE_VELOCITY_ENEMY);
			this->innerState = ENEMY_INNER_STATE::INNER_STATE_ON_UPDATE;
		}
		else if (this->innerState == ENEMY_INNER_STATE::INNER_STATE_ON_UPDATE)
		{
			this->setVelocityX(MOVE_VELOCITY_ENEMY);
			int collision = GameState::CheckCharacterMapCollision(sMapCollisionData, this->getPosition().x, this->getPosition().y);
			if (collision & COLLISION_RIGHT)
			{
				this->setPositionX((int)this->getPosition().x + 0.5f);
				this->setVelocityX(0.0f);
				this->innerState = ENEMY_INNER_STATE::INNER_STATE_ON_EXIT;
				this->counter = ENEMY_IDLE_TIME;
			}
			else
			{
				collision = GameState::CheckCharacterMapCollision(sMapCollisionData, this->getPosition().x + 0.25f, this->getPosition().y);
				if (!(collision & COLLISION_BOTTOM))
				{
					this->setPositionY((int)this->getPosition().y + 0.5f);
					this->setVelocityX(0.0f);
					this->innerState = ENEMY_INNER_STATE::INNER_STATE_ON_EXIT;
					this->counter = ENEMY_IDLE_TIME;
				}
			}
		}
		else if (this->innerState == ENEMY_INNER_STATE::INNER_STATE_ON_EXIT)
		{
			if (this->counter < 0)
			{
				this->state = ENEMY_STATE::STATE_GOING_LEFT;
				this->innerState = ENEMY_INNER_STATE::INNER_STATE_ON_ENTER;
			}
			else
			{
				this->counter -= dt;
			}
		}
	}
}