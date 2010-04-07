#ifndef CJ_GAME_HPP
#define CJ_GAME_HPP

#include <fstream>
#include <sys/time.h>
#include <irrlicht.h>
#include "Wall.hpp"
#include "Agent.hpp"

using namespace std;
using namespace irr;
using namespace video;
using namespace scene;
using namespace gui;
using namespace core;

namespace cj
{

class GameException
{
   public:
      GameException(const char *msg);
      ~GameException();
      const char *Message(void) const;

   private:
      const char *description;
};

class Game
{
   public:
      Game();
      ~Game();

   private:
      static bool exists;  //enforce only one instance
      IrrlichtDevice* device;
      IVideoDriver* driver;
      ISceneManager* smgr;
      IGUIEnvironment* guienv;
      vector<Agent*> agents;
      timespec gstart;  //game start time
      u32 gen_gap;  //time a population is evaluated in seconds
      u32 population;  //population size
      u16 generation;  //generation counter
      vector3df startvector;  //agents start at this location
      vector<u32> totscores;  //total scores
      ofstream myfile;

      void Init(void);  // Initialize game
      void Run(void);  // Run game
      void Tick(void);  // process tick
      void NewGeneration(void);  //create new generation
      AIBrain GetParent(vector<u32> scores, u32 bestscore);  //determine parent
      void CrossOver(AIBrain *mom, AIBrain *dad);  //determine crossover
};

}

#endif
