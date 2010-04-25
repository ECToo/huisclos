//#ifndef __AGENT_TEMPLATES__
//#define __AGENT_TEMPLATES__

#include "Agent_actions.hpp"

namespace cj
{

//id=draw-circle
template <typename TAgentsIterator>
Agent::ContactsList Agent::DrawCircle(TAgentsIterator it, const TAgentsIterator& end, bool debug)
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

    for( ; it != end; ++it)
    {
        pointOfInterest p;
        p.object = &(it->getBody());
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

template <typename TAgentsIterator>
void Agent::ClearCircle(TAgentsIterator begin, const TAgentsIterator& end)
{   //reset the sensing circle
    circle->setVisible(false);
    for( TAgentsIterator it = begin; it != end; ++it)
    {   //reset the lighting and the texture from DrawCircle and DrawPieSlices debug mode
        it->getBody().setMaterialFlag(EMF_LIGHTING, false);
        it->getBody().setMaterialTexture(0, driver->getTexture(texture));
    }
}

template <typename TAgentsIterator>
vector<f32> Agent::DrawPieSlices(TAgentsIterator begin, const TAgentsIterator& end, bool debug)
{   //determines activation levels of different sectors
    vector<f32> activation(4, 0.0f);
    //find distances and heading to agents
    vector<pointOfInterest> detected = DrawCircle(begin, end);
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


template <typename TAgentsIterator>
void Agent::updateSensors(const TAgentsIterator& begin, const TAgentsIterator& end)
{
	if( getRangefinder() )
	{ setRangefinderOutput( DrawFeelers( getRangefinderVisible() ) ); }// if
	if( getRadar() )
	{ setRadarOutput( DrawCircle( begin, end, getRadarVisible() ) ); }// if
	if( getActivation() )
	{ setActivationOutput( DrawPieSlices( begin, end, getActivationLevelsVisible() ) ); }// if
}// updateSensors()

template <typename TWaypointsList>
actions::ActionSequence* Agent::visitWaypoints( const TWaypointsList& pointsList, f32 speed )
{
	using namespace cj::actions;

	ActionSequence* act = new ActionSequence;

	for( typename TWaypointsList::const_iterator it = pointsList.begin(); it != pointsList.end(); ++it )
	{
//dpr("Pushed " << *it);
		act->push_back( new ActAgentSeekPosition(*this, *it, speed) );
	}// for

	assert( act->size() == pointsList.size() );

	setCurrentAction(act);

	return act;
}// visitWaypoints()

template <typename TAgentsList>
vector<Agent*> Agent::getVisibleAgents( TAgentsList& allAgents, bool countIfDead )
//vector<Agent*> Agent::getVisibleAgents( typename TAgentsList::iterator it, const typename TAgentsList::iterator end )
{
	vector<Agent*> visAgents;

	for( typename TAgentsList::iterator it = allAgents.begin(); it != allAgents.end(); ++it )
	{
		if( 	*this != *it &&
			( countIfDead || !it->isDead() ) &&
			getLineOfSightExists( *this, *it )
		  ){	visAgents.push_back(&*it);	}// if
	}// for

	return visAgents;
}// getVisibleAgents()
}// cj


//#endif
