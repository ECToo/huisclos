#include <iostream>
#include <sstream>
#include <string>
#include "irrlicht.h"

#include "Game.hpp"
#include "Agent.hpp"
#include "Wall.hpp"
#include "Coordinates.hpp"
#include "Fairy.hpp"
using namespace cj;

using namespace std;
using namespace irr;
using namespace irr::video;
using namespace irr::scene;
using namespace irr::gui;
using namespace irr::core;

#ifdef SWIG
Game* pGame;
#define main() main_impl()
//#define Game pGame = new Game;
//#define game (*pGame)

#endif

int main()
{
#ifndef SWIG
	Game game;
#else
	pGame = new Game;
	Game& game = *pGame;
#endif

	game.device().setWindowCaption(L"Final: Demo camera");

	game.setGUIVisible(false);


	Wall& wall = game.addWall();
	wall.makeWall(1,20,vector3df(0,0,20));
   wall.makeWall(1,20,vector3df(20,0,-20));
   wall.makeWall(1,20,vector3df(70,0,50));
   wall.makeWall(10,1,vector3df(0,0,-20));
   wall.makeWall(5,1,vector3df(-40,0,-60));
   wall.makeWall(5,1,vector3df(-10,0,-80));
   wall.makeWall(1,20,vector3df(-70,0,50));

   	// <Config>
   	Fairy::DEFAULT_SPEED = 400;
   	Fairy::DEFAULT_ATTACK_MULTIPLIER = 10;

	Agent& pc = game.addAgent( vector3df(0,0,90) );
	game.setPC( pc );

	Agent& faerie = game.addAgent( vector3df(0,0,100) );
	//faerie.MoveTo( vector3df(60,0,90), 50 );
	//faerie.Seek(vector3df(-50,0,-50), 100);
	//faerie.setState( faerie.Wander );
	//faerie.setState( faerie.Dead );
	faerie.setState( faerie.Fight );

	Agent& f2 = game.addAgent( vector3df(0,0,100) );
		f2.setState( f2.Wander );
	Agent& f3 = game.addAgent( vector3df(-100,0,-100) );
		f3.setState( f3.Wander );
		//game.addAgent( vector3df(60,0,100) ).setState( Agent::ATTACK );
#ifndef SWIG
	game.start();
#endif

//#ifdef SWIG
	//delete pGame;
//#endif
    return 0;
}
