#ifndef __GAMEGUI_HPP__
#define __GAMEGUI_HPP__

#include "boost/utility.hpp"
#include "boost/ptr_container/ptr_vector.hpp"
#include <irrlicht.h>
#include "Agent.hpp"
#include "Sensor.hpp"
#include "SensorWin.hpp"
#include "Action.hpp"

namespace cj
{
	class Game; // fwd dec
	namespace gui {class GameGUI;}
	class Agent;
	typedef boost::ptr_vector<Agent> AgentsList;

	namespace actions
	{
		class ActWriteMouseCoords : public IPersistentAction
		{
		public:
			typedef IGUIStaticText& (cj::gui::GameGUI::* outputWindowCallback)(void);

			ActWriteMouseCoords( Game& gameObj, const outputWindowCallback& box_callb )
			//ActWriteMouseCoords( Game& gameObj, IGUIStaticText& (cj::gui::GameGUI::*box_callb)(void) ) 
			: game(gameObj), getOutputWindow(box_callb)
			{}// ctor

			// id=dtor
			virtual ~ActWriteMouseCoords() {}

			virtual void runTick();
		private:
			Game& game;
			const outputWindowCallback getOutputWindow;
		};// ActWriteMouseCoords

		class ActUpdateAgentsListBox : public IPersistentAction
		{
		public:
			typedef IGUIListBox& (cj::gui::GameGUI::* agentsListCallback)(void);

			ActUpdateAgentsListBox( Game& gameObj, const agentsListCallback& box_callb ): game(gameObj), getAgentsListBox(box_callb)
			{}// ctor

			// dtor
			virtual ~ActUpdateAgentsListBox() {}

			virtual void runTick();
		private:
			Game& game;
			const agentsListCallback getAgentsListBox;
		};// ActUpdateAgentsListBox
	}// actions


	namespace gui
	{
		using namespace irr;
#ifndef SWIG
		using namespace irr::gui;
		using namespace irr::core;
		using namespace cj::sensor;
#endif



		//************** GAMEGUI
		// id=gui, id=gamegui
		//template <typename TGame>
		class GameGUI : public boost::noncopyable, public sensor::ISensors
		{
		public:
			static const u32 WIDTH = 550;
			static const u32 HEIGHT = 300;

			static GameGUI& instance()
			{
				assert( singleton );
				return *singleton;
			}// instance()


			// Unique IDs assigned to all GUI elements created; each, therefore, must be a singleton type.
			enum ELEMENT_ID
			{
				RANGEFINDER_BUTTON = 101,// (copied from tutorial)
				RADAR_BUTTON,
				ACTIVATION_BUTTON,
				SET_PC_BUTTON,
				AGENTS_LISTBOX,
				CREATE_AGENT_BUTTON,
				CREATE_AGENT_XBOX,
				CREATE_AGENT_YBOX,
				REMOVE_AGENT_BUTTON,
				LOG_WINDOW,

				ERROR_WINDOW // Technically it's detached, but so?
			};// ELEMENT

			// CTOR: Initialize all widget ptrs.
			// id=gui-ctor
			// <url:Game.cpp#r=gui-ctor>
			GameGUI(Game& g);

			// id=gui-dtor
			// <url:Game.cpp#r=gui-dtor>
			virtual ~GameGUI();

			// Get the rectangular coordinates representing the GUI window's size & location.
			// TODO: If I create a base for the various widgets, move this thereinto:
			recti getBB() const
			{	return guiWindow->getRelativePosition();	}// getBB()

			// Button-press handler:
			// <url:Game.cpp#r=handle-gui-button>
			bool handleButtonClicked( IGUIElement* const button );

			// Someone-tried-to-close-a-window handler:
			bool handleElementClosed( IGUIElement* const window );

			// TODO: Make private?  (Game needs to reach it, currently, in addAgent().)
			void logWindowMessage( const stringw text )
			{	logWindow->addItem( text.c_str() ); }// logMessage()

			const IGUIStaticText& getMouseOutputBox() const { return *mousePositionBox; }
			IGUIStaticText& getMouseOutputBox() { return *mousePositionBox; }

			const IGUIListBox& getAgentsListBox() const { return *agentsListBox; }
			IGUIListBox& getAgentsListBox() { return *agentsListBox; }

			// (Used by Game.)
			void addToAgentsListBox( Agent& newagent );
			// TODO: version that takes an Agent&, rather than an index?
			void removeFromAgentsListBox( s32 index );
			//void outputMouseCoords( const vector2df& pos );
			void runAgentsListBoxTick();
			void setPC( Agent& agent );
			// Removes the PC by *internal position.*  [Should be private, really.]
			void unsetPC( const AgentsList::iterator& it );
			// Idem, but lazy version, used by Game:
			void unsetPC();

			bool getVisible() const
			{	return guiWindow->isVisible();	}// getIsVisible()
			// TODO: This should affect the sensor output windows, too.
			void setVisible( bool b = true )
			{	guiWindow->setVisible(b);	}// setVisible()

			virtual void setRangefinder( const bool on=true );
			virtual void setRadar( const bool on=true );
			virtual void setActivation( const bool on=true );

			// As above, these deal only with the visual aspect; the Game counterparts are the ones that do the real work.
			virtual bool getRangefinder() const
			{	return rangefinderButtonOn;	}//
			virtual bool getRadar() const
			{	return radarButtonOn;	}//
			virtual bool getActivation() const
			{	return activationButtonOn;	}//
		private:
			static GameGUI* singleton;

			Game& game;

			// Widgets:
			IGUIWindow* guiWindow;// (The main, parent IGUIElement.  Almost all of the others are sub-elements.)
			IGUIStaticText* mousePositionBox;
			IGUIButton* rangefinderButton;
			IGUIButton* radarButton;
			IGUIButton* activationButton;
			IGUIListBox* agentsListBox;
			IGUIButton* setPCButton;
			IGUIButton* addAgentButton;
			IGUISpinBox* xBox;
			IGUISpinBox* yBox;
			IGUIButton* removeAgentButton;
			IGUIListBox* logWindow;

			// Child debug-output windows.  Note that these are not children of the guiWindow.
			RangefinderWin<Game>* rangefinderWindow;
			RadarWin<Game, Agent::ContactsList>* radarWindow;
			ActivationWin<Game>* activationWindow;

			vector2df getSpinBoxCoords() const
			{	return vector2df( xBox->getValue(), yBox->getValue() ); }// getSpinBoxCoords()

			// Creates a carping window that the user must deal with:
			void errorWindow( const stringw& text );

			// Track the states of the buttons in between the time that they are clicked and the time that the button-click is processed, that is, after the event is caught.
			bool rangefinderButtonOn;
			bool radarButtonOn;
			bool activationButtonOn;

			// HUD updater:
			ActWriteMouseCoords writeMouseCoordsAction;
			// agentsListBox updater:
			ActUpdateAgentsListBox updateAgentsListBoxAction;
		};// GameGUI
	}// gui
}// cj


#endif// inc

