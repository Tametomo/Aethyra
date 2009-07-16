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

#include <climits>

#include <guichan/exception.hpp>
#include <guichan/focushandler.hpp>

#include "resizegrip.h"
#include "window.h"
#include "windowcontainer.h"

#include "../gui.h"
#include "../layout.h"
#include "../palette.h"
#include "../skin.h"

#include "../../../core/configuration.h"
#include "../../../core/log.h"

#include "../../../core/image/image.h"

int Window::instances = 0;
int Window::mouseResize = 0;

Window::Window(const std::string& caption, bool modal, Window *parent,
               const std::string& skin, bool visible):
    gcn::Window(caption),
    mOldVisibility(false),
    mGrip(NULL),
    mParent(parent),
    mLayout(NULL),
    mWindowName("window"),
    mDefaultSkinPath(skin),
    mShowTitle(true),
    mModal(modal),
    mCloseButton(false),
    mDefaultVisible(visible),
    mSaveVisibility(true),
    mMinWinWidth(100),
    mMinWinHeight(40),
    mMaxWinWidth(graphics->getWidth()),
    mMaxWinHeight(graphics->getHeight())
{
    logger->log("Window::Window(\"%s\")", caption.c_str());

    if (!windowContainer)
        throw GCN_EXCEPTION("Window::Window(): no windowContainer set");

    if (!skinLoader)
        skinLoader = new SkinLoader();

    instances++;

    setFrameSize(0);
    setPadding(3);
    setTitleBarHeight(20);

    // Loads the skin
    mSkin = skinLoader->load(skin, mDefaultSkinPath);

    // Add this window to the window container
    windowContainer->add(this);

    if (mModal)
    {
        gui->setCursorType(Gui::CURSOR_POINTER);
        requestModalFocus();
    }

    // Windows are invisible by default
    setVisible(false);

    addWidgetListener(this);
}

Window::~Window()
{
    logger->log("Window::~Window(\"%s\")", getCaption().c_str());

    saveWindowState();

    delete mLayout;

    while (!mWidgets.empty())
        delete mWidgets.front();

    removeWidgetListener(this);

    instances--;

    mSkin->instances--;

    if (instances == 0)
        delete skinLoader;
}

void Window::setWindowContainer(WindowContainer *wc)
{
    windowContainer = wc;
}

void Window::draw(gcn::Graphics *graphics)
{
    Graphics *g = static_cast<Graphics*>(graphics);

    g->drawImageRect(0, 0, getWidth(), getHeight(), mSkin->getBorder());

    // Draw title
    if (mShowTitle)
    {
        g->setColor(guiPalette->getColor(Palette::TEXT));
        g->setFont(getFont());
        g->drawText(getCaption(), 7, 5, gcn::Graphics::LEFT);
    }

    // Draw Close Button
    if (mCloseButton)
    {
        g->drawImage(mSkin->getCloseImage(), getWidth() -
                     mSkin->getCloseImage()->getWidth() - getPadding(),
                     getPadding());
    }

    drawChildren(graphics);
}

void Window::setContentSize(int width, int height)
{
    width = width + 2 * getPadding();
    height = height + getPadding() + getTitleBarHeight();

    if (getMinWidth() > width)
        width = getMinWidth();
    else if (getMaxWidth() < width)
        width = getMaxWidth();
    if (getMinHeight() > height)
        height = getMinHeight();
    else if (getMaxHeight() < height)
        height = getMaxHeight();

    setSize(width, height);
}

void Window::setLocationRelativeTo(gcn::Widget *widget)
{
    int wx, wy;
    int x, y;

    widget->getAbsolutePosition(wx, wy);
    getAbsolutePosition(x, y);

    setPosition(getX() + (wx + (widget->getWidth() - getWidth()) / 2 - x),
                getY() + (wy + (widget->getHeight() - getHeight()) / 2 - y));
}

void Window::setLocationRelativeTo(ImageRect::ImagePosition position,
                                   const int &offsetX, const int &offsetY)
{
    setLocationRelativeTo(graphics->getWidth(), graphics->getHeight(), position,
                          offsetX, offsetY);
}

void Window::setLocationRelativeTo(const int &width, const int &height,
                                   ImageRect::ImagePosition position,
                                   const int &offsetX, const int &offsetY)
{
    int x = 0, y = 0;

    getRelativeOffset(position, x, y, width, height, 0, 0);

    setPosition(x - offsetX, y - offsetY);
}

void Window::getRelativeOffset(ImageRect::ImagePosition position, int &x, int &y,
                               const int& width, const int& height)
{
    getRelativeOffset(position, x, y, graphics->getWidth(),
                      graphics->getHeight(), width, height);
}

void Window::getRelativeOffset(ImageRect::ImagePosition position, int &x, int &y,
                               const int &conWidth, const int &conHeight,
                               const int& width, const int& height)
{
    switch (position)
    {
        case ImageRect::UPPER_CENTER:
            x += (conWidth - width) / 2;
            break;
        case ImageRect::UPPER_RIGHT:
            x += conWidth - width;
            break;
        case ImageRect::LEFT:
            y += (conHeight - height) / 2;
            break;
        case ImageRect::CENTER:
            x += (conWidth - width) / 2;
            y += (conHeight - height) / 2;
            break;
        case ImageRect::RIGHT:
            x += conWidth - width;
            y += (conHeight - height) / 2;
            break;
        case ImageRect::LOWER_LEFT:
            y += conHeight - height;
            break;
        case ImageRect::LOWER_CENTER:
            x += (conWidth - width) / 2;
            y += conHeight - height;
            break;
        case ImageRect::LOWER_RIGHT:
            x += conWidth - width;
            y += conHeight - height;
            break;
        default:
            break;
    }
}

void Window::saveRelativeLocation(const int &x, const int &y)
{
    ImageRect::ImagePosition savedPos = ImageRect::UPPER_LEFT;
    int posX = 0, posY = 0, offsetX = 0, offsetY = 0, distX = 0, distY = 0;
    int bestOffsetX = 0, bestOffsetY = 0;
    int bestSquaredDistance = INT_MAX;

    for (int pos = ImageRect::UPPER_LEFT; pos <= ImageRect::LOWER_RIGHT &&
         bestSquaredDistance > 0; pos++)
    {
        int squaredDistance = 0;

        // Get the relative position's coordinates. This will be used as a
        // multiplier to determine centered-ness.
        posX = 0; posY = 0;
        getRelativeOffset((ImageRect::ImagePosition) pos, posX, posY, getWidth(),
                           getHeight(), 0, 0);

        // Now determine the actual offset values to be used.
        offsetX = -x; offsetY = -y;
        getRelativeOffset((ImageRect::ImagePosition) pos, offsetX, offsetY, 0, 0);

        // Compare the two values to get the relative distance between the two.
        // Whichever of the two returns the lowest squared distance will cause
        // the least disruption on changing resolution, and the most likely to
        // match where the user would try to locate it themselves.
        distX = offsetX - posX;
        distY = offsetY - posY;

        squaredDistance = distX * distX + distY * distY;

        if (squaredDistance < bestSquaredDistance)
        {
            savedPos = (ImageRect::ImagePosition) pos;
            bestSquaredDistance = squaredDistance;
            bestOffsetX = offsetX;
            bestOffsetY = offsetY;
        }
    }

    mPosition = savedPos;
    mOffsetX = bestOffsetX;
    mOffsetY = bestOffsetY;

    config.setValue(mWindowName + "Position", savedPos);
    config.setValue(mWindowName + "OffsetX",  bestOffsetX);
    config.setValue(mWindowName + "OffsetY",  bestOffsetY);
}

void Window::adaptToNewSize()
{
    setLocationRelativeTo(mPosition, mOffsetX, mOffsetY);
}

void Window::setMinWidth(int width)
{
    mMinWinWidth = width > mSkin->getMinWidth() ? width : mSkin->getMinWidth();
}

void Window::setMinHeight(int height)
{
    mMinWinHeight = height > mSkin->getMinHeight() ?
                    height : mSkin->getMinHeight();
}

void Window::setMaxWidth(int width)
{
    mMaxWinWidth = width;
}

void Window::setMaxHeight(int height)
{
    mMaxWinHeight = height;
}

void Window::setResizable(bool r)
{
    if ((bool) mGrip == r)
        return;

    if (r)
    {
        mGrip = new ResizeGrip();
        mGrip->setX(getWidth() - mGrip->getWidth() - getChildrenArea().x);
        mGrip->setY(getHeight() - mGrip->getHeight() - getChildrenArea().y);
        add(mGrip);
    }
    else
    {
        remove(mGrip);
        delete mGrip;
        mGrip = NULL;
    }
}

void Window::widgetResized(const gcn::Event &event)
{
    const gcn::Rectangle area = getChildrenArea();

    if (mGrip)
        mGrip->setPosition(getWidth() - mGrip->getWidth() - area.x,
                           getHeight() - mGrip->getHeight() - area.y);

    if (mLayout)
    {
        int w = area.width;
        int h = area.height;
        mLayout->reflow(w, h);
    }
}

void Window::widgetShown(const gcn::Event& event)
{
    mVisible = true;

    requestMoveToTop();
    requestFocus();
}

void Window::widgetHidden(const gcn::Event& event)
{
    mVisible = false;

    WidgetListIterator it;

    for (it = mWidgets.begin(); it != mWidgets.end(); it++)
    {
        if (mFocusHandler->isFocused(*it))
            mFocusHandler->focusNone();
    }
}

void Window::setCloseButton(bool flag)
{
    mCloseButton = flag;
}

bool Window::isResizable()
{
    return mGrip;
}

void Window::scheduleDelete()
{
    windowContainer->scheduleDelete(this);
}

void Window::close()
{
    setVisible(false);
}

void Window::hide()
{
    const bool oldVisibility = isVisible();

    setVisible(mOldVisibility);

    mOldVisibility = oldVisibility;
}

void Window::mousePressed(gcn::MouseEvent &event)
{
    // Let Guichan move window to top and figure out title bar drag
    gcn::Window::mousePressed(event);

    if (event.getButton() == gcn::MouseEvent::LEFT)
    {
        const int x = event.getX();
        const int y = event.getY();

        // Handle close button
        if (mCloseButton)
        {
            gcn::Rectangle closeButtonRect(
                getWidth() - mSkin->getCloseImage()->getWidth() - getPadding(),
                getPadding(),
                mSkin->getCloseImage()->getWidth(),
                mSkin->getCloseImage()->getHeight());

            if (closeButtonRect.isPointInRect(x, y))
                close();
        }

        // Handle window resizing
        mouseResize = getResizeHandles(event);
    }
}

void Window::mouseReleased(gcn::MouseEvent &event)
{
    if (mGrip && mouseResize)
    {
        mouseResize = 0;
        gui->setCursorType(Gui::CURSOR_POINTER);
    }

    // This should be the responsibility of Guichan (and is from 0.8.0 on)
    mMoved = false;
}

void Window::mouseExited(gcn::MouseEvent &event)
{
    if (mGrip && !mouseResize)
        gui->setCursorType(Gui::CURSOR_POINTER);
}

void Window::mouseMoved(gcn::MouseEvent &event)
{
    int resizeHandles = getResizeHandles(event);

    // Changes the custom mouse cursor based on it's current position.
    switch (resizeHandles)
    {
        case BOTTOM | RIGHT:
            gui->setCursorType(Gui::CURSOR_RESIZE_DOWN_RIGHT);
            break;
        case BOTTOM | LEFT:
            gui->setCursorType(Gui::CURSOR_RESIZE_DOWN_LEFT);
            break;
        case BOTTOM:
            gui->setCursorType(Gui::CURSOR_RESIZE_DOWN);
            break;
        case RIGHT:
        case LEFT:
            gui->setCursorType(Gui::CURSOR_RESIZE_ACROSS);
            break;
        default:
            gui->setCursorType(Gui::CURSOR_POINTER);
    }
}

void Window::mouseDragged(gcn::MouseEvent &event)
{
    // Let Guichan handle title bar drag
    gcn::Window::mouseDragged(event);

    // Keep guichan window inside screen when it may be moved
    if (isMovable() && mMoved)
    {
        int newX = std::max(0, getX());
        int newY = std::max(0, getY());
        newX = std::min(graphics->getWidth() - getWidth(), newX);
        newY = std::min(graphics->getHeight() - getHeight(), newY);
        setPosition(newX, newY);
    }

    if (mouseResize && !mMoved)
    {
        const int dx = event.getX() - mDragOffsetX;
        const int dy = event.getY() - mDragOffsetY;
        gcn::Rectangle newDim = getDimension();

        if (mouseResize & (TOP | BOTTOM))
        {
            int newHeight = newDim.height + ((mouseResize & TOP) ? -dy : dy);
            newDim.height = std::min(mMaxWinHeight,
                                     std::max(mMinWinHeight, newHeight));

            if (mouseResize & TOP)
                newDim.y -= newDim.height - getHeight();
        }

        if (mouseResize & (LEFT | RIGHT))
        {
            int newWidth = newDim.width + ((mouseResize & LEFT) ? -dx : dx);
            newDim.width = std::min(mMaxWinWidth,
                                    std::max(mMinWinWidth, newWidth));

            if (mouseResize & LEFT)
                newDim.x -= newDim.width - getWidth();
        }

        // Keep guichan window inside screen (supports resizing any side)
        if (newDim.x < 0)
        {
            newDim.width += newDim.x;
            newDim.x = 0;
        }
        if (newDim.y < 0)
        {
            newDim.height += newDim.y;
            newDim.y = 0;
        }
        if (newDim.x + newDim.width > graphics->getWidth())
        {
            newDim.width = graphics->getWidth() - newDim.x;
        }
        if (newDim.y + newDim.height > graphics->getHeight())
        {
            newDim.height = graphics->getHeight() - newDim.y;
        }

        // Update mouse offset when dragging bottom or right border
        if (mouseResize & BOTTOM)
        {
            mDragOffsetY += newDim.height - getHeight();
        }
        if (mouseResize & RIGHT)
        {
            mDragOffsetX += newDim.width - getWidth();
        }

        // Set the new window and content dimensions
        setDimension(newDim);
    }
}

void Window::loadWindowState()
{
    const std::string &name = mWindowName;
    const std::string skinName = config.getValue(name + "Skin",
                                                 mSkin->getFilePath());
    assert(!name.empty());

    if (mGrip)
    {
        int width = (int) config.getValue(name + "WinWidth", mDefaultWidth);
        int height = (int) config.getValue(name + "WinHeight", mDefaultHeight);

        if (getMinWidth() > width)
            width = getMinWidth();
        else if (getMaxWidth() < width)
            width = getMaxWidth();
        if (getMinHeight() > height)
            height = getMinHeight();
        else if (getMaxHeight() < height)
            height = getMaxHeight();

        setSize(width, height);
    }
    else
    {
        setSize(mDefaultWidth, mDefaultHeight);
    }

    int x = (int) config.getValue(name + "WinX", -1);
    int y = (int) config.getValue(name + "WinY", -1);

    if (x != -1 || y != -1)
    {
        // These two tags are deprecated. Convert them to the new system, then
        // remove them to avoid client coordinate confusion.
        config.removeValue(mWindowName + "WinX");
        config.removeValue(mWindowName + "WinY");
        saveRelativeLocation(x, y);
    }

    int position = (int) config.getValue(name + "Position", -1);
    mOffsetX = (int) config.getValue(name + "OffsetX", mDefaultOffsetX);
    mOffsetY = (int) config.getValue(name + "OffsetY", mDefaultOffsetY);

    if (position != -1)
    {
        mPosition = (ImageRect::ImagePosition) position;
        setLocationRelativeTo(mPosition, mOffsetX, mOffsetY);
    }
    else
    {
        setLocationRelativeTo(mDefaultPosition, mDefaultOffsetX, mDefaultOffsetY);
    }

    if (mSaveVisibility)
    {
        setVisible((bool) config.getValue(name + "Visible", mDefaultVisible));
        mOldVisibility = (bool) config.getValue(name + "Hidden", false);
    }

    if (skinName.compare(mSkin->getFilePath()) != 0)
    {
        mSkin->instances--;
        mSkin = skinLoader->load(skinName, mDefaultSkinPath);
    }
}

void Window::saveWindowState()
{
    // Saving coordinates and Width and Height for resizables in the config
    if (!mWindowName.empty() && mWindowName != "window")
    {
        if (mSaveVisibility)
        {
            config.setValue(mWindowName + "Visible", isVisible());
            config.setValue(mWindowName + "Hidden", mOldVisibility);
        }
        else
        {
            config.removeValue(mWindowName + "Visible");
            config.removeValue(mWindowName + "Hidden");
        }

        config.setValue(mWindowName + "Skin", mSkin->getFilePath());

        saveRelativeLocation(getX(), getY());

        if (mGrip)
        {
            config.setValue(mWindowName + "WinWidth", getWidth());
            config.setValue(mWindowName + "WinHeight", getHeight());
        }
    }
    else if (mWindowName == "window")
    {
        config.removeValue(mWindowName + "Position");
        config.removeValue(mWindowName + "OffsetX");
        config.removeValue(mWindowName + "OffsetY");
    }
}

void Window::setDefaultSize(int defaultWidth, int defaultHeight,
                            ImageRect::ImagePosition position,
                            int offsetX, int offsetY)
{
    if (getMinWidth() > defaultWidth)
        defaultWidth = getMinWidth();
    else if (getMaxWidth() < defaultWidth)
        defaultWidth = getMaxWidth();
    if (getMinHeight() > defaultHeight)
        defaultHeight = getMinHeight();
    else if (getMaxHeight() < defaultHeight)
        defaultHeight = getMaxHeight();

    int posX = 0, posY = 0;

    getRelativeOffset(position, posX, posY, defaultWidth, defaultHeight, 0, 0);

    mDefaultWidth = defaultWidth;
    mDefaultHeight = defaultHeight;
    mDefaultPosition = position;
    mDefaultOffsetX = posX - offsetX;
    mDefaultOffsetY = posY - offsetY;
}

void Window::resetToDefaultSize()
{
    setSize(mDefaultWidth, mDefaultHeight);
    setLocationRelativeTo(mDefaultPosition, mDefaultOffsetX, mDefaultOffsetY);
    saveWindowState();
}

int Window::getResizeHandles(gcn::MouseEvent &event)
{
    int resizeHandles = 0;
    const int y = event.getY();

    if (mGrip && y > (int) mTitleBarHeight)
    {
        const int x = event.getX();

        if (!getChildrenArea().isPointInRect(x, y) && event.getSource() == this)
        {
            resizeHandles |= (x > getWidth() - resizeBorderWidth) ? RIGHT :
                              (x < resizeBorderWidth) ? LEFT : 0;
            resizeHandles |= (y > getHeight() - resizeBorderWidth) ? BOTTOM :
                              (y < resizeBorderWidth) ? TOP : 0;
        }

        if (event.getSource() == mGrip)
        {
            mDragOffsetX = x;
            mDragOffsetY = y;
            resizeHandles |= BOTTOM | RIGHT;
        }
    }

    return resizeHandles;
}

int Window::getGuiAlpha()
{
    float alpha = config.getValue("guialpha", 0.8);
    return (int) (alpha * 255.0f);
}

Layout &Window::getLayout()
{
    if (!mLayout)
        mLayout = new Layout;
    return *mLayout;
}

LayoutCell &Window::place(int x, int y, gcn::Widget *wg, int w, int h)
{
    add(wg);
    return getLayout().place(wg, x, y, w, h);
}

ContainerPlacer Window::getPlacer(int x, int y)
{
    return ContainerPlacer(this, &getLayout().at(x, y));
}

void Window::reflowLayout(int w, int h)
{
    assert(mLayout);
    mLayout->reflow(w, h);
    delete mLayout;
    mLayout = NULL;
    setContentSize(w, h);
}
