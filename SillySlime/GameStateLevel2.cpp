#include "GameStateLevel2.h"
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
static GameObject* sGameObjInstArray[GAME_OBJ_INST_MAX];			// Store all game object instance
static int			sNumGameObj;
// Player data
static Player* sPlayer;										// Pointer to the Player game object instance
static glm::vec3	sPlayer_start_position;
static int			sPlayerLives;									// The number of lives left
static int			sScore;
static int			sRespawnCountdown;								// Respawn player waiting time (in #frame)
//Sound
static ISoundEngine* SoundEngine;
// Map data
static int** sMapData;										// sMapData[Height][Width]
static int** sMapCollisionData;
static int			MAP_WIDTH;
static int			MAP_HEIGHT;
static glm::mat4	sMapMatrix;										// Transform from map space [0,MAP_SIZE] to screen space [-width/2,width/2]
static CDTMesh* sMapMesh;										// Mesh & Tex of the level, we only need 1 of these
static CDTTex* sMapTex;
static float		sMapOffset;
static bool			CULL_ON;										// enable culling
static bool			C_DOWN;
// Scrolling map data
static int			sStartCam;										// This example only scroll in X direction
static int			sLimitLeft;
static int			sLimitRight;

static irrklang::ISoundSource* BGM;
static irrklang::ISoundSource* fireSpell;
static irrklang::ISoundSource* shootArrow;
static irrklang::ISoundSource* slimeShoot;
static irrklang::ISoundSource* swordAttack;

// -------------------------------------------
// Game object instant functions
// -------------------------------------------
// functions to create/destroy a game object instance
static GameObject*  gameObjInstCreate(int type, glm::vec3 pos, glm::vec3 vel, glm::vec3 scale, float orient, bool anim, int numFrame, int currFrame, float offset)
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
static bool			checkRaycast(Character* origin, Character* target)
{
	int x1 = origin->getPosition().x;
	int y1 = origin->getPosition().y;
	int x2 = target->getPosition().x;
	int y2 = target->getPosition().y;
	if (x1 > x2 && origin->getScale().x > 0)
	{
		return false;
	}

	if (x1 < x2 && origin->getScale().x < 0)
	{
		return false;
	}

	/* Bresenham’s Line Generation Algorithm */
	//pk is initial decision making parameter
	//Note:x1&y1,x2&y2, dx&dy values are interchanged
	int dx = abs(x2 - x1);
	int dy = abs(y2 - y1);

	bool isSwap = false;
	if (dy > dx)
	{
		isSwap = true;
		std::swap(dx, dy);
		std::swap(x1, y1);
		std::swap(x2, y2);
	}

	int pk = 2 * dy - dx;
	for (int i = 0; i <= dx; i++)
	{
		//std::cout << x1 << "," << y1 << std::endl;
		//checking either to decrement or increment the value
		x1 < x2 ? x1++ : x1--;
		if (pk < 0)
		{
			//decision value will decide to plot
			//either  x1 or y1 in x's position
			if (!isSwap)
			{
				if (sMapCollisionData[y1][x1] == 1)
				{
					return false;
				}
				pk = pk + 2 * dy;
			}
			else
			{
				if (sMapCollisionData[x1][y1] == 1)
				{
					return false;
				}
				pk = pk + 2 * dy;
			}
		}
		else
		{
			y1 < y2 ? y1++ : y1--;
			if (!isSwap)
			{
				if (sMapCollisionData[y1][x1] == 1)
				{
					return false;
				}
			}
			else
			{
				if (sMapCollisionData[x1][y1] == 1)
				{
					return false;
				}
			}
			pk = pk + 2 * dy - 2 * dx;
		}
	}
	return true;
}
static int			CheckType(int type)
{
	if (type >= TYPE_ENEMY && type <= TYPE_ENEMY_END)
	{
		return TYPE_ENEMY;
	}
}

// -------------------------------------------
// Game states function
// -------------------------------------------
void GameStateLevel2Load(void)
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
	
	CDTMesh* pMesh;
	CDTTex* pTex;
	std::vector<CDTVertex> vertices;
	CDTVertex v1, v2, v3, v4;
	v1.x = -1.0f; v1.y = -0.5f; v1.z = 0.0f; v1.r = 1.0f; v1.g = 0.0f; v1.b = 0.0f; v1.u = 0.0f; v1.v = 0.0f;
	v2.x = 1.0f; v2.y = -0.5f; v2.z = 0.0f; v2.r = 0.0f; v2.g = 1.0f; v2.b = 0.0f; v2.u = 2.0f; v2.v = 0.0f;
	v3.x = 1.0f; v3.y = 0.5f; v3.z = 0.0f; v3.r = 0.0f; v3.g = 0.0f; v3.b = 1.0f; v3.u = 2.0f; v3.v = 1.0f;
	v4.x = -1.0f; v4.y = 0.5f; v4.z = 0.0f; v4.r = 1.0f; v4.g = 1.0f; v4.b = 0.0f; v4.u = 0.0f; v4.v = 1.0f;
	vertices.push_back(v1);
	vertices.push_back(v2);
	vertices.push_back(v3);
	vertices.push_back(v1);
	vertices.push_back(v3);
	vertices.push_back(v4);

	pMesh = sMeshArray + sNumMesh++;
	pTex = sTexArray + sNumTex++;
	*pMesh = CreateMesh(vertices);
	*pTex = TextureLoad("BG.png");

	// Create Player mesh/texture
	createMeshTex("Exit.png", 1.0f, 1.0f);
	// Create PinkSlime mesh/texture
	createMeshTex("PinkSlime.png", 1.0f, 1.0f);
	// Create Player mesh/texture
	createMeshTex("SlimeSheet.png", 0.25f, 1.0f);

	// Create Enemy(header) mesh/texture
	createMeshTex("nullOBJ.png", 1.0f, 1.0f);
	// Create Enemy_Warrior mesh/texture
	createMeshTex("Warrior_SpriteSheet.png", 0.5f, 1.0f);
	// Create Enemy_Archer mesh/texture
	createMeshTex("Archer_SpriteSheet.png", 0.5f, 1.0f);
	// Create Enemy_Mage mesh/texture
	createMeshTex("Mage_SpriteSheet.png", 0.5f, 1.0f);
	// Create Enemy_END(header) mesh/texture
	createMeshTex("nullOBJ.png", 1.0f, 1.0f);

	// Create Sword mesh/texture
	createMeshTex("Sword_SpriteSheet.png", 0.25, 1.0f);
	// Create Bow mesh/texture
	createMeshTex("Bow_SpriteSheet.png", 0.5f, 1.0f);
	// Create FireWand mesh/texture
	createMeshTex("FireWand_SpriteSheet.png", 0.5f, 1.0f);
	// Create Slime Shooter mesh/texture
	createMeshTex("EmptyObject.png", 1.0f, 1.0f);

	// Create Bullet(header) mesh/texture
	createMeshTex("nullOBJ.png", 1.0f, 1.0f);
	// Create Arrow mesh/texture
	createMeshTex("Arrow.png", 1.0f, 1.0f);
	// Create FireSpell mesh/texture
	createMeshTex("FireBall.png", 1.0f, 1.0f);
	// Create Slime Bullet mesh/texture
	createMeshTex("SlimeBullet.png", 1.0f, 1.0f);

	// Create UI(header) mesh/texture
	createMeshTex("nullOBJ.png", 1.0f, 1.0f);
	// Create UI_Bar(sub-header) mesh/texture
	createMeshTex("UI_Bar.png", 1.0f, 1.0f);
	// Create UI_Health mesh/texture
	createMeshTex("UI_HP.png", 1.0f, 1.0f);
	// Create UI_Health_Bar mesh/texture
	createMeshTex("UI_Health_Bar.png", 1.0f, 1.0f);
	// Create UI_Walk mesh/texture
	createMeshTex("UI_Walk.png", 1.0f, 1.0f);
	// Create UI_Jump mesh/texture
	createMeshTex("UI_Jump.png", 1.0f, 1.0f);
	// Create UI_LeftClick mesh/texture
	createMeshTex("UI_LeftClick.png", 1.0f, 1.0f);
	// Create UI_RightClick mesh/texture
	createMeshTex("UI_RightClick.png", 1.0f, 1.0f);
	// Create UI_RightClick mesh/texture
	createMeshTex("UI_MissionComplete.png", 1.0f, 1.0f);

	// Create UI_Type(header) mesh/texture
	createMeshTex("nullOBJ.png", 1.0f, 1.0f);
	// Create UI_Type_Normal mesh/texture
	createMeshTex("UI_NORMAL_Type.png", 1.0f, 1.0f);
	// Create UI_Type_Warrior mesh/texture
	createMeshTex("UI_WARRIOR_Type.png", 1.0f, 1.0f);
	// Create UI_Type_Archer mesh/texture
	createMeshTex("UI_ARCHER_Type.png", 1.0f, 1.0f);
	// Create UI_Type_Mage mesh/texture
	createMeshTex("UI_MAGE_Type.png", 1.0f, 1.0f);

	// Create Level mesh/texture
	vertices.clear();
	v1.x = -0.5f; v1.y = -0.5f; v1.z = 0.0f; v1.r = 1.0f; v1.g = 0.0f; v1.b = 0.0f; v1.u = 0.01f; v1.v = 0.01f;
	v2.x = 0.5f; v2.y = -0.5f; v2.z = 0.0f; v2.r = 0.0f; v2.g = 1.0f; v2.b = 0.0f; v2.u = 0.20f; v2.v = 0.01f;
	v3.x = 0.5f; v3.y = 0.5f; v3.z = 0.0f; v3.r = 0.0f; v3.g = 0.0f; v3.b = 1.0f; v3.u = 0.20f; v3.v = 1.0f;
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
	*sMapTex = TextureLoad("Tile.png");
	sMapOffset = 0.20f;


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
			if ((sMapData[y][x] >= 1 && sMapData[y][x] <= 5) || y == 0 || x == 0 || x == MAP_WIDTH - 1)
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


	printf("Level2: Load\n");
}


void GameStateLevel2Init(void)
{

	//-----------------------------------------
	// Create game object instance from Map
	//	0,1,2,3,4,5:	level tiles
	//  6: player, 7,8,9: enemy
	//-----------------------------------------

	GameObject* obj;
	Weapon* weapon;


	// Create GameOBJ from MapData
	for (int y = 0; y < MAP_HEIGHT; y++)
	{
		for (int x = 0; x < MAP_WIDTH; x++)
		{

			switch (sMapData[y][x])
			{
				// Player
			case 6:
			{
				obj = gameObjInstCreate(GAMEOBJ_TYPE::TYPE_PLAYER, glm::vec3(x + 0.5f, (MAP_HEIGHT - y) - 0.5f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f),
					glm::vec3(1.0f, 1.0f, 1.0f), 0.0f, true, 2, 0, 0.25f);
				sPlayer_start_position = glm::vec3(x + 0.5f, (MAP_HEIGHT - y) - 0.5f, 0.0f);
				sPlayer = dynamic_cast<Player*>(obj);
				sPlayer->setJumping(false);

				weapon = dynamic_cast<Weapon*>(gameObjInstCreate(GAMEOBJ_TYPE::TYPE_WEAPON_SLIME, sPlayer->getPosition(), glm::vec3(0.0f, 0.0f, 0.0f),
					glm::vec3(0.75f, 0.75f, 1.0f), 0.0f, false, 0, 0, 0.0f));
				sPlayer->setWeapon(weapon);
				weapon->setHolder(sPlayer);
				break;
			}
			// Enemy
			case 7:
			{
				obj = gameObjInstCreate(GAMEOBJ_TYPE::TYPE_ENEMY_WARRIOR, glm::vec3(x + 0.5f, (MAP_HEIGHT - y) - 0.5f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f),
					glm::vec3(1.0f, 1.0f, 1.0f), 0.0f, true, 2, 0, 0.5f);
				weapon = dynamic_cast<Weapon*>(gameObjInstCreate(GAMEOBJ_TYPE::TYPE_WEAPON_SWORD, glm::vec3(obj->getPosition().x, obj->getPosition().y, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f),
					glm::vec3(1.0f, 1.0f, 1.0f), 0.0f, false, 2, 0, 0.25f));
				dynamic_cast<Enemy*>(obj)->setWeapon(weapon);
				weapon->setHolder(obj);
				break;
			}
			case 8:
			{
				obj = gameObjInstCreate(GAMEOBJ_TYPE::TYPE_ENEMY_MAGE, glm::vec3(x + 0.5f, (MAP_HEIGHT - y) - 0.5f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f),
					glm::vec3(1.0f, 1.0f, 1.0f), 0.0f, true, 2, 0, 0.5f);
				weapon = dynamic_cast<Weapon*>(gameObjInstCreate(GAMEOBJ_TYPE::TYPE_WEAPON_FIRE_WAND, glm::vec3(obj->getPosition().x, obj->getPosition().y, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f),
					glm::vec3(1.0f, 1.0f, 1.0f), 0.0f, false, 0, 0, 0.0f));
				dynamic_cast<Enemy*>(obj)->setWeapon(weapon);
				weapon->setHolder(obj);
				break;
			}
			case 9:
			{
				obj = gameObjInstCreate(GAMEOBJ_TYPE::TYPE_ENEMY_ARCHER, glm::vec3(x + 0.5f, (MAP_HEIGHT - y) - 0.5f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f),
					glm::vec3(1.0f, 1.0f, 1.0f), 0.0f, true, 2, 0, 0.5f);
				weapon = dynamic_cast<Weapon*>(gameObjInstCreate(GAMEOBJ_TYPE::TYPE_WEAPON_BOW, glm::vec3(obj->getPosition().x, obj->getPosition().y, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f),
					glm::vec3(1.0f, 1.0f, 1.0f), 0.0f, false, 0, 0, 0.0f));
				dynamic_cast<Enemy*>(obj)->setWeapon(weapon);
				weapon->setHolder(obj);
				break;
			}
			case 10:
			{
				obj = gameObjInstCreate(GAMEOBJ_TYPE::TYPE_EXIT, glm::vec3(x + 0.5f, (MAP_HEIGHT - y) - 0.5f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f),
					glm::vec3(1.0f, 1.0f, 1.0f), 0.0f, false, 0, 0, 0.0f);
				obj->setCollision(true);
				break;
			}
			case 11:
			{
				obj = gameObjInstCreate(GAMEOBJ_TYPE::TYPE_ENEMY_MAGE, glm::vec3(x + 0.5f, (MAP_HEIGHT - y) - 0.5f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f),
					glm::vec3(3.0f, 3.0f, 1.0f), 0.0f, true, 2, 0, 0.5f);
				weapon = dynamic_cast<Weapon*>(gameObjInstCreate(GAMEOBJ_TYPE::TYPE_WEAPON_FIRE_WAND, glm::vec3(obj->getPosition().x, obj->getPosition().y, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f),
					glm::vec3(3.0f, 3.0f, 1.0f), 0.0f, false, 0, 0, 0.0f));
				dynamic_cast<Enemy*>(obj)->setWeapon(weapon);
				dynamic_cast<Enemy*>(obj)->setMaxHP(150.0f);
				weapon->setHolder(obj);
				weapon->setCooldown(0.3f);
				break;
			}
			default:
				break;
			}
		}
	}
	// BG
	obj = gameObjInstCreate(GAMEOBJ_TYPE::TYPE_BG, glm::vec3(20.0f, MAP_HEIGHT / 2.0f, -1.0f), glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(25.0f, 20.0f, 1.0f), 0.0f, false, 0, 0, 0.0f);

	obj = gameObjInstCreate(GAMEOBJ_TYPE::TYPE_UI_BAR, glm::vec3(sPlayer->getPosition().x + 1.0f, -0.7f, 1.0f), glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(20.0f, 1.4f, 1.0f), 0.0f, false, 0, 0, 0.0f);
	obj = gameObjInstCreate(GAMEOBJ_TYPE::TYPE_UI_HEALTH, glm::vec3(sPlayer->getPosition().x - 4.5f, -0.7f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(8.0f, 1.0f, 1.0f), 0.0f, false, 0, 0, 0.0f);
	obj = gameObjInstCreate(GAMEOBJ_TYPE::TYPE_UI_HEALTH_BAR, glm::vec3(sPlayer->getPosition().x - 2.0f, -0.7f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(5.0f, 0.5f, 1.0f), 0.0f, false, 0, 0, 0.0f);
	obj = gameObjInstCreate(GAMEOBJ_TYPE::TYPE_UI_TYPE_NORMAL, glm::vec3(sPlayer->getPosition().x + 5.0f, -0.7f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f),
		glm::vec3(10.0f, 1.25f, 1.0f), 0.0f, false, 0, 0, 0.0f);

	SetCamPosition((sPlayer->getPosition().x - (MAP_WIDTH / 2.0f) + 0.5f) * CELL_SIZE, -50.0f);

	sLimitLeft = TO_MAP_SPACE(0);
	sLimitRight = TO_MAP_SPACE(SCREEN_SIZE);

	// Initalize some data. ex. score and player life
	sRespawnCountdown = 0;
	CULL_ON = false;
	C_DOWN = false;

	// Sound
	SoundEngine = createIrrKlangDevice();
	BGM = SoundEngine->addSoundSourceFromFile("BGM_Journey_Begins.wav");		//loop or not
	BGM->setDefaultVolume(0.175f);
	fireSpell = SoundEngine->addSoundSourceFromFile("FireSpell_SFX.mp3");
	fireSpell->setDefaultVolume(0.3f);
	shootArrow = SoundEngine->addSoundSourceFromFile("ShootArrow_SFX.mp3");
	shootArrow->setDefaultVolume(0.5f);
	slimeShoot = SoundEngine->addSoundSourceFromFile("SlimeShoot_SFX.mp3");
	slimeShoot->setDefaultVolume(0.5f);
	swordAttack = SoundEngine->addSoundSourceFromFile("Sword_SFX.mp3");
	swordAttack->setDefaultVolume(0.5f);

	SoundEngine->play2D(BGM, true);


	printf("Level2: Init\n");
}


void GameStateLevel2Update(double dt, long frame, int& state)
{

	//-----------------------------------------
	// Get user input
	//-----------------------------------------
	if (sRespawnCountdown <= 0)
	{
		if ((glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) && (sPlayer->isJumping() == false))
		{
			sPlayer->setVelocityY(JUMP_VELOCITY);
			sPlayer->setJumping(true);
			sPlayer->setAnimation(false);
			SoundEngine->removeSoundSource("jump.mp3");
			SoundEngine->play2D("jump.mp3", false);
		}
		else if (sPlayer->getWeapon()->getType() == GAMEOBJ_TYPE::TYPE_WEAPON_FIRE_WAND && glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		{
			sPlayer->setFlying(true);
		}
		else
		{
			sPlayer->setFlying(false);
		}
		sPlayer->getInput();
		if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS && sPlayer->getCharacterState() != CHARACTER_STATE::STATE_ATTACK)
		{
			Weapon* weapon = dynamic_cast<Weapon*>(sPlayer->getWeapon());
			if (weapon != nullptr && weapon->getCurrentCooldown() <= 0.0f)
			{
				std::cout << "Left : " << sLimitLeft << " " << "Right : " << sLimitRight << "\n";
				double xPos, yPos;
				glfwGetCursorPos(window, &xPos, &yPos);
				xPos = SCREEN_TO_WORLD_SPACE_X(xPos, sLimitLeft);
				yPos = -yPos + SCREEN_SIZE / 2.0f;
				if (weapon->getWeaponType() == WEAPON_TYPE::WEAPON_TYPE_RANGED)
				{
					weapon->setOrientation(atan2(yPos - MAP_TO_WORLD_SPACE_Y(weapon->getPosition().y), xPos - MAP_TO_WORLD_SPACE_X(weapon->getPosition().x)));

					// Set Holder Facing Direction when Shooting with Opposite side of player
					if (weapon->getOrientation() >= -90 * DegToRad && weapon->getOrientation() <= 90 * DegToRad)
					{
						weapon->getHolder()->setScaleX(1.0f);
					}
					else
					{
						weapon->getHolder()->setScaleX(-1.0f);
					}
					float dir_x = MAP_TO_WORLD_SPACE_X(weapon->getPosition().x) - xPos;
					float dir_y = yPos - MAP_TO_WORLD_SPACE_Y(weapon->getPosition().y);

					float totalVec = pow(pow(dir_x, 2) + pow(dir_y, 2), 0.5f);
					dir_x /= totalVec;
					dir_y /= totalVec;
					GameObject* bullet = gameObjInstCreate(dynamic_cast<RangeWeapon*>(weapon)->getBulletType(), weapon->getPosition(), glm::vec3(dir_x * 20.0f * -1.0f, dir_y * 20.0f, 0.0f),
						glm::vec3(1.0f, 1.0f, 1.0f), 0.0f, false, 0, 0, 0);
					dynamic_cast<Bullet*>(bullet)->setShooter(sPlayer);
					dynamic_cast<Bullet*>(bullet)->setATK(weapon->getATK());

					if (weapon->getType() == GAMEOBJ_TYPE::TYPE_WEAPON_BOW)
					{
						SoundEngine->play2D(shootArrow, false);
					}
					else if (weapon->getType() == GAMEOBJ_TYPE::TYPE_WEAPON_SLIME)
					{
						SoundEngine->play2D(slimeShoot, false);
					}
				}
				else if (weapon->getWeaponType() == WEAPON_TYPE::WEAPON_TYPE_SPELL)
				{
					float angle = -atan2(yPos - MAP_TO_WORLD_SPACE_Y(weapon->getPosition().y), xPos - MAP_TO_WORLD_SPACE_X(weapon->getPosition().x));

					// Set Holder Facing Direction when Shooting with Opposite side of player
					if (angle >= -90 * DegToRad && angle <= 90 * DegToRad)
					{
						weapon->getHolder()->setScaleX(1.0f);
					}
					else
					{
						weapon->getHolder()->setScaleX(-1.0f);
					}
					float dir_x = MAP_TO_WORLD_SPACE_X(weapon->getPosition().x) - xPos;
					float dir_y = yPos - MAP_TO_WORLD_SPACE_Y(weapon->getPosition().y);

					float totalVec = pow(pow(dir_x, 2) + pow(dir_y, 2), 0.5f);
					dir_x /= totalVec;
					dir_y /= totalVec;

					GameObject* bullet = gameObjInstCreate(dynamic_cast<SpellWeapon*>(weapon)->getBulletType(), weapon->getPosition(), glm::vec3(dir_x * 10.0f * -1.0f, dir_y * 10.0f, 0.0f),
						glm::vec3(0.7f, 0.7f, 1.0f), 0.0f, false, 0, 0, 0);
					dynamic_cast<Bullet*>(bullet)->setShooter(sPlayer);
					dynamic_cast<Bullet*>(bullet)->setATK(weapon->getATK());
					SoundEngine->play2D(fireSpell, false);
				}
				else
				{
					sPlayer->getWeapon()->setAnimation(true);
					sPlayer->getWeapon()->setCollision(true);
					SoundEngine->play2D(swordAttack, false);
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
				if (collide && dynamic_cast<Weapon*>(pInst) != nullptr && dynamic_cast<Weapon*>(pInst)->getHolder() == nullptr)
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

					Weapon* newWeapon = dynamic_cast<Weapon*>(pInst);
					sPlayer->setWeapon(newWeapon);
					newWeapon->setHolder(sPlayer);
					newWeapon->setCollision(false);
					if (newWeapon->getType() != GAMEOBJ_TYPE::TYPE_WEAPON_SWORD)
					{
						newWeapon->setAnimation(false);
						newWeapon->setOffsetX(0.0f);
					}
					else
					{
						newWeapon->setOffsetX(0.0f);
					}
					break;
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

	//-----------------------------------------
	// Debug (Cam zoom UI)
	//-----------------------------------------
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
		GameObject*& pInst = sGameObjInstArray[i];
		// skip inactive object
		if (pInst == nullptr || pInst->getFlag() == FLAG_INACTIVE)
		{
			continue;
		}
		// If Objects are out of range [sLimitLeft, sLimitRight] -> set cull to true
		if (pInst->getPosition().x < sLimitLeft - 2.0f || pInst->getPosition().x > sLimitRight + 2.0f)
		{
			if (!(pInst->getType() >= GAMEOBJ_TYPE::TYPE_UI || pInst->getType() == GAMEOBJ_TYPE::TYPE_BG))
			{
				pInst->setCulling(true);
			}
		}
		else
		{
			pInst->setCulling(false);
		}

		if (pInst->getType() == TYPE_PLAYER)
		{

			// Apply gravity: Velocity Y = Gravity * Frame Time + Velocity Y
			if (sPlayer->isJumping())
			{
				if (sPlayer->isFlying() == true)
				{
					sPlayer->increaseVelocityY(GRAVITY * dt / 2.0f);
				}
				else
				{
					sPlayer->increaseVelocityY(GRAVITY * dt);
				}
			}
			if (sPlayer->getCurrentHitCooldown() > 0)
			{
				sPlayer->decreaseHitCooldown(dt);
				if (sPlayer->getCurrentHitCooldown() < 0)
				{
					sPlayer->setCurrentHitCooldown(0.0f);
				}
			}

			// Update position using Velocity
			pInst->updatePosition(pInst->getVelocity() * glm::vec3(dt, dt, 0.0f));

			/* Camera Scrolling */
			// If Player reach limit on Left side -> No need to Scrolling Camera -> set to static position
			if (MAP_TO_WORLD_SPACE_X(pInst->getPosition().x + 1) < MAP_TO_WORLD_SPACE_X(1.0f) + SCREEN_SIZE / 2.0f)
			{
				SetCamPosition(MAP_TO_WORLD_SPACE_X(1.0f) + SCREEN_SIZE / 2.0f, -50.0f);
			}
			// If Player reach limit on right side -> No need to Scrolling Camera -> set to static position
			else if (MAP_TO_WORLD_SPACE_X(pInst->getPosition().x + 1) > MAP_TO_WORLD_SPACE_X(MAP_WIDTH - 1) - SCREEN_SIZE / 2.0f)
			{
				SetCamPosition(MAP_TO_WORLD_SPACE_X(MAP_WIDTH - 1) - SCREEN_SIZE / 2.0f, -50.0f);
			}
			else
			{
				// Camera tracking Player position
				SetCamPosition(MAP_TO_WORLD_SPACE_X(pInst->getPosition().x + 1.0f), -50.0f);

				// Update LimitLeft and LimitRight
				sLimitLeft = pInst->getPosition().x - TO_MAP_SPACE(SCREEN_SIZE / 2.0f) + 1.0f;
				sLimitRight = pInst->getPosition().x + TO_MAP_SPACE(SCREEN_SIZE / 2.0f) + 1.5f;
			}
		}
		else if (CheckType(pInst->getType()) == TYPE_ENEMY)
		{
			Enemy* enemy = dynamic_cast<Enemy*>(pInst);

			if (enemy->getCurrentHP() <= 0)
			{
				enemy->getWeapon()->setHolder(nullptr);
				SoundEngine->play2D("kill.mp3", false);
				gameObjInstDestroy(pInst);
				continue;
			}

			if (enemy->getCurrentHitCooldown() > 0)
			{
				enemy->decreaseHitCooldown(dt);
				if (enemy->getCurrentHitCooldown() < 0)
				{
					enemy->setCurrentHitCooldown(0.0f);
				}
			}

			enemy->UpdateEnemyState(sMapCollisionData, dt);

			// Check Raycast -> Attack Player
			if (enemy->isCulling() == false && (enemy->getType() == TYPE_ENEMY_ARCHER || enemy->getType() == TYPE_ENEMY_MAGE) && checkRaycast(enemy, sPlayer))
			{
				if (enemy->getWeapon()->getCurrentCooldown() <= 0.0f)
				{
					float dir_x = sPlayer->getPosition().x - enemy->getPosition().x;
					float dir_y = sPlayer->getPosition().y - enemy->getPosition().y;

					float totalVec = pow(pow(dir_x, 2) + pow(dir_y, 2), 0.5f);
					dir_x /= totalVec;
					dir_y /= totalVec;

					GameObject* bullet;
					Weapon* weapon = enemy->getWeapon();
					if (pInst->getType() == TYPE_ENEMY_ARCHER)
					{
						weapon->setOrientation(atan2(sPlayer->getPosition().y - enemy->getPosition().y, sPlayer->getPosition().x - enemy->getPosition().x));
						bullet = gameObjInstCreate(dynamic_cast<RangeWeapon*>(weapon)->getBulletType(), weapon->getPosition(), glm::vec3(dir_x * 20.0f, totalVec * (dir_y < 0 ? -1.0f : 1.0f), 0.0f),
							glm::vec3(1.0f, 1.0f, 1.0f), 0.0f, false, 0, 0, 0);
						dynamic_cast<Bullet*>(bullet)->setATK(weapon->getATK());
						SoundEngine->play2D(shootArrow, false);
					}
					else
					{
						bullet = gameObjInstCreate(dynamic_cast<SpellWeapon*>(weapon)->getBulletType(), weapon->getPosition(), glm::vec3(dir_x * 10.0f, dir_y * 10.0f, 0.0f),
							glm::vec3(0.7f, 0.7f, 1.0f), 0.0f, false, 0, 0, 0);
						dynamic_cast<Bullet*>(bullet)->setATK(weapon->getATK());
						SoundEngine->play2D(fireSpell, false);
					}
					dynamic_cast<Bullet*>(bullet)->setShooter(pInst);
					weapon->Attack();
				}
				enemy->setVelocityX(0.0f);
			}

			if (pInst->getVelocity().x < 0 && pInst->getScale().x > 0)
			{
				pInst->setScaleX(pInst->getScale().x * -1);
			}
			else if (pInst->getVelocity().x > 0 && pInst->getScale().x < 0)
			{
				pInst->setScaleX(pInst->getScale().x * -1);
			}

			// Update enemy position
			pInst->updatePosition(pInst->getVelocity() * glm::vec3(dt, dt, 0.0f));
		}
		// If pInst is weapon
		else if (dynamic_cast<Weapon*>(pInst) != nullptr)
		{
			Weapon* weapon = dynamic_cast<Weapon*>(pInst);
			if (weapon->getHolder() == nullptr)
			{
				if (weapon->getType() != GAMEOBJ_TYPE::TYPE_WEAPON_SWORD)
				{
					weapon->setOffsetX(0.5f);
				}
				else
				{
					weapon->setOffsetX(0.75f);
				}
				weapon->setOrientation(0);
				weapon->setCollision(true);
				continue;
			}
			weapon->updateWeaponDirection();
			weapon->updateWeaponCooldown(dt);
		}
		else if (pInst->getType() >= GAMEOBJ_TYPE::TYPE_WEAPON_BULLET && pInst->getType() < GAMEOBJ_TYPE::TYPE_UI)
		{
			if (pInst->getType() == GAMEOBJ_TYPE::TYPE_WEAPON_BOW_ARROW || pInst->getType() == GAMEOBJ_TYPE::TYPE_WEAPON_SLIME_BALL)
			{
				pInst->increaseVelocityY(GRAVITY * dt);
			}
			// Update enemy position
			pInst->updatePosition(pInst->getVelocity() * glm::vec3(dt, dt, 0.0f));
		}
		else if (pInst->getType() == GAMEOBJ_TYPE::TYPE_BG)
		{
			if ((MAP_TO_WORLD_SPACE_X(sPlayer->getPosition().x + 1) < MAP_TO_WORLD_SPACE_X(1.0f) + SCREEN_SIZE / 2.0f
				|| MAP_TO_WORLD_SPACE_X(sPlayer->getPosition().x + 1) > MAP_TO_WORLD_SPACE_X(MAP_WIDTH - 1) - SCREEN_SIZE / 2.0f))
			{
				continue;
			}

			int collisionFlag = GameState::CheckCharacterMapCollision(sMapCollisionData, sPlayer->getPosition().x, sPlayer->getPosition().y, sPlayer->getScale().x, sPlayer->getScale().y);

			if (!(collisionFlag & COLLISION_RIGHT || collisionFlag & COLLISION_LEFT) && sPlayer->getVelocity().x > 0.0f)
			{
				pInst->setPositionX(pInst->getPosition().x + dt);
				if (sPlayer->getPosition().x > pInst->getPosition().x + pInst->getScale().x / 2.0f)
				{
					pInst->setPositionX(sPlayer->getPosition().x + pInst->getScale().x / 2.0f);
				}
			}
			else if (!(collisionFlag & COLLISION_RIGHT || collisionFlag & COLLISION_LEFT) && sPlayer->getVelocity().x < 0.0f)
			{
				pInst->setPositionX(pInst->getPosition().x - dt);
				if (sPlayer->getPosition().x < pInst->getPosition().x - pInst->getScale().x / 2.0f)
				{
					pInst->setPositionX(sPlayer->getPosition().x - pInst->getScale().x / 2.0f);
				}
			}
		}
		else if (pInst->getType() == GAMEOBJ_TYPE::TYPE_UI_BAR)
		{
			if (!(MAP_TO_WORLD_SPACE_X(sPlayer->getPosition().x + 1) < MAP_TO_WORLD_SPACE_X(1.0f) + SCREEN_SIZE / 2.0f
				|| MAP_TO_WORLD_SPACE_X(sPlayer->getPosition().x + 1) > MAP_TO_WORLD_SPACE_X(MAP_WIDTH - 1) - SCREEN_SIZE / 2.0f))
			{
				pInst->setPositionX(sPlayer->getPosition().x + 1.0f);
			}
		}
		else if (pInst->getType() == GAMEOBJ_TYPE::TYPE_UI_HEALTH)
		{
			if (!(MAP_TO_WORLD_SPACE_X(sPlayer->getPosition().x + 1) < MAP_TO_WORLD_SPACE_X(1.0f) + SCREEN_SIZE / 2.0f
				|| MAP_TO_WORLD_SPACE_X(sPlayer->getPosition().x + 1) > MAP_TO_WORLD_SPACE_X(MAP_WIDTH - 1) - SCREEN_SIZE / 2.0f))
			{
				pInst->setPositionX(sPlayer->getPosition().x - 4.5f);
			}
		}
		else if (pInst->getType() == GAMEOBJ_TYPE::TYPE_UI_HEALTH_BAR)
		{
			if (sPlayer->getCurrentHP() <= 0)
			{
				pInst->setScaleX(0.0f);
			}
			else
			{
				pInst->setScaleX((sPlayer->getCurrentHP() / sPlayer->getMaxHP()) * 5.0f);
			}
			if (MAP_TO_WORLD_SPACE_X(sPlayer->getPosition().x + 1) < MAP_TO_WORLD_SPACE_X(1.0f) + SCREEN_SIZE / 2.0f)
			{
				pInst->setPositionX((sLimitRight - sLimitLeft) / 2.0f - 4.0f - (1.0f - (sPlayer->getCurrentHP() / sPlayer->getMaxHP())) * 2.5f);
			}
			else if (MAP_TO_WORLD_SPACE_X(sPlayer->getPosition().x + 1) > MAP_TO_WORLD_SPACE_X(MAP_WIDTH - 1) - SCREEN_SIZE / 2.0f)
			{
				pInst->setPositionX((sLimitRight + sLimitLeft) / 2.0f - 4.5f - (1.0f - (sPlayer->getCurrentHP() / sPlayer->getMaxHP())) * 2.5f);
			}
			else
			{
				pInst->setPositionX(sPlayer->getPosition().x - 4.0f - (1.0f - (sPlayer->getCurrentHP() / sPlayer->getMaxHP())) * 2.5f);
			}
		}
		else if (pInst->getType() >= GAMEOBJ_TYPE::TYPE_UI_TYPE)
		{
			if (sPlayer->getWeapon()->getType() == GAMEOBJ_TYPE::TYPE_WEAPON_SWORD && pInst->getType() != GAMEOBJ_TYPE::TYPE_UI_TYPE_WARRIOR)
			{
				pInst->setTexture(sTexArray + GAMEOBJ_TYPE::TYPE_UI_TYPE_WARRIOR);
				pInst->setType(GAMEOBJ_TYPE::TYPE_UI_TYPE_WARRIOR);
			}
			else if (sPlayer->getWeapon()->getType() == GAMEOBJ_TYPE::TYPE_WEAPON_BOW && pInst->getType() != GAMEOBJ_TYPE::TYPE_UI_TYPE_ARCHER)
			{
				pInst->setTexture(sTexArray + GAMEOBJ_TYPE::TYPE_UI_TYPE_ARCHER);
				pInst->setType(GAMEOBJ_TYPE::TYPE_UI_TYPE_ARCHER);
			}
			else if (sPlayer->getWeapon()->getType() == GAMEOBJ_TYPE::TYPE_WEAPON_FIRE_WAND && pInst->getType() != GAMEOBJ_TYPE::TYPE_UI_TYPE_MAGE)
			{
				pInst->setTexture(sTexArray + GAMEOBJ_TYPE::TYPE_UI_TYPE_MAGE);
				pInst->setType(GAMEOBJ_TYPE::TYPE_UI_TYPE_MAGE);
			}

			if (!(MAP_TO_WORLD_SPACE_X(sPlayer->getPosition().x + 1) < MAP_TO_WORLD_SPACE_X(1.0f) + SCREEN_SIZE / 2.0f
				|| MAP_TO_WORLD_SPACE_X(sPlayer->getPosition().x + 1) > MAP_TO_WORLD_SPACE_X(MAP_WIDTH - 1) - SCREEN_SIZE / 2.0f))
			{
				pInst->setPositionX(sPlayer->getPosition().x + 5.0f);
			}
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
					if (pInst->getType() == GAMEOBJ_TYPE::TYPE_PLAYER)
					{
						pInst->setCurrentFrame(1);
					}
					else if (pInst->getType() == GAMEOBJ_TYPE::TYPE_WEAPON_SWORD)
					{
						pInst->setAnimation(false);
						pInst->setCollision(false);
						pInst->setCurrentFrame(0);
					}
					else
					{
						pInst->setCurrentFrame(0);
					}
				}

				//+ use currFrame infomation to set pInst->offsetX
				pInst->setOffsetX(pInst->getCurrentFrame() * pInst->getOffset());
			}
		}
	}

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
			int collisionFlag = GameState::CheckCharacterMapCollision(sMapCollisionData, sPlayer->getPosition().x, sPlayer->getPosition().y, sPlayer->getScale().x, sPlayer->getScale().y);

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
		else if (pInst->getType() >= GAMEOBJ_TYPE::TYPE_WEAPON_BULLET && pInst->getType() < GAMEOBJ_TYPE::TYPE_UI)
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
	//-----------------------------------------
	for (int i = 0; i < GAME_OBJ_INST_MAX; i++) {
		GameObject*& pInst1 = sGameObjInstArray[i];

		// skip inactive object
		if (pInst1 == nullptr || pInst1->getFlag() == FLAG_INACTIVE || pInst1->hasCollision() == false)
		{
			continue;
		}
		if (CheckType(pInst1->getType()) == TYPE_ENEMY && dynamic_cast<Character*>(pInst1)->getCharacterState() == CHARACTER_STATE::STATE_DIE)
		{
			continue;
		}

		for (int j = 0; j < GAME_OBJ_INST_MAX; j++)
		{
			GameObject*& pInst2 = sGameObjInstArray[j];

			// skip inactive object
			if (pInst2 == nullptr || pInst2->getFlag() == FLAG_INACTIVE)
			{
				continue;
			}
			if (CheckType(pInst2->getType()) == TYPE_ENEMY && dynamic_cast<Character*>(pInst2)->getCharacterState() == CHARACTER_STATE::STATE_DIE)
			{
				continue;
			}

			bool collide = GameState::isCollide(pInst1, pInst2, COLLISION_OFFSET, COLLISION_OFFSET);

			//+ Player vs Enemy
			//	- if the Player die, set the sRespawnCountdown > 0	
			if (pInst1->getType() == TYPE_PLAYER)
			{
				if (pInst2->getType() == TYPE_EXIT && collide)
				{
					state = 3;
					return;
				}
				else if (sPlayer->getCurrentHitCooldown() <= 0 && CheckType(pInst2->getType()) == TYPE_ENEMY && collide)
				{
					// If hit directly to Enemy HP -20
					sPlayer->decreaseHP(20.0f);
					sPlayer->setCurrentHitCooldown(sPlayer->getHitCooldown());
					sPlayer->setHurt(true);
					std::cout << "GOT HIT\n";
				}
				else if (dynamic_cast<Bullet*>(pInst2) != nullptr && dynamic_cast<Bullet*>(pInst2)->getShooter() != sPlayer && collide)
				{
					sPlayer->decreaseHP(dynamic_cast<Bullet*>(pInst2)->getATK());
					gameObjInstDestroy(pInst2);
					sPlayer->setHurt(HURTING_FRAME);
					std::cout << "GOT HIT\n";
				}
				else if (GameState::willCollide(pInst1, pInst2, COLLISION_OFFSET, COLLISION_OFFSET) && pInst2->getType() == TYPE_WEAPON_SWORD && dynamic_cast<Weapon*>(pInst2)->getHolder() != nullptr && dynamic_cast<Weapon*>(pInst2)->getCurrentCooldown() <= 0.0f)
				{
					Weapon* weapon = dynamic_cast<Weapon*>(pInst2);
					if (weapon->getHolder() == sPlayer)
					{
						continue;
					}
					weapon->setAnimation(true);
					weapon->setCollision(true);
					weapon->Attack();
					SoundEngine->play2D(swordAttack, false);
				}
				else if (sPlayer->getCurrentHitCooldown() <= 0 && pInst2->getType() == TYPE_WEAPON_SWORD && dynamic_cast<Weapon*>(pInst2)->getHolder() != nullptr && pInst2 != sPlayer->getWeapon() && collide)
				{
					sPlayer->decreaseHP(dynamic_cast<MeleeWeapon*>(pInst2)->getATK());
					sPlayer->setCurrentHitCooldown(sPlayer->getHitCooldown());
					sPlayer->setHurt(HURTING_FRAME);
					std::cout << "GOT HIT\n";
				}

				if (sPlayer->getCurrentHP() <= 0.0f && sRespawnCountdown == 0)
				{
					// Set velocity of player to 0 don't move & Don't play animation
					pInst1->setVelocity(glm::vec3(0.0f, 0.0f, 0.0f));
					pInst1->setAnimation(false);
					// Start Counting RespawnFrame
					sRespawnCountdown = 1;
					// Stop all sound & Play die sound
					SoundEngine->stopAllSounds();
					SoundEngine->play2D("die.mp3");
				}
			}
			else if (pInst1->getType() == TYPE_WEAPON_SWORD && pInst1 == sPlayer->getWeapon() && CheckType(pInst2->getType()) == TYPE_ENEMY && dynamic_cast<Character*>(pInst2)->getCurrentHitCooldown() <= 0 && collide)
			{
				Character* enemy = dynamic_cast<Character*>(pInst2);
				enemy->decreaseHP(dynamic_cast<MeleeWeapon*>(pInst1)->getATK());
				enemy->setHurt(HURTING_FRAME);
				enemy->setCurrentHitCooldown(enemy->getHitCooldown());
			}
			else if (CheckType(pInst1->getType()) == TYPE_ENEMY && dynamic_cast<Bullet*>(pInst2) != nullptr && dynamic_cast<Bullet*>(pInst2)->getShooter() == sPlayer && collide)
			{
				Character* enemy = dynamic_cast<Character*>(pInst1);
				enemy->decreaseHP(dynamic_cast<Bullet*>(pInst2)->getATK());
				enemy->setHurt(HURTING_FRAME);
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

void GameStateLevel2Draw(void) {

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
			// If Current MapData[x][y] was not between range [sLimitLeft,sLimitRight] -> No need to render
			if (x < sLimitLeft || x > sLimitRight)
			{
				continue;
			}

			//+ Only draw background cell
			if (sMapData[y][x] >= 1 && sMapData[y][x] <= 5)
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
		if (pInst == nullptr || pInst->getFlag() == FLAG_INACTIVE || pInst->isCulling() == true)
		{
			continue;
		}

		// Transform cell from map space [0,MAP_SIZE] to screen space [-width/2,width/2]
		matTransform = sMapMatrix * pInst->getModelMatrix();

		SetRenderMode(CDT_TEXTURE, 1.0f);
		SetTexture(*pInst->getTexture(), pInst->getOffsetX(), pInst->getOffsetY(), pInst->isHurt());
		SetTransform(matTransform);
		DrawMesh(*pInst->getMesh());

		if (pInst->isHurt() > 0)
		{
			pInst->setHurt(pInst->isHurt() - 1);
			if (pInst->isHurt() == 0)
			{
				pInst->setHurt(false);
			}
		}
	}


	// Swap the buffer, to present the drawing
	glfwSwapBuffers(window);
}

void GameStateLevel2Free(void) {

	// call gameObjInstDestroy for all object instances in the sGameObjInstArray
	for (int i = 0; i < GAME_OBJ_INST_MAX; i++)
	{
		gameObjInstDestroy(sGameObjInstArray[i]);
	}

	// reset camera
	ResetCam();

	// Free sound
	SoundEngine->drop();

	printf("Level2: Free\n");
}

void GameStateLevel2Unload(void) {

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


	printf("Level2: Unload\n");
}