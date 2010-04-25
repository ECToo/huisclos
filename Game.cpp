// J. Jakes-Schauer
//<url:vimscript::!./make.bsh>

#include "IO.hpp"
#include "GameGUI.hpp"
#include "Game.hpp"
#include "Wall.hpp"
#include "ErrorWindow.hpp"
#include "Agent.hpp"
#include "Coordinates.hpp"
#include "Fairy.hpp"

#include <sstream> // wostringstream
#include <algorithm> // find()
#include <iterator> // distance()
#include <cassert> // theObvious()



namespace cj
{

GameException::GameException(const char *msg)
: description(msg)
{}

GameException::~GameException()
{}

const char *GameException::Message(void) const
{  return description;  }

//*************** GAME
//id=game

// STATIC INIT
const irr::core::dimension2d<u32> Game::DEFAULT_RESOLUTION = irr::core::dimension2d<u32>(1024,768);
Game* Game::irrInstance = NULL;
bool Game::hasBeenRun = false;
const recti Game::HUDBB(20, 20, 200, 100);// Coordinates for the HUD, the upper-left-corner

//std::list<vector3df> Game::AStar( const vector3df& start, const vector3df& dest, bool debug=true )
//{
//	return walls().first().AStar(start, dest, debug);
//}// AStar()

// id=ctor
// Note: Parms pinched from /usr/local/include/irrlicht.h .
// Note: Inlined b/c will only be called once.
// TODO: Move default dimensions into a const.
//Game::Game( const irr::video::E_DRIVER_TYPE deviceType,
	//const irr::core::dimension2d<u32>& windowSize,
	//u32 bits,
	//bool fullscreen,
	//bool stencilbuffer,
	//bool vsync
Game::Game()
: curTick(0), prevTick(0),
gameState( Game::STOP ),
viewMode( Game::BIRDS_EYE ),
irrDevice(NULL /*TODO: Move createirrDevice() here?*/ ),
videoDriver(NULL),
sceneManager(NULL),
guiEnvironment(NULL),
eventReceiver(NULL),
camera(NULL),
agentsList( new AgentsList ),// default ctor
wallsList( new WallsList ),// idem
//actionsList(), // TODO: Better initializers for these two?:
persistentActionsList(),
pc(NULL),
gameGUI(NULL),
HUD(NULL),
navgraph(NULL)
{
dpr( "* GAME CTOR" );

	if( irrInstance != NULL ) throw "** Error: Attempted 2nd instantiation of cj::Game, a singleton class.";

	// INSTANTIATE GAME:
	irrInstance = this;
	assert( irrInstance == this );

	irrDevice = irr::createDevice( irr::video::EDT_OPENGL, DEFAULT_RESOLUTION);//, bits, fullscreen, stencilbuffer, vsync/*, eventReceiver*/ );// handoff
	if( irrDevice == NULL ) {  throw GameException("ERROR: Device creation failed!");  }// if

	videoDriver = irrDevice->getVideoDriver();
	if( !videoDriver ) throw "*** Graphics initialization failure.";

	sceneManager = irrDevice->getSceneManager();
	if( !sceneManager ) throw "*** SceneManager initialization failure.";

	guiEnvironment = irrDevice->getGUIEnvironment();
	if( !guiEnvironment ) throw "*** GUI initialization failure.";

	// CREATE CAMERA
	setViewMode( BIRDS_EYE );
	//camera = smgr().addCameraSceneNode(0, vector3df(0,200,0), vector3df(0,0,0));
//camera->setPosition(core::vector3df(50,50,-60));
//camera->setTarget(core::vector3df(-70,30,-60));

	assert( eventReceiver == NULL );
	// INSTANTIATE EVENT HANDLER
	eventReceiver = new cj::event::EventReceiver<Game>( *videoDriver );
	assert( eventReceiver != NULL );
	irrDevice->setEventReceiver( eventReceiver );

	// Create GUI window:
	gameGUI = new GameGUI(*this);

	// Little display:
	HUD = guienv().addStaticText(L"", HUDBB );

	navgraph = new NavGraph(smgr(), driver());

	assert( irrInstance != NULL );
	assert( irrDevice != NULL );
	assert( videoDriver != NULL );
	assert( sceneManager != NULL );
	assert( guiEnvironment != NULL );
	assert( eventReceiver != NULL );
	assert( gameGUI != NULL );
	assert( navgraph != NULL );


	// TODO: Other initializations:
	driver().setTransform(ETS_WORLD, IdentityMatrix);

}// C

// id=DTOR, id=game-dtor
Game::~Game()
{
dpr( "* GAME DTOR " );
	assert( hasBeenRun );  // Otherwise, not much point in the whole enterprise...
	hasBeenRun = false; // in case we're running in the interpreted env and wish to try again

	unsetPC();

	// Delete members:
	delete agentsList;
	delete wallsList;
	//agentsList = NULL; // const.
	// Note: I'm not happy at not being able to use a smart pointer.

	assert( eventReceiver != NULL );
	delete eventReceiver;
	eventReceiver = NULL;

	// TODO: Wrap in auto_ptr ??
	assert( gameGUI != NULL );
	delete gameGUI;
	gameGUI = NULL;

	assert( navgraph != NULL );
	delete navgraph;
	navgraph = NULL;

	// End session:
	assert( irrDevice != NULL );
	irrDevice->drop();
	irrDevice = NULL;

	assert( irrInstance != NULL );
	irrInstance = NULL;

	HUD = NULL;
dpr( "* GAME DTOR end" );
}// ~

Game::GameState Game::getState() const
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

void Game::breakToShell()
{
dpr( "Breaking." );
	assert( hasBeenRun );
	assert( getState() == Game::RUN );
	assert( !device().getTimer()->isStopped() );
	gameState = Game::BREAK;
	device().getTimer()->stop();
}// breakToShell()
void Game::resume()
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
Game::ViewMode Game::getViewMode() const
{
	assert( getIsPCSet() || viewMode != Game::FIRST_PERSON );
	return viewMode;
}//
void Game::setViewMode( Game::ViewMode mode )
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

IrrlichtDevice& Game::device()
{
	assert( irrDevice );
	return *irrDevice;
}
IVideoDriver& Game::driver()
{
	assert( videoDriver );
	return *videoDriver;
 }
ISceneManager& Game::smgr()
{
	assert( sceneManager );
	return *sceneManager;
}
irr::gui::IGUIEnvironment& Game::guienv ()
{
	assert( guiEnvironment );
	return *guiEnvironment;
}

const IrrlichtDevice& Game::device() const
{
	assert( irrDevice );
	return *irrDevice;
}
const IVideoDriver& Game::driver() const
{
	assert( videoDriver );
	return *videoDriver;
}
const ISceneManager& Game::smgr() const
{
	assert( sceneManager );
	return *sceneManager;
}
const irr::gui::IGUIEnvironment& Game::guienv() const
{
	assert( guiEnvironment );
	return *guiEnvironment;
}


//const AgentsList& agents() const;
//AgentsList& agents();
const AgentsList& Game::agents() const
{
	assert( agentsList );
	return *agentsList;
}// agents()
AgentsList& Game::agents()
{
	assert( agentsList );
	return *agentsList;
}// agents()

const WallsList& Game::walls() const
{
	assert( wallsList );
	return *wallsList;
}// walls() walls()
WallsList& Game::walls()
{
	assert( wallsList );
	return *wallsList;
}// walls()

Wall const& Game::wall() const
{	return walls().front();	}// wall()
Wall& Game::wall()
{	return walls().front();	}// wall()

const cj::event::EventReceiver<Game>& Game::receiver() const
{
	assert( eventReceiver );
	return *eventReceiver;
}// receiver()
// TODO: Make private?
cj::event::EventReceiver<Game>& Game::receiver()
{
	assert( eventReceiver );
	return *eventReceiver;
}// receiver()


const ICameraSceneNode& Game::cam() const
{
	assert( camera );
	return *camera;
}// cam()
ICameraSceneNode& Game::cam()
{
	assert( camera );
	return *camera;
}// cam()

const PersistentActionsList& Game::getPersistentActionsList() const
{	return persistentActionsList;	}//
PersistentActionsList& Game::getPersistentActionsList()
{	return persistentActionsList;	}//

bool Game::getIsPCSet() const
{	return pc != NULL;	}

const Agent& Game::getPC() const
{	return *pc;	}
Agent& Game::getPC()
{	return *pc;	}

// id=setpc
void Game::setPC( Agent& agent )
{
	// First: if a PC is already selected, de-select her:
	if( getIsPCSet() )
	{
		// But if specified Agent is already the PC, don't bother.
		if( getPC() == agent )
		{	return; /* Cheat a bit*/	}// if

		// Else
		getPC().ClearCircle( agents().begin(), agents().end() );
		// TODO: Copy sensor states more elegantly:
		agent.setRangefinder( getPC().getRangefinder() );
		agent.setRadar( getPC().getRadar() );
		agent.setActivation( getPC().getActivation() );

		assert( gui().getRangefinder() == agent.getRangefinder() );
		assert( gui().getRadar() == agent.getRadar() );
		assert( gui().getActivation() == agent.getActivation() );

		// Current PC shut off.  This could change in future.
		getPC().allSensorsOff();
	}// if
	else// debug
	{
		assert( gui().getRangefinder() == false );
		assert( gui().getRadar() == false );
		assert( gui().getActivation() == false );
	}// else

	gui().setPC( agent );

	// NB: Do this LAST:
	pc = &agent;
//dpr( "PC set to " << agent );
	assert( getIsPCSet() );
	assert( getRangefinder() == gui().getRangefinder() );
	assert( getRadar() == gui().getRadar() );
	assert( getActivation() == gui().getActivation() );
}// setPC()

// id=unset
void Game::unsetPC()
{
	// Disable:
	//if( getIsPCSet() ) throw "*** Error: Cannot unsetPC() called when no PC exists.";

	if( getIsPCSet() )
	{
		// TODO: ClearCircle() really should be a Game or GameGUI method; the Agent method should be something like 'resetLighting()'.
		getPC().ClearCircle( agents().begin(), agents().end() );

		getPC().allSensorsOff();

		// GUI button & readout state:
		gui().unsetPC();
		gui().allSensorsOff();
	}// if
	// Else do nothing

	assert( gui().getRangefinder() == false );
	assert( gui().getRadar() == false );
	assert( gui().getActivation() == false );

	// Show sensor window if already enabled.  TODO: Ineffective at the moment, because unsetting the PC clears the debug state, but this could change.

	// NB: Do this LAST.
	pc = NULL;
	assert( !getIsPCSet() );
	assert( getRangefinder() == false );
	assert( getRadar() == false );
	assert( getActivation() == false );
}// unsetPC()

inline void Game::setRangefinder( bool mode )
{
	if( getIsPCSet() )
	{
//dpr( "Checking PC? " );
		getPC().setRangefinder(mode);
//dpr( "PC rangefinder " << getPC().getRangefinder() );
		if( mode == false ) { getPC().ClearCircle( agents().begin(), agents().end() ); }
		gui().setRangefinder(mode);
	}// if
	// Else no PC; moot; do nothing.

//dpr( "Rangefinder " << getRangefinder() );
//dpr( "GUI rangefinder " << gui().getRangefinder() );
	assert( getRangefinder() == gui().getRangefinder() );
	assert( getRadar() == gui().getRadar() );
	assert( getActivation() == gui().getActivation() );
}// setRangefinder()
inline void Game::setRadar( bool mode )
{
	if( getIsPCSet() )
	{
		getPC().setRadar(mode);
		if( mode == false ) { getPC().ClearCircle( agents().begin(), agents().end() ); }
		gui().setRadar(mode);
	}// if
	assert( getRangefinder() == gui().getRangefinder() );
dpr( "Radar: " << getRadar() );
dpr( "GUI Radar: " << gui().getRadar() );
dpr( "PC Radar " << getPC().getRadar() );
	assert( getRadar() == gui().getRadar() );
	assert( getActivation() == gui().getActivation() );
}// setRadar()
inline void Game::setActivation( bool mode )
{
	if( getIsPCSet() )
	{
		getPC().setActivation(mode);
		if( mode == false ) { getPC().ClearCircle( agents().begin(), agents().end() ); }
		gui().setActivation(mode);
	}// if
	assert( getRangefinder() == gui().getRangefinder() );
	assert( getRadar() == gui().getRadar() );
	assert( getActivation() == gui().getActivation() );
}// setActivation()

bool Game::getRangefinder() const
{	return getIsPCSet() && getPC().getRangefinder();	}// getRangefinder()
bool Game::getRadar() const
{	return getIsPCSet() && getPC().getRadar();	}// getRadar()
bool Game::getActivation() const
{	return getIsPCSet() && getPC().getActivation();	}// getActivation()

AgentsList::const_iterator Game::findAgent( const Agent& agent ) const
{	return find( agents().begin(), agents().end(), agent ); }// findAgent()
AgentsList::iterator Game::findAgent( const Agent& agent )
{	return find( agents().begin(), agents().end(), agent ); }// findAgent()

AgentsList::const_iterator Game::findPC() const
{
	assert( getIsPCSet() );
	return findAgent( getPC() );
}// getPCIndex()
// Idem, non-const
AgentsList::iterator Game::findPC()
{
	assert( getIsPCSet() );
	return findAgent( getPC() );
}// getPCIndex()

const GameGUI& Game::gui() const
{
	assert( gameGUI );
	return *gameGUI;
}// gui()
GameGUI& Game::gui()
{
	assert( gameGUI );
	return *gameGUI;
}// gui()

// id=addagent
Agent& Game::addAgent( Agent::MOB type,  const irr::core::vector3df position)
{
	Agent* newagent;

	switch(type)
	{
		case Agent::FAIRY:
			newagent = &addAgent( new Fairy(device(), position) );
		break;
		// TODO: Others
	}// sw

	// Output gameGUI: TODO: Perh. move this to GUI?
	std::wostringstream msg( L"Agent ");
	msg << newagent->getBody().getID() << " created at " << position;
	gui().logWindowMessage( stringw( msg.str().c_str() ) );// TODO: Ugly set of conversions.

	return *newagent;
}// addAgent

Agent& Game::addAgent( Agent* const newagent )
{
dpr( "Game::addAgent" );
	const u32 ncount = agents().size();// debug only

	// TODO: Destruct these at end of program, via ~Game().
	//Agent* const pNewagent = new Agent( device(), "faerie.md2", "Faerie5.BMP", "", position); // Handoff, but push ISceneManager ref onto list.

	assert( newagent );
	//if( newagent == NULL ) throw "*** Err: Heap creation of new Agent failed: possible deficiency of memory??";

	// Add to list:
	agents().push_back( newagent );
	assert( agents().size() == ncount + 1 );

	// Add to GUI:
	gui().addToAgentsListBox( *newagent );

	return *newagent;
}// addAgent()


Agent& Game::addAgent( const vector3df& position )
{
	return addAgent( Agent::FAIRY, position );
	//return addAgent( smgr().getMesh(DEFAULT_MESH.c_str()), driver().getTexture(DEFAULT_TEXTURE.c_str()), position, vector3df(0.0f,0.0f,0.0f), vector3df(1.0f, 1.0f, 1.0f) );
}// addAgent()


// Deprecated:
//Agent& Game::addAgent(IAnimatedMesh* const mesh,
	//ITexture* const texture,
	//const core::vector3df& position,
	////const absVec& position,
	//const core::vector3df& rotation,
	//const core::vector3df& scale,
	//ISceneNode* const parent,
	//const s32 id,
	//bool alsoAddIfMeshPointerZero
//) {
//dpr( "Game::addAgent" );
	//const u32 ncount = agents().size();// debug only

	//// TODO: Destruct these at end of program, via ~Game().
	//Agent* const pNewagent = new Agent( device(), "faerie.md2", "Faerie5.BMP", "", position); // Handoff, but push ISceneManager ref onto list.

	//assert( pNewagent );
	////if( pNewagent == NULL ) throw "*** Err: Heap creation of new Agent failed: possible deficiency of memory??";

	//// Add to list:
	//agents().push_back( pNewagent );
	//assert( agents().size() == ncount + 1 );

	//// Add to GUI:
	//gui().addToAgentsListBox( *pNewagent );


	//// Output gameGUI: TODO: Perh. move this to GUI?
	//std::wostringstream msg( L"Agent ");
	//msg << pNewagent->getBody().getID() << " created at " << position;
	//gui().logWindowMessage( stringw( msg.str().c_str() ) );// TODO: Ugly set of conversions.

	//assert( pNewagent );
	//return *pNewagent;
//}// addAgent()

// TODO: removeAgent() and removeWall(), both forms, are nearly identical and could be refactored.
// id=remove-agent
// This version is called if a search has to be conducted.
void Game::removeAgent( Agent& agent )
{
	assert( !agents().empty() );
	// TODO: const?  Other iterator type?
	const AgentsList::iterator it = std::find( agents().begin(), agents().end(), agent );
	assert( *it == agent ); // This would mean trouble: if somehow she didn't get on the list.

	// Delegate:
	removeAgent(it);
}// removeAgent()
// The short version: an iterator specifies the location.
void Game::removeAgent( const AgentsList::iterator& it )
{
	assert( !agents().empty() );
	assert( it != agents().end() );

	agents().erase( it );

	gui().removeFromAgentsListBox( distance(agents().begin(), it) );
}// removeAgent()


// id=wall, id=add-wall, // id=addwall
Wall& Game::addWall()
{
	// TODO: Don't hardcode texture name here
	Wall* const wl = new Wall( &device(), "t351sml.jpg") ;
	//Wall* const wl = new Wall( &device(), "t351sml.jpg", position.toIrr_vector3df()) ;
	assert(wl);
	wallsList->push_back( wl );
	//wl->makeWall(length, width);

	return *wl;
}// addWall()
//Wall& Game::addWall( u32 length, u32 width, const absVec& position )
//{
	//// TODO: Don't hardcode texture name here
	//Wall* const wl = new Wall( &device(), "t351sml.jpg", position.toIrr_vector3df()) ;
	//assert(wl);
	//wallsList->push_back( wl );
	//wl->makeWall(length, width);
	////wl->makeWall(length, width, position.toIrr_vector3df());

	//VectorList expandedGeometryPoints = wl->expandGeometry( Agent::DEFAULT_GIRTH / 2.0 );

	////VectorList::iterator prevIt = expandedGeometryPoints.end();
	////PathNode* prevnode = NULL;
	//for( VectorList::iterator it = expandedGeometryPoints.begin(); it != expandedGeometryPoints.end(); ++it )
	//{
		//PathNode& newnode = navgraph->addNode( *it );
		//// FIXME: parameterize:
		//newnode.setVisible(true);

		//// FIXME: Adding edges; keep first node, bind each node to the previous, &c.  Bind last to first.
		////if( prevnode )// skip first iteration
		////{	navgraph->addEdge( *prevnode, newnode );	}// if
		////prevnode = &newnode;
		////if( prevIt == expandedGeometryPoints.end())
		////{	prevIt = expandedGeometryPoints.begin(); }// if
		////else
		////{	navgraph->addEdge( *prevIt, *it );	}// else
	//}// for
	////navgraph->addEdge( *prevnode, *expandedGeometryPoints.begin() );
	////navgraph->addEdge( *prevIt, *expandedGeometryPoints.begin() );

	//return *wl;
//}// addWall()

//inline Wall& Game::addWall( const absVec& position )
//{	return addWall(1, 1, position);	}// addWall()

void Game::removeWall( Wall& wall )
{
	assert( !walls().empty() );
	// TODO: const?  Other iterator type?
	const WallsList::iterator it = std::find( walls().begin(), walls().end(), wall );
	assert( *it == wall ); // This would mean trouble: if somehow she didn't get on the list.

	// Delegate:
	removeWall(it);
}// removeWall()
void Game::removeWall( const WallsList::iterator& it )
{
	assert( !walls().empty() );
	assert( it != walls().end() );

	walls().erase( it );

	// TODO: Do I need a list of walls?
	//gui().removeFromAgentsListBox( distance(agents().begin(), it) );
}// removeWall()




//*** MAIN LOOP:
// id=run
void Game::run()
{
//dpr( getState() );
//dpr( device().isWindowActive() );
dpr("* MAIN GAME LOOP...");
	gameState = Game::RUN;

	while( device().run() && !device().isWindowActive() ); // Messy; gives the game a chance to get focus.
	while( (getState() != Game::STOP && getState() != Game::BREAK) && device().run() )
	{
		if( !irrDevice ) {	throw "*** Where did Irrlicht go?";	}// if

		if( getState() == Game::INTERRUPT )// Focus has been lost.
		{
//dpr( "Yielding." );
			device().yield();
		}// if
		else // do a tick.
		{
			assert( !device().getTimer()->isStopped() );

			// Test for <C-d>, the Break interrupt.
			if( getBreakKeypress() )
			{	breakToShell();	}// if
			else
			{
				// "debounce" safeguard:
				static bool release;
				// Test for <C-g>, which shows|hides GUI:
				if( !getToggleGUIKeypress() )
				{	release = true;	}// if
				else
				{
					if( release && getToggleGUIKeypress() )
					{
dpr( "Setting GUI visibility to " << !getGUIVisible() );
						setGUIVisible( !getGUIVisible() );
						release = false;
					}// if
				}// if
				//if( release && !getToggleGUIKeypress() )
				//{	release = false;	}
//dpr("Tick");
				// Calculate time-slicing:
				// id=time
				prevTick = curTick;
				curTick = device().getTimer()->getTime();
				static const SColor initColors(255,120,102,136); // TODO: Allow parameterization--if I ever know exactly what these parameters do.
				driver().beginScene(true, true, initColors);

				doTickKeyboardIO();// Schedules movement actions for the PC.

				// Draw 3D stuff.
				smgr().drawAll();

				// AI routines: TODO: (wrapper method)
				for( AgentsList::iterator it = agents().begin(); it != agents().end(); ++it )
				{ 	it->updateSensors( agents().begin(), agents().end() ); }// for

				doTickAgentsActions();

				// SENSORS &c. UPDATING GUI.
				// Draw sensor gfx: comes AFTER the 3D scenery is rendered.
				getPersistentActionsList().runTick();

				// Finally, refresh GUI:
				guienv().drawAll();

				// This comes LAST:
				driver().endScene();
			}// else
		}// else
	}// while
dpr( "... Exiting loop." );
}// run()

bool Game::getGUIVisible() const
{	return gui().getVisible();	}// getIsGUIVisible()

void Game::setGUIVisible( bool b  )
{	gui().setVisible(b);	}// setGUIVisible()

//const NavGraph& Game::getNavGraph() const
//{
	//assert( navgraph );
	//return *navgraph;
//}// getNavGraph()
//NavGraph& Game::getNavGraph()
//{
	//assert( navgraph );
	//return *navgraph;
//}// getNavGraph()

// id=start
void Game::start()
{
dpr( "* Initializing." );
	assert( !hasBeenRun );
	hasBeenRun = true;
	//gameState = Game::RUN;

	// Initialize time memo:
	prevTick = device().getTimer()->getTime();

	run();
}// start()

// TODO: Put on heap; pass byref w/ a smart-ptr to avoid copying?
vector<f32> Game::drawFeelers()
{
	assert( getIsPCSet() );
	// FIXME: 'debug' parm
	return getPC().DrawFeelers( true );
}// drawFeelers()

vector<pointOfInterest> Game::drawCircle()
{
	assert( getIsPCSet() );
	// FIXME: 'debug' parm
	return getPC().DrawCircle( agents().begin(), agents().end(), true );
}// drawCircle()

vector<f32> Game::drawPieSlices()
{
	assert( getIsPCSet() );
	// FIXME: 'debug' parm
	return getPC().DrawPieSlices( agents().begin(), agents().end(), true );
}// drawPieSlices()

const vector<f32>& Game::getRangefinderOutput() const
{
	assert( getIsPCSet() );
	return getPC().getRangefinderOutput();
}// getRangefinderOutput()

// Show|hide GUI: <C-g>
bool Game::getToggleGUIKeypress() const
{	return receiver().isKeyPressed(irr::KEY_KEY_G) && receiver().ctrlPressed();	}// getToggleGUIKeypress()

// BREAK: <C-d>
bool Game::getBreakKeypress() const
{	return receiver().isKeyPressed(irr::KEY_KEY_D) && receiver().ctrlPressed();	}// getBreakKeypress()

// TODO: If not too expensive, rewrite the controls as using Agent#turn() and Agent#move() action commands.
// TODO: Every so often the turning-angle goes out of [0,360) and throws; I don't know what the deal is.
// id=keyboard, id=key
void Game::doTickKeyboardIO()
{
	if( getIsPCSet() )
	{
		//if( receiver().isKeyPressed(irr::KEY_KEY_D) || receiver().isKeyPressed(irr::KEY_KEY_A) || receiver().isKeyPressed(irr::KEY_KEY_W) || receiver().isKeyPressed(irr::KEY_KEY_S) || receiver().isKeyPressed(irr::KEY_UP) || receiver().isKeyPressed(irr::KEY_DOWN) || receiver().isKeyPressed(irr::KEY_RIGHT) || receiver().isKeyPressed(irr::KEY_LEFT))
		{
			//** FIXME: Rewrite into an Agent# method.  Not exactly easy.
//wcout << "Agent moving." << std::endl;
			//************* MOVEMENT
			// Decouple FPS from movement:
			const u32 frameDeltaTime_ms = curTick - prevTick;
			const f32 frameDeltaTime = static_cast<f32>(frameDeltaTime_ms) / 1000.00; // Time in seconds

			vector3df translation;
			f32 rotation = 0.00;
			//assert( translation == vector3df(0,0,0) );

			//** TODO: Instead of checking keys on every loop, use keypress|release events to start|stop movement.

			// "Accelerator" key pressed?a:
			f32 move_speed = (receiver().shiftPressed() ) ? Agent::MOVEMENT_SLOW : Agent::MOVEMENT_FAST ;
			f32 turn_speed_degrees = (receiver().shiftPressed() ? Agent::TURN_SLOW : Agent::TURN_FAST);


			// ********* RELATIVE MOTION
			if( receiver().isKeyPressed(irr::KEY_KEY_D) && !receiver().isKeyPressed(irr::KEY_KEY_A)  ) // Turn right
			{
				rotation += turn_speed_degrees*frameDeltaTime;
			}// if
			else if( receiver().isKeyPressed(irr::KEY_KEY_A) && !receiver().isKeyPressed(irr::KEY_KEY_D) ) // Turn left
			{
				rotation -= turn_speed_degrees*frameDeltaTime;
			}// eif

			if( receiver().isKeyPressed(irr::KEY_KEY_W) && !receiver().isKeyPressed(irr::KEY_KEY_S) ) // Forward
			{
				translation += move_speed * frameDeltaTime * getPC().getBody().getRotation().rotationToDirection( vector3df(1,0,0) );
			}// if
			else if( receiver().isKeyPressed(irr::KEY_KEY_S) && !receiver().isKeyPressed(irr::KEY_KEY_W) ) // Back
			{
				translation -= move_speed * frameDeltaTime * getPC().getBody().getRotation().rotationToDirection(vector3df(1,0,0) ); // Note: I know I could just use a (-1,0,0) base vector, but for consistency I'll stick to -=().
			}// eif

			// ********* ABSOLUTE
			// Up||down, absolute:
			if( receiver().isKeyPressed(irr::KEY_UP) && !receiver().isKeyPressed(irr::KEY_DOWN) ) // Up
			{	translation.X += move_speed * frameDeltaTime;	}// if
			else if( receiver().isKeyPressed(irr::KEY_DOWN) && !receiver().isKeyPressed(irr::KEY_UP) ) // Down
			{	translation.X -= move_speed * frameDeltaTime;	}// eif

			// Right||left, absolute.  NB: Z axis seems to be backwards!
			if( receiver().isKeyPressed(irr::KEY_RIGHT) && !receiver().isKeyPressed(irr::KEY_LEFT) ) // Right
			{	translation.Z -= move_speed * frameDeltaTime;	}// if
			else if( receiver().isKeyPressed(irr::KEY_LEFT) && !receiver().isKeyPressed(irr::KEY_RIGHT) ) // Left
			{	translation.Z += move_speed * frameDeltaTime;	}// eif



			// SET POSITION
			const vector3df newpos = getPC().getBody().getPosition() + translation; // debug

			// <TAG> JS (2)
			// getPC().getBody().setPosition(newpos);
    // assert( getPC().getBody().getPosition() == newpos );

    // <TAG> CA (2)
    bool success = getPC().MoveVector(translation);
    if(success) {} // Get rid of warning. Eventually, success needs to be used for something.

			//static const vector3df cameraOffset( 0.0, 20.0, 20.0 );
			//cam().setPosition( getPC().getBody().getPosition() + cameraOffset );

			//cam().setTarget(core::vector3df(-70,30,-60));
			//cam().setTarget( 100.0 * getPC().getBody().getRotation().rotationToDirection( vector3df(1,0,0) ) );
					//
			//getPC().addTickTranslation( newpos );

			// Move camera, too:
//			cam().setPosition( vector3df(newpos.X,cam().getPosition().Y, newpos.Z) );
//			cam().setTarget( newpos );

			vector3df newangle_v = getPC().getBody().getRotation() + vector3df(0,rotation,0);

			newangle_v.Y = rationalizeAngle(newangle_v.Y);
			assert( newangle_v.Y < 360.0f && newangle_v.Y >= 0.0f );

			// SET ROTATION
			getPC().getBody().setRotation( newangle_v );
			// TODO: Absolute_rotation
			//getPC().addTickRotation( newangle_v );
			//getActionsList().push_back( new AgentRotation(getPC(), newangle_v) );
			assert( getPC().getBody().getRotation() == newangle_v );

			getPC().getBody().updateAbsolutePosition();

			// id=camera-update
			if( getViewMode() == Game::FIRST_PERSON )
			{
				cam().setPosition( getPC().getBody().getAbsolutePosition() + (30.0 * getPC().getBody().getAbsoluteTransformation().getRotationDegrees().rotationToDirection( vector3df(1,0,0) )) );
				//cam().setPosition( getPC().getBody().getPosition() + (30.0 * getPC().getBody().getRotation().rotationToDirection( vector3df(1,0,0) )) );
				cam().setRotation( getPC().getBody().getRotation() );
				//cam().setRotation( getPC().getBody().getAbsoluteTransformation().getRotationDegrees() );
			}// if


			// FIXME: Move to GUI section:
			// Output debug coords:
			std::wostringstream coords;
			coords << L"PC " <<
			    getPC().getBody().getID() <<
			    L" (X,Y)rel " <<
			    getPC().getAbsolutePosition() <<
			    //transposeVectorCoordsDammit( getPC().getBody().getAbsolutePosition() ).c_str() <<
			    std::endl;
			// FIXME: Re-orient the angle.
			// TODO: getAbsoluteTransformation() instead?
			coords << L"Θrel " << rationalizeAngle( reorientGlobalAngleDammit( getPC().getBody().getRotation().Y ) ) << L'°';
			HUD->setText( coords.str().c_str() );
		}// if
	}// if
}// doTickKeyboardIO()

// id=Agent
void Game::doTickAgentsActions()
{
	for( AgentsList::iterator it = agents().begin(); it != agents().end(); ++it )
	{
		if( !getIsPCSet() || (getPC() != *it) )
		{
			// id=FSM tick:
			if( it->getState() == Agent::DEAD )
			{
				it->animationDie();
				// TODO: Play dead.
			}// if
			else
			{
				if(  it->getState() == Agent::MANUAL )
				{	/*Continue to doTickActions()*/	}// if
				else// FSM
				{
					vector<Agent*> agentsSeen = it->getVisibleAgents( agents() );
					//vector<Agent*> agentsSeen = it->getVisibleAgents( agents().begin(), agents().end() );

					if( it->getState() == Agent::ATTACK )
					{
						if( it->getAttackTarget() == NULL )//
						{
							if( agentsSeen.empty() ) // transition to MOVE.
							//if( !it->isEnemyVisible() )
							{
dpr( "Agent " << it->getID() << " ATTACK -> MOVE." );
								it->setState( Agent::MOVE );
							}// if
							else // pick new target
							{
dpr( "Agent " << it->getID() << " doing attack." );
								const u32 random = 0; //TODO: : rand ∈ [0,agentsSeen.size())
								//it->getNearbyRandomEnemy()
								it->Attack( *(agentsSeen.front() + random) );
							}// else
						}// if
						// else continue attacking
					}// if
					else if( it->getState() == Agent::MOVE )
					{
						// If nobody around, wander to a random point on the map.
						if( agentsSeen.empty() )
						//if( !it->isEnemyVisible() )
						{
							if( !it->getHasMoveTarget() )
							{
//dpr( "Agent " << it->getID() << " wandering." );
//dpr( walls().size() );
								//it->Seek( wall().getRandomNodePosition(), wall() );
								//assert( it->getHasMoveTarget() );

							}// if
							// Continue to doTickActions()
						}// if
						else // transition to ATTACK.
						{
dpr( "Agent " << it->getID() << " MOVE -> ATTACK." );
							it->setState( Agent::ATTACK ); }// else
					}// elif MOVE state
					else
					{	assert(false); /* Invalid state*/ }// else
				}// if not MANUAL

				it->doTickActions(static_cast<f32>(curTick - prevTick) / 1000.f); // NEXT ACTION
			}// elsif not dead
		}// if not PC
	}// for
}// doTickAgentsActions()

//// Handoff to GUI.
//inline void Game::runAgentsOutputTick()
//{
	//gui().runAgentsListBoxTick();
//}// runAgentsOutputTick()

//// FIXME:
//void Game::runSensorOutputTick()
//{
	//// TODO: sensorWindow accessors

	//if( getIsPCSet() )
	//{
		//// FIXME:
	//}// if
//}// runSensorOutputTick()

//// TODO: Put on heap; pass byref w/ a smart-ptr to avoid copying?
//inline vector<f32> Game::drawFeelers()
//{
	//assert( getIsPCSet() );
	//// FIXME: 'debug' parm
	//return getPC().DrawFeelers( true );
//}// drawFeelers()

//inline vector<pointOfInterest> Game::drawCircle()
//{
	//assert( getIsPCSet() );
	//// FIXME: 'debug' parm
	//return getPC().DrawCircle( agents().begin(), agents().end(), true );
//}// drawCircle()

//inline vector<f32> Game::drawPieSlices()
//{
	//assert( getIsPCSet() );
	//// FIXME: 'debug' parm
	//return getPC().DrawPieSlices( agents().begin(), agents().end(), true );
//}// drawPieSlices()


// Template spec for handling GUI buttons:
template <>
inline bool Game::handleIt<EGET_BUTTON_CLICKED>(IGUIElement* const button)
{
	// <url:GameGUI.cpp#r=handle-gui-button>
	return GameGUI::instance().handleButtonClicked(button);
}// handleIt()
// Template spec for disabling widget-closing signals:
template <>
inline bool Game::handleIt<EGET_ELEMENT_CLOSED>(IGUIElement* const window)
{
dpr( "handleIt() close" );
	// <url:GameGUI.cpp#r=handle-element-closed>
	return GameGUI::instance().handleElementClosed(window);
}// handleIt()


}// cj




