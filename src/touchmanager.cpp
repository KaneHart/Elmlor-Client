/*
 *  The ManaPlus Client
 *  Copyright (C) 2012  The ManaPlus Developers
 *
 *  This file is part of The ManaPlus Client.
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
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "touchmanager.h"

#include "configuration.h"
#include "graphics.h"
#include "touchactions.h"

#include "gui/theme.h"

#include "debug.h"

TouchManager touchManager;

TouchManager::TouchManager() :
    mKeyboard(nullptr),
    mPad(nullptr)
{
}

TouchManager::~TouchManager()
{
    clear();
}

void TouchManager::init()
{
#ifdef ANDROID
    loadTouchItem(&mKeyboard, "keyboard_icon.xml", false,
        nullptr, nullptr, &showKeyboard, nullptr);
#endif

    if (config.getBoolValue("showScreenJoystick"))
    {
        loadTouchItem(&mPad, "dpad.xml", true,
            &padEvents, &padClick, &padUp, &padOut);
    }
}

void TouchManager::loadTouchItem(TouchItem **item, std::string name, bool type,
                                 TouchFuncPtr fAll, TouchFuncPtr fPressed,
                                 TouchFuncPtr fReleased, TouchFuncPtr fOut)
{
    *item = nullptr;
    Theme *theme = Theme::instance();
    if (!theme)
        return;
    Skin *const skin = theme->load(name, "");
    if (skin)
    {
        const ImageRect &images = skin->getBorder();
        Image *image = images.grid[0];
        if (image)
        {
            image->incRef();
            const int x = skin->getOption("x", 10);
            const int y = type ? (mainGraphics->mHeight - image->mBounds.h) / 2
                + skin->getOption("y", 10) : skin->getOption("y", 10);
            const int pad = skin->getPadding();
            const int pad2 = 2 * pad;
            *item = new TouchItem(gcn::Rectangle(x, y,
                image->getWidth() + pad2, image->getHeight() + pad2),
                image, x + pad, y + pad, fAll, fPressed, fReleased, fOut);
            mObjects.push_back(*item);
        }
        theme->unload(skin);
    }
}

void TouchManager::clear()
{
//    unloadTouchItem(&mPad);
//    unloadTouchItem(&mKeyboard);

    for (TouchItemVectorCIter it = mObjects.begin(), it_end = mObjects.end();
         it != it_end; ++ it)
    {
        TouchItem *item = *it;
        if (item)
        {
            if (item->image)
                item->image->decRef();
            delete item;
        }
    }
    mObjects.clear();
}

void TouchManager::unloadTouchItem(TouchItem **item0)
{
    TouchItem *item = *item0;
    if (item)
    {
        if (item->image)
            item->image->decRef();
        delete item;
        *item0 = nullptr;
    }
}

void TouchManager::draw()
{
//    drawTouchItem(mPad);
    for (TouchItemVectorCIter it = mObjects.begin(), it_end = mObjects.end();
         it != it_end; ++ it)
    {
        drawTouchItem(*it);
    }
//    drawTouchItem(mKeyboard);
}

void TouchManager::drawTouchItem(const TouchItem *const item) const
{
    if (item && item->image)
        mainGraphics->drawImage(item->image, item->x, item->y);
}

bool TouchManager::processEvent(const gcn::MouseInput &mouseInput)
{
    const int x = mouseInput.getX();
    const int y = mouseInput.getY();

    for (TouchItemVectorCIter it = mObjects.begin(), it_end = mObjects.end();
         it != it_end; ++ it)
    {
        const TouchItem *const item = *it;
        if (!item)
            continue;
        const gcn::Rectangle &rect = item->rect;
        if (rect.isPointInRect(x, y))
        {
            gcn::MouseInput event = mouseInput;
            event.setX(event.getX() - item->x);
            event.setY(event.getY() - item->y);
            if (item->funcAll)
                item->funcAll(event);

            switch (mouseInput.getType())
            {
                case gcn::MouseInput::PRESSED:
                    if (item->funcPressed)
                        item->funcPressed(event);
                    break;
                case gcn::MouseInput::RELEASED:
                    if (item->funcReleased)
                        item->funcReleased(event);
                    break;
                default:
                    break;
            }
            return true;
        }
        else if (item->funcOut)
        {
            item->funcOut(mouseInput);
        }
    }
    return false;
}

bool TouchManager::isActionActive(const int index) const
{
    if (index < 0 || index > actionsSize)
        return false;
    return mActions[index];
}