#ifndef __ERRORWINDOW_HPP__
#define __ERRORWINDOW_HPP__

#include <cassert>

#include "boost/utility.hpp" // noncopyable
#include <irrlicht.h>
#include "IO.hpp"
//#include "Agent.hpp"

namespace cj
{
	namespace gui
	{
		using namespace irr;
		using namespace irr::core;
		using namespace irr::gui;

		class ErrorWindow : boost::noncopyable
		{
		public:
			// The window should close if and only if its button is clicked.  Therefore the dtor here must be controlled; therefore the ctor must be controlled.
			static void create( IGUIEnvironment& g, const vector2di& position, const stringw& text, IGUIElement* const p, const s32 ID )
			{
dpr("ErrorWindow::create");
				assert( !singleton );
				singleton = new ErrorWindow( g, position, text, p, ID ); // handoff
			}// create()
			static void remove()
			{
				assert( singleton );
				delete singleton;
				singleton = NULL;
dpr("ErrorWindow::remove");
			}// create()

		private:
			static const dimension2d<u32> SIZE;

			ErrorWindow( IGUIEnvironment& g, const vector2di& position, const stringw& text, IGUIElement* const p, const s32 ID )
			:guienv(g), 
			errorText(text),  
			parent(p),
			window( guienv.addWindow( recti( position.X, position.Y, position.X + SIZE.Width, position.Y + SIZE.Height),
				true, // holds up the program until dismissed
				L"Error",
				0, // FIXME: Is this parm inappropriate to use?
				ID)
			){
dpr( "Errorwin c'd" );
				// Error text:
				// TODO: Make textbox bigger.
				guienv.addStaticText( text.c_str(),
					recti(35,35,160,60),
					false, // border?
					true, // wordwrap?
					window);

				// 'OK' button:
				// TODO: Button that loses window:
				//game.guienv().addButton( recti(0, 0, EGDS_BUTTON_WIDTH, EGDS_BUTTON_HEIGHT), window, [> TODO: fix this<] -1, EGDT_MSG_BOX_OK, EGDT_WINDOW_CLOSE );
			}// ctor

			// Dtor
			~ErrorWindow()
			{ 	
				window->remove(); 
dpr( "Errorwin d'd" );
			}// dtor


			static ErrorWindow* singleton;

			IGUIEnvironment& guienv;
			const stringw errorText;
			IGUIElement* const parent;
			IGUIWindow* const window;

		};// ErrorWindow
	}// gui
}// cj

#endif// inc

