/*
 *  The ManaPlus Client
 *  Copyright (C) 2008  The Legend of Mazzeroth Development Team
 *  Copyright (C) 2009  The Mana World Development Team
 *  Copyright (C) 2009-2010  Andrei Karas
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

#include "gui/viewport.h"

#include "gamemodifiers.h"

#include "gui/popups/statuspopup.h"

#include "gui/widgets/label.h"

#include "being/localplayer.h"

#include "input/inputmanager.h"

#include "utils/stringutils.h"

#include "gui/fonts/font.h"

#include "debug.h"

#define addLabel(num) \
    { \
        Label *const label = mLabels[num]; \
        label->setPosition(0, y); \
        label->setForegroundColorAll(getThemeColor(Theme::POPUP), \
            getThemeColor(Theme::POPUP_OUTLINE)); \
        add(label); \
        y += fontHeight; \
    }

StatusPopup::StatusPopup() :
    Popup("StatusPopup", "statuspopup.xml")
{
    for (int f = 0; f < STATUSPOPUP_NUM_LABELS; f ++)
        mLabels[f] = new Label(this);
}

void StatusPopup::postInit()
{
    const int fontHeight = getFont()->getHeight();
    int y = 0;
    addLabel(0);
    addLabel(1);
    addLabel(2);
    addLabel(3);
    y += 4;
    addLabel(4);
    addLabel(5);
    addLabel(9);
    addLabel(10);
    y += 4;
    addLabel(6);
    addLabel(7);
    y += 4;
    addLabel(8);
    y += 4;
    addLabel(12);
    addLabel(13);
    addLabel(14);
    y += 4;
    addLabel(11);
}

StatusPopup::~StatusPopup()
{
}

void StatusPopup::update()
{
    updateLabels();

    int maxWidth = mLabels[0]->getWidth();

    for (int f = 0; f < STATUSPOPUP_NUM_LABELS; f ++)
    {
        const int width = mLabels[f]->getWidth();
        if (width > maxWidth)
            maxWidth = width;
    }

    const int pad2 = 2 * mPadding;
    maxWidth += pad2;
    setWidth(maxWidth);
    setHeight(mLabels[11]->getY()
        + mLabels[11]->getHeight() + pad2);
}

void StatusPopup::view(const int x, const int y)
{
    const int distance = 20;

    int posX = std::max(0, x - getWidth() / 2);
    int posY = y + distance;

    if (posX + getWidth() > mainGraphics->mWidth)
        posX = mainGraphics->mWidth - getWidth();
    if (posY + getHeight() > mainGraphics->mHeight)
        posY = y - getHeight() - distance;

    update();

    setPosition(posX, posY);
    setVisible(true);
    requestMoveToTop();
}

void StatusPopup::setLabelText(const int num,
                               const std::string &text,
                               const InputAction::Type key) const
{
    Label *const label = mLabels[num];
    label->setCaption(strprintf("%s  %s", text.c_str(),
        inputManager.getKeyValueString(static_cast<int>(key)).c_str()));
    label->adjustSize();
}

void StatusPopup::updateLabels() const
{
    if (!modifiers)
        return;

    setLabelText(0, modifiers->getMoveTypeString(),
        InputAction::INVERT_DIRECTION);
    setLabelText(1, modifiers->getCrazyMoveTypeString(),
        InputAction::CHANGE_CRAZY_MOVES_TYPE);
    setLabelText(2, modifiers->getMoveToTargetTypeString(),
        InputAction::CHANGE_MOVE_TO_TARGET);
    setLabelText(3, modifiers->getFollowModeString(),
        InputAction::CHANGE_FOLLOW_MODE);
    setLabelText(4, modifiers->getAttackWeaponTypeString(),
        InputAction::CHANGE_ATTACK_WEAPON_TYPE);
    setLabelText(5, modifiers->getAttackTypeString(),
        InputAction::CHANGE_ATTACK_TYPE);
    setLabelText(6, modifiers->getQuickDropCounterString(),
        InputAction::SWITCH_QUICK_DROP);
    setLabelText(7, modifiers->getPickUpTypeString(),
        InputAction::CHANGE_PICKUP_TYPE);
    setLabelText(8, modifiers->getMapDrawTypeString(),
        InputAction::PATHFIND);
    setLabelText(9, modifiers->getMagicAttackTypeString(),
        InputAction::SWITCH_MAGIC_ATTACK);
    setLabelText(10, modifiers->getPvpAttackTypeString(),
        InputAction::SWITCH_PVP_ATTACK);
    setLabelText(11, modifiers->getGameModifiersString(),
        InputAction::DISABLE_GAME_MODIFIERS);
    setLabelText(12, modifiers->getImitationModeString(),
        InputAction::CHANGE_IMITATION_MODE);
    setLabelText(13, modifiers->getAwayModeString(),
        InputAction::AWAY);
    setLabelText(14, modifiers->getCameraModeString(),
        InputAction::CAMERA);
}
