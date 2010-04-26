#ifndef CJ_STEERINGBEHAVIOR_HPP
#define CJ_STEERINGBEHAVIOR_HPP

#include <irrlicht.h>
#include <iostream>
#include <sys/time.h>

using namespace irr;
using namespace irr::core;
using namespace irr::scene;
using namespace irr::video;
using namespace std;

namespace cj
{

class SteeringBehavior
{
   public:
      SteeringBehavior(f32 ma, f32 mxf, f32 mxs, f32 mxr);
      ~SteeringBehavior();
      void SetGoal(vector3df g);  //Move to this point
      vector3df GetRotation(void);  //Return absolute rotation
      vector3df GetStep(void);  //Return next vector difference (relative)
      void SetScale(f32 multiplier);  //Speed up time (or slow it down)
      void NextStep(vector3df p, f32 h);  //Calculate the next directional step
      bool HasGoal(void);
      vector3df GetGoal(void);

   private:
      const f64 EPSILON;  //zero
      vector3df velocity;  //current velocity
      f32 timescale;  //used to accelerate time
      f32 mass;
      f32 max_force;
      f32 max_speed;
      f32 max_rotate;
      f64 interval;  //time elapsed since last step
      bool done;  //goal acheived
      timespec lastStep;  //time of last step
      timespec thisStep;  //time of this step
      vector3df goal;  //target position
      vector3df step;  //next stop location
      vector3df rotate;  //next rotation amount
      vector3df position;  //current position
      f32 heading;
      bool almost;

      f32 CalcRotation(vector3df target);  //Calculate absolute heading
      bool CalcVelocity(vector3df target, f32 dh);  //Calculate velocity
};

}

#endif
