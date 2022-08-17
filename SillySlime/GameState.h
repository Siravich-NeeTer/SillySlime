#pragma once

#include <iostream>

#include "CDT.h"
#include "GameObject.h"
#include "Player.h"
#include "Enemy.h"
#include "MeleeWeapon.h"
#include "RangeWeapon.h"
#include "SpellWeapon.h"
#include "Bullet.h"

// -------------------------------------------
// Defines
// -------------------------------------------

#define MESH_MAX					64				// The total number of Mesh (Shape)
#define TEXTURE_MAX					64				// The total number of texture
#define GAME_OBJ_INST_MAX			1024			// The total number of different game object instances
#define RESPAWN_TIME				500
#define FLAG_INACTIVE				0
#define FLAG_ACTIVE					1
#define ANIMATION_SPEED				15				// 1 = fastest (update every frame)

// Movement flags
#define GRAVITY						-37.0f
#define MOVE_VELOCITY_ENEMY			2.0f
#define ENEMY_IDLE_TIME				2.0f
#define HURTING_FRAME				5

// Collision flags
#define	COLLISION_LEFT				1<<0
#define	COLLISION_RIGHT				1<<1
#define	COLLISION_TOP				1<<2
#define	COLLISION_BOTTOM			1<<3
#define COLLISION_OFFSET			0.25f

// Map
#define CELL_SIZE					40.0f				// 1 cell = 40 pixels

// Screen Size
#define SCREEN_SIZE					800.0f

// Convert from Map Space to World Space -> 0.5f is OFFSET
//#define TO_SCREEN_SPACE_X(x)			((x - MAP_WIDTH / 2.0f) * CELL_SIZE)
//#define TO_SCREEN_SPACE_Y(y)			((y - MAP_HEIGHT / 2.0f) * CELL_SIZE)
//#define TO_WORLD_SPACE_X(x)			(SCREEN_SIZE / 2.0f + TO_SCREEN_SPACE_X(x))
//#define TO_WORLD_SPACE_Y(y)			fabs((SCREEN_SIZE / 2.0f + TO_SCREEN_SPACE_Y(y)) - SCREEN_SIZE)
//#define TO_MAP_SPACE(x)				(x / CELL_SIZE)

// Convert from Map Space to World Space -> 0.5f is OFFSET
#define MAP_TO_WORLD_SPACE_X(x)			((x - MAP_WIDTH / 2.0f) * CELL_SIZE)
#define MAP_TO_WORLD_SPACE_Y(y)			((y - MAP_HEIGHT / 2.0f) * CELL_SIZE)
#define SCREEN_TO_WORLD_SPACE_X(x,left)	(MAP_TO_WORLD_SPACE_X(left) + x)
#define SCREEN_TO_WORLD_SPACE_Y(y)		fabs((SCREEN_SIZE / 2.0f + MAP_TO_WORLD_SPACE_Y(y)) - SCREEN_SIZE)
#define TO_MAP_SPACE(x)					(x / CELL_SIZE)

// -------------------------------------------
// Game object instant functions
// -------------------------------------------

namespace GameState
{
	GameObject*  getGameObjectType(int type);
	GameObject*  GameObjectCreate(GameObject** sGameObjInstArray, CDTMesh* sMeshArray, CDTTex* sTexArray, int type, glm::vec3 pos, glm::vec3 vel, glm::vec3 scale, float orient, bool anim, int numFrame, int currFrame, float offset, int& sNumGameObj);
	void		 GameObjectDestroy(GameObject* const& pInst, int& sNumGameObj);
	bool		 willCollide(const GameObject* obj1, const GameObject* obj2, const float& offsetA = 0.0f, const float& offsetB = 0.0f);
	bool		 isCollide(const GameObject* obj1, const GameObject* obj2, const float& offsetA = 0.0f, const float& offsetB = 0.0f);
	int			 CheckCharacterMapCollision(int** sMapCollisionData, float PosX, float PosY, float offsetX, float offsetY);
	int			 CheckBulletMapCollision(int** sMapCollisionData, float PosX, float PosY, float offset);
}