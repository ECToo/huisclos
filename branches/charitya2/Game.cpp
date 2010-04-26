#include "Game.hpp"

namespace cj
{

GameException::GameException(const char *msg)
   : description(msg)
{}

GameException::~GameException()
{}

const char *GameException::Message(void) const
{  return description;  }

bool Game::exists = false;

Game::Game() : wall(NULL)
{
   if(exists)
   {  throw GameException("ERROR: Only one cj::Game can exist!");  }

   srand(time(0));
   exists = true;
   Init();
   Run();
}

Game::~Game()
{
   if(!exists)
   {  throw GameException("ERROR: No cj::Game to destruct!");  }

   for(u32 i = 0; i < agents.size(); i++)
   {
      if(agents[i] != NULL)
      {
         agents[i]->GameOver();
         delete agents[i];
      }
   }

   if(wall)
   {  delete wall;  }

   exists = false;
}

void Game::Init(void)
{
   device = createDevice(EDT_OPENGL, dimension2d<u32>(680, 420), 16);

   if (device == 0)
   {  throw GameException("ERROR: Device creation failed!");  }

   driver = device->getVideoDriver();
   smgr = device->getSceneManager();
   guienv = device->getGUIEnvironment();
   device->setWindowCaption(L"Assignment 2 Demo");
   smgr->addCameraSceneNode(0, vector3df(0,200,0), vector3df(0,0,0));
   wall = new Wall(device, "t351sml.jpg");
   wall->makeWall(1,20,vector3df(0,0,20));
   wall->makeWall(1,20,vector3df(20,0,-20));
   wall->makeWall(1,20,vector3df(70,0,50));
   wall->makeWall(10,1,vector3df(0,0,-20));
   wall->makeWall(5,1,vector3df(-40,0,-60));
   wall->makeWall(5,1,vector3df(-10,0,-80));
   wall->makeWall(1,20,vector3df(-70,0,50));;
   startvector = vector3df(40,0,0);
   population = 1;
   // mark the target with a red circle
   IBillboardSceneNode *circle = smgr->addBillboardSceneNode(0, dimension2df(10,10), vector3df(-50,0,-50));
   circle->setMaterialType(EMT_TRANSPARENT_ALPHA_CHANNEL);
   circle->setMaterialFlag(EMF_LIGHTING, false);
   circle->setMaterialTexture(0, driver->getTexture("circle.png"));

   //initial population
   for(u16 i = 0; i < population; i++)
   {
      agents.push_back(new Agent(device, "faerie.md2", "Faerie5.BMP", "", startvector));
   }

   agents[0]->Seek(vector3df(-50,0,-50), wall, true);
   //start time
   clock_gettime(CLOCK_REALTIME, &gstart);
}

void Game::Run(void)
{
   int lastFPS = 0;
   while(device->run() && driver)
   {
      driver->beginScene(true, true, SColor(255,120,102,136));
      guienv->drawAll();
      s32 fps = driver->getFPS();

      if (lastFPS != fps)
      {
         core::stringw str = L"Demo A* [";
         str += driver->getName();
         str += "] FPS:";
         str += fps;
         device->setWindowCaption(str.c_str());
         lastFPS = fps;
      }

      Tick();
      smgr->drawAll();
      driver->endScene();
   }

   device->drop();
}

void Game::Tick(void)
{
   timespec ticktime;
   clock_gettime(CLOCK_REALTIME, &ticktime);

   for(u32 i = 0; i < agents.size(); i++)
   {
      if(agents[i] != NULL)
      {  //updat all agents
         agents[i]->Update();
      }
   }
}

};
