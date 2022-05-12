#include "GameStateLevel1.h"
#include "CDT.h"

#include <iostream>
#include <fstream>
#include <string>
#include <irrKlang.h>
#include <math.h>

#include "GameState.h"
#include "GameObject.h"
#include "Player.h"
#include "Enemy.h"
#include "Weapon.h"

using namespace irrklang;

// -----------------------------------------------------
// Level variable, static - visible only in this file
// -----------------------------------------------------

static CDTMesh		sMeshArray[MESH_MAX];							// Store all unique shape/mesh in your game
static int			sNumMesh;
static CDTTex		sTexArray[TEXTURE_MAX];							// Corresponding texture of the mesh
static int			sNumTex;
static GameObject*	sGameObjInstArray[GAME_OBJ_INST_MAX];			// Store all game object instance
static int			sNumGameObj;
// Player data
static Player*		sPlayer;										// Pointer to the Player game object instance
static glm::vec3	sPlayer_start_position;
static int			sPlayerLives;									// The number of lives left
static int			sScore;
static int			sRespawnCountdown;								// Respawn player waiting time (in #frame)
//Sound
ISoundEngine*		SoundEngine;
// Map data
static int**		sMapData;										// sMapData[Height][Width]
static int**		sMapCollisionData;
static int			MAP_WIDTH;
static int			MAP_HEIGHT;
static glm::mat4	sMapMatrix;										// Transform from map space [0,MAP_SIZE] to screen space [-width/2,width/2]
static CDTMesh*		sMapMesh;										// Mesh & Tex of the level, we only need 1 of these
static CDTTex*		sMapTex;
static float		sMapOffset;
static bool			CULL_ON;										// enable culling
static bool			C_DOWN;
// Scrolling map data
static int			sStartCam;										// This example only scroll in X direction
static int			sLimitLeft;
static int			sLimitRight;

// -------------------------------------------
// Game object instant functions
// -------------------------------------------
// functions to create/destroy a game object instance
static GameObject*	gameObjInstCreate(int type, glm::vec3 pos, glm::vec3 vel, glm::vec3 scale, float orient, bool anim, int numFrame, int currFrame, float offset)
{
	return GameState::GameObjectCreate(sGameObjInstArray, sMeshArray, sTexArray, type, pos, vel, scale, orient, anim, numFrame, currFrame, offset, sNumGameObj);
}
static void			gameObjInstDestroy(GameObject* const& pInst)
{
	GameState::GameObjectDestroy(pInst, sNumGameObj);
}
static void			createMeshTex(const char* name, const float& u, const float& v)
{
	CDTMesh* pMesh;
	CDTTex* pTex;
	std::vector<CDTVertex> vertices;
	CDTVertex v1, v2, v3, v4;
	v1.x = -0.5f; v1.y = -0.5f; v1.z = 0.0f; v1.r = 1.0f; v1.g = 0.0f; v1.b = 0.0f; v1.u = 0.0f; v1.v = 0.0f;
	v2.x = 0.5f; v2.y = -0.5f; v2.z = 0.0f; v2.r = 0.0f; v2.g = 1.0f; v2.b = 0.0f; v2.u = u; v2.v = 0.0f;
	v3.x = 0.5f; v3.y = 0.5f; v3.z = 0.0f; v3.r = 0.0f; v3.g = 0.0f; v3.b = 1.0f; v3.u = u; v3.v = v;
	v4.x = -0.5f; v4.y = 0.5f; v4.z = 0.0f; v4.r = 1.0f; v4.g = 1.0f; v4.b = 0.0f; v4.u = 0.0f; v4.v = v;
	vertices.push_back(v1);
	vertices.push_back(v2);
	vertices.push_back(v3);
	vertices.push_back(v1);
	vertices.push_back(v3);
	vertices.push_back(v4);

	pMesh = sMeshArray + sNumMesh++;
	pTex = sTexArray + sNumTex++;
	*pMesh = CreateMesh(vertices);
	*pTex = TextureLoad(name);
}
static int CheckType(int type)
{
	if (type >= TYPE_ENEMY && type <= TYPE_ENEMY_END)
	{
		return TYPE_ENEMY;
	}
}

// -------------------------------------------
// Game states function
// -------------------------------------------
void GameStateLevel1Load(void) 
{

	// clear the Mesh array
	memset(sMeshArray, 0, sizeof(CDTMesh) * MESH_MAX);
	sNumMesh = 0;

	// clear the Texture array
	memset(sTexArray, 0, sizeof(CDTTex) * TEXTURE_MAX);
	sNumTex = 0;

	// clear the game object instance array
	memset(sGameObjInstArray, 0, sizeof(GameObject*) * GAME_OBJ_INST_MAX);
	sNumGameObj = 0;

	// Set the Player object instance to NULL
	sPlayer = nullptr;

	// --------------------------------------------------------------------------
	// Create all of the unique meshes/textures and put them in MeshArray/TexArray
	//		- The order of mesh MUST follow enum GAMEOBJ_TYPE 
	/// --------------------------------------------------------------------------

	// Create Player mesh/texture
	createMeshTex("mario.png", 0.25f, 1.0f);

	// Create Enemy(header) mesh/texture
	createMeshTex("turtle.png", 0.5f, 1.0f);
	// Create Enemy_Warrior mesh/texture
	createMeshTex("nullOBJ.png", 1.0f, 1.0f);
	// Create Enemy_Archer mesh/texture
	createMeshTex("nullOBJ.png", 1.0f, 1.0f);
	// Create Enemy_Mage mesh/texture
	createMeshTex("nullOBJ.png", 1.0f, 1.0f);
	// Create Enemy_END(header) mesh/texture
	createMeshTex("nullOBJ.png", 1.0f, 1.0f);

	// Create Sword mesh/texture
	createMeshTex("Sword.png", 1.0f, 1.0f);
	// Create Bow mesh/texture
	createMeshTex("Bow.png", 1.0f, 1.0f);
	// Create FireWand mesh/texture
	createMeshTex("FireWand.png", 1.0f, 1.0f);

	// Create Bullet(header) mesh/texture
	createMeshTex("nullOBJ.png", 1.0f, 1.0f);
	// Create Arrow mesh/texture
	createMeshTex("Arrow.png", 1.0f, 1.0f);
	// Create FireSpell mesh/texture
	createMeshTex("FireBall.png", 1.0f, 1.0f);


	// Temporary variable for creating Level mesh
	std::vector<CDTVertex> vertices;
	CDTVertex v1, v2, v3, v4;

	// Create Level mesh/texture
	vertices.clear();
	v1.x = -0.5f; v1.y = -0.5f; v1.z = 0.0f; v1.r = 1.0f; v1.g = 0.0f; v1.b = 0.0f; v1.u = 0.01f; v1.v = 0.01f;
	v2.x = 0.5f; v2.y = -0.5f; v2.z = 0.0f; v2.r = 0.0f; v2.g = 1.0f; v2.b = 0.0f; v2.u = 0.24f; v2.v = 0.01f;
	v3.x = 0.5f; v3.y = 0.5f; v3.z = 0.0f; v3.r = 0.0f; v3.g = 0.0f; v3.b = 1.0f; v3.u = 0.24f; v3.v = 1.0f;
	v4.x = -0.5f; v4.y = 0.5f; v4.z = 0.0f; v4.r = 1.0f; v4.g = 1.0f; v4.b = 0.0f; v4.u = 0.01f; v4.v = 1.0f;
	vertices.push_back(v1);
	vertices.push_back(v2);
	vertices.push_back(v3);
	vertices.push_back(v1);
	vertices.push_back(v3);
	vertices.push_back(v4);

	sMapMesh = sMeshArray + sNumMesh++;
	sMapTex = sTexArray + sNumTex++;
	*sMapMesh = CreateMesh(vertices);
	*sMapTex = TextureLoad("level.png");
	sMapOffset = 0.25f;


	//-----------------------------------------
	// Load level from txt file to sMapData, sMapCollisonData, sPlayer_start_position
	//	- 0	is an empty space
	//	- 1-4 are background tile
	//	- 5-7 are game objects location
	//-----------------------------------------

	std::ifstream myfile("map2.txt");
	if (myfile.is_open())
	{
		myfile >> MAP_HEIGHT;
		myfile >> MAP_WIDTH;
		sMapData = new int* [MAP_HEIGHT];
		sMapCollisionData = new int* [MAP_HEIGHT];
		for (int y = 0; y < MAP_HEIGHT; y++) 
		{
			sMapData[y] = new int[MAP_WIDTH];
			sMapCollisionData[y] = new int[MAP_WIDTH];
			for (int x = 0; x < MAP_WIDTH; x++) 
			{
				myfile >> sMapData[y][x];
			}
		}
		myfile.close();
	}

	//+ load collision data to sMapCollisionData
	//	- 0: non-blocking cell
	//	- 1: blocking cell
	//** Don't forget that sMapCollisionData index go from- 
	//**	 bottom to top not from top to bottom as in the text file
	for (int y = 0; y < MAP_HEIGHT; y++)
	{
		for (int x = 0; x < MAP_WIDTH; x++)
		{
			if (sMapData[y][x] >= 1 && sMapData[y][x] <= 4)
			{
				sMapCollisionData[(MAP_HEIGHT - 1) - y][x] = 1;
			}
			else
			{
				sMapCollisionData[(MAP_HEIGHT - 1) - y][x] = 0;
			}
		}
	}

	//-----------------------------------------
	//+ Compute Map Transformation Matrix
	//-----------------------------------------

	glm::mat4 tMat = glm::translate(glm::mat4(1.0f), glm::vec3(-MAP_WIDTH / 2.0f, -MAP_HEIGHT / 2.0f, 0.0f));
	glm::mat4 sMat = glm::scale(glm::mat4(1.0f), glm::vec3(CELL_SIZE, CELL_SIZE, 1.0f));
	sMapMatrix = sMat * tMat;


	printf("Level1: Load\n");
}


void GameStateLevel1Init(void) 
{

	//-----------------------------------------
	// Create game object instance from Map
	//	0,1,2,3,4:	level tiles
	//  5: player, 6: enemy, 7: item
	//-----------------------------------------

	GameObject* obj;
	Weapon* weapon;

	for (int y = 0; y < MAP_HEIGHT; y++)
	{
		for (int x = 0; x < MAP_WIDTH; x++)
		{

			switch (sMapData[y][x])
			{
				// Player
				case 5:
				{
					obj = gameObjInstCreate(GAMEOBJ_TYPE::TYPE_PLAYER, glm::vec3(x + 0.5f, (MAP_HEIGHT - y) - 0.5f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f),
						glm::vec3(1.0f, 1.0f, 1.0f), 0.0f, true, 2, 0, 0.25f);
					sPlayer_start_position = glm::vec3(x + 0.5f, (MAP_HEIGHT - y) - 0.5f, 0.0f);
					sPlayer = dynamic_cast<Player*>(obj);
					sPlayer->setJumping(false);

					weapon = dynamic_cast<Weapon*>(gameObjInstCreate(GAMEOBJ_TYPE::TYPE_WEAPON_BOW, sPlayer->getPosition(), glm::vec3(0.0f, 0.0f, 0.0f),
						glm::vec3(0.75f, 0.75f, 1.0f), 0.0f, false, 0, 0, 0.0f));
					sPlayer->setWeapon(weapon);
					weapon->setHolder(sPlayer);
					break;
				}
				// Enemy
				case 6:
				{
					GameObject* obj = gameObjInstCreate(GAMEOBJ_TYPE::TYPE_ENEMY, glm::vec3(x + 0.5f, (MAP_HEIGHT - y) - 0.5f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f),
						glm::vec3(1.0f, 1.0f, 1.0f), 0.0f, true, 2, 0, 0.5f);
					weapon = dynamic_cast<Weapon*>(gameObjInstCreate(GAMEOBJ_TYPE::TYPE_WEAPON_SWORD, obj->getPosition(), glm::vec3(0.0f, 0.0f, 0.0f),
						glm::vec3(0.75f, 0.75f, 1.0f), 0.0f, false, 0, 0, 0.0f));
					dynamic_cast<Character*>(obj)->setWeapon(weapon);
					weapon->setHolder(obj);
					break;
				}
				case 7:
				{
					GameObject* obj = gameObjInstCreate(GAMEOBJ_TYPE::TYPE_ENEMY, glm::vec3(x + 0.5f, (MAP_HEIGHT - y) - 0.5f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f),
						glm::vec3(1.0f, 1.0f, 1.0f), 0.0f, true, 2, 0, 0.5f);
					weapon = dynamic_cast<Weapon*>(gameObjInstCreate(GAMEOBJ_TYPE::TYPE_WEAPON_FIRE_WAND, obj->getPosition(), glm::vec3(0.0f, 0.0f, 0.0f),
						glm::vec3(0.75f, 0.75f, 1.0f), 0.0f, false, 0, 0, 0.0f));
					dynamic_cast<Character*>(obj)->setWeapon(weapon);
					weapon->setHolder(obj);
					break;
				}
				case 8:
				{
					GameObject* obj = gameObjInstCreate(GAMEOBJ_TYPE::TYPE_ENEMY, glm::vec3(x + 0.5f, (MAP_HEIGHT - y) - 0.5f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f),
						glm::vec3(1.0f, 1.0f, 1.0f), 0.0f, true, 2, 0, 0.5f);
					weapon = dynamic_cast<Weapon*>(gameObjInstCreate(GAMEOBJ_TYPE::TYPE_WEAPON_BOW, obj->getPosition(), glm::vec3(0.0f, 0.0f, 0.0f),
						glm::vec3(0.75f, 0.75f, 1.0f), 0.0f, false, 0, 0, 0.0f));
					dynamic_cast<Character*>(obj)->setWeapon(weapon);
					weapon->setHolder(obj);
					break;
				}
				default:
					break;
			}
		}
	}
	
	SetCamPosition(0.0f, -25.0f);

	// Initalize some data. ex. score and player life
	sRespawnCountdown = 0;
	CULL_ON = false;
	C_DOWN = false;

	// Sound
	SoundEngine = createIrrKlangDevice();
	//SoundEngine->play2D("mario_level.ogg", true);		//loop or not


	printf("Level1: Init\n");
}


void GameStateLevel1Update(double dt, long frame, int& state) 
{

	//-----------------------------------------
	// Get user input
	//-----------------------------------------

	if (sRespawnCountdown <= 0)
	{
		sPlayer->getInput();
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && sPlayer->getCharacterState() != CHARACTER_STATE::STATE_ATTACK)
		{
			Weapon* weapon = dynamic_cast<Weapon*>(sPlayer->getWeapon());
			if (weapon != nullptr && weapon->getCurrentCooldown() <= 0.0f)
			{
				if (weapon->getWeaponType() == WEAPON_TYPE::WEAPON_TYPE_RANGED)
				{
					double xPos, yPos;
					glfwGetCursorPos(window, &xPos, &yPos);
					weapon->setOrientation(-atan2(yPos - TO_WORLD_SPACE_Y(weapon->getPosition().y), xPos - TO_WORLD_SPACE_X(weapon->getPosition().x)));

					// Set Holder Facing Direction when Shooting with Opposite side of player
					if (weapon->getOrientation() >= -90 * DegToRad && weapon->getOrientation() <= 90 * DegToRad)
					{
						weapon->getHolder()->setScaleX(1.0f);
					}
					else
					{
						weapon->getHolder()->setScaleX(-1.0f);
					}
					std::cout << TO_WORLD_SPACE_X(weapon->getPosition().x) << " | " << xPos << "\n";
					float vec_x = TO_WORLD_SPACE_X(weapon->getPosition().x) - xPos;
					float vec_y = TO_WORLD_SPACE_Y(weapon->getPosition().y) - yPos;
					//std::cout << angle << "\n";

					float totalVec = pow(pow(vec_x, 2) + pow(vec_y, 2), 0.5f);
					vec_x /= totalVec;
					vec_y /= totalVec;
					GameObject* bullet = gameObjInstCreate(dynamic_cast<RangeWeapon*>(weapon)->getBulletType(), weapon->getPosition(), glm::vec3(vec_x * 20.0f * -1.0f, vec_y * 20.0f, 0.0f),
						glm::vec3(1.0f, 1.0f, 1.0f), 0.0f, false, 0, 0, 0);
					dynamic_cast<Bullet*>(bullet)->setShooter(sPlayer);
				}
				else if (weapon->getWeaponType() == WEAPON_TYPE::WEAPON_TYPE_SPELL)
				{
					double xPos, yPos;
					glfwGetCursorPos(window, &xPos, &yPos);
					float angle = -atan2(yPos - TO_WORLD_SPACE_Y(weapon->getPosition().y), xPos - TO_WORLD_SPACE_X(weapon->getPosition().x));

					// Set Holder Facing Direction when Shooting with Opposite side of player
					if (angle >= -90 * DegToRad && angle <= 90 * DegToRad)
					{
						weapon->getHolder()->setScaleX(1.0f);
					}
					else
					{
						weapon->getHolder()->setScaleX(-1.0f);
					}
					float vec_x = TO_WORLD_SPACE_X(weapon->getPosition().x) - xPos;
					float vec_y = TO_WORLD_SPACE_Y(weapon->getPosition().y) - yPos;
					
					float totalVec = pow(pow(vec_x, 2) + pow(vec_y, 2), 0.5f);
					vec_x /= totalVec;
					vec_y /= totalVec;

					GameObject* bullet = gameObjInstCreate(dynamic_cast<SpellWeapon*>(weapon)->getBulletType(), weapon->getPosition(), glm::vec3(vec_x * 10.0f * -1.0f, vec_y * 10.0f, 0.0f),
						glm::vec3(1.0f, 1.0f, 1.0f), 0.0f, false, 0, 0, 0);
					dynamic_cast<Bullet*>(bullet)->setShooter(sPlayer);
				}
				dynamic_cast<Weapon*>(sPlayer->getWeapon())->Attack();
				sPlayer->setCharacterState(CHARACTER_STATE::STATE_ATTACK);
			}
		}
		else if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
		{
			sPlayer->setCharacterState(CHARACTER_STATE::STATE_IDLE);
		}

		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
		{
			for (int idx = 0; idx < GAME_OBJ_INST_MAX; idx++)
			{
				GameObject*& pInst = sGameObjInstArray[idx];

				if (pInst == nullptr || pInst->getFlag() == FLAG_INACTIVE)
				{
					continue;
				}

				bool collide = GameState::isCollide(sPlayer, pInst);
				if (collide && CheckType(pInst->getType()) == TYPE_ENEMY)
				{
					if (dynamic_cast<Character*>(pInst)->getCharacterState() == CHARACTER_STATE::STATE_DIE)
					{
						for (int idx2 = 0; idx2 < GAME_OBJ_INST_MAX; idx2++)
						{
							GameObject*& pInst2 = sGameObjInstArray[idx2];

							if (pInst2 == nullptr || pInst2->getFlag() == FLAG_INACTIVE)
							{
								continue;
							}
							if (pInst2 == sPlayer->getWeapon())
							{
								gameObjInstDestroy(pInst2);
								break;
							}
						}
						Weapon* newWeapon = dynamic_cast<Weapon*>(dynamic_cast<Character*>(pInst)->getWeapon());
						sPlayer->setWeapon(newWeapon);
						newWeapon->setHolder(sPlayer);
						gameObjInstDestroy(pInst);
					}
				}
			}
		}
	}
	else
	{
		//+ update sRespawnCountdown
		sRespawnCountdown++;
		if (sRespawnCountdown >= RESPAWN_TIME)
		{
			state = 2;
		}
	}

	// Cam zoom UI
	if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
	{
		ZoomIn(0.1f);
	}
	if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
	{
		ZoomOut(0.1f);
	}


	//---------------------------------------------------------
	// Update all game obj position using velocity 
	//---------------------------------------------------------
	
	for (int i = 0; i < GAME_OBJ_INST_MAX; i++)
	{
		GameObject* pInst = sGameObjInstArray[i];

		// skip inactive object
		if (pInst == nullptr || pInst->getFlag() == FLAG_INACTIVE)
		{
			continue;
		}

		if (pInst->getType() == TYPE_PLAYER) 
		{

			// Apply gravity: Velocity Y = Gravity * Frame Time + Velocity Y
			if (sPlayer->isJumping()) 
			{
				sPlayer->increaseVelocityY(GRAVITY * dt);
			}

			// Update position using Velocity
			pInst->updatePosition(pInst->getVelocity() * glm::vec3(dt, dt, 0.0f));
			
			// If Player reach limit on Left side -> No need to Scrolling Camera -> set to static position
			if (TO_SCREEN_SPACE_X(pInst->getPosition().x) < TO_SCREEN_SPACE_X(0) + SCREEN_SIZE / 2.0f)
			{
				SetCamPosition(TO_SCREEN_SPACE_X(0) + SCREEN_SIZE / 2.0f, 0.0f);
			}
			// If Player reach limit on right side -> No need to Scrolling Camera -> set to static position
			else if (TO_SCREEN_SPACE_X(pInst->getPosition().x) > TO_SCREEN_SPACE_X(MAP_WIDTH) - SCREEN_SIZE / 2.0f)
			{
				SetCamPosition(TO_SCREEN_SPACE_X(MAP_WIDTH) - SCREEN_SIZE / 2.0f, 0.0f);
			}
			else
			{
				// Camera tracking Player position
				SetCamPosition(TO_SCREEN_SPACE_X(pInst->getPosition().x), 0.0f);

				// Update LimitLeft and LimitRight
				sLimitLeft = pInst->getPosition().x - TO_MAP_SPACE(SCREEN_SIZE / 2.0f);
				sLimitRight = pInst->getPosition().x + TO_MAP_SPACE(SCREEN_SIZE / 2.0f);
				std::cout << "L : " << sLimitLeft << " | " << "R :" << sLimitRight << "\n";
			}
		}
		else if (CheckType(pInst->getType()) == TYPE_ENEMY)
		{
			dynamic_cast<Enemy*>(pInst)->UpdateEnemyState(sMapCollisionData, dt);

			if (pInst->getVelocity().x < 0)
			{
				pInst->setScaleX(-1.0f);
			}
			else if (pInst->getVelocity().x > 0)
			{
				pInst->setScaleX(1.0f);
			}

			// Update enemy position
			pInst->updatePosition(pInst->getVelocity() * glm::vec3(dt, dt, 0.0f));
		}
		// If pInst is weapon
		else if (dynamic_cast<Weapon*>(pInst) != nullptr)
		{
			Weapon* weapon = dynamic_cast<Weapon*>(pInst);
			weapon->updateWeaponDirection();
			weapon->updateWeaponCooldown(dt);
		}
		else if (pInst->getType() >= GAMEOBJ_TYPE::TYPE_WEAPON_BULLET)
		{
			if (pInst->getType() == GAMEOBJ_TYPE::TYPE_WEAPON_BOW_ARROW)
			{
				pInst->increaseVelocityY(GRAVITY * dt);
			}
			// Update enemy position
			pInst->updatePosition(pInst->getVelocity()* glm::vec3(dt, dt, 0.0f));
		}

	}

	//-----------------------------------------
	// Update animation for animated object 
	//-----------------------------------------
	if (frame % ANIMATION_SPEED == 0)
	{
		for (int i = 0; i < GAME_OBJ_INST_MAX; i++)
		{
			GameObject* pInst = sGameObjInstArray[i];

			// skip inactive object
			if (pInst == nullptr || pInst->getFlag() == FLAG_INACTIVE)
			{
				continue;
			}

			// if this is an animated object
			if (pInst->isAnimation())
			{

				//+ increment pInst->currFrame
				pInst->setCurrentFrame(pInst->getCurrentFrame() + 1);

				//	- if we reach the last frame then set the current frame back to frame 0
				if (pInst->getCurrentFrame() > pInst->getTotalFrame())
				{
					pInst->setCurrentFrame(0);
				}


				//+ use currFrame infomation to set pInst->offsetX
				pInst->setOffsetX(pInst->getCurrentFrame() * pInst->getOffset());

			}
		}
	}

	//-----------------------------------------
	// Update some game obj behavior
	//-----------------------------------------

	//-----------------------------------------
	// Check for collsion with the Map
	//-----------------------------------------
	for (int i = 0; i < GAME_OBJ_INST_MAX; i++)
	{
		GameObject*& pInst = sGameObjInstArray[i];

		// skip inactive object
		if (pInst == nullptr || pInst->getFlag() == FLAG_INACTIVE)
		{
			continue;
		}

		if (pInst->getType() == GAMEOBJ_TYPE::TYPE_PLAYER)
		{
			// Update Player position, velocity, jumping states when the player collide with the map
			int collisionFlag = GameState::CheckCharacterMapCollision(sMapCollisionData, sPlayer->getPosition().x, sPlayer->getPosition().y);

			// Collide Left
			if (collisionFlag & COLLISION_LEFT)
			{
				sPlayer->setPositionX((int)sPlayer->getPosition().x + 0.5f);
			}

			//+ Collide Right
			if (collisionFlag & COLLISION_RIGHT)
			{
				sPlayer->setPositionX((int)sPlayer->getPosition().x + 0.5f);
			}

			//+ Collide Top
			if (collisionFlag & COLLISION_TOP)
			{
				sPlayer->setPositionY((int)sPlayer->getPosition().y + 0.5f);
				sPlayer->setVelocityY(0.0f);
			}

			//+ Player is on the ground or just landed on the ground
			if (collisionFlag & COLLISION_BOTTOM)
			{
				sPlayer->setPositionY((int)sPlayer->getPosition().y + 0.5f);
				sPlayer->setVelocityY(0.0f);
				sPlayer->setJumping(false);
			}
			//+ Player is jumping/falling
			else
			{
				sPlayer->setJumping(true);
			}
		}
		else if (pInst->getType() >= GAMEOBJ_TYPE::TYPE_WEAPON_BULLET)
		{
			// Update Player position, velocity, jumping states when the player collide with the map
			int collisionFlag = GameState::CheckBulletMapCollision(sMapCollisionData, pInst->getPosition().x, pInst->getPosition().y, 0.1f);
			
			if (collisionFlag & COLLISION_LEFT || collisionFlag & COLLISION_RIGHT || 
				collisionFlag & COLLISION_TOP || collisionFlag & COLLISION_BOTTOM)
			{
				gameObjInstDestroy(pInst);
			}
		}
	}



	//-----------------------------------------
	// Check for collsion between game objects
	//	- Player vs Enemy
	//	- Player vs Item
	//-----------------------------------------

	for (int i = 0; i < GAME_OBJ_INST_MAX; i++) {
		GameObject*& pInst1 = sGameObjInstArray[i];

		// skip inactive object
		if (pInst1 == nullptr || pInst1->getFlag() == FLAG_INACTIVE || pInst1->hasCollision() == false)
		{
			continue;
		}

		for (int j = 0; j < GAME_OBJ_INST_MAX; j++)
		{
			GameObject*& pInst2 = sGameObjInstArray[j];

			// skip inactive object
			if (pInst2 == nullptr || pInst2->getFlag() == FLAG_INACTIVE || pInst2->hasCollision() == false)
			{
				continue;
			}

			bool collide = GameState::isCollide(pInst1, pInst2, COLLISION_OFFSET, COLLISION_OFFSET);

			//+ Player vs Enemy
			//	- if the Player die, set the sRespawnCountdown > 0	
			if (pInst1->getType() == TYPE_PLAYER && CheckType(pInst2->getType()) == TYPE_ENEMY && collide)
			{
				Character* enemy = dynamic_cast<Character*>(pInst2);
				if (enemy->getCharacterState() == CHARACTER_STATE::STATE_DIE)
				{
					continue;
				}
				if (sRespawnCountdown == 0)
				{
					// Set velocity of player to 0 don't move & Don't play animation
					pInst1->setVelocity(glm::vec3(0.0f, 0.0f, 0.0f));
					pInst1->setAnimation(false);
					// Start Counting RespawnFrame
					sRespawnCountdown = 1;
					// Stop all sound & Play die sound
					SoundEngine->stopAllSounds();
					SoundEngine->play2D("die.wav");
				}
			}
			else if (pInst1->getType() == TYPE_WEAPON_SWORD && pInst1 == sPlayer->getWeapon() && CheckType(pInst2->getType()) == TYPE_ENEMY && collide)
			{
				Character* enemy = dynamic_cast<Character*>(pInst2);
				if (enemy->getCharacterState() != CHARACTER_STATE::STATE_DIE)
				{
					enemy->setCharacterState(CHARACTER_STATE::STATE_DIE);
					enemy->setScaleX(0.75f);
					enemy->setScaleY(0.75f);
					enemy->setAnimation(false);
					enemy->setVelocityX(0.0f);
					enemy->setVelocityY(0.0f);
				}
			}
			else if (CheckType(pInst1->getType()) == TYPE_ENEMY && dynamic_cast<Bullet*>(pInst2) != nullptr && dynamic_cast<Bullet*>(pInst2)->getShooter() == sPlayer && collide)
			{
				Character* enemy = dynamic_cast<Character*>(pInst1);
				if (enemy->getCharacterState() != CHARACTER_STATE::STATE_DIE)
				{
					enemy->setCharacterState(CHARACTER_STATE::STATE_DIE);
					enemy->setScaleX(0.75f);
					enemy->setScaleY(0.75f);
					enemy->setAnimation(false);
					enemy->setVelocityX(0.0f);
					enemy->setVelocityY(0.0f);
				}
				gameObjInstDestroy(pInst2);
				break;
			}
		}
	}

	//-----------------------------------------
	// Update modelMatrix of all game obj
	//-----------------------------------------

	for (int i = 0; i < GAME_OBJ_INST_MAX; i++) 
	{
		GameObject* pInst = sGameObjInstArray[i];

		// skip inactive object
		if (pInst == nullptr || pInst->getFlag() == FLAG_INACTIVE)
		{
			continue;
		}

		// Compute the scaling matrix, rotation matrix(around z axis) and translation matrix
		// Concatenate the 3 matrix to from Model Matrix
		glm::mat4 rMat = glm::rotate(glm::mat4(1.0f), pInst->getOrientation(), glm::vec3(0.0f, 0.0f, 1.0f));
		glm::mat4 sMat = glm::scale(glm::mat4(1.0f), pInst->getScale());
		glm::mat4 tMat = glm::translate(glm::mat4(1.0f), pInst->getPosition());
		pInst->setModelMatrix(tMat * sMat * rMat);
	}

	double fps = 1.0 / dt;
	//printf("Level1: Update @> %f fps, frame>%ld\n", fps, frame);
	//printf("Life> %i\n", sPlayerLives);
	//printf("Score> %i\n", sScore);
	//printf("num obj> %i\n", sNumGameObj);
}

void GameStateLevel1Draw(void) {

	// Clear the screen
	glClearColor(0.0f, 0.5f, 1.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	//--------------------------------------------------------
	// Draw Level
	//--------------------------------------------------------
	glm::mat4 matTransform;
	glm::mat4 cellMatrix;

	for (int y = 0; y < MAP_HEIGHT; y++)
	{
		for (int x = 0; x < MAP_WIDTH; x++)
		{

			//+ Only draw background cell
			if (sMapData[y][x] >= 1 && sMapData[y][x] <= 4)
			{

				//+ Find transformation matrix of each cell, cellMatrix is just a translation matrix
				cellMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(x + 0.5f, (MAP_HEIGHT - y) - 0.5f, 0.0f));

				// Transform cell from map space [0,MAP_SIZE] to screen space [-width/2,width/2]
				matTransform = sMapMatrix * cellMatrix;

				// Render each cell
				SetRenderMode(CDT_TEXTURE, 1.0f);
				SetTexture(*sMapTex, sMapOffset * (sMapData[y][x] - 1), 0.0f);
				SetTransform(matTransform);
				DrawMesh(*sMapMesh);
			}
		}
	}



	//--------------------------------------------------------
	// Draw all game object instance in the sGameObjInstArray
	//--------------------------------------------------------

	for (int i = 0; i < GAME_OBJ_INST_MAX; i++) {
		GameObject* pInst = sGameObjInstArray[i];


		// skip inactive object
		if (pInst == nullptr || pInst->getFlag() == FLAG_INACTIVE)
		{
			continue;
		}

		// Transform cell from map space [0,MAP_SIZE] to screen space [-width/2,width/2]
		matTransform = sMapMatrix * pInst->getModelMatrix();

		SetRenderMode(CDT_TEXTURE, 1.0f);
		SetTexture(*pInst->getTexture(), pInst->getOffsetX(), pInst->getOffsetY());
		SetTransform(matTransform);
		DrawMesh(*pInst->getMesh());
	}


	// Swap the buffer, to present the drawing
	glfwSwapBuffers(window);
}

void GameStateLevel1Free(void) {

	// call gameObjInstDestroy for all object instances in the sGameObjInstArray
	for (int i = 0; i < GAME_OBJ_INST_MAX; i++) 
	{
		gameObjInstDestroy(sGameObjInstArray[i]);
	}

	// reset camera
	ResetCam();

	// Free sound
	SoundEngine->drop();

	printf("Level1: Free\n");
}

void GameStateLevel1Unload(void) {

	// Unload all meshes in MeshArray
	for (int i = 0; i < sNumMesh; i++) 
	{
		UnloadMesh(sMeshArray[i]);
	}

	// Unload all textures in TexArray
	for (int i = 0; i < sNumTex; i++) 
	{
		TextureUnload(sTexArray[i]);
	}

	// Unload Level
	for (int i = 0; i < MAP_HEIGHT; ++i) 
	{
		delete[] sMapData[i];
		delete[] sMapCollisionData[i];
	}
	delete[] sMapData;
	delete[] sMapCollisionData;


	printf("Level1: Unload\n");
}