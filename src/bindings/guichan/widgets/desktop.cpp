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

#include <guichan/actionlistener.hpp>

#include "button.h"
#include "desktop.h"
#include "label.h"
#include "progressbar.h"
#include "window.h"

#include "../graphics.h"

#include "../dialogs/okdialog.h"
#include "../dialogs/setupdialog.h"

#include "../sdl/sdlgraphics.h"

#include "../../../core/configuration.h"
#include "../../../core/log.h"
#include "../../../core/resourcemanager.h"

#include "../../../core/image/image.h"
#include "../../../core/image/wallpapermanager.h"

#include "../../../core/utils/dtor.h"
#include "../../../core/utils/gettext.h"

State error = ERROR_STATE;

namespace
{
    struct SetupListener : public gcn::ActionListener
    {
        /**
         * Called when receiving actions from widget.
         */
        void action(const gcn::ActionEvent &event);
    } listener;


    struct ErrorListener : public gcn::ActionListener
    {
        void action(const gcn::ActionEvent &event)
        {
            state = error;
        }
    } errorListener;
}

Desktop::Desktop():
    Container()
{
    currentDialog = NULL;

    setWidth(config.getValue("screenwidth", defaultScreenWidth));
    setHeight(config.getValue("screenheight", defaultScreenHeight));
    setOpaque(false);

#ifdef PACKAGE_VERSION
    versionLabel = new Label(PACKAGE_VERSION);
    add(versionLabel, 2, 2);
#endif

    progressBar = new ProgressBar(0.0f, 100, 20, gcn::Color(168, 116, 31));
    progressBar->setVisible(false);
    progressBar->setPosition(5, getHeight() - 5 - progressBar->getHeight());
    progressBar->toggleThrobbing(true);

    progressLabel = new Label();
    progressLabel->setPosition(15 + progressBar->getWidth(), 4 +
                               progressBar->getY());

    setup = new Button(_("Setup"), "Setup", &listener);
    setup->setPosition(getWidth() - setup->getWidth() - 3, 3);

    // Set the most appropriate wallpaper, based on screen width
    Wallpaper::loadWallpapers();

    wallpaperName = Wallpaper::getWallpaper(getWidth(), getHeight());

    login_wallpaper = NULL;
    login_wallpaper = changeWallpaper(wallpaperName);

    if (!login_wallpaper)
        logger->log("Couldn't load %s as wallpaper", wallpaperName.c_str());

    add(progressBar);
    add(progressLabel);
    add(setup);
}

Desktop::~Desktop()
{
#ifdef PACKAGE_VERSION
    destroy(versionLabel);
#endif
    removeCurrentDialog();

    if (login_wallpaper)
        login_wallpaper->decRef();

    login_wallpaper = NULL;
}

void Desktop::resize()
{
    const int newScreenWidth = graphics->getWidth();
    const int newScreenHeight = graphics->getHeight();

#ifdef PACKAGE_VERSION
    versionLabel->adjustSize();
#endif
    progressLabel->adjustSize();
    setup->adjustSize();

    if (getWidth() != newScreenWidth || getHeight() != newScreenHeight)
    {
        setWidth(newScreenWidth);
        setHeight(newScreenHeight);

        std::string tempWallpaper = Wallpaper::getWallpaper(getWidth(),
                                                            getHeight());
        Image *temp = changeWallpaper(tempWallpaper);

        if (temp)
        {
            wallpaperName = tempWallpaper;
            login_wallpaper->decRef();
            login_wallpaper = temp;
        }

        progressBar->setPosition(5, getHeight() - 5 - 
                                 progressBar->getHeight());
        progressLabel->setPosition(15 + progressBar->getWidth(), 4 +
                                   progressBar->getY());

        if (currentDialog)
            positionDialog(currentDialog);
    }

    setup->setPosition(getWidth() - setup->getWidth() - 3, 3);
}

Image *Desktop::changeWallpaper(const std::string &wallpaper)
{
    ResourceManager *manager = ResourceManager::getInstance();
    SDLGraphics *g = dynamic_cast<SDLGraphics*>(graphics);
    Image *temp = NULL;

    if ((getWidth() != Wallpaper::getWidth(wallpaper) ||
         getHeight() != Wallpaper::getHeight(wallpaper)) && g)
    {
        const double aspectRatio = (double) Wallpaper::getWidth(wallpaper) /
                                   (double) Wallpaper::getHeight(wallpaper);
        const double newAspectRatio = (double) getWidth() /
                                      (double) getHeight();

        int width = getWidth();
        int height = getHeight();

        if (newAspectRatio > aspectRatio)
            width = (int) (height * aspectRatio);
        else if (aspectRatio > newAspectRatio)
            height = (int) (height / aspectRatio);

        temp = manager->getResizedImage(wallpaper, width, height);
    }
    else if (!login_wallpaper || wallpaper.compare(wallpaperName) != 0 ||
             (g && (getWidth() == Wallpaper::getWidth(wallpaper) &&
              getHeight() == Wallpaper::getHeight(wallpaper))))
    {
        temp = manager->getImage(wallpaper);
    }

    return temp;
}

void Desktop::reload()
{
    // Set the most appropriate wallpaper, based on screen width
    Wallpaper::loadWallpapers();

    resize();
}

void Desktop::changeCurrentDialog(Window* window)
{
    if (currentDialog)
        removeCurrentDialog();

    currentDialog = window;
    positionDialog(currentDialog);
}

void Desktop::showError(OkDialog* window, State errorState)
{
    error = errorState;
    currentDialog = window;
    currentDialog->addActionListener(&errorListener);
    currentDialog = NULL;
}

Window* Desktop::getCurrentDialog()
{
    return currentDialog;
}

void Desktop::removeCurrentDialog()
{
    destroy(currentDialog);
}

void Desktop::resetProgressBar()
{
    progressBar->reset();
    progressBar->setVisible(false);
    progressLabel->setCaption("");
}

void Desktop::useProgressBar(std::string message)
{
    removeCurrentDialog();
    progressLabel->setCaption(message);
    progressLabel->adjustSize();
    progressBar->reset();
    progressBar->setVisible(true);
}

void Desktop::positionDialog(Window *dialog, float xPos, float yPos)
{
    if (!dialog)
        return;

    const int width = std::min((int) (getWidth() * xPos), getWidth() -
                               dialog->getWidth());

    const int height = std::min((int) (getHeight() * yPos), getHeight() -
                                dialog->getHeight());

    dialog->setPosition(width, height);
    dialog->saveRelativeLocation(width, height);
    dialog->setVisible(true);
}

void Desktop::draw(gcn::Graphics *graphics)
{
    Graphics *g = static_cast<Graphics*>(graphics);

    if (!login_wallpaper || getWidth() > login_wallpaper->getWidth() ||
         getHeight() > login_wallpaper->getHeight())
    {
        g->setColor(gcn::Color(255, 255, 255));
        g->fillRectangle(gcn::Rectangle(0, 0, g->getWidth(), g->getHeight()));
    }

    if (login_wallpaper)
    {
        g->drawImage(login_wallpaper, (getWidth() - login_wallpaper->getWidth())
                     / 2, (getHeight() - login_wallpaper->getHeight()) / 2);
    }

    Container::draw(graphics);
}

void Desktop::logic()
{
    if (!isVisible())
        return;

    Container::logic();
}

void SetupListener::action(const gcn::ActionEvent &event)
{
    Window *window = NULL;

    if (event.getId() == "Setup")
        window = setupWindow;

    if (window)
    {
        window->setVisible(!window->isVisible());

        if (window->isVisible())
            window->requestMoveToTop();
    }
}
