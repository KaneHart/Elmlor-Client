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

#include "gui/viewport.h"

#include "actormanager.h"
#include "configuration.h"
#include "game.h"
#include "settings.h"
#include "sdlshared.h"
#include "textmanager.h"

#include "resources/mapitemtype.h"

#include "resources/map/map.h"
#include "resources/map/mapitem.h"
#include "resources/map/maptype.h"
#include "resources/map/speciallayer.h"

#include "being/localplayer.h"
#include "being/playerinfo.h"

#include "input/inputmanager.h"

#include "gui/gui.h"

#include "gui/fonts/font.h"

#include "gui/popups/beingpopup.h"
#include "gui/popups/popupmenu.h"
#include "gui/popups/textpopup.h"

#include "gui/windows/ministatuswindow.h"

#include "utils/delete2.h"

#include "debug.h"

Viewport *viewport = nullptr;

extern volatile int tick_time;
extern MiniStatusWindow *miniStatusWindow;

Viewport::Viewport() :
    WindowContainer(nullptr),
    MouseListener(),
    mMap(nullptr),
    mPopupMenu(new PopupMenu),
    mHoverBeing(nullptr),
    mHoverItem(nullptr),
    mHoverSign(nullptr),
    mBeingPopup(new BeingPopup),
    mTextPopup(new TextPopup),
    mScrollRadius(config.getIntValue("ScrollRadius")),
    mScrollLaziness(config.getIntValue("ScrollLaziness")),
    mScrollCenterOffsetX(config.getIntValue("ScrollCenterOffsetX")),
    mScrollCenterOffsetY(config.getIntValue("ScrollCenterOffsetY")),
    mMouseX(0),
    mMouseY(0),
    mMousePressX(0),
    mMousePressY(0),
    mPixelViewX(0),
    mPixelViewY(0),
    mLocalWalkTime(-1),
    mCameraRelativeX(0),
    mCameraRelativeY(0),
    mShowBeingPopup(config.getBoolValue("showBeingPopup")),
    mSelfMouseHeal(config.getBoolValue("selfMouseHeal")),
    mEnableLazyScrolling(config.getBoolValue("enableLazyScrolling")),
    mMouseDirectionMove(config.getBoolValue("mouseDirectionMove")),
    mLongMouseClick(config.getBoolValue("longmouseclick")),
    mMouseClicked(false),
    mPlayerFollowMouse(false)
{
    mBeingPopup->postInit();
    mPopupMenu->postInit();
    mTextPopup->postInit();

    setOpaque(false);
    addMouseListener(this);

    config.addListener("ScrollLaziness", this);
    config.addListener("ScrollRadius", this);
    config.addListener("showBeingPopup", this);
    config.addListener("selfMouseHeal", this);
    config.addListener("enableLazyScrolling", this);
    config.addListener("mouseDirectionMove", this);
    config.addListener("longmouseclick", this);

    setFocusable(true);
}

Viewport::~Viewport()
{
    config.removeListeners(this);
    CHECKLISTENERS
    delete2(mPopupMenu);
    delete2(mBeingPopup);
    delete2(mTextPopup);
}

void Viewport::setMap(Map *const map)
{
    if (mMap && map)
        map->setDrawLayersFlags(mMap->getDrawLayersFlags());
    mMap = map;
}

void Viewport::draw(Graphics *graphics)
{
    BLOCK_START("Viewport::draw 1")
    static int lastTick = tick_time;

    if (!mMap || !player_node)
    {
        graphics->setColor(Color(64, 64, 64));
        graphics->fillRectangle(
                Rect(0, 0, getWidth(), getHeight()));
        BLOCK_END("Viewport::draw 1")
        return;
    }

    // Avoid freaking out when tick_time overflows
    if (tick_time < lastTick)
        lastTick = tick_time;

    // Calculate viewpoint
    const int midTileX = (graphics->mWidth + mScrollCenterOffsetX) / 2;
    const int midTileY = (graphics->mHeight + mScrollCenterOffsetY) / 2;

    const Vector &playerPos = player_node->getPosition();
    const int player_x = static_cast<int>(playerPos.x)
                         - midTileX + mCameraRelativeX;
    const int player_y = static_cast<int>(playerPos.y)
                         - midTileY + mCameraRelativeY;

    if (mScrollLaziness < 1)
        mScrollLaziness = 1;  // Avoids division by zero

    if (mEnableLazyScrolling)
    {
        int cnt = 0;

        // Apply lazy scrolling
        while (lastTick < tick_time && cnt < mapTileSize)
        {
            if (player_x > mPixelViewX + mScrollRadius)
            {
                mPixelViewX += static_cast<int>(
                    static_cast<float>(player_x
                    - mPixelViewX - mScrollRadius) /
                    static_cast<float>(mScrollLaziness));
            }
            if (player_x < mPixelViewX - mScrollRadius)
            {
                mPixelViewX += static_cast<int>(
                    static_cast<float>(player_x
                    - mPixelViewX + mScrollRadius) /
                    static_cast<float>(mScrollLaziness));
            }
            if (player_y > mPixelViewY + mScrollRadius)
            {
                mPixelViewY += static_cast<int>(
                    static_cast<float>(player_y
                    - mPixelViewY - mScrollRadius) /
                    static_cast<float>(mScrollLaziness));
            }
            if (player_y < mPixelViewY - mScrollRadius)
            {
                mPixelViewY += static_cast<int>(
                    static_cast<float>(player_y
                    - mPixelViewY + mScrollRadius) /
                    static_cast<float>(mScrollLaziness));
            }
            lastTick ++;
            cnt ++;
        }

        // Auto center when player is off screen
        if (cnt > 30 || player_x - mPixelViewX
            > graphics->mWidth / 2 || mPixelViewX
            - player_x > graphics->mWidth / 2 || mPixelViewY
            - player_y > graphics->getHeight() / 2 ||  player_y
            - mPixelViewY > graphics->getHeight() / 2)
        {
            if (player_x <= 0 || player_y <= 0)
            {
                logger->log("incorrect player position: %d, %d, %d, %d",
                    player_x, player_y, mPixelViewX, mPixelViewY);
                if (player_node)
                {
                    logger->log("tile position: %d, %d",
                        player_node->getTileX(), player_node->getTileY());
                }
            }
            mPixelViewX = player_x;
            mPixelViewY = player_y;
        }
    }
    else
    {
        mPixelViewX = player_x;
        mPixelViewY = player_y;
    }

    // Don't move camera so that the end of the map is on screen
    const int viewXmax =
        mMap->getWidth() * mMap->getTileWidth() - graphics->mWidth;
    const int viewYmax =
        mMap->getHeight() * mMap->getTileHeight() - graphics->mHeight;

    if (mPixelViewX < 0)
        mPixelViewX = 0;
    if (mPixelViewY < 0)
        mPixelViewY = 0;
    if (mPixelViewX > viewXmax)
        mPixelViewX = viewXmax;
    if (mPixelViewY > viewYmax)
        mPixelViewY = viewYmax;

    // Draw tiles and sprites
    mMap->draw(graphics, mPixelViewX, mPixelViewY);

    const MapType::MapType drawType = settings.mapDrawType;
    if (drawType != MapType::NORMAL)
    {
        if (drawType != MapType::SPECIAL4)
        {
            mMap->drawCollision(graphics, mPixelViewX,
                mPixelViewY, drawType);
        }
        if (drawType == MapType::DEBUG)
            drawDebugPath(graphics);
    }

    if (player_node->getCheckNameSetting())
    {
        player_node->setCheckNameSetting(false);
        player_node->setName(player_node->getName());
    }

    // Draw text
    if (textManager)
        textManager->draw(graphics, mPixelViewX, mPixelViewY);

    // Draw player names, speech, and emotion sprite as needed
    const ActorSprites &actors = actorManager->getAll();
    FOR_EACH (ActorSpritesIterator, it, actors)
    {
        if ((*it)->getType() == ActorType::FLOOR_ITEM)
            continue;
        Being *const b = static_cast<Being*>(*it);
        b->drawSpeech(mPixelViewX, mPixelViewY);
        b->drawEmotion(graphics, mPixelViewX, mPixelViewY);
    }

    if (miniStatusWindow)
        miniStatusWindow->drawIcons(graphics);

    // Draw contained widgets
    WindowContainer::draw(graphics);
    BLOCK_END("Viewport::draw 1")
}

void Viewport::logic()
{
    BLOCK_START("Viewport::logic")
    // Make the player follow the mouse position
    // if the mouse is dragged elsewhere than in a window.
    Gui::getMouseState(&mMouseX, &mMouseY);
    BLOCK_END("Viewport::logic")
}

void Viewport::followMouse()
{
    if (!gui)
        return;
    const MouseStateType button = Gui::getMouseState(&mMouseX, &mMouseY);
    // If the left button is dragged
    if (mPlayerFollowMouse && (button & SDL_BUTTON(1)))
    {
        // We create a mouse event and send it to mouseDragged.
        MouseEvent event(nullptr,
            MouseEventType::DRAGGED,
            MouseButton::LEFT,
            mMouseX,
            mMouseY,
            0);

        walkByMouse(event);
    }
}

void Viewport::drawDebugPath(Graphics *const graphics)
{
    if (!player_node || !userPalette || !actorManager || !mMap || !gui)
        return;

    Gui::getMouseState(&mMouseX, &mMouseY);

    static Path debugPath;
    static Vector lastMouseDestination = Vector(0.0F, 0.0F);
    const int mousePosX = mMouseX + mPixelViewX;
    const int mousePosY = mMouseY + mPixelViewY;
    Vector mouseDestination(mousePosX, mousePosY);

    if (mouseDestination.x != lastMouseDestination.x
        || mouseDestination.y != lastMouseDestination.y)
    {
        const Vector &playerPos = player_node->getPosition();

        debugPath = mMap->findPath(
            static_cast<int>(playerPos.x - mapTileSize / 2) / mapTileSize,
            static_cast<int>(playerPos.y - mapTileSize) / mapTileSize,
            mousePosX / mapTileSize, mousePosY / mapTileSize,
            player_node->getWalkMask(),
            500);
        lastMouseDestination = mouseDestination;
    }
    drawPath(graphics, debugPath, userPalette->getColorWithAlpha(
        UserPalette::ROAD_POINT));

    const ActorSprites &actors = actorManager->getAll();
    FOR_EACH (ActorSpritesConstIterator, it, actors)
    {
        const Being *const being = dynamic_cast<const Being*>(*it);
        if (being && being != player_node)
        {
            const Path &beingPath = being->getPath();
            drawPath(graphics, beingPath, userPalette->getColorWithAlpha(
                UserPalette::ROAD_POINT));
        }
    }
}

void Viewport::drawPath(Graphics *const graphics,
                        const Path &path,
                        const Color &color) const
{
    graphics->setColor(color);
    Font *const font = getFont();

    int cnt = 1;
    FOR_EACH (Path::const_iterator, i, path)
    {
        const int squareX = i->x * mapTileSize - mPixelViewX + 12;
        const int squareY = i->y * mapTileSize - mPixelViewY + 12;

        graphics->fillRectangle(Rect(squareX, squareY, 8, 8));
        if (mMap)
        {
            const std::string str = toString(cnt);
            font->drawString(graphics, str, squareX + 4
                - font->getWidth(str) / 2, squareY + 12);
        }
        cnt ++;
    }
}

bool Viewport::openContextMenu(const MouseEvent &event)
{
    mPlayerFollowMouse = false;
    const int eventX = event.getX();
    const int eventY = event.getY();
    if (mHoverBeing)
    {
        validateSpeed();
        if (actorManager)
        {
            std::vector<ActorSprite*> beings;
            const int x = mMouseX + mPixelViewX;
            const int y = mMouseY + mPixelViewY;
            actorManager->findBeingsByPixel(beings, x, y, true);
            if (beings.size() > 1)
                mPopupMenu->showPopup(eventX, eventY, beings);
            else
                mPopupMenu->showPopup(eventX, eventY, mHoverBeing);
            return true;
        }
    }
    else if (mHoverItem)
    {
        validateSpeed();
        mPopupMenu->showPopup(eventX, eventY, mHoverItem);
        return true;
    }
    else if (mHoverSign)
    {
        validateSpeed();
        mPopupMenu->showPopup(eventX, eventY, mHoverSign);
        return true;
    }
    else if (settings.cameraMode)
    {
        if (!mMap)
            return false;
        mPopupMenu->showMapPopup(eventX, eventY,
            (mMouseX + mPixelViewX) / mMap->getTileWidth(),
            (mMouseY + mPixelViewY) / mMap->getTileHeight());
        return true;
    }
    return false;
}

bool Viewport::leftMouseAction()
{
    // Interact with some being
    if (mHoverBeing)
    {
        if (!mHoverBeing->isAlive())
            return true;

        if (mHoverBeing->canTalk())
        {
            validateSpeed();
            mHoverBeing->talkTo();
            return true;
        }
        else
        {
            const ActorType::Type type = mHoverBeing->getType();
            if (type == ActorType::PLAYER)
            {
                validateSpeed();
                if (actorManager)
                {
                    if (player_node != mHoverBeing || mSelfMouseHeal)
                        actorManager->heal(mHoverBeing);
                    if (player_node == mHoverBeing && mHoverItem)
                        player_node->pickUp(mHoverItem);
                    return true;
                }
            }
            else if (type == ActorType::MONSTER || type == ActorType::NPC)
            {
                if (player_node->withinAttackRange(mHoverBeing) ||
                    inputManager.isActionActive(static_cast<int>(
                    InputAction::ATTACK)))
                {
                    validateSpeed();
                    if (!mStatsReUpdated && player_node != mHoverBeing)
                    {
                        player_node->attack(mHoverBeing,
                            !inputManager.isActionActive(
                            static_cast<int>(InputAction::STOP_ATTACK)));
                        return true;
                    }
                }
                else if (!inputManager.isActionActive(static_cast<int>(
                         InputAction::ATTACK)))
                {
                    validateSpeed();
                    if (!mStatsReUpdated && player_node != mHoverBeing)
                    {
                        player_node->setGotoTarget(mHoverBeing);
                        return true;
                    }
                }
            }
        }
    }
    // Picks up a item if we clicked on one
    if (mHoverItem)
    {
        validateSpeed();
        player_node->pickUp(mHoverItem);
    }
    // Just walk around
    else if (!inputManager.isActionActive(static_cast<int>(
             InputAction::ATTACK)))
    {
        validateSpeed();
        player_node->stopAttack();
        player_node->cancelFollow();
        mPlayerFollowMouse = true;

        // Make the player go to the mouse position
        followMouse();
    }
    return false;
}

void Viewport::mousePressed(MouseEvent &event)
{
    if (event.getSource() != this || event.isConsumed())
        return;

    mMouseClicked = true;
    // Check if we are alive and kickin'
    if (!mMap || !player_node)
        return;

    // Check if we are busy
    // if commented, allow context menu if npc dialog open
    if (PlayerInfo::isTalking())
        return;

    mMousePressX = event.getX();
    mMousePressY = event.getY();
    const unsigned int eventButton = event.getButton();
    const int pixelX = mMousePressX + mPixelViewX;
    const int pixelY = mMousePressY + mPixelViewY;

    // Right click might open a popup
    if (eventButton == MouseButton::RIGHT)
    {
        if (openContextMenu(event))
            return;
    }

    // If a popup is active, just remove it
    if (mPopupMenu->isPopupVisible())
    {
        mPlayerFollowMouse = false;
        mPopupMenu->setVisible(false);
        return;
    }

    // Left click can cause different actions
    if (!mLongMouseClick && eventButton == MouseButton::LEFT)
    {
        if (leftMouseAction())
        {
            mPlayerFollowMouse = false;
            return;
        }
    }
    else if (eventButton == MouseButton::MIDDLE)
    {
        mPlayerFollowMouse = false;
        validateSpeed();
        // Find the being nearest to the clicked position
        if (actorManager)
        {
            Being *const target = actorManager->findNearestLivingBeing(
                pixelX, pixelY, 20, ActorType::MONSTER, nullptr);

            if (target)
                player_node->setTarget(target);
        }
    }
}

void Viewport::walkByMouse(const MouseEvent &event)
{
    if (!mMap || !player_node)
        return;
    if (mPlayerFollowMouse
        && !inputManager.isActionActive(InputAction::STOP_ATTACK)
        && !inputManager.isActionActive(InputAction::UNTARGET))
    {
        if (!mMouseDirectionMove)
            mPlayerFollowMouse = false;
        if (mLocalWalkTime != player_node->getActionTime())
        {
            mLocalWalkTime = cur_time;
            player_node->unSetPickUpTarget();
            int playerX = player_node->getTileX();
            int playerY = player_node->getTileY();
            if (mMouseDirectionMove)
            {
                const int width = mainGraphics->mWidth / 2;
                const int height = mainGraphics->mHeight / 2;
                const float wh = static_cast<float>(width)
                    / static_cast<float>(height);
                int x = event.getX() - width;
                int y = event.getY() - height;
                if (!x && !y)
                    return;
                const int x2 = abs(x);
                const int y2 = abs(y);
                const float diff = 2;
                int dx = 0;
                int dy = 0;
                if (x2 > y2)
                {
                    if (y2 && static_cast<float>(x2) / static_cast<float>(y2)
                        / wh > diff)
                    {
                        y = 0;
                    }
                }
                else
                {
                    if (x2 && y2 * wh / x2 > diff)
                        x = 0;
                }
                if (x > 0)
                    dx = 1;
                else if (x < 0)
                    dx = -1;
                if (y > 0)
                    dy = 1;
                else if (y < 0)
                    dy = -1;

                if (mMap->getWalk(playerX + dx, playerY + dy))
                {
                    player_node->navigateTo(playerX + dx, playerY + dy);
                }
                else
                {
                    if (dx && dy)
                    {
                        // try avoid diagonal collision
                        if (x2 > y2)
                        {
                            if (mMap->getWalk(playerX + dx, playerY))
                                dy = 0;
                            else
                                dx = 0;
                        }
                        else
                        {
                            if (mMap->getWalk(playerX, playerY + dy))
                                dx = 0;
                            else
                                dy = 0;
                        }
                    }
                    else
                    {
                        // try avoid vertical or horisontal collision
                        if (!dx)
                        {
                            if (mMap->getWalk(playerX + 1, playerY + dy))
                                dx = 1;
                            if (mMap->getWalk(playerX - 1, playerY + dy))
                                dx = -1;
                        }
                        if (!dy)
                        {
                            if (mMap->getWalk(playerX + dx, playerY + 1))
                                dy = 1;
                            if (mMap->getWalk(playerX + dx, playerY - 1))
                                dy = -1;
                        }
                    }
                    player_node->navigateTo(playerX + dx, playerY + dy);
                }
            }
            else
            {
                const int destX = static_cast<int>((event.getX() + mPixelViewX)
                    / static_cast<float>(mMap->getTileWidth()));
                const int destY = static_cast<int>((event.getY() + mPixelViewY)
                    / static_cast<float>(mMap->getTileHeight()));
                if (playerX != destX || playerY != destY)
                {
                    if (!player_node->navigateTo(destX, destY))
                    {
                        if (playerX > destX)
                            playerX --;
                        else if (playerX < destX)
                            playerX ++;
                        if (playerY > destY)
                            playerY --;
                        else if (playerY < destY)
                            playerY ++;
                        if (mMap->getWalk(playerX, playerY, 0))
                            player_node->navigateTo(playerX, playerY);
                    }
                }
            }
        }
    }
}

void Viewport::mouseDragged(MouseEvent &event)
{
    if (event.getSource() != this || event.isConsumed())
    {
        mPlayerFollowMouse = false;
        return;
    }
    if (mMouseClicked)
    {
        if (abs(event.getX() - mMousePressX) > 32
            || abs(event.getY() - mMousePressY) > 32)
        {
            mPlayerFollowMouse = true;
        }
    }

    walkByMouse(event);
}

void Viewport::mouseReleased(MouseEvent &event)
{
    mPlayerFollowMouse = false;
    mLocalWalkTime = -1;
    if (mLongMouseClick && mMouseClicked)
    {
        mMouseClicked = false;
        if (event.getSource() != this || event.isConsumed())
            return;
        const unsigned int eventButton = event.getButton();
        if (eventButton == MouseButton::LEFT)
        {
            // long button press
            if (gui && gui->isLongPress())
            {
                if (openContextMenu(event))
                {
                    gui->resetClickCount();
                    return;
                }
            }
            else
            {
                if (leftMouseAction())
                    return;
            }
            walkByMouse(event);
        }
    }
}

void Viewport::showPopup(Window *const parent,
                         const int x, const int y,
                         Item *const item,
                         const bool isInventory)
{
    mPopupMenu->showPopup(parent, x, y, item, isInventory);
}

void Viewport::showPopup(MapItem *const item)
{
    mPopupMenu->showPopup(mMouseX, mMouseY, item);
}

void Viewport::showPopup(Window *const parent,
                         Item *const item,
                         const bool isInventory)
{
    mPopupMenu->showPopup(parent, mMouseX, mMouseY, item, isInventory);
}

void Viewport::showItemPopup(Item *const item)
{
    mPopupMenu->showItemPopup(mMouseX, mMouseY, item);
}

void Viewport::showItemPopup(const int itemId,
                             const unsigned char color)
{
    mPopupMenu->showItemPopup(mMouseX, mMouseY, itemId, color);
}

void Viewport::showDropPopup(Item *const item)
{
    mPopupMenu->showDropPopup(mMouseX, mMouseY, item);
}

void Viewport::showOutfitsPopup(const int x, const int y)
{
    mPopupMenu->showOutfitsPopup(x, y);
}

void Viewport::showOutfitsPopup()
{
    mPopupMenu->showOutfitsPopup(mMouseX, mMouseY);
}

void Viewport::showSpellPopup(TextCommand *const cmd)
{
    mPopupMenu->showSpellPopup(mMouseX, mMouseY, cmd);
}

void Viewport::showChatPopup(const int x, const int y,
                             ChatTab *const tab)
{
    mPopupMenu->showChatPopup(x, y, tab);
}

void Viewport::showChatPopup(ChatTab *const tab)
{
    mPopupMenu->showChatPopup(mMouseX, mMouseY, tab);
}

void Viewport::showPopup(const int x, const int y,
                         const Being *const being)
{
    mPopupMenu->showPopup(x, y, being);
}

void Viewport::showPopup(const Being *const being)
{
    mPopupMenu->showPopup(mMouseX, mMouseY, being);
}

void Viewport::showPlayerPopup(const std::string &nick)
{
    mPopupMenu->showPlayerPopup(mMouseX, mMouseY, nick);
}

void Viewport::showPopup(const int x, const int y,
                         Button *const button)
{
    mPopupMenu->showPopup(x, y, button);
}

void Viewport::showPopup(const int x, const int y,
                         const ProgressBar *const bar)
{
    mPopupMenu->showPopup(x, y, bar);
}

void Viewport::showAttackMonsterPopup(const std::string &name,
                                      const int type)
{
    mPopupMenu->showAttackMonsterPopup(mMouseX, mMouseY, name, type);
}

void Viewport::showPickupItemPopup(const std::string &name)
{
    mPopupMenu->showPickupItemPopup(mMouseX, mMouseY, name);
}

void Viewport::showUndressPopup(const int x, const int y,
                                const Being *const being,
                                Item *const item)
{
    mPopupMenu->showUndressPopup(x, y, being, item);
}

void Viewport::showMapPopup(const int x, const int y)
{
    mPopupMenu->showMapPopup(mMouseX, mMouseY, x, y);
}

void Viewport::showTextFieldPopup(TextField *const input)
{
    mPopupMenu->showTextFieldPopup(mMouseX, mMouseY, input);
}

void Viewport::showLinkPopup(const std::string &link)
{
    mPopupMenu->showLinkPopup(mMouseX, mMouseY, link);
}

void Viewport::showWindowsPopup()
{
    mPopupMenu->showWindowsPopup(mMouseX, mMouseY);
}

void Viewport::showNpcDialogPopup(const int npcId)
{
    mPopupMenu->showNpcDialogPopup(npcId, mMouseX, mMouseY);
}

void Viewport::showWindowPopup(Window *const window)
{
    mPopupMenu->showWindowPopup(window, mMouseX, mMouseY);
}

void Viewport::closePopupMenu()
{
    if (mPopupMenu)
        mPopupMenu->handleLink("cancel", nullptr);
}

void Viewport::optionChanged(const std::string &name)
{
    if (name == "ScrollLaziness")
        mScrollLaziness = config.getIntValue("ScrollLaziness");
    else if (name == "ScrollRadius")
        mScrollRadius = config.getIntValue("ScrollRadius");
    else if (name == "showBeingPopup")
        mShowBeingPopup = config.getBoolValue("showBeingPopup");
    else if (name == "selfMouseHeal")
        mSelfMouseHeal = config.getBoolValue("selfMouseHeal");
    else if (name == "enableLazyScrolling")
        mEnableLazyScrolling = config.getBoolValue("enableLazyScrolling");
    else if (name == "mouseDirectionMove")
        mMouseDirectionMove = config.getBoolValue("mouseDirectionMove");
    else if (name == "longmouseclick")
        mLongMouseClick = config.getBoolValue("longmouseclick");
}

void Viewport::mouseMoved(MouseEvent &event A_UNUSED)
{
    // Check if we are on the map
    if (!mMap || !player_node || !actorManager)
        return;

    if (mMouseDirectionMove)
        mPlayerFollowMouse = false;

    const int x = mMouseX + mPixelViewX;
    const int y = mMouseY + mPixelViewY;

    ActorType::Type type = ActorType::UNKNOWN;
    mHoverBeing = actorManager->findBeingByPixel(x, y, true);
    if (mHoverBeing)
        type = mHoverBeing->getType();
    if (mHoverBeing
        && (type == ActorType::PLAYER
        || type == ActorType::NPC
        || type == ActorType::PET))
    {
        mTextPopup->setVisible(false);
        if (mShowBeingPopup)
            mBeingPopup->show(mMouseX, mMouseY, mHoverBeing);
    }
    else
    {
        mBeingPopup->setVisible(false);
    }

    mHoverItem = actorManager->findItem(x / mMap->getTileWidth(),
        y / mMap->getTileHeight());

    if (!mHoverBeing && !mHoverItem)
    {
        const SpecialLayer *const specialLayer = mMap->getSpecialLayer();
        if (specialLayer)
        {
            const int mouseTileX = (mMouseX + mPixelViewX)
                / mMap->getTileWidth();
            const int mouseTileY = (mMouseY + mPixelViewY)
                / mMap->getTileHeight();

            mHoverSign = specialLayer->getTile(mouseTileX, mouseTileY);
            if (mHoverSign && mHoverSign->getType() != MapItemType::EMPTY)
            {
                if (!mHoverSign->getComment().empty())
                {
                    mBeingPopup->setVisible(false);
                    mTextPopup->show(mMouseX, mMouseY,
                        mHoverSign->getComment());
                }
                else
                {
                    if (mTextPopup->isPopupVisible())
                        mTextPopup->setVisible(false);
                }
                gui->setCursorType(Cursor::CURSOR_UP);
                return;
            }
        }
    }
    if (mTextPopup->isPopupVisible())
        mTextPopup->setVisible(false);

    if (mHoverBeing)
    {
        switch (type)
        {
            case ActorType::NPC:
                gui->setCursorType(mHoverBeing->getHoverCursor());
                break;

            case ActorType::MONSTER:
                gui->setCursorType(mHoverBeing->getHoverCursor());
                break;

            case ActorType::PORTAL:
                gui->setCursorType(mHoverBeing->getHoverCursor());
                break;

            case ActorType::AVATAR:
            case ActorType::FLOOR_ITEM:
            case ActorType::UNKNOWN:
            case ActorType::PLAYER:
            case ActorType::PET:
            default:
                gui->setCursorType(Cursor::CURSOR_POINTER);
                break;
        }
    }
    // Item mouseover
    else if (mHoverItem)
    {
        gui->setCursorType(mHoverItem->getHoverCursor());
    }
    else
    {
        gui->setCursorType(Cursor::CURSOR_POINTER);
    }
}

void Viewport::toggleMapDrawType()
{
    settings.mapDrawType = static_cast<MapType::MapType>(
        static_cast<int>(settings.mapDrawType) + 1);
    if (settings.mapDrawType > MapType::BLACKWHITE)
        settings.mapDrawType = MapType::NORMAL;
    if (mMap)
        mMap->setDrawLayersFlags(settings.mapDrawType);
}

void Viewport::toggleCameraMode()
{
    settings.cameraMode ++;
    if (settings.cameraMode > 1)
        settings.cameraMode = 0;
    if (!settings.cameraMode)
    {
        mCameraRelativeX = 0;
        mCameraRelativeY = 0;
    }
    UpdateStatusListener::distributeEvent();
}

void Viewport::hideBeingPopup()
{
    if (mBeingPopup)
        mBeingPopup->setVisible(false);
    if (mTextPopup)
        mTextPopup->setVisible(false);
}

void Viewport::clearHover(const ActorSprite *const actor)
{
    if (mHoverBeing == actor)
        mHoverBeing = nullptr;

    if (mHoverItem == actor)
        mHoverItem = nullptr;
}

void Viewport::cleanHoverItems()
{
    mHoverBeing = nullptr;
    mHoverItem = nullptr;
    mHoverSign = nullptr;
}

void Viewport::moveCamera(const int dx, const int dy)
{
    mCameraRelativeX += dx;
    mCameraRelativeY += dy;
}

bool Viewport::isPopupMenuVisible() const
{
    return mPopupMenu ? mPopupMenu->isPopupVisible() : false;
}

void Viewport::moveCameraToActor(const int actorId,
                                 const int x, const int y)
{
    if (!player_node || !actorManager)
        return;

    const Actor *const actor = actorManager->findBeing(actorId);
    if (!actor)
        return;
    const Vector &actorPos = actor->getPosition();
    const Vector &playerPos = player_node->getPosition();
    settings.cameraMode = 1;
    mCameraRelativeX = static_cast<int>(actorPos.x - playerPos.x) + x;
    mCameraRelativeY = static_cast<int>(actorPos.y - playerPos.y) + y;
}

void Viewport::moveCameraToPosition(const int x, const int y)
{
    if (!player_node)
        return;

    const Vector &playerPos = player_node->getPosition();
    settings.cameraMode = 1;

    mCameraRelativeX = x - static_cast<int>(playerPos.x);
    mCameraRelativeY = y - static_cast<int>(playerPos.y);
}

void Viewport::moveCameraRelative(const int x, const int y)
{
    settings.cameraMode = 1;
    mCameraRelativeX += x;
    mCameraRelativeY += y;
}

void Viewport::returnCamera()
{
    settings.cameraMode = 0;
    mCameraRelativeX = 0;
    mCameraRelativeY = 0;
}

void Viewport::validateSpeed()
{
    if (!inputManager.isActionActive(static_cast<int>(
        InputAction::TARGET_ATTACK)) && !inputManager.isActionActive(
        static_cast<int>(InputAction::ATTACK)))
    {
        if (Game::instance())
            Game::instance()->setValidSpeed();
    }
}

void Viewport::clearPopup()
{
    if (mPopupMenu)
        mPopupMenu->clear();
}
