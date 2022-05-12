#include "Bullet.h"
#include "GameState.h"

Bullet::Bullet()
{
	this->setCollision(true);
}

// Getter Implement
float Bullet::getATK() const { return this->ATK; }
GameObject* Bullet::getShooter() const { return this->shooter; }

// Setter Implement
void Bullet::setATK(const float& atk) { this->ATK = atk; }
void Bullet::setShooter(GameObject* character) { this->shooter = character; }
void Bullet::updatePosition(const glm::vec3& position)
{
	this->setOrientation(atan2(this->getVelocity().y, this->getVelocity().x));
	GameObject::updatePosition(position);
}