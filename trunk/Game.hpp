// J. Jakes-Schauer
#ifndef __GAME_HPP__
#define __GAME_HPP__

#include <cassert>
//#include <iostream>
//#include <sstream>
//#include <vector>
#include "boost/ptr_container/ptr_vector.hpp"
#include "boost/utility.hpp"

#include <irrlicht.h>

#include "GameGUI.hpp"
#include "Agent.hpp"
#include "Sensor.hpp"
#include "Wall.hpp"
#include "EventReceiver.hpp"
#include "SensorWin.hpp"
#include "Action.hpp"
#include "Coordinates.hpp"
#include "Navigation.hpp"

// <url:vimscript::%s/\\v(\\s)(core::)/\\1irr::\\2/g>
// <url:vimscript::%s/\\v(\\s)(video::)/\\1irr::\\2/g>
// <url:vimscript::%s/\\v(\\s)(scene::)/\\1irr::\\2/g>
// <url:vimscript::%s/\\v(\\s)(gui::)/\\1irr::\\2/g>

//* TODO: Functions for toggling 'debugmode mode', or 'control mode', or w/e, wh/ activates the HUD, among whatever else.


// TODO: Check invariants more often.
namespace cj
{
	using namespace cj::actions;
	using namespace cj::nav;

	//class Game;// fwd dec for GameGUI
	//typedef boost::ptr_vector<Agent> AgentsList; // migrated to GameGUI.hpp
	typedef boost::ptr_vector<Wall> WallsList;
	// FIXME: Type??
	//typedef std::vector<IAction> ActionsList;
	//typedef boost::ptr_vector<ITickAction> ActionsList;
	//typedef std::list<IPersistentAction*> PersistentActionsList;



	static const stringw DEFAULT_MESH = "faerie.md2";
	static const stringw DEFAULT_TEXTURE = "Faerie5.BMP";

	//using namespace irr;
	using namespace irr::core;
	using namespace irr::gui;
	//using namespace irr::video;
	//using namespace irr::scene;

	using namespace cj::gui;

	//**************** GAME EXCEPTION
	class GameException
	{
	   public:
	      GameException(const char *msg);
	      ~GameException();
	      const char *Message(void) const;

	   private:
	      const char *description;
	};// 

	//************** GAME
	// id=game
	// NB: Singleton class; DO NOT TRY TO SUBCLASS!
	class Game : public boost::noncopyable, public cj::sensor::ISensors
	{
	// id=public
	public:

		//static std::list<vector3df> AStar( const vector3df& start, const vector3df& dest, bool debug=true );

		// (Used by EventHandler:)
		template <EGUI_EVENT_TYPE>
		inline static bool handleIt(IGUIElement* const whateverElement) { return false; }// ⁅Yes, I know 'inline' is redundant.⁆

		// STATIC CONSTANT
		static const irr::core::dimension2d<u32> DEFAULT_RESOLUTION;

		// id=ctor
		Game();
		// id=DTOR, id=game-dtor
		virtual ~Game();

		enum GameState
		{
			STOP,
			RUN,
			BREAK,
			INTERRUPT
		};// enum

		// The keyboard shortcut is <C-d>.  This will end the program unless it is being embedded in a larger process; a Lisp session, for example.
		void breakToShell();
		void resume();

		GameState getState() const;
		
		// id=mode, id=viewmode
		enum ViewMode
		{
			BIRDS_EYE,
			FIRST_PERSON
		};// enum

		ViewMode getViewMode() const;
		void setViewMode( ViewMode mode );

		IrrlichtDevice& device();
		IVideoDriver& driver();
		ISceneManager& smgr();
		irr::gui::IGUIEnvironment& guienv ();
		const IrrlichtDevice& device() const;
		const IVideoDriver& driver() const;
		const ISceneManager& smgr() const;
		const irr::gui::IGUIEnvironment& guienv() const;

		//const AgentsList& agents() const;
		//AgentsList& agents();
		const AgentsList& agents() const;
		AgentsList& agents();
		const WallsList& walls() const;
		WallsList& walls();
		const cj::event::EventReceiver<Game>& receiver() const;
		// TODO: Make private?
		cj::event::EventReceiver<Game>& receiver();
		const ICameraSceneNode& cam() const;
		ICameraSceneNode& cam();

		const PersistentActionsList& getPersistentActionsList() const;
		PersistentActionsList& getPersistentActionsList();

		// PC predicate:
		bool getIsPCSet() const;
		const Agent& getPC() const;
		Agent& getPC();

		// Takes control of a character w/ the keyboard; pass NULL to unset.
		void setPC( Agent& agent );
		void unsetPC();

		// No effect if no PC is set:
		virtual void setRangefinder( bool mode=true );
		virtual void setRadar( bool mode=true );
		virtual void setActivation( bool mode=true );

		virtual bool getRangefinder() const;
		virtual bool getRadar() const;
		virtual bool getActivation() const;

		// Returns an iterator to the Agent if it be a valid reference.  [Don't know why it wouldn't.].  Or the usual end() iterator on failure.
		//inline s32 Game::findAgent( const Agent& agent ) const
		inline AgentsList::const_iterator findAgent( const Agent& agent ) const
		{	return find( agents().begin(), agents().end(), agent ); }// findAgent()
		inline AgentsList::iterator findAgent( const Agent& agent )
		{	return find( agents().begin(), agents().end(), agent ); }// findAgent()

		inline AgentsList::const_iterator findPC() const
		{
			assert( getIsPCSet() );
			return findAgent( getPC() );
		}// getPCIndex()
		// Idem, non-const
		inline AgentsList::iterator findPC()
		{
			assert( getIsPCSet() );
			return findAgent( getPC() );
		}// getPCIndex()

		const GameGUI& gui() const
		{
			assert( gameGUI );
			return *gameGUI;
		}// gui()
		GameGUI& gui()
		{
			assert( gameGUI );
			return *gameGUI;
		}// gui()

		Agent& addAgent(IAnimatedMesh* const mesh,
			ITexture* const texture,
			//const absVec& position = absVec(),
			const irr::core::vector3df& position = irr::core::vector3df(0,0,0),
			const irr::core::vector3df& rotation = irr::core::vector3df(0,0,0),
			const irr::core::vector3df& scale = irr::core::vector3df(1.0f, 1.0f, 1.0f),
			ISceneNode* const parent = 0,
			const s32 id = -1,// TODO: Currently ignored.
			bool alsoAddIfMeshPointerZero = false);
		// Idem, quickie lazy version:
		Agent& addAgent( const vector3df& position = vector3df() );

		// Removes & deallocates an Agent in O(n).  Do NOT reference an Agent not currently in-scene!
		void removeAgent( Agent& agent );
		// Idem, but removes the "it'th" Agent.
		void removeAgent( const AgentsList::iterator& it );

		Wall& addWall();
		//// Create a default wall : 1×1 block @ origin, or to taste.
		//Wall& addWall( u32 length = 1, u32 width = 1, const absVec& position = absVec() );
		//// Idem: Default wall size, but you want to provide a position.
		//inline Wall& addWall( const absVec& position )
		//{	return addWall(1, 1, position);	}// addWall()
		void removeWall( Wall& wall );
		// TODO: iterator v.
		void removeWall( const WallsList::iterator& it );

		bool getGUIVisible() const;
		void setGUIVisible( bool b = true );

		//const NavGraph& getNavGraph() const;
		//NavGraph& getNavGraph();

		//*** MAIN LOOP:
		// id=start
		void start();

		// TODO: Put on heap; pass byref w/ a smart-ptr to avoid copying?
		inline vector<f32> drawFeelers()
		{
			assert( getIsPCSet() );
			// FIXME: 'debug' parm
			return getPC().DrawFeelers( true );
		}// drawFeelers()

		inline vector<pointOfInterest> drawCircle()
		{
			assert( getIsPCSet() );
			// FIXME: 'debug' parm
			return getPC().DrawCircle( agents().begin(), agents().end(), true );
		}// drawCircle()

		inline vector<f32> drawPieSlices()
		{
			assert( getIsPCSet() );
			// FIXME: 'debug' parm
			return getPC().DrawPieSlices( agents().begin(), agents().end(), true );
		}// drawPieSlices()

		const vector<f32>& getRangefinderOutput() const
		{
			assert( getIsPCSet() );
			return getPC().getRangefinderOutput();
		}// getRangefinderOutput()

		// Show|hide GUI: <C-g>
		bool getToggleGUIKeypress() const
		{	return receiver().isKeyPressed(irr::KEY_KEY_G) && receiver().ctrlPressed();	}// getToggleGUIKeypress()

		// BREAK: <C-d>
		bool getBreakKeypress() const
		{	return receiver().isKeyPressed(irr::KEY_KEY_D) && receiver().ctrlPressed();	}// getBreakKeypress()
	// id=private
	private:
		// CLASS DATA
		static Game* irrInstance;
		static bool hasBeenRun;
		static const recti HUDBB;

		// INSTANCE DATA
		u32 curTick;
		u32 prevTick;
		GameState gameState;
		ViewMode viewMode;// whether the camera is overhead or *in* the character's head.
		// (Had to write these names out: no cheating and duplicating the short versions.)
		IrrlichtDevice* irrDevice;
		IVideoDriver* videoDriver;
		ISceneManager* sceneManager;
		irr::gui::IGUIEnvironment* guiEnvironment;
		cj::event::EventReceiver<Game>* eventReceiver;
		ICameraSceneNode* camera;
		// NB: Declared as a ptr to control destruction order.  I would still prefer to eliminate pointer vars as much as possible:
		AgentsList* const agentsList;
		//std::vector<Agent*> agentsList;
		WallsList* const wallsList;
		// FIXME:
		//ActionsList actionsList;
		PersistentActionsList persistentActionsList;
		Agent* pc;
		GameGUI* gameGUI;
		// (This is the little readout in the upper-left corner:)
		// TODO: Wrap this in a RAII class, store as pointer, &c., &c:
		IGUIStaticText* HUD;
		NavGraph* navgraph;

		// Used by run() & resume().
		void run();

		// "TICK" METHODS
		// id=tick
		// Internal util methods:
		void doTickKeyboardIO();

		// Executes Agent movements &c.:
		void doTickAgentsActions();

		// Updates readout of mouse coords in realtime, on each tick.
		//void runMouseOutputTick();

		// Updates Agents' coords in realtime in GameGUI list:
		//void runAgentsOutputTick();

		// If sensors are enabled, this handles them:
		//void runSensorOutputTick();
	};// Game
}// cj

#endif // guard

