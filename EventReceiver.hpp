// General custom event-handler class that logs mousePos position when the mousePos moves and keychar state when a key is pressed, in addition to providing a templated switch mechanism for handling GUI events which places some specific burdens of definition on the client but should allow inlining of the cases for efficiency.
// Note: For this to be properly effective, THandlerClient must implement something like that shown in class Basic, below,
// which he can then specialize on the template parameter for individual events.
// 	To be honest, I'm not sure whether there is really significant merit in this approach.  It doesn't save compiler time.  But I liked the idea of being able to build on the handler without modifying its file.  Maybe I'll think of something else at some point.
// TODO: I guess this would be a natural place for concept-checking...
// TODO: Currently it isn't possible to make extensions to the handlers without modifying the THandlerClient class; this may be able to be changed.
// -JJS, 20 Feb 2010

#ifndef __EVENTRECEIVER_HPP__
#define __EVENTRECEIVER_HPP__

#include <cassert>

#include <irrlicht.h>

//#include "IO.hpp"
#include "Coordinates.hpp"

namespace cj
{
	namespace event
	{
#ifndef SWIG
		using irr::SEvent;
		using irr::u32;
		using irr::KEY_KEY_CODES_COUNT;
#endif
		using namespace irr;
		using namespace irr::core;
		using namespace irr::gui;


		// Using this, default, type for the THandlerClient namespace (which is what it serves to be) limits EventHandler to tracking I/O.
		// TODO: IGUIElement& i/s/o pointer?
		class Basic
		{
		public:
			template <EGUI_EVENT_TYPE>
			inline static bool handleIt(IGUIElement*) { return false; }
		private:
			Basic();
		};// Basic

		//id=event
		// FIXME: THandlerClient -> TGUIHandler, so I can pass in GameGUI i/s/o Game??
		template <typename THandlerClient = cj::event::Basic>
		class EventReceiver : public irr::IEventReceiver // TODO: Singleton??
		{
		public:
			// Callback:
			//bool OnEvent(const SEvent& event);
			bool OnEvent(const SEvent& event)
			{
				bool trapped = false;

				switch( event.EventType )
				{
					case irr::EET_KEY_INPUT_EVENT:
						trapped = keyEvent( event );
					break;
					case irr::EET_MOUSE_INPUT_EVENT:
						trapped = mouseEvent( event );
					break;
					case irr::EET_GUI_EVENT:
						trapped = guiEvent( event );
					break;
					case irr::EET_JOYSTICK_INPUT_EVENT:
						// NOP
					break;
					case irr::EET_LOG_TEXT_EVENT:
						// NOP
					break;
					case irr::EET_USER_EVENT:
						// NOP
					break;
					case irr::EGUIET_FORCE_32_BIT:
						// NOP ⁅I don't even know what this does.⁆
					break;
				    	default:
						assert( false );// unreachable by a correct program
					break;
				}// sw

				return trapped;// (Means that the event is *not* trapped and will continue to percolate.
			}// OnEvent()

			// id=CTOR
			EventReceiver(const irr::video::IVideoDriver& d)
			: buckyShift(false), buckyCtrl(false),
			mousePos(d)// default
			{
				// TODO: Even if we insist on using a C-style array, I believe there is a terser initializer for this.
				for (u32 i=0; i<KEY_KEY_CODES_COUNT; ++i)
				{	KeyIsDown[i] = false;	}// for
			}// C

			// ACCESORS:
			bool isKeyPressed(irr::EKEY_CODE keyCode) const
			{ 	return KeyIsDown[keyCode]; }// isKeyPressed()
			const mouseVec& mouse() const
			{	return mousePos;	}// mouse()
			bool shiftPressed() const
			{	return buckyShift;	}// shift
			bool ctrlPressed() const
			{	return buckyCtrl;	}// shift
			// TODO: Alt ??

		protected:
			bool buckyShift;
			bool buckyCtrl;
			bool KeyIsDown[KEY_KEY_CODES_COUNT]; // TODO: std::vector or, even better, boost::array
			mouseVec mousePos;

			// Log key states.
			bool keyEvent( const SEvent& event )
			{
				// TODO: Eventually we may want to assign external handlers here, as with guiEvent.

				assert( event.EventType == irr::EET_KEY_INPUT_EVENT );
				// Store state of most-recently-activated key:
				KeyIsDown[event.KeyInput.Key] = event.KeyInput.PressedDown;
				// Store bucky bits:
				buckyShift = event.KeyInput.Shift;
				buckyCtrl = event.KeyInput.Control;

				return false;
			}// keyEvent()

			// Log mouse coords.  NB: These are relative to the window, i.e., to the upper-left corner!
			bool mouseEvent( const SEvent& event )
			{
				assert( event.EventType == irr::EET_MOUSE_INPUT_EVENT );
				if( event.MouseInput.Event == EMIE_MOUSE_MOVED )
				{
					mousePos.setX( event.MouseInput.X );
					mousePos.setY( event.MouseInput.Y );
				}// if

				return false;
			}// mouseEvent()

			bool guiEvent( const SEvent& event )
			{
				assert( event.EventType == irr::EET_GUI_EVENT );

				bool trapped = false;

				// The func-template calls here effectively allow the switch statement to be outsourced to a different file.
				switch( event.GUIEvent.EventType )
				{
  					case EGET_ELEMENT_FOCUS_LOST:
						trapped = THandlerClient::template handleIt<EGET_ELEMENT_FOCUS_LOST>(event.GUIEvent.Caller);
					break;
 					case EGET_ELEMENT_FOCUSED:
						trapped = THandlerClient::template handleIt<EGET_ELEMENT_FOCUSED>(event.GUIEvent.Caller);
					break;
 					case EGET_ELEMENT_HOVERED:
						trapped = THandlerClient::template handleIt<EGET_ELEMENT_HOVERED>(event.GUIEvent.Caller);
					break;
 					case EGET_ELEMENT_LEFT:
						trapped = THandlerClient::template handleIt<EGET_ELEMENT_LEFT>(event.GUIEvent.Caller);
					break;
  					case EGET_ELEMENT_CLOSED:
						trapped = THandlerClient::template handleIt<EGET_ELEMENT_CLOSED>(event.GUIEvent.Caller);
//dpr( "Close-event trapped? " << trapped );
					break;
 					case EGET_BUTTON_CLICKED:
						trapped = THandlerClient::template handleIt<EGET_BUTTON_CLICKED>(event.GUIEvent.Caller);
					break;
 					case EGET_SCROLL_BAR_CHANGED:
						trapped = THandlerClient::template handleIt<EGET_SCROLL_BAR_CHANGED>(event.GUIEvent.Caller);
					break;
 					case EGET_CHECKBOX_CHANGED:
						trapped = THandlerClient::template handleIt<EGET_CHECKBOX_CHANGED>(event.GUIEvent.Caller);
					break;
  					case EGET_LISTBOX_CHANGED:
						trapped = THandlerClient::template handleIt<EGET_LISTBOX_CHANGED>(event.GUIEvent.Caller);
					break;
 					case EGET_LISTBOX_SELECTED_AGAIN:
						trapped = THandlerClient::template handleIt<EGET_LISTBOX_SELECTED_AGAIN>(event.GUIEvent.Caller);
					break;
 					case EGET_FILE_SELECTED:
						trapped = THandlerClient::template handleIt<EGET_FILE_SELECTED>(event.GUIEvent.Caller);
					break;
 					case EGET_DIRECTORY_SELECTED:
						trapped = THandlerClient::template handleIt<EGET_DIRECTORY_SELECTED>(event.GUIEvent.Caller);
					break;
  					case EGET_FILE_CHOOSE_DIALOG_CANCELLED:
						trapped = THandlerClient::template handleIt<EGET_FILE_CHOOSE_DIALOG_CANCELLED>(event.GUIEvent.Caller);
					break;
 					case EGET_MESSAGEBOX_YES:
						trapped = THandlerClient::template handleIt<EGET_MESSAGEBOX_YES>(event.GUIEvent.Caller);
					break;
 					case EGET_MESSAGEBOX_NO:
						trapped = THandlerClient::template handleIt<EGET_MESSAGEBOX_NO>(event.GUIEvent.Caller);
					break;
 					case EGET_MESSAGEBOX_OK:
						trapped = THandlerClient::template handleIt<EGET_MESSAGEBOX_OK>(event.GUIEvent.Caller);
					break;
  					case EGET_MESSAGEBOX_CANCEL:
						trapped = THandlerClient::template handleIt<EGET_MESSAGEBOX_CANCEL>(event.GUIEvent.Caller);
					break;
 					case EGET_EDITBOX_ENTER:
						trapped = THandlerClient::template handleIt<EGET_EDITBOX_ENTER>(event.GUIEvent.Caller);
					break;
 					case EGET_EDITBOX_CHANGED:
						trapped = THandlerClient::template handleIt<EGET_EDITBOX_CHANGED>(event.GUIEvent.Caller);
					break;
 					case EGET_EDITBOX_MARKING_CHANGED:
						trapped = THandlerClient::template handleIt<EGET_EDITBOX_MARKING_CHANGED>(event.GUIEvent.Caller);
					break;
  					case EGET_TAB_CHANGED:
						trapped = THandlerClient::template handleIt<EGET_TAB_CHANGED>(event.GUIEvent.Caller);
					break;
 					case EGET_MENU_ITEM_SELECTED:
						trapped = THandlerClient::template handleIt<EGET_MENU_ITEM_SELECTED>(event.GUIEvent.Caller);
					break;
 					case EGET_COMBO_BOX_CHANGED:
						trapped = THandlerClient::template handleIt<EGET_COMBO_BOX_CHANGED>(event.GUIEvent.Caller);
					break;
 					case EGET_SPINBOX_CHANGED:
						trapped = THandlerClient::template handleIt<EGET_SPINBOX_CHANGED>(event.GUIEvent.Caller);
					break;

  					case EGET_TABLE_CHANGED:
						trapped = THandlerClient::template handleIt<EGET_TABLE_CHANGED>(event.GUIEvent.Caller);
					break;
 					case EGET_TABLE_HEADER_CHANGED:
						trapped = THandlerClient::template handleIt<EGET_TABLE_HEADER_CHANGED>(event.GUIEvent.Caller);
					break;
 					case EGET_TABLE_SELECTED_AGAIN:
						trapped = THandlerClient::template handleIt<EGET_TABLE_SELECTED_AGAIN>(event.GUIEvent.Caller);
					break;
 					case EGET_TREEVIEW_NODE_DESELECT:
						trapped = THandlerClient::template handleIt<EGET_TREEVIEW_NODE_DESELECT>(event.GUIEvent.Caller);
					break;

  					case EGET_TREEVIEW_NODE_SELECT:
						trapped = THandlerClient::template handleIt<EGET_TREEVIEW_NODE_SELECT>(event.GUIEvent.Caller);
					break;
 					case EGET_TREEVIEW_NODE_EXPAND:
						trapped = THandlerClient::template handleIt<EGET_TREEVIEW_NODE_EXPAND>(event.GUIEvent.Caller);
					break;
 					case EGET_TREEVIEW_NODE_COLLAPS: // sic
						trapped = THandlerClient::template handleIt<EGET_TREEVIEW_NODE_COLLAPS>(event.GUIEvent.Caller);
					break;
 					case EGET_COUNT:
						trapped = THandlerClient::template handleIt<EGET_COUNT>(event.GUIEvent.Caller);
					break;
					default:
						assert(false);// unreachable
					break;
				}// sw

				return trapped;
			}// guiEvent()
		};// EventReceiver

	}// event

}// cj

#endif // inc

