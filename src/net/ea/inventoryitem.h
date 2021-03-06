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

#ifndef NET_EA_INVENTORYITEM_H
#define NET_EA_INVENTORYITEM_H

#if defined(__GXX_EXPERIMENTAL_CXX0X__)
#include <cstdint>
#else
#include <stdint.h>
#endif

#include "localconsts.h"

namespace Ea
{

/**
 * Used to cache storage data until we get size data for it.
 */
class InventoryItem final
{
    public:
        int slot;
        int id;
        int quantity;
        uint8_t refine;
        unsigned char color;
        bool equip;

        InventoryItem(const int slot0, const int id0, const int quantity0,
                      const uint8_t refine0, const unsigned char color0,
                      const bool equip0) :
            slot(slot0),
            id(id0),
            quantity(quantity0),
            refine(refine0),
            color(color0),
            equip(equip0)
        {
        }
};

}  // namespace Ea

#endif  // NET_EA_INVENTORYITEM_H
