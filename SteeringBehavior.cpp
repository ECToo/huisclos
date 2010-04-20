#include "SteeringBehavior.hpp"

namespace cj
{

SteeringBehavior::SteeringBehavior(const f32 maxrot, const f32 maxvel, const f32 maxacc)
   : EPSILON(0.00001), MAXROTATE(maxrot), MAXVELOCITY(maxvel), MAXACCEL(maxacc), done(true),
   position(0,0,0), velocity(0,0,0), timescale(1)
{}

SteeringBehavior::~SteeringBehavior()
{}

void SteeringBehavior::SetGoal(vector3df g)
{
   done = false;
   goal = g;
   step = position = rotate = heading = velocity = vector3df(0,0,0);
   //Timestamp of start time recorded as the time of the last step
   clock_gettime(CLOCK_REALTIME, &lastStep);
}

vector3df SteeringBehavior::GetRotation(void)
{
   return rotate;
}

vector3df SteeringBehavior::GetStep(void)
{
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
   //Heading is one unit directly in front of position and rotated
   heading = vector3df(1,0,0);
   transform.rotateVect(heading);
   //distance vector to target
   vector3df target = goal - position;
   f64 magnitude = target.getLength();

   //if disance or velocity are non-zero
   if(magnitude > EPSILON)
   {
      //find the amount of rotation
      f32 r = CalcRotation(target);
      heading.rotateXZBy(r, vector3df(0,0,0));
      rotate = vector3df(0,r,0);
      //Calculate velocity

      if(CalcVelocity(target, r))
      {
         //Calculate distance covered
         step = velocity * interval;
      }
   }
   else
   {  //else, we are there
      step = goal - position;
      velocity = vector3df(0,0,0);
      rotate = vector3df(0,0,0);
      done = true;
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
   //Normalize vector to target
   vector3df normtarget = target;
   normtarget.normalize();
   //Use the cross product to get the direction of the angle.
   vector3df cp = heading.crossProduct(normtarget);
   //Calculate the angle to turn
   f32 angle = heading.dotProduct(normtarget);
   //For x very close to 1, acos(x) = nan; maybe because of floating point errors where x is 1.0000000000001
   //In reality, x should never be greater than 1. If it is close to 1, make angle 0.
   if(fabs(angle - 1) < EPSILON)
   {  angle = 0;  }
   else
   {  angle = acos(angle);  }
   //For some reason, Irrlicht actually uses degrees, not rads
   angle *= (180/PI);
   //Don't exceed maximum rotation amount
   if(angle > MAXROTATE * interval)
   {  angle = MAXROTATE * interval;  }
   //Adjust direction of turn to the smaller turn
   if(cp.Y < 0)
   {  angle *= -1;  }
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
   f32 maxacc = MAXACCEL;
   if(r > EPSILON)
   {  maxacc = 0;  }
   else if(target.getLength() + velocity.getLength()*interval < EPSILON)
   {  maxacc = -MAXACCEL;  }

   //If we reach the goal on the next step
   if(distance + EPSILON < velmag * interval + MAXACCEL * interval)
   {
      rotate = velocity = vector3df(0,0,0);
      step = goal - position;
      go = false;
   }
   else
   {  //Else, increase the velocity if it is not at max in the direction of the heading
      velocity = velocity.getLength() * heading + heading * maxacc * interval;
      if(velocity.getLength() > MAXVELOCITY)
      {  velocity = heading * MAXVELOCITY;  }
   }

   return go;
}

vector3df SteeringBehavior::GetGoal(void)
{
   return goal;
}

}
