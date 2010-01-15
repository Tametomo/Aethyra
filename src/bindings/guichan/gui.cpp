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

#include <guichan/exception.hpp>
#include <guichan/image.hpp>
#include <guichan/imagefont.hpp>

#include "graphics.h"
#include "gui.h"
#include "palette.h"
#include "skin.h"
#include "truetypefont.h"

#include "dialogs/okdialog.h"

#include "handlers/focushandler.h"

#include "sdl/sdlinput.h"

#include "widgets/container.h"
#include "widgets/desktop.h"
#include "widgets/popup.h"
#include "widgets/window.h"

#include "../../core/configlistener.h"
#include "../../core/configuration.h"
#include "../../core/log.h"
#include "../../core/resourcemanager.h"

#include "../../core/image/image.h"
#include "../../core/image/imageset.h"
#include "../../core/image/imageloader.h"

#include "../../core/utils/dtor.h"
#include "../../core/utils/gettext.h"

#include "../../eathena/gui/viewport.h"

typedef std::list<gcn::Widget*> Widgets;
typedef Widgets::iterator WidgetIterator;

// Guichan stuff
Gui *gui = NULL;
SDLInput *guiInput = NULL;

// Bolded font
gcn::Font *mBoldFont = NULL;

volatile int tick_time;
volatile int fps = 0, frame = 0;

FPSmanager fpsm;

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
            else if (name == "mousealpha")
            {
                mGui->setMouseAlpha(config.getValue("mousealpha", 0.7f));
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
    mMaxMouseCursorAlpha(1.0f),
    mMouseCursorAlpha(1.0f),
    mMouseX(0),
    mMouseY(0),
    mButtonState(0),
    mMouseInactivityTimer(0),
    mCursorType(CURSOR_POINTER)
{
    logger->log("Initializing GUI...");

    SDL_initFramerate(&fpsm);

    // Set graphics
    setGraphics(graphics);

    // Set image loader
    static ImageLoader imageLoader;
    gcn::Image::setImageLoader(&imageLoader);

    // Set input
    guiInput = new SDLInput();
    setInput(guiInput);

    // Set focus handler
    destroy(mFocusHandler);
    mFocusHandler = new FocusHandler();

    // Initialize timers
    fps = 0;
    tick_time = 0;
    SDL_AddTimer(10, nextTick, NULL);                     // Logic counter
    SDL_AddTimer(1000, nextSecond, NULL);                 // Seconds counter

    // Initialize top GUI widget
    Container *guiTop = new Container();
    guiTop->setDimension(gcn::Rectangle(0, 0, graphics->getWidth(),
                                        graphics->getHeight()));
    guiTop->setOpaque(false);
    windowContainer = guiTop;
    setTop(guiTop);

    ResourceManager *resman = ResourceManager::getInstance();

    // Set global font
    std::string path = resman->getPath("fonts/dejavusans.ttf");
    try
    {
        const int fontSize = config.getValue("fontSize", 11);
        mGuiFont = resman->getFont(path, fontSize);
        mInfoParticleFont = resman->getFont(path, fontSize, 1);
        mBoldFont = resman->getFont(path, fontSize, 1);
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

    // Set the initial mouse cursor opacity
    mMaxMouseCursorAlpha = config.getValue("mousealpha", 0.7f);
    config.addListener("mousealpha", mConfigListener);

    // Initialize frame limiting
    config.addListener("fpslimit", mConfigListener);
    framerateChanged();
}

Gui::~Gui()
{
    config.removeListener("customcursor", mConfigListener);
    config.removeListener("fpslimit", mConfigListener);
    config.removeListener("mousealpha", mConfigListener);
    destroy(mConfigListener);

    if (mMouseCursors)
        mMouseCursors->decRef();

    mGuiFont->decRef();
    mBoldFont->decRef();
    mInfoParticleFont->decRef();
    delete getTop();

    destroy(guiInput);
}

void Gui::resize(const int width, const int height)
{
    if (width < 0 || height < 0 || (width == graphics->getWidth() &&
        height == graphics->getHeight()))
        return;

    Widgets widgets = windowContainer->getWidgetList();
    WidgetIterator iter;

    // First save the window positions, adaptToNewSize will position
    // them based on the saved positions.
    for (iter = widgets.begin(); iter != widgets.end(); ++iter)
    {
        Window* window = dynamic_cast<Window*>(*iter);

        if (window)
            window->saveWindowState();
    }

    Graphics *graphics = static_cast<Graphics*>(mGraphics);

    graphics->resizeVideoMode(width, height);

    mTop->setDimension(gcn::Rectangle(0, 0, graphics->getWidth(),
                                      graphics->getHeight()));

    if (viewport)
        viewport->setDimension(gcn::Rectangle(0, 0, graphics->getWidth(),
                                              graphics->getHeight()));

    if (desktop)
        desktop->resize();

    // Reposition all the open sub-windows and popups.
    // The rest of the windows will reposition themselves on opening.
    for (iter = widgets.begin(); iter != widgets.end(); ++iter)
    {
        Window* window = dynamic_cast<Window*>(*iter);

        if (window)
        {
            window->adaptToNewSize();
            continue;
        }

        Popup* popup = dynamic_cast<Popup*>(*iter);
        if (popup)
        {
            popup->adaptToNewSize();
            continue;
        }
    }

    config.setValue("screenwidth", width);
    config.setValue("screenheight", height);
}

void Gui::logic()
{
    gcn::Gui::logic();

    // Update the screen when application is active, delay otherwise.
    if (SDL_GetAppState() & SDL_APPACTIVE)
    {
        draw();
        graphics->updateScreen();

        // Fade out mouse cursor after extended inactivity
        if (get_elapsed_time(mMouseInactivityTimer) < 15000)
        {
            const double alpha = std::min(mMouseCursorAlpha + 0.05, 1.0);
            mMouseCursorAlpha = std::min(mMaxMouseCursorAlpha, alpha);
        }
        else if (mMouseInactivityTimer > MAX_TIME)
        {
            mMouseInactivityTimer -= MAX_TIME;
        }
        else
        {
            mMouseCursorAlpha = std::max(0.0, mMouseCursorAlpha - 0.005);
        }
    }

    frame++;
    SDL_framerateDelay(&fpsm);

    guiPalette->advanceGradient();
}

void Gui::framerateChanged()
{
    const int fpsLimit = config.getValue("fpslimit", 0);

    SDL_setFramerate(&fpsm, fpsLimit > 0 ? fpsLimit : 60);
}

void Gui::draw()
{
    mGraphics->pushClipArea(getTop()->getDimension());
    getTop()->draw(mGraphics);

    mButtonState = SDL_GetMouseState(&mMouseX, &mMouseY);

    if ((SDL_GetAppState() & SDL_APPMOUSEFOCUS || mButtonState & SDL_BUTTON(1))
         && mCustomCursor && mMouseCursorAlpha > 0.0f)
    {
        Image *mouseCursor = mMouseCursors->get(mCursorType);
        mouseCursor->setAlpha(mMouseCursorAlpha);

        static_cast<Graphics*>(mGraphics)->drawImage(mouseCursor, mMouseX - 15,
                                                     mMouseY - 17);
    }

    mGraphics->popClipArea();
}

gcn::Font* Gui::getFont() const
{
    return mGuiFont;
}

gcn::Font* Gui::getBoldFont() const
{
    return mBoldFont;
}

gcn::Font* Gui::getInfoParticleFont() const
{
    return mInfoParticleFont;
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
    mMouseInactivityTimer = tick_time;
}

void Gui::handleMouseWheelMovedDown(const gcn::MouseInput& mouseInput)
{
    gcn::Widget* sourceWidget = getMouseEventSource(mouseInput.getX(),
                                                    mouseInput.getY());
    gcn::Widget* focusedWidget = mFocusHandler->getFocused();

    if (mFocusHandler->getDraggedWidget() != NULL)
        sourceWidget = mFocusHandler->getDraggedWidget();

    if (!sourceWidget)
        sourceWidget = focusedWidget;

    if (!sourceWidget)
        return;

    int widgetX, widgetY;
    sourceWidget->getAbsolutePosition(widgetX, widgetY);

    widgetX = mouseInput.getX() - widgetX;
    widgetY = mouseInput.getY() - widgetY;

    gcn::MouseEvent sourceEvent(sourceWidget, mShiftPressed, mControlPressed,
                                mAltPressed, mMetaPressed,
                                gcn::MouseEvent::WHEEL_MOVED_DOWN,
                                mouseInput.getButton(), widgetX, widgetY,
                                mClickCount);

    distributeMouseWheelEvent(sourceWidget, sourceEvent);

    if (sourceEvent.isConsumed() || !focusedWidget)
        return;

    focusedWidget->getAbsolutePosition(widgetX, widgetY);

    widgetX = mouseInput.getX() - widgetX;
    widgetY = mouseInput.getY() - widgetY;

    gcn::MouseEvent focusedEvent(sourceWidget, mShiftPressed, mControlPressed,
                                 mAltPressed, mMetaPressed,
                                 gcn::MouseEvent::WHEEL_MOVED_DOWN,
                                 mouseInput.getButton(), widgetX, widgetY,
                                 mClickCount);

    distributeMouseWheelEvent(focusedWidget, focusedEvent);
}

void Gui::handleMouseWheelMovedUp(const gcn::MouseInput& mouseInput)
{
    gcn::Widget* sourceWidget = getMouseEventSource(mouseInput.getX(),
                                                    mouseInput.getY());
    gcn::Widget* focusedWidget = mFocusHandler->getFocused();

    if (mFocusHandler->getDraggedWidget() != NULL)
        sourceWidget = mFocusHandler->getDraggedWidget();

    if (!sourceWidget)
        sourceWidget = focusedWidget;

    if (!sourceWidget)
        return;

    int widgetX, widgetY;
    sourceWidget->getAbsolutePosition(widgetX, widgetY);

    widgetX = mouseInput.getX() - widgetX;
    widgetY = mouseInput.getY() - widgetY;

    gcn::MouseEvent sourceEvent(sourceWidget, mShiftPressed, mControlPressed,
                                mAltPressed, mMetaPressed,
                                gcn::MouseEvent::WHEEL_MOVED_UP,
                                mouseInput.getButton(), widgetX, widgetY,
                                mClickCount);

    distributeMouseWheelEvent(sourceWidget, sourceEvent);

    if (sourceEvent.isConsumed() || !focusedWidget)
        return;

    focusedWidget->getAbsolutePosition(widgetX, widgetY);

    widgetX = mouseInput.getX() - widgetX;
    widgetY = mouseInput.getY() - widgetY;

    gcn::MouseEvent focusedEvent(sourceWidget, mShiftPressed, mControlPressed,
                                 mAltPressed, mMetaPressed,
                                 gcn::MouseEvent::WHEEL_MOVED_UP,
                                 mouseInput.getButton(), widgetX, widgetY,
                                 mClickCount);

    distributeMouseWheelEvent(focusedWidget, focusedEvent);
}

void Gui::distributeMouseWheelEvent(gcn::Widget* source, gcn::MouseEvent 
                                    &mouseEvent)
{
    gcn::Widget* parent = source;
    gcn::Widget* widget = source;

    if ((mFocusHandler->getModalFocused() != NULL && !widget->isModalFocused()) ||
        (mFocusHandler->getModalMouseInputFocused() != NULL &&
        !widget->isModalMouseInputFocused()))
    {
        return;
    }

    while (parent != NULL)
    {
        // If the widget has been removed due to input
        // cancel the distribution.
        if (!gcn::Widget::widgetExists(widget))
            break;

        parent = (gcn::Widget*) widget->getParent();

        if (widget->isEnabled())
        {                                      
            std::list<gcn::MouseListener*> mouseListeners = widget->_getMouseListeners();

            // Send the event to all mouse listeners of the widget.
            for (std::list<gcn::MouseListener*>::iterator it = mouseListeners.begin();
                 it != mouseListeners.end(); ++it)
            {
                switch (mouseEvent.getType())
                {
                    case gcn::MouseEvent::WHEEL_MOVED_UP:
                        (*it)->mouseWheelMovedUp(mouseEvent);
                        break;
                    case gcn::MouseEvent::WHEEL_MOVED_DOWN:
                        (*it)->mouseWheelMovedDown(mouseEvent);
                        break;
                    default:
                        throw GCN_EXCEPTION("Unknown mouse event type.");
                }                    
            }
        }

        gcn::Widget* swap = widget;
        widget = parent;
        parent = (gcn::Widget*) swap->getParent();

        // If a non modal focused widget has been reach and we have modal focus,
        // cancel the distribution.
        if (mFocusHandler->getModalFocused() != NULL && !widget->isModalFocused())
            break;

        // If a non modal mouse input focused widget has been reached and we
        // have modal mouse input focus cancel the distribution.
        if (mFocusHandler->getModalMouseInputFocused() != NULL &&
            !widget->isModalMouseInputFocused())
        {
            break;
        }
    }
}

const int Gui::getFontHeight() const
{
    return mGuiFont->getHeight();
}

void Gui::storeFocus()
{
    FocusHandler *fh = static_cast<FocusHandler*>(mFocusHandler);

    fh->storeFocus();
}

void Gui::restoreFocus()
{
    FocusHandler *fh = static_cast<FocusHandler*>(mFocusHandler);

    fh->restoreFocus();
}

void Gui::changeFontSize(const int size)
{
    ResourceManager *resman = ResourceManager::getInstance();
    const std::string path = resman->getPath("fonts/dejavusans.ttf");

    TrueTypeFont *guiFont = static_cast<TrueTypeFont*>(getTop()->getFont());
    TrueTypeFont *boldFont = static_cast<TrueTypeFont*>(getBoldFont());
    TrueTypeFont *infoParticleFont = static_cast<TrueTypeFont*>(getInfoParticleFont());

    boldFont->decRef();
    mBoldFont = resman->getFont(path, size, 1);
    infoParticleFont->decRef();
    mInfoParticleFont = resman->getFont(path, size, 1);
    guiFont->decRef();
    mGuiFont = resman->getFont(path, size);
    getTop()->setGlobalFont(mGuiFont);

    Widgets widgets = windowContainer->getWidgetList();
    WidgetIterator iter;

    for (iter = widgets.begin(); iter != widgets.end(); ++iter)
    {
        Popup* popup = dynamic_cast<Popup*>(*iter);
        Window* window = dynamic_cast<Window*>(*iter);

        if (popup)
            popup->adaptToNewSize();

        if (window)
            window->refreshLayout();
    }

    if (desktop)
        desktop->resize();
}
