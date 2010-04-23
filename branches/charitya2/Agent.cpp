#include "Agent.hpp"

namespace cj
{

// Counter for generating the next free ID#:
s32 Agent::nextAvailableID = 700;

s32 Agent::genID()
{  return ++nextAvailableID;  }

Agent::Agent(IrrlichtDevice* d, stringw mesh, stringw t, stringw h, vector3df p)
 : device(d), path(h), texture(t), wheel(360,80,100)
{
    //initialize naming short cuts
    driver = device->getVideoDriver();
    smgr = device->getSceneManager();
    cmgr = smgr->getSceneCollisionManager();
    //add the scene node to the scene manager
    body = smgr->addAnimatedMeshSceneNode(smgr->getMesh(mesh), 0, genID() );// NB: genID() arg added to keep agents' IDs from all being '-1'.
    //turn lighting off since we do not have a light source
    body->setMaterialFlag(EMF_LIGHTING, false);
    //default stand animation
    body->setMD2Animation(scene::EMAT_STAND);
    //texture for the agent
    body->setMaterialTexture(0, driver->getTexture(texture));
    //place the agent in the world
    body->setPosition(p);
    //default sensor settings
    resolution = 7;
    awareness = 360;
    range = 30;
    //create the sensing circle and adjust the lighting and texture
    circle = smgr->addBillboardSceneNode(body, dimension2df(range*2,range*2));
    circle->setMaterialType(EMT_TRANSPARENT_ALPHA_CHANNEL);
    circle->setMaterialFlag(EMF_LIGHTING, false);
    circle->setMaterialTexture(0, driver->getTexture(path + "circle.png"));
    //circle is not drawn unless needed, so make it invisible
    circle->setVisible(false);
    body->updateAbsolutePosition();
    won = false;
    remove = true;
    stuck = 0;
}

Agent::Agent(IrrlichtDevice* d, stringw mesh, stringw t, stringw h, vector3df p, AIBrain b)
 : device(d), path(h), texture(t), wheel(360,80,100), brain(b)
{
    //initialize naming short cuts
    driver = device->getVideoDriver();
    smgr = device->getSceneManager();
    cmgr = smgr->getSceneCollisionManager();
    //add the scene node to the scene manager
    body = smgr->addAnimatedMeshSceneNode(smgr->getMesh(mesh), 0, genID() );// NB: genID() arg added to keep agents' IDs from all being '-1'.
    //turn lighting off since we do not have a light source
    body->setMaterialFlag(EMF_LIGHTING, false);
    //default stand animation
    body->setMD2Animation(scene::EMAT_STAND);
    //texture for the agent
    //body->setMaterialTexture(0, driver->getTexture(texture));
    //place the agent in the world
    body->setPosition(p);
    //default sensor settings
    resolution = 7;
    awareness = 360;
    range = 50;
    //create the sensing circle and adjust the lighting and texture
    circle = smgr->addBillboardSceneNode(body, dimension2df(range*2,range*2));
    circle->setMaterialType(EMT_TRANSPARENT_ALPHA_CHANNEL);
    circle->setMaterialFlag(EMF_LIGHTING, false);
    circle->setMaterialTexture(0, driver->getTexture(path + "circle.png"));
    //circle is not drawn unless needed, so make it invisible
    circle->setVisible(false);
    body->updateAbsolutePosition();
    won = false;
    remove = true;
    stuck = 0;
}

Agent::~Agent()
{
   if(remove)
   {  body->remove();  }
}

void Agent::Reset(vector3df p, AIBrain b)
{
   body->setPosition(p);
   body->updateAbsolutePosition();
   won = false;
   route.clear();
}

IAnimatedMeshSceneNode* Agent::getBody(void)
{  return body;  }

vector<f32> Agent::DrawFeelers(bool debug)
{  //defaults for just one feeler
   vector<f32> endpoints(resolution);
   s32 begin = 0;
   s32 end = 0;
   s32 step = 1;
   //since the driver draws everything in the world coordinate system
   //we need to translate and rotate everything by the agent's transform
   matrix4 transform = body->getAbsoluteTransformation();

   if(resolution != 1)
   {  //start half of the awareness angle to the left
      begin = (s32)awareness / (-2);
      //and end half of the awareness to the right
      end = awareness / 2;
      //degree of seperation of feelers
      step = awareness / (resolution - 1);
   }

    vector<f32>::iterator it = endpoints.end();
    for(s32 i = begin; i <= end; i += step)
    {
        line3d<f32> line;
        //start at the center of the agent
        line.start = transform.getTranslation();
        //end at the edge of the range
        line.end = vector3df(range,0,0);
        //transform it according to the agent's heading
        transform.transformVect(line.end);
        //rotate endpoint to the next angle
        line.end.rotateXZBy(i, line.start);
        vector3df point;
        triangle3df outtri;
        //make sure we are using a clear material to start
        SMaterial basic = SMaterial();
        basic.setFlag(EMF_LIGHTING, false);
        driver->setMaterial(basic);
        //draw relative to absolute coordinates
        driver->setTransform(video::ETS_WORLD, matrix4());

        if(cmgr->getSceneNodeAndCollisionPointFromRay(line, point, outtri))
      {   //collision point modifies the endpoint and outlines the hit triangle
		    line.end = point;
		    if(debug)
		    {  driver->draw3DTriangle(outtri, video::SColor(255,0,0,130));  }
        }

        //draw the feeler and add the distance to collision to the result list
        if(debug)
        {  driver->draw3DLine(line.start, line.end, SColor(102,120,255,136));  }

        *(--it) = line.start.getDistanceFrom(line.end);
    }

    return endpoints;
}

vector<pointOfInterest> Agent::DrawCircle(const vector<Agent*>& agents, bool debug)
{   //draw sensing circle and find detected agents in the list
    vector<pointOfInterest> detected;
    //circle for visual debugging
    if(debug)
    {  circle->setVisible(true);  }
    //draw with a clear material so that lines show up
    SMaterial basic = SMaterial();
    basic.setFlag(EMF_LIGHTING, false);
    driver->setMaterial(basic);
    //draw lines in world coordinate system
    driver->setTransform(video::ETS_WORLD, core::matrix4());
    matrix4 transform = body->getAbsoluteTransformation();

    for(u32 i = 0; i < agents.size(); ++i)
    {
        pointOfInterest p;
        p.object = agents.at(i)->getBody();
        //do not count yourself
        if(p.object == body)
        {  continue; }

        //figure distance to other agent
        matrix4 other = p.object->getAbsoluteTransformation();
        p.range = transform.getTranslation().getDistanceFrom(other.getTranslation());

        if(p.range <= range)
        {   //if other agent is in range
            if(debug)
            {   //visual debug aids
                //draw a line to the other agent
                line3d<f32> line(transform.getTranslation(), other.getTranslation());
                driver->draw3DLine(line.start, line.end, SColor(0,50,50,136));
                //since we have no light source, this makes the agent black
                p.object->setMaterialFlag(EMF_LIGHTING, true);
            }

            //figure the heading relative to the agent
            vector3df relative = other.getTranslation() - transform.getTranslation();
            //normalize the vector for easy computation
            relative.normalize();
            //The active agent faces along the X axis in her local coordinate
            //system, so create an already normalized vector.
            vector3df aheading(1,0,0);
            transform.rotateVect(aheading);
            //Use the cross product to get the direction of the angle.
            vector3df cp = aheading.crossProduct(relative);
            //cross product angle behaves strange, so use dotproduct for the angle
            //FIX ME: use cross product to avoid extra calculation
            p.heading = aheading.dotProduct(relative);
            p.heading = acos(p.heading) * (180/PI);
            //correct rounding messiness
            if((p.heading * p.heading) <= 0.0000005)
            {  p.heading = 0;  }
            //This only works for our particular setup where the agents are in the XZ
            //plane. If that changes, this will need to change to determine the direction
            //of rotation from the agent's heading to the detected agent's location.
            if(cp.Y > 0.0005) //did not use 0 to take rounding errors into account
            {  p.heading = 360 - p.heading;  }
            //add the other agent to the list of interesting points
	    detected.push_back(p);
        }
        else
        {   //reset the lighting and material if agent is not in range
            p.object->setMaterialFlag(EMF_LIGHTING, false);
            p.object->setMaterialTexture(0, driver->getTexture(texture));
        }
    }

    return detected;
}

void Agent::ClearCircle(const vector<Agent*>& agents)
{   //reset the sensing circle
    circle->setVisible(false);
    for(u32 i = 0; i < agents.size(); i++)
    {   //reset the lighting and the texture from DrawCircle and DrawPieSlices debug mode
        agents.at(i)->getBody()->setMaterialFlag(EMF_LIGHTING, false);
        agents.at(i)->getBody()->setMaterialTexture(0, driver->getTexture(texture));
    }
}

vector<f32> Agent::DrawPieSlices(const vector<Agent*>& agents, bool debug)
{   //determines activation levels of different sectors
    vector<f32> activation(4, 0.0f);
    //find distances and heading to agents
    vector<pointOfInterest> detected = DrawCircle(agents);
    //draw with a clear material so that lines show up
    SMaterial basic = SMaterial();
    basic.setFlag(EMF_LIGHTING, false);
    basic.Thickness = 3;
    driver->setMaterial(basic);
    //draw lines in world coordinate system
    driver->setTransform(video::ETS_WORLD, core::matrix4());
    matrix4 transform = body->getAbsoluteTransformation();

    if(debug)
    {  //visual debugging aids
        circle->setVisible(true);
        line3d<f32> line;
        //start at the center of the agent
        line.start = transform.getTranslation();
        //end at the edge of the range
        line.end = vector3df(range,0,0);
        //transform it according to the agent's heading
        transform.transformVect(line.end);
        //rotate endpoint to 45 degrees and draw a line
        line.end.rotateXZBy(45, line.start);
        driver->draw3DLine(line.start, line.end, SColor(0,0,0,0));

        for(u32 i = 0; i < 3; i++)
        {   //draw the other 3 lines 90 degrees apart
            line.end.rotateXZBy(90, line.start);
            driver->draw3DLine(line.start, line.end, SColor(0,0,0,0));
        }
    }

    for(u32 i = 0; i < detected.size(); i++)
    {   //for all detected agents, add range to that agent's sector
        if(detected.at(i).heading <= 45  || detected.at(i).heading > 315)
        {   //extra modifier for something that is in the forward sector
            activation.at(0) += ((range - detected.at(i).range) * 2);
            detected.at(i).object->setMaterialTexture(0, driver->getTexture(path + "purple.jpg"));
        }
        else if(detected.at(i).heading <= 135  && detected.at(i).heading > 45)
        {   //sector to the left
            activation.at(1) += (range - detected.at(i).range);
            detected.at(i).object->setMaterialTexture(0, driver->getTexture(path + "green.jpg"));
        }
        else if(detected.at(i).heading <= 225  && detected.at(i).heading > 135)
        {   //sector to the rear is less noticable
            activation.at(2) += ((range - detected.at(i).range) / 2);
            detected.at(i).object->setMaterialTexture(0, driver->getTexture(path + "yellow.jpg"));
        }
        else if(detected.at(i).heading <= 315  && detected.at(i).heading > 225)
        {   //sector to the right
            activation.at(3) += (range - detected.at(i).range);
            detected.at(i).object->setMaterialTexture(0, driver->getTexture(path + "blue.jpg"));
        }
    }

    return activation;
}

void Agent::SetResolution(u32 r)
{   //resolution must be odd and at least 1
    resolution = r;
    if(resolution <= 2)
    {  resolution = 1;  }
    else if(resolution % 2 == 0)
    {  --resolution;  }
}

void Agent::SetRange(u32 r)
{  range = r;  }

void Agent::SetAwareness(u32 a)
{  awareness = a;  }

vector<f32> Agent::MoveVector(vector3df distance)
{
    // remember old values
    u32 old_awareness = awareness;
    u32 old_range = range;
    u32 old_resolution = resolution;
    // to speed up collision detection and make sure we detect from all angles
    awareness = 360;
    range = 7;
    resolution = 7;
    // record success or failure of movement
    bool moved = false;
    vector3df old_position = body->getPosition();
    vector<f32> cpoints = DrawFeelers(false);
    f32 oldc = 0;
    f32 newc = 0;

    // find collision amounts for current position
    for(u32 i = 0; i < cpoints.size(); i++)
    {  oldc += cpoints.at(i);  }

    // find collision amounts for new position
    body->setPosition(old_position + distance);
    // NOTE: MUST call this anytime you use setPosition and want to use the new
    // position of the IAnimatedSceneNode in the same tick
    body->updateAbsolutePosition();
    cpoints = DrawFeelers(false);
    for(u32 i = 0; i < cpoints.size(); i++)
    {  newc += cpoints.at(i);  }

    // the 0.0001 is for rounding errors and to slightly favor movement
    if(oldc - 0.0001 > newc)
    {
        moved = false;
        body->setPosition(old_position);
    }
   else
   {
      line3d<f32> line;
      //start at old position
      line.start = old_position;
      //end at the new position
      line.end = old_position + distance;
      vector3df point;
      triangle3df outtri;
      //draw relative to absolute coordinates
      driver->setTransform(video::ETS_WORLD, matrix4());

      if(cmgr->getSceneNodeAndCollisionPointFromRay(line, point, outtri))
		{  //new position is not in line of sight of old position, so we
         //move only to the collision point - range
         vector3df d(distance);
         d.normalize();
         d *= range;
         body->setPosition(point - d);
      }

      moved = true;
   }

    // reset to orignal values
    awareness = old_awareness;
    range = old_range;
    resolution = old_resolution;
    if(!moved)
    {  cpoints.clear();  }

    return cpoints;
}

void Agent::Update(void)
{
   vector<f32> feelers = Sense();
   Think(feelers);
   Act();
}

vector<f32> Agent::Sense(void)
{
   return DrawFeelers(false);
}

void Agent::Think(vector<f32> feelers)
{
   matrix4 bodytrans = body->getAbsoluteTransformation();

   if(wheel.HasGoal())
   {  //we haven't found the goal yet
      //input for brain is the 5 feelers scaled by range to be a value 0 to 1
      for(u16 i = 0; i < feelers.size(); i++)
      {  feelers[i] /= (range-5);  }
      //next input for brain is the normal vector to the target
      vector3df goalnorm = wheel.GetGoal() - bodytrans.getTranslation();
      goalnorm.normalize();
      feelers.push_back(goalnorm.X);
      feelers.push_back(goalnorm.Z);
      //get the output
      vector<f64> output = brain.ChangeGoal(feelers);

      //the last output is whether or not to create a subgoal
      if(output.back() > 0.5 || stuck > 0)
      {  //make sure we keep the original goal
         if(route.empty())
         {  route.push_back(wheel.GetGoal());  }
         s32 sign = 1;
         if(stuck > 0)
         {  sign = -1;  }
         //new goal in a direction determined by output
         wheel.SetGoal(vector3df(range/2 - sign*range*output.at(0), 0, range/2 - sign*range*output.at(1)));
      }

      //calculate next step to goal or subgoal
      wheel.NextStep(bodytrans);
   }
   else if(!route.empty())
   {  //we found the subgoal, back to the original goal
      wheel.SetGoal(route.front());
      route.erase(route.begin());
      wheel.NextStep(bodytrans);;
   }
   else if(!won)
   {  //we found the original goal, update end clock time if not done
      won = true;
      stuck = 0;
      clock_gettime(CLOCK_REALTIME, &gend);
   }
}

void Agent::Act(void)
{
   if(wheel.HasGoal())
   {
      //Set rotation and attempt to move to position (checks for collisions)
      body->setRotation(body->getRotation() + wheel.GetRotation());
      if(!MoveVector(wheel.GetStep()).empty())
      {  stuck = 0;  }
      else
      {  stuck++;  }
   }
}

void Agent::Seek(vector3df goal, Wall *w, bool track)
{
   route = w->AStar(body->getPosition(), goal, 2, track);
   route.push_back(goal);
   if(track)  //only track the goal passed in from game
   {  clock_gettime(CLOCK_REALTIME, &gstart);  }
   stuck = 0;
}

u32 Agent::GetFitness(vector3df goal, vector3df startpos)
{
   u32 fitness = 0;

   if(won)
   {  //this individual found the goal for this start vector
      f64 interval = (gend.tv_sec - gstart.tv_sec) + ((gend.tv_nsec - gstart.tv_nsec)/1000000000.00);
      fitness = 5000 - interval * 3;
   }
   else
   {  //did not find the goal
      fitness = 1000 - (goal - body->getAbsoluteTransformation().getTranslation()).getLength()
         + (startpos - body->getAbsoluteTransformation().getTranslation()).getLength();
      fitness /= (stuck + 1);
   }

   return fitness;
}

void Agent::GameOver(void)
{
   remove = false;
}

AIBrain Agent::GetBrain(void)
{  return brain;  }

void Agent::SmartNavigate(void)
{
   matrix4 bodytrans = body->getAbsoluteTransformation();
   if(wheel.HasGoal())
   {
       //Set rotation and attempt to move to position (checks for collisions)
      cout << "current: " << body->getRotation().Y << " change: " << wheel.GetRotation().Y << endl;
      f32 newangle = body->getRotation().Y + wheel.GetRotation().Y;
      if(newangle > 360)
      {  newangle -= 360;  }
      body->setRotation(vector3df(0,newangle,0));

      //if(!MoveVector(wheel.GetStep()).empty())
      //{
         //make sure we keep the original goal
         //if(route.empty())
         //{  route.push_back(wheel.GetGoal());  }

      //}
      wheel.NextStep(bodytrans);
   }
   else if(!route.empty())
   {  //we found the subgoal, back to the original goal
      wheel.SetGoal(route.front());
      route.erase(route.begin());
      wheel.NextStep(bodytrans);;
   }
}

}
