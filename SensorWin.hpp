// TODO: SensorWin -> OutputWindow
// TODO: Namespaces cj & sensor
// FIXME: Need to be templates??

#ifndef __SENSORWIN_HPP__
#define __SENSORWIN_HPP__

#include <cassert>
#include <irrlicht.h>
//#include <iostream>
#include <sstream>
#include <vector>
#include "boost/utility.hpp"

#include "IO.hpp"
#include "Action.hpp"

namespace cj
{
	using namespace std;
	using namespace irr;
	using namespace irr::core;
	using namespace irr::gui;

	// Moved to IO.hpp:
	// "Normalize," or "rationalize," or whatever, an angle:
	//inline f32 rationalizeAngle( f32 theta )

	inline f32 reorientLocalAngleDammit( const f32 theta )
	{ 	return 90 + theta; 	}//  reorientLocalAngleDammit()

	template <typename TGame> class RangefinderWin; // fwd dec
	template <typename TGame, typename TList> class RadarWin;
	template <typename TGame> class ActivationWin;

	namespace actions
	{
		template <typename TGame>
		class ActUpdateRangefinderOutputWindow : public IPersistentAction
		{
		public:
			ActUpdateRangefinderOutputWindow( const TGame& g,	RangefinderWin<TGame>& win, IGUIListBox& walls ): game(g), window(win), wallsListBox(walls) {}// ctor
			virtual ~ActUpdateRangefinderOutputWindow() {}

			virtual void runTick()
			{
				wallsListBox.clear();// don't forget!

				if( game.getIsPCSet() )
				{
					vector<f32> contacts = game.getRangefinderOutput();
					// NB: Don't use an iterator here; it makes printing the index unnecessarily difficult.
					for( u32 i=0; i < contacts.size(); ++i )
					{
						std::wostringstream txt;
						txt << i << L": " << contacts.at(i);
						wallsListBox.addItem( txt.str().c_str(), -1 );
					}// for
				}// if
			}// run()
		private:
			const TGame& game;
			RangefinderWin<TGame>& window;
			IGUIListBox& wallsListBox;
		};// ActUpdateRangefinderOutputWindow

		template <typename TGame, typename TList>
		class ActUpdateRadarOutputWindow : public IPersistentAction
		{
		public:
			ActUpdateRadarOutputWindow( TGame& g, RadarWin<TGame, Agent::ContactsList>& win, IGUIListBox& agents ): game(g), window(win), agentsListBox(agents) {}// ctor
			virtual ~ActUpdateRadarOutputWindow() {}

			virtual void runTick()
			{
				TList contacts = game.drawCircle();

				agentsListBox.clear();// don't forget!
				for( u32 i=0; i < contacts.size(); ++i )
				{
	//wcout << i << endl;
					std::wostringstream txt;
					txt << L"npc " << contacts.at(i).object->getID() << L": distance " << contacts.at(i).range << L" @ " << rationalizeAngle( reorientLocalAngleDammit( contacts.at(i).heading) )  << L"°rel <- PC " << game.getPC().getBody().getID();
					agentsListBox.addItem( txt.str().c_str(), -1 );
				}// for
			}// runTick()
		private:
			TGame& game;
			RadarWin<TGame, Agent::ContactsList>& window;
			IGUIListBox& agentsListBox;
		};// ActUpdateRadarOutputWindow

		// ********* id=ACTIVATION
		template <typename TGame>
		class ActUpdateActivationOutputWindow : public IPersistentAction
		{
		public:
			ActUpdateActivationOutputWindow( TGame& g, ActivationWin<TGame>& win, IGUIListBox& levels ): game(g), window(win), levelsListBox(levels) {}// ctor
			virtual ~ActUpdateActivationOutputWindow() {}

			virtual void runTick()
			{
				vector<f32> activationLevels = game.drawPieSlices();

				levelsListBox.clear();
				for( u32 i=0; i < activationLevels.size(); ++i )
				{
					std::wostringstream txt;
					txt << L"Sector " << i << L" level: " << activationLevels.at(i);
					levelsListBox.addItem( txt.str().c_str(), -1 );
				}// for
			}// runTick()
		private:
			TGame& game;
			ActivationWin<TGame>& window;
			IGUIListBox& levelsListBox;
		};// ActUpdateActivationOutputWindow

	}// actions

	// ************** WINDOWS

	using namespace actions;

	// TGame type passed up by subclasses.  Its purpose is to delay being instantiated until the client has.
	template <typename TGame>
	class SensorWin : boost::noncopyable// TODO: Does this propagate its effect to subclasses?
	{
	public:
		static const u32 WIDTH = 200;
		// TODO: Vary this per the subclass:
		static const u32 HEIGHT = 250;

		SensorWin( IGUIEnvironment& guienv, const stringw& caption, const recti& pos )
		: sensorWindow( guienv.addWindow( pos, false, caption.c_str())
		) {
			assert( sensorWindow );
		}// ctor

		virtual ~SensorWin()
		{
			sensorWindow->remove();
			sensorWindow = NULL;
		}// dtor

		//virtual void runTick( TGame& caller ) = 0;
	protected:

		IGUIWindow* sensorWindow;


		// Accessor:
		IGUIWindow& getSensorWindow()
		{
			assert( sensorWindow );
			return *sensorWindow;
		}// getSensorWindow()

	//private:
		// Disabled
		//SensorWin(SensorWin&);
		//void operator=(SensorWin&);
	};// SensorWin

	// ******************* WINDOW IMPLEMENTATIONS
	template <typename TGame>
	class RangefinderWin : public SensorWin<TGame>
	{
	public:
		RangefinderWin(TGame& g, IGUIEnvironment& guienv, const vector2di& upperLeftCorner )
		// TODO: Code in a minimum-distance > 10.
		: SensorWin<TGame>( guienv, L"Rangefinder \"Vibrissæ\"", recti( upperLeftCorner.X, upperLeftCorner.Y, upperLeftCorner.X + SensorWin<TGame>::WIDTH, upperLeftCorner.Y + SensorWin<TGame>::HEIGHT ) ),
		game(g),
		wallsListBox(guienv.addListBox( recti(10,30,300,500), &SensorWin<TGame>::getSensorWindow(), -1, true )),
		outputAction( game, *this, *wallsListBox )
		{
			// TODO: Multiplied problem: These should not be accessing the Game's list directly, even through an accessor.
			game.getPersistentActionsList().queueAction( outputAction );
		}// ctor

		virtual ~RangefinderWin()
		{
			game.getPersistentActionsList().removeAction( outputAction );
			wallsListBox->remove();
		}// dtor
	protected:
		TGame& game;
		IGUIListBox* const wallsListBox;

		ActUpdateRangefinderOutputWindow<TGame> outputAction;
	};// RangefinderWin


	// TODO: ⁅Uh, I can't remember why I templated the list type.⁆
	template <typename TGame, typename TList>
	class RadarWin : public SensorWin<TGame>
	{
	public:
		RadarWin(TGame& g, IGUIEnvironment& guienv, const vector2di& upperLeftCorner )
		: SensorWin<TGame>( guienv, L"Radar Ring", recti( upperLeftCorner.X, upperLeftCorner.Y, upperLeftCorner.X + SensorWin<TGame>::WIDTH, upperLeftCorner.Y + SensorWin<TGame>::HEIGHT ) ), game(g), agentsListBox( guienv.addListBox( recti(10,30,500,500), &SensorWin<TGame>::getSensorWindow(), -1, true ) ), outputAction( game, *this, *agentsListBox )
		{
			game.getPersistentActionsList().queueAction( outputAction );
		}// ctor

		virtual ~RadarWin()
		{
			game.getPersistentActionsList().removeAction( outputAction );
			agentsListBox->remove();
		}// dtor

	protected:
		TGame& game;
		IGUIListBox* const agentsListBox;
		ActUpdateRadarOutputWindow<TGame, Agent::ContactsList> outputAction;
	};// RadarWin

	// ********* ACTIVATION
	// id=act
	// TODO: Separate width|height constants for each type of window: Activation-level window, in particular, needs not to be so big.
	template <typename TGame>
	class ActivationWin : public SensorWin<TGame>
	{
	public:
		ActivationWin(TGame& g, IGUIEnvironment& guienv, const vector2di& upperLeftCorner )
		: SensorWin<TGame>( guienv, L"Activation \"Pie\"", recti( upperLeftCorner.X, upperLeftCorner.Y, upperLeftCorner.X + SensorWin<TGame>::WIDTH, upperLeftCorner.Y + SensorWin<TGame>::HEIGHT ) ), game(g), levelsListBox( guienv.addListBox( recti(10,30,500,500), &SensorWin<TGame>::getSensorWindow(), -1, true ) ), outputAction( game, *this, *levelsListBox )
		{
			game.getPersistentActionsList().queueAction( outputAction );
		}// ctor

		virtual ~ActivationWin()
		{
			game.getPersistentActionsList().removeAction( outputAction );
			levelsListBox->remove();
		}// dtor
	protected:
		TGame& game;
		IGUIListBox* const levelsListBox;
		ActUpdateActivationOutputWindow<TGame> outputAction;

	//private:
		//// Disabled
		//ActivationWin(ActivationWin&);
		//void operator=(ActivationWin&);
	};// ActivationWin
}// cj

#endif // inc

