#include "SteeringBehavior.hpp"

namespace cj
{

SteeringBehavior::SteeringBehavior(f32 ma, f32 mxf, f32 mxs) : EPSILON(0.001)
{
   velocity = vector3df(0,0,0);
   timescale = 1;
   mass = ma;
   max_force = mxf;
   max_speed = mxs;
   interval = 0.00;
}

SteeringBehavior::~SteeringBehavior()
{}

void SteeringBehavior::SetGoal(vector3df g)
{
   done = false;
   almost = false;
   goal = g;
   velocity = vector3df(0,0,0);
   cout << "SET GOAL!" << endl;
   //Timestamp of start time recorded as the time of the last step
   clock_gettime(CLOCK_REALTIME, &lastStep);
}

vector3df SteeringBehavior::GetRotation(void)
{
   return rotate;
}

vector3df SteeringBehavior::GetStep(void)
{
   if(almost)
   {  done = true;  }
   return step;
}

void SteeringBehavior::SetScale(f32 multiplier)
{
   timescale = multiplier;
}

void SteeringBehavior::NextStep(matrix4 transform)
{
   clock_gettime(CLOCK_REALTIME, &thisStep);
   //Figure time between steps
   interval = (thisStep.tv_sec - lastStep.tv_sec) + ((thisStep.tv_nsec - lastStep.tv_nsec)/1000000000.00);
   //Scale interval
   interval *= timescale;
   position = transform.getTranslation();
   //Heading is the rotation about the Y-Axis
   heading = transform.getRotationDegrees().Y;
   //distance vector to target
   vector3df target = goal - position;
   cout << "position Z: " << position.Z << " Y: " << position.Y << " X: " << position.X << endl;
   cout << "goal Z: " << goal.Z << " Y: " << goal.Y << " X: " << goal.X << endl;
   f64 magnitude = target.getLength();

   //if disance or velocity are non-zero
   if(magnitude > EPSILON)
   {
      //find the amount of rotation
      f32 r = CalcRotation(target) - heading;
      rotate = vector3df(0,r,0);
      //Calculate velocity

      //if(CalcVelocity(target, r))
      //{
         //Calculate distance covered
         //step = velocity * interval;
      //}
   }
   else
   {  //else, we are there
      step = goal - position;
      velocity = vector3df(0,0,0);
      rotate = vector3df(0,0,0);
      almost = true;
   }

   //record time stamp for next step
   lastStep = thisStep;
}

bool SteeringBehavior::HasGoal(void)
{
   return !done;
}

f32 SteeringBehavior::CalcRotation(vector3df target)
{
   cout << "target Z: " << target.Z << " Y: " << target.Y << " X: " << target.X << endl;
   f32 angle = atan2(target.Z, target.X);
   angle *= 180/PI;
   if(angle > 360 - EPSILON)
   {  angle = 0.00;  }
   cout << "Angle: " << angle << endl;
   return angle;
}

bool SteeringBehavior::CalcVelocity(vector3df target, f32 r)
{
   bool go = true;
   f32 distance = target.getLength();
   f32 velmag = velocity.getLength();
   //Scale acceleration according to how far off our heading is from the target
   r /= interval;
   //This favors rotation over increasing velocity
   //f32 maxacc = MAXACCEL - (MAXACCEL * r/MAXROTATE);

   //If we reach the goal on the next step
   if(distance + EPSILON < velmag * interval)
   {
      velocity = vector3df(0,0,0);
      step = goal - position;
      go = false;
   }
   else
   {  //Else, increase the velocity if it is not at max in the direction of the heading
      velocity = velocity * heading;
   }

   return go;
}

vector3df SteeringBehavior::GetGoal(void)
{
   return goal;
}

}
