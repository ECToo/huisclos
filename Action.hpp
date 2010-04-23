#ifndef __ACTION_HPP__
#define __ACTION_HPP__

#include <irrlicht.h>
#include <list>
#include "boost/ptr_container/ptr_list.hpp"
#include "boost/ptr_container/ptr_deque.hpp"

#ifndef NDEBUG
#include "IO.hpp"
#include <algorithm>
#endif

namespace cj
{
	namespace actions
	{
		using namespace irr;
		using namespace irr::core;

		//using namespace std;

		// ABC:
		class ITickAction : public boost::noncopyable
		{
		public:
			//ITickAction();
			virtual ~ITickAction() {}

			virtual bool operator==(const ITickAction& rhs) const {	return this==&rhs;	}

			// (Unlike the Persistent* version, note the "completion" retval.)
			virtual bool runTick( f32 frameDelta ) = 0;
		};// ITickAction

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

		//TODO: Is it possible to use an adaptor queue, such as public std::queue<ITickAction, boost::ptr_deque<ITickAction> > ?
		class ActionSequence : public ITickAction, public boost::ptr_deque<ITickAction>
		{
		public:
			virtual ~ActionSequence() {}
			virtual bool operator==(const ActionSequence& rhs) const {	return this==&rhs;	}

			virtual bool runTick( f32 frameDelta )
			{
				if( !empty() )
				{
					const bool finished = front().runTick( frameDelta );
					if( finished )
					{	
//dpr("Popped.");				
						pop_front();	
					}// if
				}// if

				return( empty() );
			}// runTick()
		};// ActionSequence



		//class ActionsList : public boost::noncopyable, private boost::ptr_list<ITickAction>
		//{
		//public:
			//ActionsList(IrrlichtDevice& dev): device(dev), curTick(0), prevTick(0) {}
			//virtual ~ActionsList() {}

			//virtual void queueAction( ITickAction* const action )
			//{
				//// (It should not be in the list:)
				//assert( find( begin(), end(), *action ) == end() );
				//push_back(action);
				//// (It should now be in the list:)
				//assert( find( begin(), end(), *action ) != end() );
			//}// queueAction()

			//void runTick()
			//{
				//prevTick = curTick;
				//curTick = device.getTimer()->getTime();
				//const f32 frameDeltaTime = static_cast<f32>(curTick - prevTick) / 1000.f; // Time in seconds

				//iterator it = begin();
				//while( it != end() )
				////for( iterator it = begin(); it != end(); ++it )
				//{
					//const bool finished = it->runTick( frameDeltaTime );
					//if( finished )
					//{
						//// FIXME: Is the iterator borked?
						//it = erase(it);
					//}// if
					//else
					//{	++it;	}// else
				//}// while
			//}// runTick()

		//private:
			//IrrlichtDevice& device;
			//u32 curTick;
			//u32 prevTick;
		//};// ActionsList

		class ActionsList : public boost::noncopyable, private boost::ptr_list<ITickAction>
		{
		public:
			//ActionsList(IrrlichtDevice& dev): device(dev), curTick(0), prevTick(0) {}
			virtual ~ActionsList() {}

			virtual void queueAction( ITickAction* const action )
			{
				// (It should not be in the list:)
				assert( find( begin(), end(), *action ) == end() );
				push_back(action);
				// (It should now be in the list:)
				assert( find( begin(), end(), *action ) != end() );
			}// queueAction()

			virtual void runTick( f32 frameDeltaTime )
			{
				//prevTick = curTick;
				//curTick = device.getTimer()->getTime();
				//const f32 frameDeltaTime = static_cast<f32>(curTick - prevTick) / 1000.f; // Time in seconds

				iterator it = begin();
				while( it != end() )
				//for( iterator it = begin(); it != end(); ++it )
				{
					const bool finished = it->runTick( frameDeltaTime );
					if( finished )
					{
						// FIXME:
						// If the action is intended to "chain," load the next action.
						//if( it->getNextAction() != NULL )
						//{
							//insert( it, it->getNextAction() );
						//}// if
						// FIXME: Is the iterator borked after an erase() call?
						it = erase(it);
					}// if
					else
					{	++it;	}// else
				}// while
			}// runTick()

			using boost::ptr_list<ITickAction>::erase;

			using boost::ptr_list<ITickAction>::iterator;
			using boost::ptr_list<ITickAction>::begin;
			using boost::ptr_list<ITickAction>::end;

		//private:
			//IrrlichtDevice& device;
			//u32 curTick;
			//u32 prevTick;
		};// ActionsList

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

