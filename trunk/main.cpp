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

int main()
{
	Game game;

	game.device().setWindowCaption(L"Final: Demo camera");

	game.setGUIVisible(false);


	game.addWall( 2, 10, cj::absVec(40,25) );
	game.addWall( 10, 2, cj::absVec(0,-100) );

	Agent& faerie = game.addAgent();// default version
	game.setPC( faerie );

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


	game.start();

    return 0;
}
