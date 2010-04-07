#ifndef CJ_STEERINGBEHAVIOR_HPP
#define CJ_STEERINGBEHAVIOR_HPP

#include <irrlicht.h>
#include <iostream>
#include <sys/time.h>

using namespace irr;
using namespace irr::core;
using namespace irr::scene;
using namespace irr::video;

namespace cj
{

class SteeringBehavior
{
   public:
      SteeringBehavior(const f32 maxrot, const f32 maxvel, const f32 maxacc);
      ~SteeringBehavior();
      void SetGoal(vector3df g);  //Move to this point
      vector3df GetRotation(void);  //Return absolute rotation
      vector3df GetStep(void);  //Return next vector difference (relative)
      void SetScale(f32 multiplier);  //Speed up time (or slow it down)
      void NextStep(matrix4 transform);  //Calculate the next directional step
      bool HasGoal(void);
      vector3df GetGoal(void);

   private:
      const f64 EPSILON;  //zero
      const f32 MAXROTATE;  //degrees per second
      const f32 MAXVELOCITY;  //game units per second
      const f32 MAXACCEL;  //game units per second^2
      bool done;  //goal acheived
      vector3df goal;  //target position
      vector3df step;  //next stop location
      vector3df rotate;  //next rotation amount
      vector3df position;  //current position
      vector3df velocity;  //current velocity
      vector3df heading;  //current heading directly forward
      timespec lastStep;  //time of last step
      timespec thisStep;  //time of this step
      f64 interval;  //time elapsed since last step
      f32 timescale;  //used to accelerate time

      f32 CalcRotation(vector3df target);  //Calculate absolute heading
      bool CalcVelocity(vector3df target, f32 r);  //Calculate velocity
};

}

#endif
