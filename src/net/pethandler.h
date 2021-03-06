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

#ifndef NET_PETHANDLER_H
#define NET_PETHANDLER_H

#include "being/being.h"

namespace Net
{

class PetHandler notfinal
{
    public:
        virtual ~PetHandler()
        { }

        virtual void move(const Being *const being,
                          const int petId,
                          const int x1, const int y1,
                          const int x2, const int y2) const = 0;

        virtual void spawn(const Being *const being,
                           const int petId,
                           const int x, const int y) const = 0;

        virtual void emote(const uint8_t emoteId, const int petId) = 0;
};

}  // namespace Net

#endif  // NET_PETHANDLER_H
