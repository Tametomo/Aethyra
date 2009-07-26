/*
 *  Aethyra
 *  Copyright (C) 2004  The Mana World Development Team
 *  Copyright (C) 2009  Aethyra Development Team
 *
 *  This file is part of Aethyra derived from original code
 *  from The Mana World.
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef GUI_H
#define GUI_H

#include <guichan/gui.hpp>

#include <SDL/SDL_framerate.h>

#include <SDL_types.h>
#include <string>

#include "guichanfwd.h"

class Game;
class Graphics;
class GuiConfigListener;
class ImageSet;
class SDLInput;
class TrueTypeFont;

extern FPSmanager fpsm;

extern volatile int fps;
extern volatile int tick_time;
/**
 * \defgroup GUI Core GUI related classes (widgets)
 */

/**
 * \defgroup Interface User interface related classes (windows, dialogs)
 */

/**
 * Main GUI class.
 *
 * \ingroup GUI
 */
class Gui : public gcn::Gui
{
    public:
        /**
         * Constructor.
         */
        Gui(Graphics *screen);

        /**
         * Destructor.
         */
        ~Gui();

        /**
         * Performs logic of the GUI. Overridden to track mouse pointer
         * activity.
         */
        void logic();

        /**
         * Draws the whole Gui by calling draw functions down in the
         * Gui hierarchy. It also draws the mouse pointer.
         */
        void draw();

        /**
         * Return game font.
         */
        gcn::Font* getFont() const;

        /**
         * Return bolded game font.
         */
        gcn::Font* getBoldFont() const;

        /**
         * Return the Font used for "Info Particles", i.e. ones showing, what
         * you picked up, etc.
         */
        gcn::Font* getInfoParticleFont() const;

        /**
         * Return game font height.
         */
        const int getFontHeight() const;

        /**
         * Sets whether a custom cursor should be rendered.
         */
        void setUseCustomCursor(bool customCursor);

        /**
         * Sets which cursor should be used.
         */
        void setCursorType(int index) { mCursorType = index; }

        /**
         * Sets the opacity of the mouse cursor
         */
        void setMouseAlpha(float alpha) { mMaxMouseCursorAlpha = alpha; }

        /**
         * Frame rate has been changed. Adjust it accordingly.
         */
        void framerateChanged();

        /**
         * Change the size of the display.
         */
        void resize(const int &width, const int &height);

        /**
         * Returns mouse x in pixels.
         */
        const int &getMouseX() const { return mMouseX; }

        /**
         * Returns mouse y in pixels.
         */
        const int &getMouseY() const { return mMouseY; }

        /**
         * Returns the current mouse button state.
         */
        const Uint8 &getButtonState() const { return mButtonState; }

        /**
         * Cursors are in graphic order from left to right.
         * CURSOR_POINTER should be left untouched.
         * CURSOR_TOTAL should always be last.
         */
        enum {
            CURSOR_POINTER = 0,
            CURSOR_RESIZE_ACROSS,
            CURSOR_RESIZE_DOWN,
            CURSOR_RESIZE_DOWN_LEFT,
            CURSOR_RESIZE_DOWN_RIGHT,
            CURSOR_TOTAL
        };

        /**
         * Changes the in game font size.
         */
        void changeFontSize(const int &size);

    protected:
        void handleMouseMoved(const gcn::MouseInput &mouseInput);

    private:
        GuiConfigListener *mConfigListener;
        TrueTypeFont *mGuiFont;               /**< The global GUI font */
        TrueTypeFont *mInfoParticleFont;      /**< Font for Info Particles*/
        TrueTypeFont *mBoldFont;              /**< Font for bolded text*/
        bool mCustomCursor;                   /**< Show custom cursor */
        ImageSet *mMouseCursors;              /**< Mouse cursor images */
        float mMaxMouseCursorAlpha;           /**< Cursor opacity/transparency. */
        float mMouseCursorAlpha;              /**< Current cursor opacity/transparency.
                                                   Changes as the cursor fades
                                                   out due to inactivity. */
        int mMouseX;                          /**< Current mouse X position in
                                                   pixels. */
        int mMouseY;                          /**< Current mouse Y position in
                                                   pixels. */
        Uint8 mButtonState;                   /**< Current mouse button state */        
        int mMouseInactivityTimer;
        int mCursorType;

        /** Used to determine whether to draw the next frame. */
        int mDrawTime;

        /** The minimum frame time (used for frame limiting). */
        int mMinFrameTime;
};

/**
 * Returns elapsed time. (Warning: supposes the delay is always < 100 seconds)
 */
int get_elapsed_time(int start_time);

extern Gui *gui;                              /**< The GUI system */
extern SDLInput *guiInput;                    /**< GUI input */

#endif // GUI_H
