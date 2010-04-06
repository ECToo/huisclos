#include "Agent.hpp"

namespace cj {


const f32 Agent::MOVEMENT_SLOW = 12.0f;
const f32 Agent::MOVEMENT_FAST = 60.0f;
const f32 Agent::TURN_SLOW = 70.0f;
const f32 Agent::TURN_FAST = 180.0f;

// Counter for generating the next free ID#:
s32 Agent::nextAvailableID = 0;


// id=ctor ⁅This is a search marker for me in Vim⁆
//Agent::Agent(IrrlichtDevice* d, [>PersistentActionsList& pal,<] stringw mesh, stringw t, stringw h, const vector3df& p)
 //: [>persistentActionsList(pal),*/ device(d), path(h), texture(t), feelersOutput(), nearbyAgents(), activationLevels()/*, sensorsAction(*this)<]
//{
	//AgentCtorImpl();
//}

Agent::Agent(IrrlichtDevice* d, /*PersistentActionsList& pal,*/ stringw mesh, stringw t, stringw h, const absVec& p)
 : /*persistentActionsList(pal),*/ device(d), path(h), texture(t), 
	rangefinderVisible(false), radarVisible(false), activationLevelsVisible(false),
	feelersOutput(), nearbyAgents(), activationLevels()/*, sensorsAction(*this)*/
{
	AgentCtorImpl( mesh, p.toIrr_vector3df() );
}

void Agent::AgentCtorImpl(stringw mesh, const vector3df& p)
{
    //initialize naming short cuts
    driver = device->getVideoDriver();
    smgr = device->getSceneManager();
    cmgr = smgr->getSceneCollisionManager();
    //add the scene node to the scene manager
    body = smgr->addAnimatedMeshSceneNode(smgr->getMesh(mesh), 0, genID() );// NB: genID() arg added to keep agents' IDs from being the default '-1'.
    //turn lighting off since we do not have a light source
    body->setMaterialFlag(EMF_LIGHTING, false);
    //default stand animation
    body->setMD2Animation(scene::EMAT_STAND);
    //texture for the agent
    body->setMaterialTexture(0, driver->getTexture(texture));
    //place the agent in the world
    body->setPosition( p );
    //default sensor settings
    resolution = 7;
    awareness = 180;
    range = 50;
    // id=circle
    //create the sensing circle and adjust the lighting and texture
    circle = smgr->addBillboardSceneNode(body, dimension2df(range*2,range*2));
    circle->setMaterialType(EMT_TRANSPARENT_ALPHA_CHANNEL);
    circle->setMaterialFlag(EMF_LIGHTING, false);
    circle->setMaterialTexture(0, driver->getTexture(path + "circle.png"));
    //circle is not drawn unless needed, so make it invisible
    circle->setVisible(false);
}

// id=dtor
Agent::~Agent()
{
	// TODO: This call was segfaulting when the program closes until I stored the agents() vector in a pointer to control the deletion order.  Still looking for a better way.  Base-class fiddling is a nuisance.
	body->remove();
	body = NULL;
}

// TODO: inline
void Agent::turn( const relAngle& ang, f32 speed )
{ 	actionsList.queueAction( new actions::ActAgentTurn(*this, ang, speed) ); }// turn()

// TODO: inline
// id=move
actions::ActAgentMove* const Agent::move( const relVec& dest, f32 speed )
{ 	
	actions::ActAgentMove* const newact = new actions::ActAgentMove(*this, dest, speed);
	actionsList.queueAction( newact ); 
	return newact;
}// move()

// id=seek
actions::ActAgentMove* const Agent::seek( const absVec& dest, f32 speed, f32 turnspeed )
{
	actions::ActAgentMove* const newact = move( dest.to_relVec(getBody()), speed );
	if( turnspeed != 0.0 )
	{	turn( dest.to_relAngle(getBody()), turnspeed );	}// if
	return newact;
}// seek()

vector<f32> Agent::DrawFeelers(bool debug)
{   //defaults for just one feeler
    vector<f32> endpoints(resolution);
    s32 begin = 0;
    s32 end = 0;
    s32 step = 1;
    //since the driver draws everything in the world coordinate system
    //we need to translate and rotate everything by the agent's transform
    matrix4 transform = body->getAbsoluteTransformation();

    if(resolution != 1)
    {   //start half of the awareness angle to the left
        begin = (s32)awareness / (-2);
        //and end half of the awareness to the right
        end = awareness / 2;
        //degree of seperation of feelers
        step = awareness / (resolution - 1);
    }

    // ⁅I need both an accumulator and a stepwise counter here; I added the iterator.⁆
    vector<f32>::iterator it = endpoints.end(); // TODO: Better as a reverse iterator?
    for(  int i = begin; i <= end; i += step)
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


// OPERATORS
// ==|=!(): Standard address check.
inline bool Agent::operator==(const Agent& rhs) const
{	return( this == &rhs ); }// ==()
inline bool Agent::operator!=(const Agent& rhs) const
{	return !( this == &rhs ); }// !=()

// id=moveAtomic
bool Agent::moveAtomic( const relVec& dest )
{
    // remember old values
    u32 old_awareness = awareness;
    u32 old_range = range;
    u32 old_resolution = resolution;
    // to speed up collision detection and make sure we detect from all angles
    awareness = 360;
    range = 5;  // NOTE: adjust to your liking, this allows hair to go through the walls
    resolution = 7;
    // record success or failure of movement
    bool moved = false;
    relVec old_position = getPosition();
    //vector3df old_position = body->getPosition();

    //vector<f32> cpoints = DrawFeelers(false);
    f32 oldc = 0;
    f32 newc = 0;

    // find collision amounts for current position
    for(vector<f32>::const_iterator it = getRangefinderOutput().begin(); it != getRangefinderOutput().end(); ++it )
    {  oldc += *it;  }
    //for(u32 i = 0; i < cpoints.size(); i++)
    //{  oldc += cpoints.at(i);  }

    // find collision amounts for new position
    setPosition( old_position + dest );

// TODO: Do I need this?
    // NOTE: MUST call this anytime you use setPosition and want to use the new
    // position of the IAnimatedSceneNode in the same tick
    body->updateAbsolutePosition();

    const vector<f32> cpoints = DrawFeelers(false);

    for(u32 i = 0; i < cpoints.size(); i++)
    {  newc += cpoints.at(i);  }

    // the 0.0001 is for rounding errors and to slightly favor movement
    if(oldc - 0.0001 > newc)
    {
        moved = false;
        setPosition(old_position);
    }
    else
    { moved = true; }

    // reset to orignal values
    awareness = old_awareness;
    range = old_range;
    resolution = old_resolution;

    return moved;
}

//inline void Agent::turnAtomic( const relAngle& theta )
//{	setRotation( getRotation() + theta );	}// turnAtomic()


}// cj

