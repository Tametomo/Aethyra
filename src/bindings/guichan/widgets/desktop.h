/*
 *  Aethyra
 *  Copyright (C) 2009  Aethyra Development Team
 *
 *  This file is part of Aethyra.
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

#ifndef DESKTOP_H
#define DESKTOP_H

#include <string>

#include "container.h"

#include "../guichanfwd.h"

class Window;
class Image;
class OkDialog;
class ProgressBar;

/**
 * A desktop for the game instance. Used when outside of the game.
 */
class Desktop : public Container
{
    public:
        /**
         * Constructor.
         */
        Desktop();
        ~Desktop();

        /**
         * Resizes the desktop's wallpaper.
         */
        void resize();

        /**
         * Reloads the wallpapers, then resizes the desktop as needed.
         */
        void reload();

        /**
         * Changes the state's current dialog to the supplied dialog.
         */
        void changeCurrentDialog(Window* window);

        /**
         * Shows an error dialog. The state variable is used to determine what
         * state the client should switch to after the error.
         */
        void showError(OkDialog* window, unsigned char state);

        /**
         * Returns the current dialog for the current state.
         */
        Window* getCurrentDialog();

        /**
         * Deletes the state's current dialog.
         */
        void removeCurrentDialog();

        /**
         * Resets the progress bar on the desktop.
         */
        void resetProgressBar();

        /**
         * Use the progress bar on the desktop with the given label message.
         */
        void useProgressBar(std::string message);

        /**
         * Draws the desktop, and all children widgets on top of it.
         */
        void draw(gcn::Graphics *graphics);

        /**
         * Logic (updates components' size and infos)
         */
        void logic();

        /**
         * Changes the wallpaper the desktop uses
         */
        Image *changeWallpaper(const std::string &wallpaper);

    private:
        /**
         * Positions the given dialog according to the Desktop widget's
         * positioning rules.
         *
         * \param dialog The dialog to position.
         * \param xPos Scale value in between 0-1 for the relative X coordinate
         *             position. 0.5, for example, would center the dialog
         *             horizontally.
         * \param yPos Scale value in between 0-1 for the relative Y coordinate
         *             position. 0.5, for example, would center the dialog
         *             vertically.
         */
        void positionDialog(Window *dialog, float xPos = 0.625f,
                            float yPos = 0.625f);

        ProgressBar *progressBar;
        gcn::Label *progressLabel;
        gcn::Button *setup;
        gcn::Label *versionLabel;
        Window *currentDialog;

        std::string wallpaperName;
        Image *login_wallpaper;
};

extern Desktop *desktop;

#endif
