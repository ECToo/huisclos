#include <iostream>
#include <sstream>
#include <string>
#include "irrlicht.h"

#include "Game.hpp"
#include "Agent.hpp"
#include "Wall.hpp"
#include "Coordinates.hpp"
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


	Wall wall = game.addWall();
	wall.makeWall(1,20,vector3df(0,0,20));
	wall.makeWall(1,20,vector3df(70,0,50));
	wall.makeWall(10,1,vector3df(0,0,-20));
	wall.makeWall(1,20,vector3df(-75,0,50));

	Agent& pc = game.addAgent( vector3df(0,0,90) );
	game.setPC( pc );

	Agent& faerie = game.addAgent( vector3df(0,0,100) );

	//faerie.Goto( vector3df(30,0,0), 30 );
	faerie.Seek(vector3df(-50,0,-50), 10, wall, true);
      //agents[0]->Seek(vector3df(-50,0,-50), wall, true);

	//game.setViewMode( Game::FIRST_PERSON );

	//Agent& faerie = game.addAgent( game.smgr().getMesh(DEFAULT_MESH.c_str()), game.driver().getTexture(DEFAULT_TEXTURE.c_str()), vector3df(), vector3df(0.0f,0.0f,0.0f), vector3df(1.0f, 1.0f, 1.0f) );
	//// Make controllable:
	//cj::Wall wall( &game.device(), "t351sml.jpg");
	//wall.makeWall(2,10,vector3df(40,0,25));
	//wall.makeWall(10,2,vector3df(0,0,-60));

	//game.setDebugMode( Debug::RANGEFINDER );
	//game.setDebugMode( Debug::RADAR );

    //IMeshSceneNode* wall = game.smgr().addCubeSceneNode(5);
    //wall->setPosition(vector3df(10,0,0));
    //wall->setMaterialTexture(0, game.driver().getTexture("t351sml.jpg"));
    //wall->setMaterialFlag(EMF_LIGHTING, false);

#ifndef SWIG
	game.start();
#endif

//#ifdef SWIG
	//delete pGame;
//#endif
    return 0;
}
