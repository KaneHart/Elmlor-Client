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

#ifndef DIRS_H
#define DIRS_H

#include "localconsts.h"

class Dirs final
{
    public:
        static void initUsersDir();

        static void updateDataPath();

        static void extractDataDir();

        static void mountDataDir();

        static void initRootDir();

        static void initHomeDir();

        static void initLocalDataDir();

        static void initTempDir();

        static void initConfigDir();

        static void initUpdatesDir();

        static void initScreenshotDir();
};

#endif  // DIRS_H