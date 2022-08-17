#include "GameState.h"

GameObject* GameState::getGameObjectType(int type)
{
	GameObject* obj;
	if (type >= GAMEOBJ_TYPE::TYPE_UI)
	{
		obj = new GameObject();
	}
	else if (type >= GAMEOBJ_TYPE::TYPE_WEAPON_BULLET)
	{
		obj = new Bullet();
	}
	else if (type == GAMEOBJ_TYPE::TYPE_WEAPON_SWORD)
	{
		obj =  new MeleeWeapon();
		dynamic_cast<MeleeWeapon*>(obj)->setATK(40.0f);
		dynamic_cast<MeleeWeapon*>(obj)->setCooldown(1.5f);
		obj->setCollision(false);
	}
	else if (type == GAMEOBJ_TYPE::TYPE_WEAPON_BOW)
	{
		obj = new RangeWeapon();
		dynamic_cast<RangeWeapon*>(obj)->setATK(25.0f);
		dynamic_cast<RangeWeapon*>(obj)->setBulletType(TYPE_WEAPON_BOW_ARROW);
		dynamic_cast<RangeWeapon*>(obj)->setCooldown(0.5f);
		obj->setCollision(false);
	}
	else if (type == GAMEOBJ_TYPE::TYPE_WEAPON_FIRE_WAND)
	{
		obj = new SpellWeapon();
		dynamic_cast<SpellWeapon*>(obj)->setATK(35.0f);
		dynamic_cast<SpellWeapon*>(obj)->setBulletType(TYPE_WEAPON_FIRE_SPELL);
		dynamic_cast<SpellWeapon*>(obj)->setCooldown(0.75f);
		obj->setCollision(false);
	}

	else if (type == GAMEOBJ_TYPE::TYPE_WEAPON_SLIME)
	{
		obj = new RangeWeapon();
		dynamic_cast<RangeWeapon*>(obj)->setATK(25.0f);
		dynamic_cast<RangeWeapon*>(obj)->setBulletType(TYPE_WEAPON_SLIME_BALL);
		dynamic_cast<RangeWeapon*>(obj)->setCooldown(1.0f);
		obj->setCollision(false);
	}
	else if (type >= GAMEOBJ_TYPE::TYPE_ENEMY)
	{
		obj = new Enemy();
		if (type == GAMEOBJ_TYPE::TYPE_ENEMY_WARRIOR)
		{
			dynamic_cast<Enemy*>(obj)->setMaxHP(70.0f);
		}
		else if (type == GAMEOBJ_TYPE::TYPE_ENEMY_ARCHER)
		{
			dynamic_cast<Enemy*>(obj)->setMaxHP(50.0f);
		}
		else
		{
			dynamic_cast<Enemy*>(obj)->setMaxHP(40.0f);
		}
	}
	else if (type == GAMEOBJ_TYPE::TYPE_PLAYER)
	{
		obj = new Player();
		dynamic_cast<Player*>(obj)->setMaxHP(250.0f);
	}
	else
	{
		obj = new GameObject();
	}
	return obj;
}

GameObject* GameState::GameObjectCreate(GameObject** sGameObjInstArray, CDTMesh* sMeshArray, CDTTex* sTexArray, int type, glm::vec3 pos, glm::vec3 vel, glm::vec3 scale, float orient, bool anim, int numFrame, int currFrame, float offset, int &sNumGameObj)
{
	// loop through all object instance array to find the free slot
	for (int i = 0; i < GAME_OBJ_INST_MAX; i++)
	{
		if (sGameObjInstArray[i] == nullptr)
		{
			GameObject* pInst;
			pInst = getGameObjectType(type);

			// Object Component
			pInst->setMesh(sMeshArray + type);
			pInst->setTexture(sTexArray + type);
			pInst->setType(type);
			pInst->setFlag(FLAG_ACTIVE);
			pInst->setPosition(pos);
			pInst->setVelocity(vel);
			pInst->setScale(scale);
			pInst->setOrientation(orient);
			pInst->setModelMatrix(glm::mat4(1.0f));
			// Animation
			pInst->setAnimation(anim);
			pInst->setTotalFrame(numFrame);
			pInst->setCurrentFrame(currFrame);
			pInst->setOffset(offset);
			pInst->setOffsetX(0);
			pInst->setOffsetY(0);
			pInst->setHurt(0);

			sNumGameObj++;
			sGameObjInstArray[i] = pInst;
			return pInst;
		}
	}

	// Cannot find empty slot => return 0
	return nullptr;
}

void GameState::GameObjectDestroy(GameObject* const& pInst, int& sNumGameObj)
{
	if (pInst == nullptr)
	{
		return;
	}

	GameObject*& ptr = const_cast<GameObject*&>(pInst);
	delete pInst;
	ptr = nullptr;
	sNumGameObj--;
}

bool GameState::willCollide(const GameObject* obj1, const GameObject* obj2, const float& offsetA, const float& offsetB)
{
	float rightA = obj1->getRightBound() - offsetA;
	float leftA = obj1->getLeftBound() + offsetA;
	float topA = obj1->getTopBound() - offsetA;
	float bottomA = obj1->getBottomBound() + offsetA;

	float rightB = obj2->getRightBound() - offsetB;
	float leftB = obj2->getLeftBound() + offsetB;
	float topB = obj2->getTopBound() - offsetB;
	float bottomB = obj2->getBottomBound() + offsetB;

	return !(leftA > rightB || leftB > rightA || bottomA > topB || bottomB > topA);
}

bool GameState::isCollide(const GameObject* obj1, const GameObject* obj2, const float& offsetA, const float& offsetB)
{
	if (obj1->hasCollision() == false || obj2->hasCollision() == false)
	{
		return false;
	}

	return willCollide(obj1, obj2, offsetA, offsetB);
}

//+ This fucntion return collison flags
//	- 1-left, 2-right, 4-top, 8-bottom 
//	- each side is checked with 2 hot spots
int GameState::CheckCharacterMapCollision(int** sMapCollisionData, float PosX, float PosY, float offsetX, float offsetY)
{
	offsetX = fabs(offsetX) / 2.0f;
	offsetY = fabs(offsetY) / 2.0f;

	int result = 0;

	// Check Left Collision
	if (sMapCollisionData[(int)(PosY + offsetY / 2.0f)][(int)(PosX - offsetX)] || sMapCollisionData[(int)(PosY - offsetY / 2.0f)][(int)(PosX - offsetX)])
	{
		result |= COLLISION_LEFT;
	}

	// Check Right Collision
	if (sMapCollisionData[(int)(PosY + offsetY / 2.0f)][(int)(PosX + offsetX)] || sMapCollisionData[(int)(PosY - offsetY / 2.0f)][(int)(PosX + offsetX)])
	{
		result |= COLLISION_RIGHT;
	}

	// Check Top Collision
	if (sMapCollisionData[(int)(PosY + offsetY)][(int)(PosX - offsetX / 2.0f)] || sMapCollisionData[(int)(PosY + offsetY)][(int)(PosX + offsetX / 2.0f)])
	{
		result |= COLLISION_TOP;
	}

	// Check Bottom Collision (Special : Y Need to be lower)
	if (sMapCollisionData[(int)(PosY - offsetY - 0.1f)][(int)(PosX - offsetX / 2.0f)] || sMapCollisionData[(int)(PosY - offsetY - 0.1f)][(int)(PosX + offsetX / 2.0f)])
	{
		result |= COLLISION_BOTTOM;
	}

	return result;
}

//+ This fucntion return collison flags
//	- 1-left, 2-right, 4-top, 8-bottom 
//	- each side is checked with 2 hot spots
int GameState::CheckBulletMapCollision(int** sMapCollisionData, float PosX, float PosY, float offset)
{
	int result = 0;

	// Check Left Collision
	if (sMapCollisionData[(int)(PosY + offset / 2.0f)][(int)(PosX - offset)] || sMapCollisionData[(int)(PosY - offset / 2.0f)][(int)(PosX - offset)])
	{
		result |= COLLISION_LEFT;
	}

	// Check Right Collision
	if (sMapCollisionData[(int)(PosY + offset / 2.0f)][(int)(PosX + offset)] || sMapCollisionData[(int)(PosY - offset / 2.0f)][(int)(PosX + offset)])
	{
		result |= COLLISION_RIGHT;
	}

	// Check Top Collision
	if (sMapCollisionData[(int)(PosY + offset)][(int)(PosX - offset / 2.0f)] || sMapCollisionData[(int)(PosY + offset)][(int)(PosX + offset / 2.0f)])
	{
		result |= COLLISION_TOP;
	}

	// Check Bottom Collision (Special : Y Need to be lower)
	if (sMapCollisionData[(int)(PosY - offset)][(int)(PosX - offset / 2.0f)] || sMapCollisionData[(int)(PosY - offset)][(int)(PosX + offset / 2.0f)])
	{
		result |= COLLISION_BOTTOM;
	}

	return result;
}