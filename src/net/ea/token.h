/*
 *  The ManaPlus Client
 *  Copyright (C) 2009  The Mana World Development Team
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

#include "being/being.h"

#ifndef NET_EA_TOKEN_H
#define NET_EA_TOKEN_H

struct Token
{
    int account_ID;
    int session_ID1;
    int session_ID2;
    Gender::Type sex;

    void clear()
    {
        account_ID = 0;
        session_ID1 = 0;
        session_ID2 = 0;
        sex = Gender::UNSPECIFIED;
    }
};

#endif  // NET_EA_TOKEN_H
