#ifndef CJ_AGENT_HPP
#define CJ_AGENT_HPP

#include <irrlicht.h>
#include <vector>
#include <list>
#include <iostream>
#include <sys/time.h>
#include "SteeringBehavior.hpp"
#include "AIBrain.hpp"
#include "Wall.hpp"

using namespace irr;
using namespace irr::core;
using namespace irr::scene;
using namespace irr::video;
using std::vector;

namespace cj
{

struct pointOfInterest
{   //Simplified struct for now, class later
    f32 heading;
    f32 range;
    IAnimatedMeshSceneNode* object;
};

class Agent
{
   public:
		static s32 genID();
		Agent(IrrlichtDevice* d, stringw mesh, stringw t, stringw h, vector3df p);
		Agent(IrrlichtDevice* d, stringw mesh, stringw t, stringw h, vector3df p, AIBrain b);
		virtual ~Agent();
		vector<f32> DrawFeelers(bool debug = false);  //wall collision detection
      vector<pointOfInterest> DrawCircle(const vector<Agent*>& agents, bool debug = false);  //shows circular range and agents in range
      void ClearCircle(const vector<Agent*>& agents);  //must be called to turn off DrawCircle
      vector<f32> DrawPieSlices(const vector<Agent*>& agents, bool debug = false);  //activation sectors
      void SetResolution(u32 r);  //change resolution
      void SetRange(u32 r);  //change range
      void SetAwareness(u32 a);  //change awareness
      vector<f32> MoveVector(vector3df distance);  //COLLISON MOVEMENT
		IAnimatedMeshSceneNode* getBody(void);  //return the scene node
		void Update(void);  //sense,think,act
		void Seek(vector3df goal, Wall *w, bool track = false);  //go to a point
		u32 GetFitness(vector3df goal, vector3df startpos);  //score for reaching goal
		void GameOver(void);
		AIBrain GetBrain(void);  //for baby making
		void Reset(vector3df p, AIBrain b);
		void SmartNavigate(void);

   private:
		static s32 nextAvailableID;
      IrrlichtDevice *device; //get driver and smgr from this
      IVideoDriver* driver; //for drawing
      ISceneManager* smgr; //needed for adding cubes
      ISceneCollisionManager* cmgr; //for collision detection
      IAnimatedMeshSceneNode* body;  //scene node
      IBillboardSceneNode * circle;  //node for circle
      u32 awareness;  //view range
      u32 resolution;  //density of sensors
      u32 range;  //range of sensors
      stringw path;  //directory of textures
      stringw texture;  //path+file of the body's texture
      SteeringBehavior wheel;  //for seek function
      std::list<vector3df> route;  //stack of subgoals
      AIBrain brain;  //neural network
      timespec gstart;  //start time of goal
      timespec gend;  //end time, found goal
      bool won;  //goal is reached
      bool remove;
      int stuck;

      vector<f32> Sense(void);  //look for walls
      void Think(vector<f32> feelers); //strategize for walls
      void Act(void);  //move to target
	};
}

#endif
