/*
 *  The ManaPlus Client
 *  Copyright (C) 2008-2009  The Mana World Development Team
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

#include "gui/widgets/tabs/tradetab.h"

#include "chatlogger.h"

#include "gui/chatconsts.h"

#include "utils/gettext.h"

#include "debug.h"

TradeTab *tradeChatTab = nullptr;

TradeTab::TradeTab(const Widget2 *const widget) :
    // TRANSLATORS: trade chat tab name
    ChatTab(widget, _("Trade"), TRADE_CHANNEL)
{
}

TradeTab::~TradeTab()
{
}

void TradeTab::handleInput(const std::string &msg)
{
    std::string str("\302\202" + msg);
    ChatTab::handleInput(str);
}

void TradeTab::saveToLogFile(const std::string &msg) const
{
    if (chatLogger)
        chatLogger->log(std::string("#Trade"), std::string(msg));
}
