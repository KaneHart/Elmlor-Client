/*
 *  The ManaPlus Client
 *  Copyright (C) 2013-2014  The ManaPlus Developers
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

#include "net/tmwa/pethandler.h"

#include "gui/chatconsts.h"

#include "net/net.h"

#include "net/chathandler.h"

#include "debug.h"

extern Net::PetHandler *petHandler;

namespace TmwAthena
{

PetHandler::PetHandler() :
    MessageHandler(),
    mRandCounter(1000)
{
    static const uint16_t _messages[] =
    {
        0
    };
    handledMessages = _messages;
    petHandler = this;
}

void PetHandler::handleMessage(Net::MessageIn &msg A_UNUSED)
{
    BLOCK_START("PetHandler::handleMessage")
    BLOCK_END("PetHandler::handleMessage")
}

void PetHandler::move(const Being *const being A_UNUSED,
                      const int petId A_UNUSED,
                      const int x1 A_UNUSED, const int y1 A_UNUSED,
                      const int x2 A_UNUSED, const int y2 A_UNUSED) const
{
}

void PetHandler::spawn(const Being *const being A_UNUSED,
                       const int petId A_UNUSED,
                       const int x A_UNUSED, const int y A_UNUSED) const
{
}

void PetHandler::emote(const uint8_t emoteId, const int petId A_UNUSED)
{
    mRandCounter ++;
    if (mRandCounter > 10000)
        mRandCounter = 1000;

    Net::getChatHandler()->talk(strprintf("\302\202\302e%dz%d",
        static_cast<int>(emoteId), mRandCounter), GENERAL_CHANNEL);
}

}  // namespace TmwAthena
