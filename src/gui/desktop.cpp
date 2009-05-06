/*
 *  Aethyra
 *  Copyright (C) 2004  The Mana World Development Team
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

#include "desktop.h"
#include "okdialog.h"
#include "setup.h"

#include "../configuration.h"
#include "../log.h"
#include "../main.h"

#include "../bindings/guichan/widgets/button.h"
#include "../bindings/guichan/widgets/label.h"
#include "../bindings/guichan/widgets/progressbar.h"
#include "../bindings/guichan/widgets/window.h"

#include "../resources/image.h"
#include "../resources/resourcemanager.h"
#include "../resources/wallpaper.h"

#include "../utils/gettext.h"

unsigned char error = NULL;

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

    setWidth((int) config.getValue("getWidth()", defaultScreenWidth));
    setHeight((int) config.getValue("getHeight()", defaultScreenHeight));
    setOpaque(false);

#ifdef PACKAGE_VERSION
    versionLabel = new Label(PACKAGE_VERSION);
    add(versionLabel, 2, 2);
#endif

    progressBar = new ProgressBar(0.0f, 100, 20, 168, 116, 31);
    progressBar->setVisible(false);

    progressLabel = new Label();

    setup = new Button(_("Setup"), "Setup", &listener);
    setup->setPosition(getWidth() - setup->getWidth() - 3, 3);

    // Set the most appropriate wallpaper, based on screen width
    Wallpaper::loadWallpapers();

    wallpaperName = Wallpaper::getWallpaper(getWidth(), getHeight());

    login_wallpaper = ResourceManager::getInstance()->getImage(wallpaperName);

    if (!login_wallpaper)
        logger->log("Couldn't load %s as wallpaper", wallpaperName.c_str());

    add(progressBar, 5, getHeight() - 5 - progressBar->getHeight());
    add(progressLabel, 15 + progressBar->getWidth(), progressBar->getY() + 4);
    add(setup);
}

Desktop::~Desktop()
{
#ifdef PACKAGE_VERSION
    delete versionLabel;
    versionLabel = NULL;
#endif
    delete progressBar;
    delete progressLabel;
    delete setup;
    progressBar = NULL;
    progressLabel = NULL;
    currentDialog = NULL;
    setup = NULL;

    if (login_wallpaper)
        login_wallpaper->decRef();
    login_wallpaper = NULL;
}

void Desktop::resize()
{
    const int newScreenWidth = graphics->getWidth();
    const int newScreenHeight = graphics->getHeight();

    if (getWidth() != newScreenWidth || getHeight() != newScreenHeight)
    {
        setWidth(newScreenWidth);
        setHeight(newScreenHeight);

#ifndef WIN32
        progressBar->setPosition(5, getHeight() - 5 - 
                                 progressBar->getHeight());
        progressLabel->setPosition(15 + progressBar->getWidth(), 4 +
                                   progressBar->getY());
        setup->setPosition(getWidth() - setup->getWidth() - 3, 3);

        if (currentDialog)
            positionDialog(currentDialog);
#endif
    }
}

void Desktop::reload()
{
    // Set the most appropriate wallpaper, based on screen width
    Wallpaper::loadWallpapers();

    resize();
}

void Desktop::changeCurrentDialog(Window* window)
{
    currentDialog = window;
    positionDialog(currentDialog);
}

void Desktop::showError(OkDialog* window, unsigned char errorState)
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
    delete currentDialog;
    currentDialog = NULL;
}

void Desktop::resetProgressBar()
{
    progressBar->setVisible(false);
    progressBar->reset();
    progressLabel->setCaption("");
}

void Desktop::useProgressBar(std::string message)
{
    resetProgressBar();
    progressBar->setVisible(true);
    progressLabel->setCaption(message);
    progressLabel->adjustSize();
}

void Desktop::positionDialog(Window *dialog)
{
    dialog->setPosition(std::min(getWidth() * 5 / 8, getWidth() - 
                        dialog->getWidth()), std::min(getHeight() * 5 / 8,
                        getHeight() - dialog->getHeight()));
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
    std::string tempWallpaper = Wallpaper::getWallpaper(getWidth(),
                                                        getHeight());

    if (tempWallpaper.compare(wallpaperName) != 0)
    {
        wallpaperName = tempWallpaper;
        Image *temp = ResourceManager::getInstance()->getImage(tempWallpaper);

        if (temp)
        {
            login_wallpaper->decRef();
            login_wallpaper = temp;
        }

        resize();
    }

    if (progressBar->isVisible())
    {
        progressBar->setProgress(progressBar->getProgress() + 0.005f);

        if (progressBar->getProgress() == 1.0f)
            progressBar->reset();
    }

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
