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

		// (Used by EventHandler:)
		template <EGUI_EVENT_TYPE>
		inline static bool handleIt(IGUIElement* const whateverElement) { return false; }// ⁅Yes, I know 'inline' is redundant.⁆

		// STATIC CONSTANT
		static const irr::core::dimension2d<u32> DEFAULT_RESOLUTION;

		// id=ctor
		Game();
		//Game( const irr::video::E_DRIVER_TYPE deviceType = irr::video::EDT_OPENGL,
			//const irr::core::dimension2d<u32>& windowSize = DEFAULT_RESOLUTION,
			//u32 bits = 16, bool fullscreen = false,
			//bool stencilbuffer = false,
			//bool vsync = false
		//);


		// id=DTOR, id=game-dtor
		virtual ~Game();

		enum GameState
		{
			STOP,
			RUN,
			BREAK,
			INTERRUPT
		};// enum

		enum ViewMode
		{
			BIRDS_EYE,
			FIRST_PERSON
		};// enum

		// ACCESSORS
		GameState getState() const
		{
			if( gameState == Game::STOP )
			{	return Game::STOP;	}// if
			else if( gameState == Game::BREAK )
			{	return Game::BREAK;	}// elif
			else if(device().isWindowActive())
			{	return Game::RUN;	}// elif
			else
			{	return Game::INTERRUPT;	}// else
		}// getState()

		// The keyboard shortcut is <C-d>.  This will end the program unless it is being embedded in a larger process; a Lisp session, for example.
		void breakToShell()
		{
dpr( "Breaking." );
			assert( hasBeenRun );
			assert( getState() == Game::RUN );
			assert( !device().getTimer()->isStopped() );
			gameState = Game::BREAK;
			device().getTimer()->stop();
		}// breakToShell()
		void resume()
		{
dpr( "Resuming (un-breaking?)..." );
			assert( hasBeenRun );
			assert( getState() == Game::BREAK );
			assert( device().getTimer()->isStopped() );
			//gameState = Game::RUN;
			device().getTimer()->start();

			run();
		}// resume()

		// id=mode, id=viewmode
		ViewMode getViewMode() const 
		{	
			assert( getIsPCSet() || viewMode != Game::FIRST_PERSON );
			return viewMode;	
		}// 
		void setViewMode( ViewMode mode ) 
		{	
			if( camera ) { camera->remove(); }// if
			switch( mode )
			{
				case Game::BIRDS_EYE:
					camera = smgr().addCameraSceneNode(0, vector3df(0,200,0), vector3df(0,0,0));
					break;
				case Game::FIRST_PERSON:
					assert( getIsPCSet() );
					camera = smgr().addCameraSceneNode( 0, vector3df(0,20,0) );
					//camera = smgr().addCameraSceneNode( &getPC().getBody(), vector3df(0,20,0), getPC().getBody().getRotation() );
					cam().bindTargetAndRotation(true);
					break;
			}// sw
			assert( camera );
			viewMode = mode;	
		}// setViewMode()

		IrrlichtDevice& device()
		{
			assert( irrDevice );
			return *irrDevice;
		}
		IVideoDriver& driver()
		{
			assert( videoDriver );
			return *videoDriver;
		 }
		ISceneManager& smgr()
		{
			assert( sceneManager );
			return *sceneManager;
		}
		irr::gui::IGUIEnvironment& guienv ()
		{
			assert( guiEnvironment );
			return *guiEnvironment;
		}

		const IrrlichtDevice& device() const
		{
			assert( irrDevice );
			return *irrDevice;
		}
		const IVideoDriver& driver() const
		{
			assert( videoDriver );
			return *videoDriver;
		}
		const ISceneManager& smgr() const
		{
			assert( sceneManager );
			return *sceneManager;
		}
		const irr::gui::IGUIEnvironment& guienv() const
		{
			assert( guiEnvironment );
			return *guiEnvironment;
		}


		//const AgentsList& agents() const;
		//AgentsList& agents();
		const AgentsList& agents() const
		{
			assert( agentsList );
			return *agentsList;
		}// agents()
		AgentsList& agents()
		{
			assert( agentsList );
			return *agentsList;
		}// agents()

		const WallsList& walls() const
		{
			assert( wallsList );
			return *wallsList;
		}// walls() walls()
		WallsList& walls()
		{
			assert( wallsList );
			return *wallsList;
		}// walls()

		const cj::event::EventReceiver<Game>& receiver() const
		{
			assert( eventReceiver );
			return *eventReceiver;
		}// receiver()
		// TODO: Make private?
		cj::event::EventReceiver<Game>& receiver()
		{
			assert( eventReceiver );
			return *eventReceiver;
		}// receiver()


		const ICameraSceneNode& cam() const
		{
			assert( camera );
			return *camera;
		}// cam()
		 ICameraSceneNode& cam()
		{
			assert( camera );
			return *camera;
		}// cam()


		 // TODO: Reenable if necess:
		//const ActionsList& getActionsList() const
		//{	return actionsList;	}//
		//ActionsList& getActionsList()
		//{	return actionsList;	}//
		const PersistentActionsList& getPersistentActionsList() const
		{	return persistentActionsList;	}//
		PersistentActionsList& getPersistentActionsList()
		{	return persistentActionsList;	}//

		// Is a PC currently defined?
		bool getIsPCSet() const
		{	return pc != NULL;	}

		const Agent& getPC() const
		{	return *pc;	}
		Agent& getPC()
		{	return *pc;	}

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
		// TODO: Const version.
		//AgentsList::const_iterator findAgent( const Agent& agent ) const;
		//AgentsList::iterator findAgent( const Agent& agent );

		//// Wrapper for the above.  May be a waste of space.
		//AgentsList::const_iterator findPC() const;
		//AgentsList::iterator findPC();
		//s32 getPCIndex() const;
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


		// TODO: Currently the ID parm is unused.
		Agent& addAgent(IAnimatedMesh* const mesh,
			ITexture* const texture,
			//const absVec& position = absVec(),
			const irr::core::vector3df& position = irr::core::vector3df(0,0,0),
			const irr::core::vector3df& rotation = irr::core::vector3df(0,0,0),
			const irr::core::vector3df& scale = irr::core::vector3df(1.0f, 1.0f, 1.0f),
			ISceneNode* const parent = 0,
			const s32 id = -1,
			bool alsoAddIfMeshPointerZero = false);
		// Idem, quickie lazy version:
		Agent& addAgent( const vector3df& position = vector3df() )
		{
			return addAgent( smgr().getMesh(DEFAULT_MESH.c_str()), driver().getTexture(DEFAULT_TEXTURE.c_str()), position, vector3df(0.0f,0.0f,0.0f), vector3df(1.0f, 1.0f, 1.0f) );
		}// addAgent()
		//Agent& addAgent( const absVec& position = absVec() )
		//{
			//// TODO: I guess I should put all that other stuff into static consts as well;
			//return addAgent( smgr().getMesh(DEFAULT_MESH.c_str()), driver().getTexture(DEFAULT_TEXTURE.c_str()), position, vector3df(0.0f,0.0f,0.0f), vector3df(1.0f, 1.0f, 1.0f) );
		//}// addAgent()

		// Removes & deallocates an Agent in O(n).  Do NOT reference an Agent not inscene!
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


		bool getGUIVisible() const
		{	return gui().getVisible();	}// getIsGUIVisible()

		void setGUIVisible( bool b = true )
		{	gui().setVisible(b);	}// setGUIVisible()

		const NavGraph& getNavGraph() const 
		{ 
			assert( navgraph );
			return *navgraph;
		}// getNavGraph()
		NavGraph& getNavGraph()
		{ 
			assert( navgraph );
			return *navgraph;
		}// getNavGraph()

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

