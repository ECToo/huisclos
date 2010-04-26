#include "SteeringBehavior.hpp"

namespace cj
{

SteeringBehavior::SteeringBehavior(f32 ma, f32 mxf, f32 mxs, f32 mxr) : EPSILON(0.0001)
{
   velocity = goal = step = rotate = position = vector3df(0,0,0);
   timescale = 1;
   mass = ma;
   max_force = mxf;
   max_speed = mxs;
   max_rotate = mxr;
   interval = heading = 0.00;
   done = true;
   almost = true;
   clock_gettime(CLOCK_REALTIME, &lastStep);
   clock_gettime(CLOCK_REALTIME, &thisStep);
}

SteeringBehavior::~SteeringBehavior()
{}

void SteeringBehavior::SetGoal(vector3df g)
{
   done = false;
   goal = g;
   almost = false;
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

void SteeringBehavior::NextStep(vector3df p, f32 h)
{
   clock_gettime(CLOCK_REALTIME, &thisStep);
   //Figure time between steps
   interval = (thisStep.tv_sec - lastStep.tv_sec) + ((thisStep.tv_nsec - lastStep.tv_nsec)/1000000000.00);
   //Scale interval
   interval *= timescale;
   position = p;
   //Heading is the rotation about the Y-Axis
   heading = h;
   //distance vector to target
   vector3df target = goal - position;
   f64 magnitude = target.getLength();

   //if disance or velocity are non-zero
   if(magnitude > EPSILON + max_speed * interval)
   {
      //find the amount of rotation
      f32 dr = CalcRotation(target);
      rotate = vector3df(0,dr,0);
      heading += dr;
      CalcVelocity(target, dr);
      step = velocity * interval;
   }
   else
   {  //else, we are there
      step = goal - position;
      almost = true;
      rotate = velocity = vector3df(0,0,0);
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
   f32 angle = atan2(target.Z, target.X);
   angle *= 180/PI;
   angle *= -1;
   angle -= heading;
   if(angle < -180)
   {  angle += 360;  }
   else if(angle > 180)
   {  angle -= 360;  }

   if(fabs(angle) / interval > max_rotate)
   {
      if(angle < 0)
      {  angle = -max_rotate * interval;  }
      else
      {  angle = max_rotate * interval;  }
   }

   return angle;
}

bool SteeringBehavior::CalcVelocity(vector3df target, f32 dh)
{
   bool go = true;
   vector3df dv = target.normalize() * max_speed - velocity;
   f32 force = mass * (dv.getLength()/interval);
   if(force > max_force)
   {  dv = target * ((max_force/mass) * interval);  }
   velocity += dv;
   return go;
}

vector3df SteeringBehavior::GetGoal(void)
{
   return goal;
}

}
