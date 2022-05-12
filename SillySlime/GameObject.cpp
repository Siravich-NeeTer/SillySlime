#include "GameObject.h"

#include "Player.h"

GameObject::~GameObject() {}

// Function
float GameObject::getLeftBound() const
{
	return position.x - fabs(scale.x / 2);
}

float GameObject::getRightBound() const
{
	return position.x + fabs(scale.x / 2);
}

float GameObject::getTopBound() const
{
	return position.y + fabs(scale.x / 2);
}


float GameObject::getBottomBound() const
{
	return position.y - fabs(scale.x / 2);
}

/* GameObject */
// Getter Implement
CDTMesh* GameObject::getMesh() const { return this->mesh; }
CDTTex* GameObject::getTexture() const { return this->tex; }
int GameObject::getType() const { return this->type; }
int GameObject::getFlag() const { return this->flag; }
glm::vec3 GameObject::getPosition() const { return this->position; }
glm::vec3 GameObject::getVelocity() const { return this->velocity; }
glm::vec3 GameObject::getScale() const { return this->scale; }
float GameObject::getOrientation() const { return this->orientation; }
glm::mat4 GameObject::getModelMatrix() const { return this->modelMatrix; }
int GameObject::getTotalFrame() const { return this->totalFrame; }
int GameObject::getCurrentFrame() const { return this->currentFrame; }
bool GameObject::isAnimation() const { return this->animation; }
float GameObject::getOffset() const { return this->offset; }
float GameObject::getOffsetX() const { return this->offsetX; }
float GameObject::getOffsetY() const { return this->offsetY; }
bool GameObject::hasCollision() const { return this->collision; }

// Setter Implement
void GameObject::setMesh(CDTMesh* mesh) { this->mesh = mesh; }
void GameObject::setTexture(CDTTex* texture) { this->tex = texture; }
void GameObject::setType(const int& type) { this->type = type; }
void GameObject::setFlag(const int& flag) { this->flag = flag; }
void GameObject::setPosition(const glm::vec3& position) { this->position = position; }
void GameObject::setPositionX(const float& position_x) { this->position.x = position_x; }
void GameObject::setPositionY(const float& position_y) { this->position.y = position_y; }
void GameObject::updatePosition(const glm::vec3& position) { this->position += position; }

void GameObject::setVelocity(const glm::vec3& velocity) { this->velocity = velocity; }
void GameObject::setVelocityX(const float& velocity_x) { this->velocity.x = velocity_x; }
void GameObject::setVelocityY(const float& velocity_y) { this->velocity.y = velocity_y; }
void GameObject::increaseVelocityX(const float& velocity_x) { this->velocity.x += velocity_x; }
void GameObject::increaseVelocityY(const float& velocity_y) { this->velocity.y += velocity_y; }

void GameObject::setScale(const glm::vec3& scale) { this->scale = scale; }
void GameObject::setScaleX(const float& scale_x) { this->scale.x = scale_x; }
void GameObject::setScaleY(const float& scale_y) { this->scale.y = scale_y; }
void GameObject::setOrientation(const float& orientation) { this->orientation = orientation; }
void GameObject::setModelMatrix(const glm::mat4& modelMatrix) { this->modelMatrix = modelMatrix; }

void GameObject::setTotalFrame(const int& totalFrame) { this->totalFrame = totalFrame; }
void GameObject::setCurrentFrame(const int& frame) { this->currentFrame = frame; }
void GameObject::setAnimation(const bool& anim) { this->animation = anim; }

void GameObject::setOffset(const float& offset) { this->offset = offset; }
void GameObject::setOffsetX(const float& x) { this->offsetX = x; }
void GameObject::setOffsetY(const float& y) { this->offsetY = y; }

void GameObject::setCollision(const bool& collision) { this->collision = collision; }