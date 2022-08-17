// Include standard headers
#include <stdio.h>
#include <stdlib.h>
#include <vector>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "system.h"
#include "CDT.h"
#include "GameStateLevel1.h"
#include "GameStateLevel2.h"
#include "GameStateLevel3.h"

// game state list
enum{ LEVEL1 = 0, LEVEL2, LEVEL3, RESTART, QUIT };

// variables to keep track the current, previous and next game state
unsigned int	gGameStateInit;
unsigned int	gGameStateCurr;
unsigned int	gGameStatePrev;
unsigned int	gGameStateNext;

// pointer to functions for game state levelX functions
void(*GameStateLoad)()						= 0;
void(*GameStateInit)()						= 0;
void(*GameStateUpdate)(double,long,int&)	= 0;
void(*GameStateDraw)()						= 0;
void(*GameStateFree)()						= 0;
void(*GameStateUnload)()					= 0;

// key manager, for one time pressing
bool Rdown = false;
bool Sdown = false;
bool Ndown = false;

// frame rate
double	frametime = 0;
long	framenumber = 0;

// windows
int		win_width = 800;
int		win_height = 850;


int main(void)
{

	// Initialize the System (GFW, GLEW, Input, Create window)
	SystemInit(win_width, win_height, "Silly Slime");
	CDTInit(win_width, win_height);

	// Initialize Game State (to level 1)
	gGameStateInit	= LEVEL1;
	gGameStateCurr	= gGameStateInit;
	gGameStatePrev	= gGameStateInit;
	gGameStateNext	= gGameStateInit;

	while(gGameStateCurr != QUIT){
		
		// Loading cases
		if (gGameStateCurr == RESTART){
			gGameStateCurr = gGameStatePrev;
			gGameStateNext = gGameStateCurr;
		}
		else if (gGameStateCurr == LEVEL3)
		{

			GameStateLoad = GameStateLevel3Load;
			GameStateInit = GameStateLevel3Init;
			GameStateUpdate = GameStateLevel3Update;
			GameStateDraw = GameStateLevel3Draw;
			GameStateFree = GameStateLevel3Free;
			GameStateUnload = GameStateLevel3Unload;
			GameStateLoad();
		}
		else if (gGameStateCurr == LEVEL2)
		{
			GameStateLoad	= GameStateLevel2Load;
			GameStateInit	= GameStateLevel2Init;
			GameStateUpdate = GameStateLevel2Update;
			GameStateDraw	= GameStateLevel2Draw;
			GameStateFree	= GameStateLevel2Free;
			GameStateUnload = GameStateLevel2Unload;
			GameStateLoad();
		}
		else{	//LEVEL1
			GameStateLoad	= GameStateLevel1Load;
			GameStateInit	= GameStateLevel1Init;
			GameStateUpdate = GameStateLevel1Update;
			GameStateDraw	= GameStateLevel1Draw;
			GameStateFree	= GameStateLevel1Free;
			GameStateUnload = GameStateLevel1Unload;
			GameStateLoad();
		}
		
		FrameInit();
		GameStateInit();
		framenumber = 0;
		

		while (gGameStateCurr == gGameStateNext){
			
			frametime = FrameStart();
			framenumber++;

			// read input
			glfwPollEvents();

			int state = 0;
			GameStateUpdate(frametime, framenumber, state);
			GameStateDraw();

			// Check return state from Update()
			if (state == 2){
				gGameStateNext = RESTART;
			}
			

			// Check if the ESC key was pressed or the window was closed
			if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS || glfwWindowShouldClose(window) == 1){
				gGameStateNext = QUIT;
			}

			// Check if User want to restart level
			if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS && !Rdown){
				gGameStateNext = RESTART;
				Rdown = true;
			}
			if (glfwGetKey(window, GLFW_KEY_R) == GLFW_RELEASE && Rdown){Rdown = false;}

			// Check if User want to change level
			if (state == 3)
			{
				if (gGameStateCurr == LEVEL1)
				{
					gGameStateNext = LEVEL2;
				}
				else if (gGameStateCurr == LEVEL2)
				{
					gGameStateNext = LEVEL3;
				}
				else
				{
					gGameStateNext = LEVEL1;
				}
			}

			FrameEnd();
		}

		GameStateFree();

		if (gGameStateNext != RESTART){
			GameStateUnload();
		}

		gGameStatePrev = gGameStateCurr;
		gGameStateCurr = gGameStateNext;
	} 


	// Do system clean up before quit
	CDTShutdown();
	SystemShutdown();
	
	return 0;
}

