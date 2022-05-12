#pragma once

#include "GameState.h"
#include "Character.h"

//State machine states
enum ENEMY_STATE
{
	STATE_NONE,
	STATE_GOING_LEFT,
	STATE_GOING_RIGHT
};

//State machine inner states
enum ENEMY_INNER_STATE
{
	INNER_STATE_ON_ENTER,
	INNER_STATE_ON_UPDATE,
	INNER_STATE_ON_EXIT
};

class Enemy : public Character
{
	private:
		//state machine data
		enum ENEMY_STATE			state;
		enum ENEMY_INNER_STATE		innerState;
		double						counter;		// use in state machine

	public:
		Enemy();
		void UpdateEnemyState(int** sMapCollisionData, double dt);
};