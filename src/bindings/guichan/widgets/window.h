/*
 *  Aethyra
 *  Copyright (C) 2004  The Mana World Development Team
 *
 *  This file is part of Aethyra based on original code
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

#ifndef WINDOW_H
#define WINDOW_H

#include <guichan/widgetlistener.hpp>

#include <guichan/widgets/window.hpp>

#include "../graphics.h"
#include "../guichanfwd.h"

class ContainerPlacer;
class Layout;
class LayoutCell;
class ResizeGrip;
class Skin;
class SkinLoader;
class WindowContainer;

/**
 * A window. This window can be dragged around and has a title bar. Windows are
 * invisible by default.
 *
 * \ingroup GUI
 */
class Window : public gcn::Window, gcn::WidgetListener
{
    public:
        /**
         * Constructor. Initializes the title to the given text and hooks
         * itself into the window container.
         *
         * @param caption The initial window title, "Window" by default.
         * @param modal   Block input to other windows.
         * @param parent  The parent window. This is the window standing above
         *                this one in the window hiearchy. When reordering,
         *                a window will never go below its parent window.
         * @param skin    The location where the window's skin XML can be found.
         * @param visible Whether the window is visible by default.
         */
        Window(const std::string &caption = "Window", bool modal = false,
               Window *parent = NULL, const std::string &skin = "graphics/gui/gui.xml",
               bool visible = false);

        /**
         * Destructor. Deletes all the added widgets.
         */
        ~Window();

        /**
         * Sets the window container to be used by new windows.
         */
        static void setWindowContainer(WindowContainer *windowContainer);

        /**
         * Draws the window.
         */
        void draw(gcn::Graphics *graphics);

        /**
         * Sets the size of this window.
         */
        void setContentSize(int width, int height);

        /**
         * Sets the location relative to the given widget.
         */
        void setLocationRelativeTo(gcn::Widget *widget);

        /**
         * Sets the location relative to the given enumerated position.
         */
        void setLocationRelativeTo(ImageRect::ImagePosition position,
                                   const int &offsetX = 0,
                                   const int &offsetY = 0);

        /**
         * Sets the location relative to the given enumerated position.
         */
        void setLocationRelativeTo(ImageRect::ImagePosition position,
                                   const int &width, const int &height,
                                   const int &offsetX, const int &offsetY);

        /**
         * Converts a given x and y coordinate to a relative coordinate.
         * Currently used to get coordinates which can be adapted to any screen
         * resolution.
         */
        void saveRelativeLocation(const int &x, const int &y);

        /**
         * Gets the offsets to use for a particular position in comparison to
         * the window resolution.
         */
        void getRelativeOffset(ImageRect::ImagePosition position, int &x, int &y,
                               const int& width, const int& height);

        /**
         * Gets the offsets to use for a particular position in comparison to
         * a supplied width and height.
         */
        void getRelativeOffset(ImageRect::ImagePosition position, int &x, int &y,
                               const int &conWidth, const int &conHeight,
                               const int& width, const int& height);

        /**
         * Sets whether or not the window can be resized.
         */
        void setResizable(bool resize);

        /**
         * Called whenever the widget changes size.
         */
        void widgetResized(const gcn::Event &event);

        /**
         * Sets whether or not the window has a close button.
         */
        void setCloseButton(bool flag);

        /**
         * Returns whether the window can be resized.
         */
        bool isResizable();

        /**
         * Sets the minimum width of the window.
         */
        void setMinWidth(int width);

        /**
         * Sets the minimum height of the window.
         */
        void setMinHeight(int height);

        /**
         * Sets the maximum width of the window.
         */
        void setMaxWidth(int width);

        /**
         * Sets the minimum height of the window.
         */
        void setMaxHeight(int height);

        /**
         * Gets the minimum width of the window.
         */
        int getMinWidth() const { return mMinWinWidth; }

        /**
         * Gets the minimum height of the window.
         */
        int getMinHeight() const { return mMinWinHeight; }

        /**
         * Gets the maximum width of the window.
         */
        int getMaxWidth() const { return mMaxWinWidth; }

        /**
         * Gets the minimum height of the window.
         */
        int getMaxHeight() const { return mMaxWinHeight; }

        /**
         * Sets flag to show a title or not.
         */
        void setShowTitle(bool flag) { mShowTitle = flag; }

        /**
         * Returns the parent window.
         *
         * @return The parent window or <code>NULL</code> if there is none.
         */
        Window *getParentWindow() const { return mParent; }

        /**
         * Schedule this window for deletion. It will be deleted at the start
         * of the next logic update.
         */
        void scheduleDelete();

        /**
         * Starts window resizing when appropriate.
         */
        void mousePressed(gcn::MouseEvent &event);

        /**
         * Implements window resizing and makes sure the window is not
         * dragged/resized outside of the screen.
         */
        virtual void mouseDragged(gcn::MouseEvent &event);

        /**
         * Implements custom cursor image changing context, based on mouse
         * relative position.
         */
        virtual void mouseMoved(gcn::MouseEvent &event);

        /**
         * When the mouse button has been let go, this ensures that the mouse
         * custom cursor is restored back to it's standard image.
         */
        virtual void mouseReleased(gcn::MouseEvent &event);

        /**
         * When the mouse leaves the window this ensures that the custom cursor
         * is restored back to it's standard image.
         */
        virtual void mouseExited(gcn::MouseEvent &event);

        /**
         * Sets the name of the window. This is not the window title.
         */
        void setWindowName(const std::string &name) { mWindowName = name; }

        /**
         * Returns the name of the window. This is not the window title.
         */
        const std::string& getWindowName() { return mWindowName; }

        /**
         * Reads the position (and the size for resizable windows) in the
         * configuration based on the given string.
         * Uses the default values when config values are missing.
         * Don't forget to set these default values and resizable before
         * calling this function.
         */
        void loadWindowState();

        /**
         * Saves the window state so that when the window is reloaded, it'll
         * maintain its previous state and location.
         */
        void saveWindowState();

        /**
         * Hides/unhides a window
         */
        virtual void hide();

        /**
         * Allow for overriding the setVisible function. Not used for the Window
         * class, but used in a few windows that use this widget.
         */
        virtual void setVisible(bool visible) { gcn::Window::setVisible(visible); }

        /**
         * Set the default win pos and size.
         * (which can be different of the actual ones.)
         */
        void setDefaultSize(int defaultX, int defaultY,
                            int defaultWidth, int defaultHeight);

        /**
         * Set the default win pos and size.
         * (which can be different of the actual ones.)
         * This version of setDefaultSize sets the window's position based
         * on a relative enumerated position, rather than a coordinate position.
         */
        void setDefaultSize(int defaultWidth, int defaultHeight,
                            ImageRect::ImagePosition position,
                            int offsetx = 0, int offsetY = 0);

        /**
         * Reset the win pos and size to default. Don't forget to set defaults
         * first.
         */
        virtual void resetToDefaultSize();

        /**
         * Reset the win pos and size according to change of size.
         */
        void adaptToNewSize();

        /**
         * Gets the layout handler for this window.
         */
        Layout &getLayout();

        /**
         * Computes the position of the widgets according to the current
         * layout. Resizes the window so that the layout fits. Deletes the
         * layout.
         * @param w if non-zero, force the window to this width.
         * @param h if non-zero, force the window to this height.
         * @note This function is meant to be called with fixed-size windows.
         */
        void reflowLayout(int w = 0, int h = 0);

        /**
         * Adds a widget to the window and sets it at given cell.
         */
        LayoutCell &place(int x, int y, gcn::Widget *, int w = 1, int h = 1);

        /**
         * Returns a proxy for adding widgets in an inner table of the layout.
         */
        ContainerPlacer getPlacer(int x, int y);

        /**
         * Overrideable functionality for when the window is to close. This
         * allows for class implementations to clean up or do certain actions
         * on window close they couldn't do otherwise.
         */
        virtual void close();

        /**
         * Gets the alpha value used by the window, in a GUIChan usable format.
         */
        int getGuiAlpha();

    protected:
        bool mOldVisibility;          /**< Whether the window was previously
                                           viewable before a hide */

        Skin* mSkin;                  /**< Skin in use by this window */
    private:
        enum ResizeHandles
        {
            TOP    = 0x01,
            RIGHT  = 0x02,
            BOTTOM = 0x04,
            LEFT   = 0x08
        };

        /**
         * Determines if the mouse is in a resize area and returns appropriate
         * resize handles. Also initializes drag offset in case the resize
         * grip is used.
         *
         * @see ResizeHandles
         */
        int getResizeHandles(gcn::MouseEvent &event);

        ResizeGrip *mGrip;            /**< Resize grip */
        Window *mParent;              /**< The parent window */
        Layout *mLayout;              /**< Layout handler */
        std::string mWindowName;      /**< Name of the window */
        std::string mDefaultSkinPath; /**< Default skin path for this window */
        bool mShowTitle;              /**< Window has a title bar */
        bool mModal;                  /**< Window is modal */
        bool mCloseButton;            /**< Window has a close button */
        bool mDefaultVisible;         /**< Default visibility */
        int mMinWinWidth;             /**< Minimum window width */
        int mMinWinHeight;            /**< Minimum window height */
        int mMaxWinWidth;             /**< Maximum window width */
        int mMaxWinHeight;            /**< Maximum window height */
        int mDefaultX;                /**< Default window X position */
        int mDefaultY;                /**< Default window Y position */
        int mDefaultWidth;            /**< Default window width */
        int mDefaultHeight;           /**< Default window height */

        // Window "sector" system for placing windows. This provides a best
        // effort adaptation of a user's window locations on window resize.
        // Only needed when loading a window's initial location, saving it, or
        // changing resolutions.
        ImageRect::ImagePosition mPosition; /**< Window "sector" location */
        int mOffsetX ;                      /**< X offset from the "sector" location */
        int mOffsetY;                       /**< Y offset from the "sector" location */

        // Default Window "sector" location
        ImageRect::ImagePosition mDefaultPosition;
        // Default X offset from the "sector" location
        int mDefaultOffsetX;
        // Default Y offset from the "sector" location
        int mDefaultOffsetY;

        static int mouseResize;       /**< Active resize handles */
        static int instances;         /**< Number of Window instances */

        /**
         * The width of the resize border. Is independent of the actual window
         * border width, and determines mostly the size of the corner area
         * where two borders are moved at the same time.
         */
        static const int resizeBorderWidth = 10;
};

#endif
