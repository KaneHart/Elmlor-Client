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

#include "resources/db/monsterdb.h"

#include "logger.h"

#include "resources/beingcommon.h"
#include "resources/beinginfo.h"
#include "resources/spritereference.h"

#include "resources/map/blockmask.h"

#include "utils/dtor.h"
#include "utils/gettext.h"

#include "configuration.h"

#include "debug.h"

static const unsigned int OLD_TMWATHENA_OFFSET = 1002;

namespace
{
    BeingInfos mMonsterInfos;
    bool mLoaded = false;
}

void MonsterDB::load()
{
    if (mLoaded)
        unload();

    logger->log1("Initializing monster database...");
    loadXmlFile(paths.getStringValue("monstersFile"));
    loadXmlFile(paths.getStringValue("monstersPatchFile"));
    loadXmlDir("monstersPatchDir", loadXmlFile);

    mLoaded = true;
}

void MonsterDB::loadXmlFile(const std::string &fileName)
{
    XML::Document doc(fileName);
    const XmlNodePtr rootNode = doc.rootNode();

    if (!rootNode || !xmlNameEqual(rootNode, "monsters"))
    {
        logger->log("Monster Database: Error while loading %s!",
            paths.getStringValue("monstersFile").c_str());
        mLoaded = true;
        return;
    }

    const int offset = XML::getProperty(rootNode,
        "offset", OLD_TMWATHENA_OFFSET);

    // iterate <monster>s
    for_each_xml_child_node(monsterNode, rootNode)
    {
        if (xmlNameEqual(monsterNode, "include"))
        {
            const std::string name = XML::getProperty(monsterNode, "name", "");
            if (!name.empty())
                loadXmlFile(name);
            continue;
        }
        if (!xmlNameEqual(monsterNode, "monster"))
            continue;

        const int id = XML::getProperty(monsterNode, "id", 0);
        BeingInfo *currentInfo = nullptr;
        if (mMonsterInfos.find(id + offset) != mMonsterInfos.end())
        {
            logger->log("MonsterDB: Redefinition of monster ID %d", id);
            currentInfo = mMonsterInfos[id + offset];
        }
        if (!currentInfo)
            currentInfo = new BeingInfo;

        currentInfo->setWalkMask(BlockMask::WALL
            | BlockMask::CHARACTER | BlockMask::MONSTER);
        currentInfo->setBlockType(BlockType::MONSTER);

        currentInfo->setName(XML::langProperty(
            // TRANSLATORS: unknown info name
            monsterNode, "name", _("unnamed")));

        BeingCommon::readBasicAttributes(currentInfo, monsterNode, "attack");

        currentInfo->setMaxHP(XML::getProperty(monsterNode, "maxHP", 0));

        currentInfo->setDeadSortOffsetY(XML::getProperty(
            monsterNode, "deadSortOffsetY", 31));

        currentInfo->setColorsList(XML::getProperty(monsterNode,
            "colors", ""));

        unsigned char block = 0;
        std::string walkStr = XML::getProperty(
            monsterNode, "walkType", "walk");
        if (walkStr == "walk")
            block = BlockMask::WATER | BlockMask::AIR;
        else if (walkStr == "fly")
            block = 0;
        else if (walkStr == "swim")
            block = BlockMask::GROUND | BlockMask::AIR;
        else if (walkStr == "walkswim" || walkStr == "swimwalk")
            block = BlockMask::AIR;

        currentInfo->setWalkMask(static_cast<unsigned char>(
            BlockMask::WALL | block));

        if (currentInfo->getMaxHP())
            currentInfo->setStaticMaxHP(true);

        SpriteDisplay display;

        // iterate <sprite>s and <sound>s
        for_each_xml_child_node(spriteNode, monsterNode)
        {
            if (xmlNameEqual(spriteNode, "sprite"))
            {
                if (!spriteNode->xmlChildrenNode)
                    continue;

                SpriteReference *const currentSprite = new SpriteReference;
                currentSprite->sprite = reinterpret_cast<const char*>(
                    spriteNode->xmlChildrenNode->content);

                currentSprite->variant = XML::getProperty(
                    spriteNode, "variant", 0);
                display.sprites.push_back(currentSprite);
            }
            else if (xmlNameEqual(spriteNode, "sound"))
            {
                if (!spriteNode->xmlChildrenNode)
                    continue;

                const std::string event = XML::getProperty(
                    spriteNode, "event", "");
                const int delay = XML::getProperty(
                    spriteNode, "delay", 0);
                const char *const filename = reinterpret_cast<const char*>(
                    spriteNode->xmlChildrenNode->content);

                if (event == "hit")
                {
                    currentInfo->addSound(ItemSoundEvent::HIT,
                        filename, delay);
                }
                else if (event == "miss")
                {
                    currentInfo->addSound(ItemSoundEvent::MISS,
                        filename, delay);
                }
                else if (event == "hurt")
                {
                    currentInfo->addSound(ItemSoundEvent::HURT,
                        filename, delay);
                }
                else if (event == "die")
                {
                    currentInfo->addSound(ItemSoundEvent::DIE,
                        filename, delay);
                }
                else if (event == "move")
                {
                    currentInfo->addSound(ItemSoundEvent::MOVE,
                        filename, delay);
                }
                else if (event == "sit")
                {
                    currentInfo->addSound(ItemSoundEvent::SIT,
                        filename, delay);
                }
                else if (event == "sittop")
                {
                    currentInfo->addSound(ItemSoundEvent::SITTOP,
                        filename, delay);
                }
                else if (event == "spawn")
                {
                    currentInfo->addSound(ItemSoundEvent::SPAWN,
                        filename, delay);
                }
                else
                {
                    logger->log("MonsterDB: Warning, sound effect %s for "
                                "unknown event %s of monster %s",
                                filename, event.c_str(),
                                currentInfo->getName().c_str());
                }
            }
            else if (xmlNameEqual(spriteNode, "attack"))
            {
                const int attackId = XML::getProperty(spriteNode, "id", 0);
                const int effectId = XML::getProperty(
                    spriteNode, "effect-id", paths.getIntValue("effectId"));
                const int hitEffectId = XML::getProperty(spriteNode,
                    "hit-effect-id", paths.getIntValue("hitEffectId"));
                const int criticalHitEffectId = XML::getProperty(spriteNode,
                    "critical-hit-effect-id",
                    paths.getIntValue("criticalHitEffectId"));
                const int missEffectId = XML::getProperty(spriteNode,
                    "miss-effect-id", paths.getIntValue("missEffectId"));

                const std::string spriteAction = XML::getProperty(
                    spriteNode, "action", "attack");
                const std::string skySpriteAction = XML::getProperty(
                    spriteNode, "skyaction", "skyattack");
                const std::string waterSpriteAction = XML::getProperty(
                    spriteNode, "wateraction", "waterattack");

                const std::string missileParticle = XML::getProperty(
                        spriteNode, "missile-particle", "");

                currentInfo->addAttack(attackId, spriteAction, skySpriteAction,
                    waterSpriteAction, effectId, hitEffectId,
                    criticalHitEffectId, missEffectId, missileParticle);
            }
            else if (xmlNameEqual(spriteNode, "particlefx"))
            {
                if (!spriteNode->xmlChildrenNode)
                    continue;

                display.particles.push_back(reinterpret_cast<const char*>(
                    spriteNode->xmlChildrenNode->content));
            }
        }
        currentInfo->setDisplay(display);

        mMonsterInfos[id + offset] = currentInfo;
    }
}

void MonsterDB::unload()
{
    delete_all(mMonsterInfos);
    mMonsterInfos.clear();

    mLoaded = false;
}


BeingInfo *MonsterDB::get(const int id)
{
    BeingInfoIterator i = mMonsterInfos.find(id);

    if (i == mMonsterInfos.end())
    {
        i = mMonsterInfos.find(id + OLD_TMWATHENA_OFFSET);
        if (i == mMonsterInfos.end())
        {
            logger->log("MonsterDB: Warning, unknown monster ID %d requested",
                        id);
            return BeingInfo::unknown;
        }
        else
        {
            return i->second;
        }
    }
    else
    {
        return i->second;
    }
}
