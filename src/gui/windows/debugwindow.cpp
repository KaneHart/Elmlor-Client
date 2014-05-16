/*
 *  The ManaPlus Client
 *  Copyright (C) 2004-2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  The Mana Developers
 *  Copyright (C) 2011-2014  The ManaPlus Developers
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

#include "gui/windows/debugwindow.h"

#include "game.h"
#include "main.h"

#include "being/localplayer.h"

#include "particle/particle.h"

#include "gui/viewport.h"

#include "gui/windows/setupwindow.h"

#include "gui/widgets/containerplacer.h"
#include "gui/widgets/label.h"
#include "gui/widgets/layoutcell.h"
#include "gui/widgets/layouthelper.h"
#include "gui/widgets/tabbedarea.h"

#include "gui/widgets/tabs/debugwindowtabs.h"

#include "resources/imagehelper.h"

#include "resources/map/map.h"

#include "net/packetcounters.h"

#include "utils/delete2.h"
#include "utils/gettext.h"
#include "utils/stringutils.h"
#include "utils/timer.h"

#include "debug.h"

DebugWindow::DebugWindow() :
    // TRANSLATORS: debug window name
    Window(_("Debug"), false, nullptr, "debug.xml"),
    mTabs(new TabbedArea(this)),
    mMapWidget(new MapDebugTab(this)),
    mTargetWidget(new TargetDebugTab(this)),
    mNetWidget(new NetDebugTab(this))
{
    mTabs->postInit();
    setWindowName("Debug");
    if (setupWindow)
        setupWindow->registerWindowForReset(this);

    setResizable(true);
    setCloseButton(true);
    setSaveVisible(true);
    setStickyButtonLock(true);

    setDefaultSize(400, 300, ImageRect::CENTER);

    // TRANSLATORS: debug window tab
    mTabs->addTab(std::string(_("Map")), mMapWidget);
    // TRANSLATORS: debug window tab
    mTabs->addTab(std::string(_("Target")), mTargetWidget);
    // TRANSLATORS: debug window tab
    mTabs->addTab(std::string(_("Net")), mNetWidget);

    mTabs->setDimension(Rect(0, 0, 600, 300));

    const int w = mDimension.width;
    const int h = mDimension.height;
    mMapWidget->resize(w, h);
    mTargetWidget->resize(w, h);
    mNetWidget->resize(w, h);
    loadWindowState();
    enableVisibleSound(true);
}

DebugWindow::~DebugWindow()
{
    delete2(mMapWidget);
    delete2(mTargetWidget);
    delete2(mNetWidget);
}

void DebugWindow::postInit()
{
    add(mTabs);
}

void DebugWindow::slowLogic()
{
    BLOCK_START("DebugWindow::slowLogic")
    if (!isWindowVisible() || !mTabs)
    {
        BLOCK_END("DebugWindow::slowLogic")
        return;
    }

    switch (mTabs->getSelectedTabIndex())
    {
        default:
        case 0:
            mMapWidget->logic();
            break;
        case 1:
            mTargetWidget->logic();
            break;
        case 2:
            mNetWidget->logic();
            break;
    }

    if (player_node)
        player_node->tryPingRequest();
    BLOCK_END("DebugWindow::slowLogic")
}

void DebugWindow::draw(Graphics *g)
{
    BLOCK_START("DebugWindow::draw")
    Window::draw(g);

    if (player_node)
    {
        const Being *const target = player_node->getTarget();
        if (target)
        {
            target->draw(g, -target->getPixelX() + mapTileSize / 2
                + mDimension.width / 2, -target->getPixelY() + mapTileSize
                + mDimension.height / 2);
        }
    }
    BLOCK_END("DebugWindow::draw")
}

void DebugWindow::widgetResized(const Event &event)
{
    Window::widgetResized(event);

    mTabs->setDimension(Rect(0, 0,
        mDimension.width, mDimension.height));
}

#ifdef USE_PROFILER
void DebugWindow::logicChildren()
{
    BLOCK_START("DebugWindow::logicChildren")
    BasicContainer::logicChildren();
    BLOCK_END("DebugWindow::logicChildren")
}
#endif

