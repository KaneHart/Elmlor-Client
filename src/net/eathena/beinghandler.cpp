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

#include "net/eathena/beinghandler.h"

#include "actormanager.h"
#include "guild.h"
#include "guildmanager.h"
#include "party.h"

#include "being/localplayer.h"

#include "input/keyboardconfig.h"

#include "gui/windows/socialwindow.h"
#include "gui/windows/killstats.h"

#include "gui/windows/outfitwindow.h"

#include "net/eathena/messageout.h"
#include "net/eathena/protocol.h"
#include "net/eathena/sprite.h"

#include "resources/iteminfo.h"

#include "resources/db/itemdb.h"

#include "utils/stringutils.h"
#include "utils/timer.h"

#include "debug.h"

extern Net::BeingHandler *beingHandler;
extern int serverVersion;

namespace EAthena
{

BeingHandler::BeingHandler(const bool enableSync) :
    MessageHandler(),
    Ea::BeingHandler(enableSync)
{
    static const uint16_t _messages[] =
    {
        SMSG_BEING_VISIBLE,
        SMSG_BEING_MOVE,
        SMSG_BEING_MOVE2,
        SMSG_BEING_REMOVE,
        SMSG_SKILL_DAMAGE,
        SMSG_BEING_ACTION,
        SMSG_BEING_SELFEFFECT,
        SMSG_BEING_EMOTION,
        SMSG_BEING_CHANGE_LOOKS,
        SMSG_BEING_CHANGE_LOOKS2,
        SMSG_BEING_NAME_RESPONSE,
        SMSG_BEING_NAME_RESPONSE2,
        SMSG_PLAYER_GUILD_PARTY_INFO,
        SMSG_BEING_CHANGE_DIRECTION,
        SMSG_PLAYER_UPDATE_1,
        SMSG_PLAYER_UPDATE_2,
        SMSG_PLAYER_MOVE,
        SMSG_PLAYER_STOP,
        SMSG_PLAYER_MOVE_TO_ATTACK,
        SMSG_PLAYER_STATUS_CHANGE,
        SMSG_BEING_STATUS_CHANGE,
        SMSG_BEING_RESURRECT,
        SMSG_SOLVE_CHAR_NAME,
        SMSG_BEING_SPAWN,
        SMSG_SKILL_CASTING,
        SMSG_SKILL_CAST_CANCEL,
        SMSG_SKILL_NO_DAMAGE,
        SMSG_BEING_IP_RESPONSE,
        SMSG_PVP_MAP_MODE,
        SMSG_PVP_SET,
        0
    };
    handledMessages = _messages;
    beingHandler = this;
}

void BeingHandler::requestNameById(const int id) const
{
    MessageOut outMsg(CMSG_NAME_REQUEST);
    outMsg.writeInt32(id);  // readLong(2));
}

void BeingHandler::handleMessage(Net::MessageIn &msg)
{
    switch (msg.getId())
    {
        case SMSG_BEING_VISIBLE:  // changed
        case SMSG_BEING_MOVE:
            processBeingVisibleOrMove(msg, msg.getId() == SMSG_BEING_VISIBLE);
            break;

        case SMSG_BEING_MOVE2:
            processBeingMove2(msg);
            break;

        case SMSG_BEING_SPAWN:  // changed
            processBeingSpawn(msg);
            break;

        case SMSG_BEING_REMOVE:
            processBeingRemove(msg);
            break;

        case SMSG_BEING_RESURRECT:
            processBeingResurrect(msg);
            break;

        case SMSG_SKILL_DAMAGE:
            processSkillDamage(msg);
            break;

        case SMSG_BEING_ACTION:
            processBeingAction(msg);
            break;

        case SMSG_BEING_SELFEFFECT:
            processBeingSelfEffect(msg);
            break;

        case SMSG_BEING_EMOTION:
            processBeingEmotion(msg);
            break;

        case SMSG_BEING_CHANGE_LOOKS:
        case SMSG_BEING_CHANGE_LOOKS2:
            processBeingChangeLook(msg,
                msg.getId() == SMSG_BEING_CHANGE_LOOKS2);
            break;

        case SMSG_BEING_NAME_RESPONSE:
            processNameResponse(msg);
            break;

        case SMSG_BEING_NAME_RESPONSE2:
            processNameResponse2(msg);
            break;

        case SMSG_BEING_IP_RESPONSE:
            processIpResponse(msg);
            break;

        case SMSG_SOLVE_CHAR_NAME:
            break;

        case SMSG_PLAYER_GUILD_PARTY_INFO:
            processPlayerGuilPartyInfo(msg);
            break;

        case SMSG_BEING_CHANGE_DIRECTION:
            processBeingChangeDirection(msg);
            break;

        case SMSG_PLAYER_UPDATE_1:
        case SMSG_PLAYER_UPDATE_2:
        case SMSG_PLAYER_MOVE:
            int type;
            switch (msg.getId())
            {
                case SMSG_PLAYER_UPDATE_1:
                    type = 1;
                    break;
                case SMSG_PLAYER_UPDATE_2:
                    type = 2;
                    break;
                case SMSG_PLAYER_MOVE:
                    type = 3;
                    break;
                default:
                    return;
            }
            processPlayerMoveUpdate(msg, type);

            break;
        case SMSG_PLAYER_STOP:
            processPlayerStop(msg);
            break;

        case SMSG_PLAYER_MOVE_TO_ATTACK:
            processPlayerMoveToAttack(msg);
            break;

        case SMSG_PLAYER_STATUS_CHANGE:
            processPlaterStatusChange(msg);
            break;

        case SMSG_BEING_STATUS_CHANGE:
            processBeingStatusChange(msg);
            break;

        case SMSG_SKILL_CASTING:
            processSkilCasting(msg);
            break;

        case SMSG_SKILL_CAST_CANCEL:
            msg.readInt32();    // id
            break;

        case SMSG_SKILL_NO_DAMAGE:
            processSkillNoDamage(msg);
            break;

        case SMSG_PVP_MAP_MODE:
            processPvpMapMode(msg);
            break;

        case SMSG_PVP_SET:
            processPvpSet(msg);
            break;

        default:
            break;
    }
}

void BeingHandler::undress(Being *const being) const
{
    being->setSprite(SPRITE_BOTTOMCLOTHES, 0);
    being->setSprite(SPRITE_TOPCLOTHES, 0);
    being->setSprite(SPRITE_HAT, 0);
    being->setSprite(SPRITE_SHOE, 0);
    being->setSprite(SPRITE_GLOVES, 0);
//    being->setSprite(SPRITE_WEAPON, 0, "", true);
}

void BeingHandler::processBeingChangeLook(Net::MessageIn &msg,
                                          const bool look2) const
{
    if (!actorManager)
        return;

    /*
      * SMSG_BEING_CHANGE_LOOKS (0x00c3) and
      * SMSG_BEING_CHANGE_LOOKS2 (0x01d7) do basically the same
      * thing.  The difference is that ...LOOKS carries a single
      * 8 bit value, where ...LOOKS2 carries two 16 bit values.
      *
      * If type = 2, then the first 16 bit value is the weapon ID,
      * and the second 16 bit value is the shield ID.  If no
      * shield is equipped, or type is not 2, then the second
      * 16 bit value will be 0.
      */

    Being *const dstBeing = actorManager->findBeing(msg.readInt32());
    if (!dstBeing)
        return;

    const uint8_t type = msg.readUInt8();
    int id = 0;
    unsigned int id2 = 0U;
    const std::string color;

    if (!look2)
    {
        id = static_cast<int>(msg.readUInt8());
        id2 = 1U;    // default color
    }
    else
    {        // SMSG_BEING_CHANGE_LOOKS2
        id = msg.readInt16();
        if (type == 2)
            id2 = msg.readInt16();
        else
            id2 = 1;
//        color.clear();
    }

    if (dstBeing->getType() == ActorType::PLAYER)
        dstBeing->setOtherTime();

    if (!player_node)
        return;

    switch (type)
    {
        case 0:     // change race
            dstBeing->setSubtype(static_cast<uint16_t>(id), 0);
            break;
        case 1:     // eAthena LOOK_HAIR
            dstBeing->setSpriteID(SPRITE_HAIR, id *-1);
            break;
        case 2:     // Weapon ID in id, Shield ID in id2
            dstBeing->setSprite(SPRITE_WEAPON, id, "", 1, true);
            if (!mHideShield)
                dstBeing->setSprite(SPRITE_SHIELD, id2);
            player_node->imitateOutfit(dstBeing, SPRITE_SHIELD);
            break;
        case 3:     // Change lower headgear for eAthena, pants for us
            dstBeing->setSprite(SPRITE_BOTTOMCLOTHES, id, color,
                static_cast<unsigned char>(id2));
            player_node->imitateOutfit(dstBeing, SPRITE_BOTTOMCLOTHES);
            break;
        case 4:     // Change upper headgear for eAthena, hat for us
            dstBeing->setSprite(SPRITE_HAT, id, color,
                static_cast<unsigned char>(id2));
            player_node->imitateOutfit(dstBeing, SPRITE_HAT);
            break;
        case 5:     // Change middle headgear for eathena, armor for us
            dstBeing->setSprite(SPRITE_TOPCLOTHES, id, color,
                static_cast<unsigned char>(id2));
            player_node->imitateOutfit(dstBeing, SPRITE_TOPCLOTHES);
            break;
        case 6:     // eAthena LOOK_HAIR_COLOR
            dstBeing->setSpriteColor(SPRITE_HAIR, ItemDB::get(
                dstBeing->getSpriteID(SPRITE_HAIR)).getDyeColorsString(id));
            break;
        case 7:     // Clothes color
            // ignoring it
            break;
        case 8:     // eAthena LOOK_SHIELD
            if (!mHideShield)
            {
                dstBeing->setSprite(SPRITE_SHIELD, id, color,
                    static_cast<unsigned char>(id2));
            }
            player_node->imitateOutfit(dstBeing, SPRITE_SHIELD);
            break;
        case 9:     // eAthena LOOK_SHOES
            dstBeing->setSprite(SPRITE_SHOE, id, color,
                static_cast<unsigned char>(id2));
            player_node->imitateOutfit(dstBeing, SPRITE_SHOE);
            break;
        case 10:   // LOOK_GLOVES
            dstBeing->setSprite(SPRITE_GLOVES, id, color,
                static_cast<unsigned char>(id2));
            player_node->imitateOutfit(dstBeing, SPRITE_GLOVES);
            break;
        case 11:  // LOOK_CAPE
            dstBeing->setSprite(SPRITE_CAPE, id, color,
                static_cast<unsigned char>(id2));
            player_node->imitateOutfit(dstBeing, SPRITE_CAPE);
            break;
        case 12:
            dstBeing->setSprite(SPRITE_MISC1, id, color,
                static_cast<unsigned char>(id2));
            player_node->imitateOutfit(dstBeing, SPRITE_MISC1);
            break;
        case 13:
            dstBeing->setSprite(SPRITE_MISC2, id, color,
                static_cast<unsigned char>(id2));
            player_node->imitateOutfit(dstBeing, SPRITE_MISC2);
            break;
        case 14:
            dstBeing->setSprite(SPRITE_EVOL1, id, color,
                static_cast<unsigned char>(id2));
            player_node->imitateOutfit(dstBeing, SPRITE_EVOL1);
            break;
        case 15:
            dstBeing->setSprite(SPRITE_EVOL2, id, color,
                static_cast<unsigned char>(id2));
            player_node->imitateOutfit(dstBeing, SPRITE_EVOL2);
            break;
        default:
            logger->log("QQQ3 CHANGE_LOOKS: unsupported type: "
                    "%d, id: %d", type, id);
            logger->log("ID: " + toString(dstBeing->getId()));
            logger->log("name: " + toString(dstBeing->getName()));
            break;
    }
}

void BeingHandler::processNameResponse2(Net::MessageIn &msg)
{
    if (!actorManager || !player_node)
        return;

    const int len = msg.readInt16();
    const int beingId = msg.readInt32();
    const std::string str = msg.readString(len - 8);
    Being *const dstBeing = actorManager->findBeing(beingId);
    if (dstBeing)
    {
        if (beingId == player_node->getId())
        {
            player_node->pingResponse();
        }
        else
        {
            dstBeing->setName(str);
            dstBeing->updateGuild();
            dstBeing->addToCache();

            if (dstBeing->getType() == ActorType::PLAYER)
                dstBeing->updateColors();

            if (player_node)
            {
                const Party *const party = player_node->getParty();
                if (party && party->isMember(dstBeing->getId()))
                {
                    PartyMember *const member = party->getMember(
                        dstBeing->getId());

                    if (member)
                        member->setName(dstBeing->getName());
                }
                player_node->checkNewName(dstBeing);
            }
        }
    }
}

void BeingHandler::processPlayerMoveUpdate(Net::MessageIn &msg,
                                           const int msgType) const
{
    if (!actorManager || !player_node)
        return;

    // An update about a player, potentially including movement.
    const int id = msg.readInt32();
    const int16_t speed = msg.readInt16();
    const uint16_t stunMode = msg.readInt16();  // opt1
    uint32_t statusEffects = msg.readInt16();   // opt2
    statusEffects |= (static_cast<uint16_t>(msg.readInt16()))
        << 16U;  // status.options; Aethyra uses this as misc2
    const int16_t job = msg.readInt16();
    int disguiseId = 0;
    if (id < 110000000 && job >= 1000)
        disguiseId = job;

    Being *dstBeing = actorManager->findBeing(id);

    if (!dstBeing)
    {
        if (actorManager->isBlocked(id) == true)
            return;

        dstBeing = createBeing(id, job);

        if (!dstBeing)
            return;
    }
    else if (disguiseId)
    {
        actorManager->undelete(dstBeing);
        if (serverVersion < 1)
            requestNameById(id);
    }

    uint8_t dir = dstBeing->getDirectionDelayed();
    if (dir)
    {
        if (dir != dstBeing->getDirection())
            dstBeing->setDirection(dir);
    }

    if (Party *const party = player_node->getParty())
    {
        if (party->isMember(id))
            dstBeing->setParty(party);
    }

    dstBeing->setWalkSpeed(Vector(speed, speed, 0));
    dstBeing->setSubtype(job, 0);

    const int hairStyle = msg.readInt16();
    const uint16_t weapon = msg.readInt16();
    const uint16_t shield = msg.readInt16();
    const uint16_t headBottom = msg.readInt16();

    if (msgType == 3)
        msg.readInt32();  // server tick

    const uint16_t headTop = msg.readInt16();
    const uint16_t headMid = msg.readInt16();
    const int hairColor = msg.readInt16();

    msg.readUInt8();
    msg.readUInt8();
    msg.readUInt8();
    msg.readUInt8();  // unused

    const int guild = msg.readInt32();  // guild

    if (!guildManager || !GuildManager::getEnableGuildBot())
    {
        if (guild == 0)
            dstBeing->clearGuilds();
        else
            dstBeing->setGuild(Guild::getGuild(static_cast<int16_t>(guild)));
    }

    msg.readInt16();  // emblem
    msg.readInt16();  // manner
    dstBeing->setStatusEffectBlock(32, msg.readInt16());  // opt3
    msg.readUInt8();  // karma
    // reserving bit for future usage
    dstBeing->setGender(Being::intToGender(
        static_cast<uint8_t>(msg.readUInt8() & 3U)));

    if (!disguiseId)
    {
        // Set these after the gender, as the sprites may be gender-specific
        dstBeing->updateSprite(SPRITE_WEAPON, weapon, "", 1, true);
        if (!mHideShield)
            dstBeing->updateSprite(SPRITE_SHIELD, shield);
        dstBeing->updateSprite(SPRITE_BOTTOMCLOTHES, headBottom);
        dstBeing->updateSprite(SPRITE_TOPCLOTHES, headMid);
        dstBeing->updateSprite(SPRITE_HAT, headTop);
        dstBeing->updateSprite(SPRITE_HAIR, hairStyle * -1,
            ItemDB::get(-hairStyle).getDyeColorsString(hairColor));
    }
    player_node->imitateOutfit(dstBeing);

    if (msgType == 3)
    {
        uint16_t srcX, srcY, dstX, dstY;
        msg.readCoordinatePair(srcX, srcY, dstX, dstY);

        player_node->followMoveTo(dstBeing, srcX, srcY, dstX, dstY);

        dstBeing->setTileCoords(srcX, srcY);
        dstBeing->setDestination(dstX, dstY);

        // because server don't send direction in move packet,
        // we fixing it

        if (srcX != dstX || srcY != dstY)
        {
            const int d = dstBeing->calcDirection(dstX, dstY);

            if (d && dstBeing->getDirection() != d)
                dstBeing->setDirectionDelayed(static_cast<uint8_t>(d));
        }

        if (player_node->getCurrentAction() != BeingAction::STAND)
            player_node->imitateAction(dstBeing, BeingAction::STAND);
        if (player_node->getDirection() != dstBeing->getDirection())
        {
            player_node->imitateDirection(dstBeing,
                dstBeing->getDirection());
        }
    }
    else
    {
        uint16_t x, y;
        msg.readCoordinates(x, y, dir);
        dstBeing->setTileCoords(x, y);
        dstBeing->setDirection(dir);

        player_node->imitateDirection(dstBeing, dir);
    }

    const uint16_t gmstatus = msg.readInt16();

    if (gmstatus & 0x80)
        dstBeing->setGM(true);

    if (msgType == 1 || msgType == 2)
    {
        const uint8_t type = msg.readUInt8();
        switch (type)
        {
            case 0:
                dstBeing->setAction(BeingAction::STAND, 0);
                player_node->imitateAction(dstBeing, BeingAction::STAND);
                break;

            case 1:
                if (dstBeing->getCurrentAction() != BeingAction::DEAD)
                {
                    dstBeing->setAction(BeingAction::DEAD, 0);
                    dstBeing->recalcSpritesOrder();
                }
                break;

            case 2:
                dstBeing->setAction(BeingAction::SIT, 0);
                player_node->imitateAction(dstBeing, BeingAction::SIT);
                break;

            default:
                // need set stay state?
                logger->log("QQQ2 SMSG_PLAYER_UPDATE_1:"
                    + toString(id) + " " + toString(type));
                logger->log("dstBeing id:" + toString(dstBeing->getId()));
                logger->log("dstBeing name:" + dstBeing->getName());
                break;
        }
    }
    else if (msgType == 3)
    {
        msg.readUInt8();  // unknown
    }

    const int level = static_cast<int>(msg.readUInt8());  // Lv

    if (level)
        dstBeing->setLevel(level);

    msg.readUInt8();  // unknown

    if (dstBeing->getType() != ActorType::PLAYER
        || msgType != 3)
    {
        dstBeing->setActionTime(tick_time);
    }

    dstBeing->setStunMode(stunMode);
    dstBeing->setStatusEffectBlock(0, static_cast<uint16_t>(
        (statusEffects >> 16) & 0xffffU));
    dstBeing->setStatusEffectBlock(16, static_cast<uint16_t>(
        statusEffects & 0xffffU));

    if (msgType == 3 && dstBeing->getType() == ActorType::PLAYER)
        dstBeing->setMoveTime();
}

void BeingHandler::processBeingVisibleOrMove(Net::MessageIn &msg,
                                             const bool visible)
{
    if (!actorManager)
        return;

    if (visible)
        msg.readUInt8();  // padding?

    // Information about a being in range
    const int id = msg.readInt32();
    int spawnId;
    if (id == mSpawnId)
        spawnId = mSpawnId;
    else
        spawnId = 0;
    mSpawnId = 0;
    int16_t speed = msg.readInt16();
    const uint16_t stunMode = msg.readInt16();  // opt1
    uint32_t statusEffects = msg.readInt16();   // opt2
    statusEffects |= (static_cast<uint16_t>(
        msg.readInt16())) << 16U;  // option
    const int16_t job = msg.readInt16();  // class

    Being *dstBeing = actorManager->findBeing(id);

    if (dstBeing && dstBeing->getType() == ActorType::MONSTER
        && !dstBeing->isAlive())
    {
        actorManager->destroy(dstBeing);
        actorManager->erase(dstBeing);
        dstBeing = nullptr;
    }

    if (!dstBeing)
    {
        // Being with id >= 110000000 and job 0 are better
        // known as ghosts, so don't create those.
        if (job == 0 && id >= 110000000)
            return;

        if (actorManager->isBlocked(id) == true)
            return;

        dstBeing = createBeing(id, job);

        if (!dstBeing)
            return;

        if (job == 1022 && killStats)
            killStats->jackoAlive(dstBeing->getId());
    }
    else
    {
        // undeleting marked for deletion being
        if (dstBeing->getType() == ActorType::NPC)
            actorManager->undelete(dstBeing);
    }

    if (dstBeing->getType() == ActorType::PLAYER)
        dstBeing->setMoveTime();

    if (spawnId)
    {
        dstBeing->setAction(BeingAction::SPAWN, 0);
    }
    else if (visible)
    {
        dstBeing->clearPath();
        dstBeing->setActionTime(tick_time);
        dstBeing->setAction(BeingAction::STAND, 0);
    }

    // Prevent division by 0 when calculating frame
    if (speed == 0)
        speed = 150;

    dstBeing->setWalkSpeed(Vector(speed, speed, 0));
    dstBeing->setSubtype(job, 0);
    if (dstBeing->getType() == ActorType::MONSTER && player_node)
        player_node->checkNewName(dstBeing);

    const int hairStyle = msg.readInt16();
    const uint16_t weapon = msg.readInt16();
    const uint16_t headBottom = msg.readInt16();

    const uint16_t shield = msg.readInt16();
    const uint16_t headTop = msg.readInt16();
    const uint16_t headMid = msg.readInt16();
    const int hairColor = msg.readInt16();
    const uint16_t shoes = msg.readInt16();  // clothes color

    uint16_t gloves;
    if (dstBeing->getType() == ActorType::MONSTER)
    {
        msg.readInt32();
        msg.readInt32();
        gloves = 0;
    }
    else
    {
        gloves = msg.readInt16();  // head dir - "abused" as gloves
        msg.readInt32();           // guild
        msg.readInt16();           // guild emblem
    }

    msg.readInt16();  // manner
    dstBeing->setStatusEffectBlock(32, msg.readInt16());  // opt3
    msg.readUInt8();   // karma
    uint8_t gender = msg.readUInt8();

    if (dstBeing->getType() == ActorType::PLAYER)
    {
        gender &= 3;
        dstBeing->setGender(Being::intToGender(gender));
        // Set these after the gender, as the sprites may be gender-specific
        setSprite(dstBeing, SPRITE_HAIR, hairStyle * -1,
            ItemDB::get(-hairStyle).getDyeColorsString(hairColor));
        setSprite(dstBeing, SPRITE_BOTTOMCLOTHES, headBottom);
        setSprite(dstBeing, SPRITE_TOPCLOTHES, headMid);
        setSprite(dstBeing, SPRITE_HAT, headTop);
        setSprite(dstBeing, SPRITE_SHOE, shoes);
        setSprite(dstBeing, SPRITE_GLOVES, gloves);
        setSprite(dstBeing, SPRITE_WEAPON, weapon, "", 1, true);
        if (!mHideShield)
            setSprite(dstBeing, SPRITE_SHIELD, shield);
    }
    else if (dstBeing->getType() == ActorType::NPC)
    {
        switch (gender)
        {
            case 2:
                dstBeing->setGender(Gender::FEMALE);
                break;
            case 3:
                dstBeing->setGender(Gender::MALE);
                break;
            case 4:
                dstBeing->setGender(Gender::OTHER);
                break;
            default:
                dstBeing->setGender(Gender::UNSPECIFIED);
                break;
        }
    }

    if (!visible)
    {
        uint16_t srcX, srcY, dstX, dstY;
        msg.readCoordinatePair(srcX, srcY, dstX, dstY);
        dstBeing->setAction(BeingAction::STAND, 0);
        dstBeing->setTileCoords(srcX, srcY);
        dstBeing->setDestination(dstX, dstY);
    }
    else
    {
        uint8_t dir;
        uint16_t x, y;
        msg.readCoordinates(x, y, dir);
        dstBeing->setTileCoords(x, y);

        if (job == 45 && socialWindow && outfitWindow)
        {
            const int num = socialWindow->getPortalIndex(x, y);
            if (num >= 0)
            {
                dstBeing->setName(keyboard.getKeyShortString(
                    outfitWindow->keyName(num)));
            }
            else
            {
                dstBeing->setName("");
            }
        }

        dstBeing->setDirection(dir);
    }

    msg.readUInt8();  // unknown
    msg.readUInt8();  // state / sit
    msg.readInt16();  // level

    dstBeing->setStunMode(stunMode);
    dstBeing->setStatusEffectBlock(0, static_cast<uint16_t>(
        (statusEffects >> 16) & 0xffffU));
    dstBeing->setStatusEffectBlock(16, static_cast<uint16_t>(
        statusEffects & 0xffffU));
}

}  // namespace EAthena
