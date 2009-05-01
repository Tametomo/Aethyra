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

#include <typeinfo>

#include <guichan/exception.hpp>
#include <guichan/image.hpp>
#include <guichan/imagefont.hpp>

#include "graphics.h"
#include "gui.h"
#include "palette.h"
#include "skin.h"
#include "truetypefont.h"

#include "handlers/focushandler.h"

#include "sdl/sdlinput.h"

#include "widgets/textbox.h"
#include "widgets/window.h"
#include "widgets/windowcontainer.h"

#include "../../configlistener.h"
#include "../../configuration.h"
#include "../../log.h"

#include "../../gui/menuwindow.h"
#include "../../gui/viewport.h"

#include "../../resources/image.h"
#include "../../resources/imageset.h"
#include "../../resources/imageloader.h"
#include "../../resources/resourcemanager.h"

// Guichan stuff
Gui *gui = 0;
Viewport *viewport = 0;                    /**< Viewport on the map. */
SDLInput *guiInput = 0;

// Bolded font
gcn::Font *boldFont = 0;

volatile int tick_time;
volatile int fps = 0, frame = 0;

const int MAX_TIME = 10000;

class GuiConfigListener : public ConfigListener
{
    public:
        GuiConfigListener(Gui *g):
            mGui(g)
        {}

        void optionChanged(const std::string &name)
        {
            if (name == "customcursor")
            {
                bool bCustomCursor = config.getValue("customcursor", 1) == 1;
                mGui->setUseCustomCursor(bCustomCursor);
            }
            else if (name == "fpslimit")
            {
                mGui->framerateChanged();
            }
        }
    private:
        Gui *mGui;
};

/**
 * Advances game logic counter.
 */
Uint32 nextTick(Uint32 interval, void *param)
{
    tick_time++;
    if (tick_time == MAX_TIME)
        tick_time = 0;

    return interval;
}

/**
 * Updates fps.
 */
Uint32 nextSecond(Uint32 interval, void *param)
{
    fps = frame;
    frame = 0;

    return interval;
}

int get_elapsed_time(int start_time)
{
    if (start_time <= tick_time)
        return (tick_time - start_time) * 10;
    else
        return (tick_time + (MAX_TIME - start_time)) * 10;
}

Gui::Gui(Graphics *graphics):
    mCustomCursor(false),
    mMouseCursors(NULL),
    mMaxMouseCursorAlpha(0.7f),
    mMouseCursorAlpha(mMaxMouseCursorAlpha),
    mMouseInactivityTimer(0),
    mCursorType(CURSOR_POINTER)
{
    logger->log("Initializing GUI...");
    // Set graphics
    setGraphics(graphics);

    // Set image loader
    static ImageLoader imageLoader;
    gcn::Image::setImageLoader(&imageLoader);

    // Set input
    guiInput = new SDLInput;
    setInput(guiInput);

    // Set focus handler
    delete mFocusHandler;
    mFocusHandler = new FocusHandler;

    // Initialize timers
    tick_time = 0;
    SDL_AddTimer(10, nextTick, NULL);                     // Logic counter
    SDL_AddTimer(1000, nextSecond, NULL);                 // Seconds counter

    // Initialize top GUI widget
    WindowContainer *guiTop = new WindowContainer();
    guiTop->setDimension(gcn::Rectangle(0, 0,
                graphics->getWidth(), graphics->getHeight()));
    guiTop->setOpaque(false);
    Window::setWindowContainer(guiTop);
    setTop(guiTop);

    ResourceManager *resman = ResourceManager::getInstance();

    // Set global font
    std::string path = resman->getPath("fonts/dejavusans.ttf");
    try
    {
        const int fontSize = (int)config.getValue("fontSize", 11);
        mGuiFont = new TrueTypeFont(path, fontSize);
        mInfoParticleFont = new TrueTypeFont(path, fontSize, 1);
        boldFont = new TrueTypeFont(path, fontSize, 1);
    }
    catch (gcn::Exception e)
    {
        logger->error(std::string("Unable to load dejavusans.ttf: ")
            + e.getMessage());
    }

    gcn::Widget::setGlobalFont(mGuiFont);

    // Initialize mouse cursor and listen for changes to the option
    setUseCustomCursor(config.getValue("customcursor", 1) == 1);
    mConfigListener = new GuiConfigListener(this);
    config.addListener("customcursor", mConfigListener);

    // Initialize frame limiting
    config.addListener("fpslimit", mConfigListener);
    framerateChanged();

    // Create the viewport
    viewport = new Viewport();
    viewport->setDimension(gcn::Rectangle(0, 0,
                graphics->getWidth(), graphics->getHeight()));
    guiTop->add(viewport);
}

Gui::~Gui()
{
    config.removeListener("customcursor", mConfigListener);
    config.removeListener("fpslimit", mConfigListener);
    delete mConfigListener;

    if (mMouseCursors)
        mMouseCursors->decRef();

    delete mGuiFont;
    delete boldFont;
    delete mInfoParticleFont;
    delete viewport;
    delete getTop();

    delete guiInput;
}

void Gui::resize(Graphics *graphics)
{
   WindowContainer *guiTop = static_cast<WindowContainer*>(getTop());
   guiTop->setDimension(gcn::Rectangle(0, 0,
                                       graphics->getWidth(), graphics->getHeight()));
   Window::setWindowContainer(guiTop);
   viewport->setDimension(gcn::Rectangle(0, 0,
                                         graphics->getWidth(), graphics->getHeight()));
}
void Gui::logic()
{
    // Update the screen when application is active, delay otherwise.
    if (SDL_GetAppState() & SDL_APPACTIVE)
    {
        // Draw a frame if either frames are not limited or enough time has
        // passed since the last frame.
        if (!mMinFrameTime || get_elapsed_time(mDrawTime / 10) > mMinFrameTime)
        {
            frame++;
            draw();
            graphics->updateScreen();
            mDrawTime += mMinFrameTime;

            // Make sure to wrap mDrawTime, since tick_time will wrap.
            if (mDrawTime > MAX_TIME * 10)
                mDrawTime -= MAX_TIME * 10;
        }
        else
        {
            SDL_Delay(10);
        }
    }
    else
    {
        SDL_Delay(10);
        mDrawTime = tick_time * 10;
    }

    // Fade out mouse cursor after extended inactivity
    if (mMouseInactivityTimer < 100 * 15)
    {
        ++mMouseInactivityTimer;
        mMouseCursorAlpha = std::min(mMaxMouseCursorAlpha, mMouseCursorAlpha + 0.05f);
    }
    else
        mMouseCursorAlpha = std::max(0.0f, mMouseCursorAlpha - 0.005f);

    guiPalette->advanceGradient();

    gcn::Gui::logic();
}

void Gui::framerateChanged()
{
    int fpsLimit = (int) config.getValue("fpslimit", 0);

    // Calculate new minimum frame time. If one isn't set, use 60 FPS.
    // (1000 / 60 is 16.66) Since the client can go well above the refresh
    // rates for monitors now in OpenGL mode, this cutoff is done to help
    // conserve on CPU time.
    mMinFrameTime = fpsLimit ? 1000 / fpsLimit : 16;

    // Reset draw time to current time
    mDrawTime = tick_time * 10;
}

void Gui::draw()
{
    mGraphics->pushClipArea(getTop()->getDimension());
    getTop()->draw(mGraphics);

    int mouseX, mouseY;
    Uint8 button = SDL_GetMouseState(&mouseX, &mouseY);

    if ((SDL_GetAppState() & SDL_APPMOUSEFOCUS || button & SDL_BUTTON(1)) &&
         mCustomCursor && mMouseCursorAlpha > 0.0f)
    {
        Image *mouseCursor = mMouseCursors->get(mCursorType);
        mouseCursor->setAlpha(mMouseCursorAlpha);

        static_cast<Graphics*>(mGraphics)->drawImage(mouseCursor, mouseX - 15,
                                                     mouseY - 17);
    }

    mGraphics->popClipArea();
}

bool Gui::isInputFocused()
{
    gcn::Widget* widget = mFocusHandler->getFocused();

    // Both the TextBox from the SpeechBubble popup and the ItemPopup maintain
    // focus throughout their lives without surrendering it, since they are
    // force repositioned all of the time. Because of this, they need to be
    // released, or else their continual focus adding will cause input to be put
    // into a dead state while they're visible if a GUIChan native key is
    // pressed.
    while (widget && typeid(*widget) == typeid(TextBox))
    {
        mFocusHandler->remove(widget);
        widget = mFocusHandler->getFocused();
    }

    return (widget && widget != windowContainer);
}

gcn::Widget* Gui::getFocused()
{
    gcn::Widget* widget = mFocusHandler->getFocused();

    return (widget);
}

void Gui::setUseCustomCursor(bool customCursor)
{
    if (customCursor != mCustomCursor)
    {
        mCustomCursor = customCursor;

        if (mCustomCursor)
        {
            // Hide the SDL mouse cursor
            SDL_ShowCursor(SDL_DISABLE);

            // Load the mouse cursor
            ResourceManager *resman = ResourceManager::getInstance();
            mMouseCursors =
                resman->getImageSet("graphics/gui/mouse.png", 40, 40);

            if (!mMouseCursors)
                logger->error("Unable to load mouse cursors.");
        }
        else
        {
            // Show the SDL mouse cursor
            SDL_ShowCursor(SDL_ENABLE);

            // Unload the mouse cursor
            if (mMouseCursors)
            {
                mMouseCursors->decRef();
                mMouseCursors = NULL;
            }
        }
    }
}

void Gui::handleMouseMoved(const gcn::MouseInput &mouseInput)
{
    gcn::Gui::handleMouseMoved(mouseInput);
    mMouseInactivityTimer = 0;
}

const int Gui::getFontHeight() const
{
    return mGuiFont->getHeight();
}
