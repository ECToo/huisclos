#include "EventReceiver.hpp"
#include "GameGUI.hpp"
#include "Game.hpp"
#include "IO.hpp"
#include "Agent.hpp"
#include "Wall.hpp"
#include "SensorWin.hpp"
#include "ErrorWindow.hpp"
#include "Coordinates.hpp"

namespace cj
{
	namespace actions
	{
		void ActWriteMouseCoords::runTick()
		{
			std::wostringstream msg;
			msg << L"Mouse (X,Y)abs:  " << game.receiver().mouse();
			//msg << L"Mouse (X,Y)abs:  " << vector2df(game.receiver().mouse().x() - static_cast<float>(game.driver().getScreenSize().Width)/2.0, game.receiver().mouse().y() + static_cast<float>(game.driver().getScreenSize().Height)/2.0 );
			(game.gui().*getOutputWindow)().setText( msg.str().c_str() );
		}// run()

		void ActUpdateAgentsListBox::runTick()
		{
			// TODO: More elegant loop:
			for( u32 i=0; i < (game.gui().*getAgentsListBox)().getItemCount(); ++i )
			{
				std::wostringstream agentTxt;
				agentTxt << stringw((game.gui().*getAgentsListBox)().getListItem(i)).subString(0,4).c_str();
				//assert( agentTxt.size() > 0 );

				Agent& agent = game.agents().at(i);
				agentTxt << agent.getBody().getID() << L": " << agent.getAbsolutePosition() << L" @ " <<  rationalizeAngle( reorientGlobalAngleDammit( agent.getBody().getRotation().Y ) ) << L'°';
				//agentTxt << agent.getBody().getID() << L": " << transposeVectorCoordsDammit( agent.getBody().getAbsolutePosition() ).c_str() << L" @ " <<  rationalizeAngle( reorientGlobalAngleDammit( agent.getBody().getRotation().Y ) ) << L'°';
				(game.gui().*getAgentsListBox)().setItem(i, agentTxt.str().c_str(), -1 );
			}// for
		}// run()
	}// actions

	namespace gui
	{
		// This only has to hold out until a constructor is called:
		GameGUI* GameGUI::singleton = NULL;

		// GUI
		// id=ctor
		// id=gui-ctor
		GameGUI::GameGUI(Game& g)
		: game(g),
		// TODO: Would be wrap these in boost::scoped_ptr and skip this 'NULL' business.
		guiWindow(NULL),
		mousePositionBox(NULL),
		rangefinderButton(NULL),
		radarButton(NULL),
		activationButton(NULL),
		agentsListBox(NULL),
		setPCButton(NULL),
		addAgentButton(NULL),
		xBox(NULL),
		yBox(NULL),
		removeAgentButton(NULL),
		logWindow(NULL),
		rangefinderWindow(NULL),
		radarWindow(NULL),
		activationWindow(NULL),
		rangefinderButtonOn(false),
		radarButtonOn(false),
		activationButtonOn(false),
		writeMouseCoordsAction(game, &GameGUI::getMouseOutputBox ),
		updateAgentsListBoxAction(game, &GameGUI::getAgentsListBox)
		{
			// TODO: Provide a way to open the window after you close it.  Further, this ctor should build its agentsList from game.agents() if that is not empty.
			// Make sole instance accessible:
			singleton = this;

			// (Config:)
			static const s32 MIN_X = 10;
			static const s32 MIN_Y = 20;
			static const s32 MIN_SPACE = 10;
			static const s32 BIG_SPACE = 30;

			// id=window, id=guiwindow, id=gui
			guiWindow = game.guienv().addWindow(
				//recti(100 + Context.counter, 100 + Context.counter, 1000 + Context.counter, 1000 + Context.counter),
				recti(100 , 100 , 100 + WIDTH , 100 + HEIGHT ),
				false, // modal?
				L"Debug");

			// New window:
			static const recti mouseOutputLoc(MIN_X, MIN_Y,120,35);
			mousePositionBox = game.guienv().addStaticText(L"", mouseOutputLoc, true, false, guiWindow );
			assert( mousePositionBox );

			static const s32 BASIC_BUTTON_HEIGHT = 30;
			static const vector2di BUTTON_WIDTH_HEIGHT(65,BASIC_BUTTON_HEIGHT);
			static const position2di rangefinderButtonPos(150, MIN_Y);
			static const position2di radarButtonPos(rangefinderButtonPos.X + BUTTON_WIDTH_HEIGHT.X + MIN_SPACE, MIN_Y);
			static const position2di activationButtonPos(radarButtonPos.X + BUTTON_WIDTH_HEIGHT.X + MIN_SPACE, MIN_Y);
			//,240,110,240 + 32);
			//const recti rangefinderButtonPos(10,240,110,240 + 32);
			rangefinderButton = game.guienv().addButton( recti(rangefinderButtonPos.X, rangefinderButtonPos.Y, rangefinderButtonPos.X + BUTTON_WIDTH_HEIGHT.X, rangefinderButtonPos.Y + BUTTON_WIDTH_HEIGHT.Y), guiWindow, RANGEFINDER_BUTTON, L"Rangefinder", L"Activates rangefinder sensor.");
			assert( rangefinderButton  );
			rangefinderButton->setIsPushButton();

			radarButton = game.guienv().addButton( recti(radarButtonPos.X, radarButtonPos.Y, radarButtonPos.X + BUTTON_WIDTH_HEIGHT.X, radarButtonPos.Y + BUTTON_WIDTH_HEIGHT.Y), guiWindow, RADAR_BUTTON, L"Radar", L"Activates circular radar.");
			assert( radarButton   );
			radarButton->setIsPushButton();

			activationButton = game.guienv().addButton( recti(activationButtonPos.X, activationButtonPos.Y, activationButtonPos.X + BUTTON_WIDTH_HEIGHT.X, activationButtonPos.Y + BUTTON_WIDTH_HEIGHT.Y), guiWindow, ACTIVATION_BUTTON, L"Activation", L"Activation-level sensor.");
			assert( activationButton );
			activationButton ->setIsPushButton();

			// AGENTS MANAGER
			// id=agentslist, id=listbox
			static const vector2di LIST_SIZE(180, 200);
			static const vector2di LIST_POS(0,0);
			static const vector2di AGENTSLIST_POS( MIN_X, mouseOutputLoc.UpperLeftCorner.Y + mouseOutputLoc.getHeight() + BIG_SPACE );

			//const recti agentsListLabelBB(
			static const recti agentsListBB(AGENTSLIST_POS.X + LIST_POS.X,
					AGENTSLIST_POS.Y + LIST_POS.Y,
					AGENTSLIST_POS.X + LIST_POS.X + LIST_SIZE.X,
					AGENTSLIST_POS.Y + LIST_POS.Y + LIST_SIZE.Y);
			agentsListBox = game.guienv().addListBox( agentsListBB, guiWindow, AGENTS_LISTBOX );
			assert( agentsListBox );

			// Button to set|unset PC:
			static const recti setPCButtonBB( agentsListBB.LowerRightCorner.X + MIN_X, agentsListBB.UpperLeftCorner.Y, agentsListBB.LowerRightCorner.X + MIN_X + 70, agentsListBB.UpperLeftCorner.Y + BASIC_BUTTON_HEIGHT );
			setPCButton = game.guienv().addButton( setPCButtonBB, guiWindow, SET_PC_BUTTON, L"Set PC", L"Set PC (controllable agent) to currently-selected." );
			assert( setPCButton );
			// TODO: Change this so that PC status can be switched.
			setPCButton->setIsPushButton();

			// Button to create an Agent:
			static const recti addAgentButtonBB( setPCButtonBB.UpperLeftCorner.X,
					setPCButtonBB.LowerRightCorner.Y + BIG_SPACE,
					setPCButtonBB.LowerRightCorner.X + 50,
					setPCButtonBB.LowerRightCorner.Y + BASIC_BUTTON_HEIGHT + BIG_SPACE );
			addAgentButton = game.guienv().addButton( addAgentButtonBB, guiWindow, CREATE_AGENT_BUTTON, L"Add new Agent", L"Generate a new Agent at the coordinates specified below." );
			assert( addAgentButton );

			// Spinner boxes:
			static const recti addAgentXBB( addAgentButtonBB.UpperLeftCorner.X,
					addAgentButtonBB.LowerRightCorner.Y + MIN_SPACE,
					addAgentButtonBB.UpperLeftCorner.X + (addAgentButtonBB.getWidth() - MIN_SPACE)/2,
					addAgentButtonBB.LowerRightCorner.Y + MIN_SPACE + 20);
			static const recti addAgentYBB( addAgentXBB.LowerRightCorner.X + MIN_SPACE,
					addAgentXBB.UpperLeftCorner.Y,
					addAgentButtonBB.LowerRightCorner.X,
					addAgentXBB.LowerRightCorner.Y );

			xBox = game.guienv().addSpinBox( L"X", addAgentXBB, true, guiWindow, CREATE_AGENT_XBOX );
			assert( xBox );
			yBox = game.guienv().addSpinBox( L"Y", addAgentYBB, true, guiWindow, CREATE_AGENT_YBOX );
			assert( yBox );

			// Agent remover:

			static const recti removeAgentButtonBB( addAgentButtonBB.UpperLeftCorner.X,
					addAgentXBB.LowerRightCorner.Y + MIN_SPACE,
					addAgentButtonBB.LowerRightCorner.X,
					addAgentXBB.LowerRightCorner.Y + MIN_SPACE + BASIC_BUTTON_HEIGHT );
			removeAgentButton = game.guienv().addButton( removeAgentButtonBB, guiWindow, REMOVE_AGENT_BUTTON, L"Remove Agent", L"Remove currently-selected Agent." );
			assert( removeAgentButton );

			// Log window:
			// id=logwindow
			static const recti logWindowBB( removeAgentButtonBB.LowerRightCorner.X + BIG_SPACE,
					agentsListBB.UpperLeftCorner.Y,
					removeAgentButtonBB.LowerRightCorner.X + BIG_SPACE + agentsListBB.getWidth(),
					agentsListBB.LowerRightCorner.Y );
			logWindow = game.guienv().addListBox( logWindowBB, guiWindow, LOG_WINDOW );
			assert( logWindow );

			// Schedule HUD mouse output:
			game.getPersistentActionsList().queueAction( writeMouseCoordsAction );
			// Schedule list updater:
			game.getPersistentActionsList().queueAction( updateAgentsListBoxAction );
		}// ctor

		// DTOR
		// id=dtor>
		GameGUI::~GameGUI()
		{
			singleton = NULL; // completeness

			// De-schedule HUD output: Remove address of writeMouseCoordsAction from list:
			game.getPersistentActionsList().removeAction( writeMouseCoordsAction );
			// De-schedule list update:
			game.getPersistentActionsList().removeAction( updateAgentsListBoxAction );

			guiWindow->remove();
		}// ~GameGUI

		// id=handle-gui-button, id=button
		// \param button:
		// TODO: We have the same polymorphism problem here as with handleElementClosed(), below.
		// TODO: ErrorWindow when a sensor button clicked and no PC set.
		bool GameGUI::handleButtonClicked( IGUIElement* const button )
		{
			// FIXME: Fill out:
			switch( button->getID() )
			{
				case RANGEFINDER_BUTTON:
				{
					const bool newState = rangefinderButton->isPressed();// convenience
					if( game.getIsPCSet() )
					{
						assert( rangefinderButton->isPushButton() );
						assert( getRangefinder() != newState );
						game.setRangefinder(newState);
					}// if
					else
					{	rangefinderButton->setPressed( !newState );	}// else
				break;
				}// case
				case RADAR_BUTTON:
				{
					const bool newState = radarButton->isPressed();// convenience
					if( game.getIsPCSet() )
					{
						assert( radarButton->isPushButton() );
						assert( getRadar() != newState );
						game.setRadar(newState);
					}// if
					else
					{	radarButton->setPressed( !newState );	}// else
				break;
				}// case
				case ACTIVATION_BUTTON:
				{
					const bool newState = activationButton->isPressed();// convenience
					if( game.getIsPCSet() )
					{
						assert( activationButton->isPushButton() );
						assert( getActivation() != newState );
						game.setActivation(newState);
					}// if
					else
					{	activationButton->setPressed( !newState );	}// else
				break;
				}// case
				//// id=set-pc-button, id=pc
				case SET_PC_BUTTON:
					assert( setPCButton->isPushButton() );

					// no PC -> yes PC
					if( setPCButton->isPressed() )
					{
						if( agentsListBox->getItemCount() == 0 )
						{
							setPCButton->setPressed(false);// Don't forget!

							errorWindow( L"Create an Agent first, please." );
							logWindowMessage( L"* Err: Can't set PC, no Agents existent" );

							assert( !setPCButton->isPressed() );
						}// if
						else
						{
							s32 selection = agentsListBox->getSelected();

							// If only one Agent exists, target her automatically:
							if( agentsListBox->getItemCount() == 1 )
							{	selection = 0;	}// if

							// No agent selected: Error.
							if( selection == -1 )
							{
								setPCButton->setPressed(false);// Don't forget!
								errorWindow( L"Select an Agent from the list first. Please." );
								logWindowMessage( L"* Err: Can't set PC, none selected" );
								assert( !setPCButton->isPressed() );
							}// if
							else// SET PC
							{
								assert( !game.getIsPCSet() );
								// Handoff to Game:
								game.setPC( game.agents().at(selection) );
								assert( game.getIsPCSet() );
							}// else
						}// else
					}// if
					else // UNSET PC
					{
						game.unsetPC();
						assert( !game.getIsPCSet() );
					}// else
				break;
				case CREATE_AGENT_BUTTON:
				{
					// id=create-agent
					assert( agentsListBox->getItemCount() == game.agents().size() );// Both lists should be the same size.

					const vector2df position = getSpinBoxCoords();
					// TODO: Check range on coordinates.

					game.addAgent( game.smgr().getMesh(DEFAULT_MESH.c_str()), game.driver().getTexture(DEFAULT_TEXTURE.c_str()), absVec::from_vector2df(position).toIrr_vector3df(), vector3df(0.0f,0.0f,0.0f), vector3df(1.0f, 1.0f, 1.0f) );
					//game.addAgent( game.smgr().getMesh(DEFAULT_MESH.c_str()), game.driver().getTexture(DEFAULT_TEXTURE.c_str()), absVec::from_vector2df(position),[>TRANSPOSE_2D_COORDS_DAMMIT(position.X,position.Y),<] vector3df(0.0f,0.0f,0.0f), vector3df(1.0f, 1.0f, 1.0f) );
				break;
				}// case
				// id=remove-agent-button
				case REMOVE_AGENT_BUTTON:
				{
					if( agentsListBox->getItemCount() == 0 )
					{
						errorWindow( L"No Agent exists." );
						logWindowMessage( L"* Err: Can't delete agent; none exists" );
					}// if
					else
					{
						s32 selection = agentsListBox->getSelected();


						// Same as above.  If only one Agent exists, target her automatically:
						if( agentsListBox->getItemCount() == 1 )
						{	selection = 0;	}// if

						if( selection == -1 )// none selected
						{
							errorWindow( L"Select an agent from the list for deletion." );
							logWindowMessage( L"* Err: Can't delete agent; none selected" );
						}// if
						else
						{
							// TODO: Confirmation prompt??
							AgentsList::iterator it = game.agents().begin() + selection;

							//Agent& ag = *it;

							if( *it == game.getPC() )
							{
								game.unsetPC();
								//unsetPC( selection );
								assert( !game.getIsPCSet() );
							}// if


							// TODO: Consider an Agent::getID() wrapper for this:
							const u32 id = it->getBody().getID() ;

							// (Using this form skips the O(n) search:)
							game.removeAgent( it );
							//it = NULL;

							stringw msg = L"Agent ";
							msg += id;
							msg += " removed";
							logWindowMessage( msg );

							if( game.agents().empty() )
							{	logWindowMessage( L"And then there were none... ." );	}// if

						}// else
					}// else
				break;
				}// case
				default:
					assert( false ); // All buttons should be handled
				break;
			}// sw

			// Currently, no real need to trap events:
			return false;
		}// handleButtonClicked()

		// Only allow the element to close if it's an error popup.
		// TODO: Using a func-template case statement alternative, as we do with EGUI_EVENT_TYPE in EventReceiver.hpp, would require polymorphic switching on the IGUIElement*, which would in turn require modification of this interface and all the widget classes.
		// id=handle-element-closed
		bool GameGUI::handleElementClosed( IGUIElement* const window )
		{
dpr("Close-window event");
			if( window->getID() == ERROR_WINDOW )
			{
dpr("Closing error window");
				ErrorWindow::remove();
				//return false; // DON'T do this AND explicitly destroy the window!
			}// if
			//else
			//{	return true;	}// else
			return true;
		}// handleElementClosed()

		// id=error-window
		// Spawns a modal ErrorWindow at the mouse's current location.
		// Pop a complaint dialog.
		inline void GameGUI::errorWindow( const stringw& text )
		{	ErrorWindow::create( game.guienv(), vector2di(game.receiver().mouse().x(), game.receiver().mouse().y() ) , text, guiWindow, ERROR_WINDOW ); }// errorWindow()


		// Util method: adds agent to display list.  (No other internal state is changed).  TODO: Would be more elegant to make the list into its own class and put this thereinto.
		// id=add-to-agents-listbox
		void GameGUI::addToAgentsListBox( Agent& newagent )
		{
			std::wostringstream agentTxt;
			agentTxt << L"npc " <<
				newagent.getBody().getID() <<
				L": (X,Y)abs: "; //<< Moved to tick loop.
				//transposeVectorCoordsDammit( newagent->getBody().getAbsolutePosition() ).c_str();
			const u32 index = agentsListBox->addItem( agentTxt.str().c_str(), -1 /*TODO: Icon??*/ );
			assert( index == game.agents().size() - 1 );// Still the same size.
		}// void addToAgentsListBox( )

		// Util method: removes agent from debug list.
		void GameGUI::removeFromAgentsListBox( s32 index )
		{
			const u32 sz = agentsListBox->getItemCount(); // debug
			assert( sz == game.agents().size() + 1 );// (because that list has already been modified)
			assert( sz > 0 );
			assert( static_cast<u32>(index) < sz );

			agentsListBox->removeItem( index );
	//dpr( agentsListBox->getItemCount() );
	//dpr( sz );
	//dpr(game.agents().size() );
			assert( agentsListBox->getItemCount()  == sz - 1 );
			assert( agentsListBox->getItemCount() == game.agents().size() );
		}// removeFromAgentsListBox()

		void GameGUI::setPC( Agent& agent )
		{
			//const AgentsList::iterator it = game.findAgent(agent);
		       //const s32 index = game.findAgent(agent);
			assert( !game.agents().empty() );
			const s32 index = distance( game.agents().begin(), game.findAgent(agent) );
	//dpr( index );
			assert( index > -1 );
			//assert( it != game.agents().end() );

			assert( static_cast<u32>(index) < agentsListBox->getItemCount() );
			// Set 'PC' visual flag:
			stringw entry( agentsListBox->getListItem(index) );
			assert( entry[0] == L'n' );
			entry[0] = L'*';
			assert( entry[1] == L'p' );
			entry[1] = L'P';
			assert( entry[2] == L'c' );
			entry[2] = L'C';
			agentsListBox->setItem( static_cast<s32>(index), entry.c_str(), -1 );

			// TODO: This equality check seems as though it ought to work, but the assertion fails.  Isn't ==() defined for this type?
	//dpr( agentsListBox->getListItem(index) );
	//dpr( entry.c_str() );
			//assert( agentsListBox->getListItem(index) == entry.c_str() );


			stringw msg = L"Agent ";
			//msg += game.getPC().body()->getID(); // No goodif this hasn't been set yet!
			msg += agent.getBody().getID();
			msg += " designated PC.";
			logWindowMessage( msg );

			setPCButton->setPressed();
			assert( setPCButton->isPressed() );
		}// setPC()

		// Note: Moved into its own func bcs. may be used by <url:#r=remove-agent-button>.
		// Error if PC DNE!
		void GameGUI::unsetPC( const AgentsList::iterator& it )
		{
			assert( game.getIsPCSet() );
			assert( it != game.agents().end() );
			const s32 index = distance( game.agents().begin(), it);
			//const Agent* const pc = game.getPC();

			assert( index >= 0 );
			assert( static_cast<u32>(index) < game.agents().size() );

	//dpr( game.getPC() );
	//dpr( (find( game.agents().begin(), game.agents().end(), game.getPC() ) == game.agents().end()) );
	//dpr( index );
	//dpr( game.agents().size() );

			// Unset 'PC' visual flag:
			assert( static_cast<u32>(index) < agentsListBox->getItemCount() );
			stringw entry( agentsListBox->getListItem(index) );
			assert( entry[0] == L'*' );
			entry[0] = L'n';
			assert( entry[1] == L'P' );
			entry[1] = L'p';
			assert( entry[2] == L'C' );
			entry[2] = L'c';
			agentsListBox->setItem( static_cast<s32>(index), entry.c_str(), -1 );

			// TODO: Same as above: equality check seems as though it ought to work but doesn't.
			//assert( agentsListBox->getListItem(index) == entry.c_str() );


			// TODO: Add ID 'index' to message:
			logWindowMessage( L"PC unset." );

			//game.setPC(NULL);// do this LAST

			// Note: Depending on where this func was called from, the following may not be needed; but I think it's probably cheaper not even to check.
			setPCButton->setPressed( false );
			assert( !setPCButton->isPressed() );
		}// unsetPC()

		// TODO: 'inline' causes this to not link properly (on GCC, anyway):
		void GameGUI::unsetPC()
		{ 	unsetPC( game.findPC() ); }// unsetPC()

		void GameGUI::setRangefinder( const bool newState )
		{
dpr( "Setting rangefinder button to " << newState );
			// Location of the upper-left corner where the new output dialog will appear:
			const vector2di pos(getBB().UpperLeftCorner.X, getBB().LowerRightCorner.Y );

			if( getRangefinder() != newState )
			{
				// Turn on iff off:
				if( newState )
				{
					assert( rangefinderWindow == NULL );
dpr( "Creating rangefinder window" );
					rangefinderWindow = new RangefinderWin<Game>( game, game.guienv(), pos );
				}// if
				// Otherwise turn off:
				else
				{
					assert( rangefinderWindow != NULL );
					delete rangefinderWindow;
					rangefinderWindow = NULL;
				}// else

				rangefinderButton->setPressed( newState );
				rangefinderButtonOn = newState;
			}// if

			assert( getRangefinder() == newState);
		}// setRangefinder()

		void GameGUI::setRadar( const bool newState )
		{
			// Location of the upper-left corner where the new output dialog will appear:
			// TODO: Move:
			const s32 offset = 20;
			const vector2di pos(getBB().UpperLeftCorner.X + offset, getBB().LowerRightCorner.Y );

			if( getRadar() != newState )
			{
				// Turn on iff off:
				if( newState )
				{
					assert( radarWindow == NULL );
					radarWindow = new RadarWin<Game, Agent::ContactsList>(game, game.guienv(), pos );
				}// if
				// Otherwise turn off:
				else
				{
					assert( radarWindow != NULL );
					delete radarWindow;
					radarWindow = NULL;
				}// else

				radarButton->setPressed( newState );
				radarButtonOn = newState;
			}// if

			assert( getRadar() == newState);
		}// setRadar()
		void GameGUI::setActivation( const bool newState )
		{
			// Location of the upper-left corner where the new output dialog will appear:
			// TODO: Move:
			const s32 offset = 40;
			const vector2di pos(getBB().UpperLeftCorner.X + offset, getBB().LowerRightCorner.Y );

			if( getActivation() != newState )
			{
				// Turn on iff off:
				if( newState )
				{
					assert( activationWindow == NULL );
					activationWindow = new ActivationWin<Game>( game, game.guienv(), pos );
				}// if
				// Otherwise turn off:
				else
				{
					assert( activationWindow != NULL );
					delete activationWindow;
					activationWindow = NULL;
				}// else

				activationButton->setPressed( newState );
				activationButtonOn = newState;
			}// if

			assert( getActivation() == newState);
		}// setActivation()

		// Inline:
		void GameGUI::runAgentsListBoxTick()
		{
			// TODO: More elegant loop:
			for( u32 i=0; i < agentsListBox->getItemCount(); ++i )
			{
				std::wostringstream agentTxt;
				agentTxt << stringw(agentsListBox->getListItem(i)).subString(0,4).c_str();
				//assert( agentTxt.size() > 0 );

				Agent& agent = game.agents().at(i);
				agentTxt << agent.getBody().getID() << L": " << agent.getAbsolutePosition() << L" @ " <<  rationalizeAngle( reorientGlobalAngleDammit( agent.getBody().getRotation().Y ) ) << L'°';
				//agentTxt << agent.getBody().getID() << L": " << transposeVectorCoordsDammit( agent.getBody().getAbsolutePosition() ).c_str() << L" @ " <<  rationalizeAngle( reorientGlobalAngleDammit( agent.getBody().getRotation().Y ) ) << L'°';
				agentsListBox->setItem(i, agentTxt.str().c_str(), -1 );
			}// for
		}// runAgentsListBoxTick()
	}// gui
}// cj

