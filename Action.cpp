#include "Action.hpp"

//using namespace irr;
//using namespace irr::core;


namespace cj
{

bool Timed::periodMet( const f32 frameDeltaTime)
{
	elapsedTime += frameDeltaTime;
	if( elapsedTime >= period )
	{
		elapsedTime = 0.00;
		return true;
	}// if
	else
	{	return false;	}// else
}// periodMet()

namespace actions
{

// id=eq
void ActionSeq::push_back( IAction* act )
{
	assert( !started() );
	ActionsList::push_back(act);
}// push_back()

void ActionSeq::start()
{
dpr( "Sequence started." );
	assert( !empty() );
	assert( !started() );

	curAction = begin();
	assert( !curAction->started() );
	curAction->start();
	assert(!started());
	IAction::start();// chain up
}// start()

bool ActionSeq::runTick( f32 deltaTime )
{
	assert( started() );
	assert( !empty() );
	assert( curAction != end() );

	if( curAction->runTick( deltaTime ) )
	{	
		assert( curAction->started() );
		++curAction;	
		if( curAction != end() )
		{
			assert( !curAction->started() );
			curAction->start();
		}// if
	}// if

//dpr("Done? " << ( curAction == end() ));
	return ( curAction == end() );
}// runTick()


}// actions
}// cj

