#ifndef __ACTION_HPP__
#define __ACTION_HPP__

#include <irrlicht.h>
#include <list>
#include <vector>
//#include "boost/ptr_container/ptr_list.hpp"
//#include "boost/ptr_container/ptr_deque.hpp"
#include "boost/ptr_container/ptr_vector.hpp"

#ifndef NDEBUG
#include "IO.hpp"
#include <algorithm>
#endif

namespace cj
{
using namespace irr;
using namespace irr::core;

using namespace std;

// id=timed
class Timed
{
public:
	Timed( u32 rate ): period( 1.00 / f32(rate) ), elapsedTime(0.00)
	{}
	virtual bool periodMet( const f32 frameDeltaTime);
private:
	f32 period;
	f32 elapsedTime;
};// Timed

namespace actions
{


// fwd dec
class IAction;

typedef boost::ptr_vector<IAction> ActionsList;

// id=iaction
class IAction : boost::noncopyable
{
public:
	IAction(): hasStarted(false)
	{}
	virtual ~IAction()
	{
		//assert( started() );
	}// d
	virtual void start()
	{
		assert( !started() );
		hasStarted = true;
	}// start()
	virtual bool runTick( f32 deltaTime ) = 0;

	bool started() const
	{	return hasStarted;	}//
private:
	bool hasStarted;
};// IAction

// id=Seq
class ActionSeq : public IAction, private ActionsList
{
public:
	virtual ~ActionSeq() {}
	virtual void push_back( IAction* act );
	virtual void start();
	virtual bool runTick( f32 deltaTime );
	using ActionsList::size;
private:
	ActionsList::iterator curAction;
};// ActionSeq()

// id=conj
//class ActionSet : public IAction, private ActionsList
//{
//public:
	//ActionSet(): started(false)
	//{}

	//virtual ~ActionSet() {}

	//virtual void start()
	//{
		//assert( !empty() );
		//assert( !started );
		//started = true;

		//for( ActionsList::iterator aIt = begin(); aIt != end(); ++aIt )
		//{	aIt->start();	}// for
	//}// start()

	//virtual void push_back( IAction* act )
	//{
		//assert( !started );
		//push_back(act);
		//finishedActions.push_back(false);
	//}//

	//virtual bool runTick( const f32 deltaTime )
	//{
		//assert( started );
		//assert( !empty() );
		//for( ActionsList::iterator aIt = begin(), vector<bool>::const_iterator bIt = finishedActions.begin(); aIt != end(); ++aIt, ++bIt )
		//{
			//if( !(*bIt) )
			//{	*bIt = aIt->runTick( deltaTime );	}// if
		//}// for

		//for( vector<bool>::const_iterator bIt = finishedActions.begin(); bIt != finishedActions.end(); ++bIt )
		//{
			//if( *bIt == false )
			//{	return false;	}// if
		//}// for
		//return true;
	//}// runTick()

		////(loop for a in actions
			 ////for b in vdone
			 ////doing (setf b (runTick a f32))
			 ////finally (return (every #'truep vdone)))
	//using ActionsList::size;
//private:
	//vector<bool> finishedActions;
	//bool started;
//};// ActionSet

class IPersistentAction : public boost::noncopyable
{
public:
	IPersistentAction() {}
	virtual ~IPersistentAction() {}
	virtual bool operator==(const IPersistentAction& rhs) const {	return this==&rhs;	}

	virtual void runTick() = 0;
};// IPersistentAction

using namespace irr;
using namespace irr::core;

// TODO: Why, again, was it that I used a std::list here?
class PersistentActionsList : public boost::noncopyable, private std::list<IPersistentAction*>
{
public:
	PersistentActionsList (): curTick(0), prevTick(0) {}
	virtual ~PersistentActionsList() {}

	virtual void queueAction( IPersistentAction& action )
	{
dpr( "Adding pers. action" );
		push_back(&action);
	}// queueAction()
	virtual void removeAction( IPersistentAction& action )
	{
dpr( "Removing pers. action" );
		// (It should be in the list:)
		assert( find( begin(), end(), &action ) !=  end() );
		remove(&action);
	}// removeAction()


	void runTick()
	{
		for( iterator it = begin(); it != end(); ++it )
		{ 	(*it)->runTick();	}// for
	}// runTick()

private:
	//const IrrlichtDevice& device;
	u32 curTick;
	u32 prevTick;
};// PersistentActionsList

}// actions
}// cj

#endif// inc

