#include "Character.h"

Character::Character()
{
	characterState = CHARACTER_STATE::STATE_IDLE;
	frontVec = glm::vec3(1.0f, 0.0f, 0.0f);
	hitCooldown = 2.0f;
	currentHitCooldown = 0.0f;
}

Character::~Character() 
{
}

// Getter Implement
float Character::getMaxHP() { return this->maxHP; }
float Character::getCurrentHP() { return this->currentHP; }
float Character::getHitCooldown() { return this->hitCooldown; }
float Character::getCurrentHitCooldown() { return this->currentHitCooldown; }
float Character::getAGI() { return this->AGI; }
bool Character::isJumping() { return this->jumping; }
Weapon* Character::getWeapon() const { return this->weapon; }
int Character::getCharacterState() { return this->characterState; }
glm::vec3 Character::getFrontVec() { return this->frontVec; }

// Setter Implement
void Character::setMaxHP(float hp) 
{ 
	this->maxHP = hp;
	this->currentHP = hp;
}
void Character::setCurrentHP(float hp) { this->currentHP = hp; }
void Character::setCurrentHitCooldown(float cooldown) { this->currentHitCooldown = cooldown; }
void Character::setAGI(float agi) { this->AGI = agi; }
void Character::setJumping(bool jumping) { this->jumping = jumping; }
void Character::setWeapon(Weapon* weapon) { this->weapon = weapon; }
void Character::setCharacterState(CHARACTER_STATE state) { this->characterState = state; }
void Character::setFrontVector(glm::vec3 vec) { this->frontVec = vec; }
// Override Implement(GameObject.h)
void Character::setScaleX(const float& scale_x)
{
	GameObject::setScaleX(scale_x);
	this->setFrontVector(glm::vec3((scale_x > 0 ? 1.0f : -1.0f), 0.0f, 0.0f));
}

// Take Actions Implement
void Character::decreaseHP(float hp) { this->currentHP -= hp; }
void Character::decreaseHitCooldown(float dt) { this->currentHitCooldown -= dt; }
