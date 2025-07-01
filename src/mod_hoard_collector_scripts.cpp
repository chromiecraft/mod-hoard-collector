/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 */

#include "ScriptMgr.h"
#include "Player.h"
#include "Config.h"
#include "Chat.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"

enum HoarderActions
{
    HOARDER_ACTION_BACKPACK_ITEMS = 1,
};

class npc_hoard_the_collector : public CreatureScript
{
public:
    npc_hoard_the_collector() : CreatureScript("npc_hoard_the_collector") {}

    struct npc_hoard_the_collectorAI : ScriptedAI
    {
        npc_hoard_the_collectorAI(Creature* creature) : ScriptedAI(creature) {};
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_hoard_the_collectorAI(creature);
    }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        player->PlayerTalkClass->GetGossipMenu().AddMenuItem(0, 0, "Store items from my main backpack.", 0, HOARDER_ACTION_BACKPACK_ITEMS, "", 0);
        player->PlayerTalkClass->SendGossipMenu(1, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
    {
        player->PlayerTalkClass->ClearMenus();

        uint32 menuItem = 0;

        if (action == HOARDER_ACTION_BACKPACK_ITEMS)
        {
            for (uint8 i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; ++i)
            {
                if (Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                    if (ItemTemplate const* itemTemplate = item->GetTemplate())
                        player->PlayerTalkClass->GetGossipMenu().AddMenuItem(menuItem++, 0, GetItemIcon(item->GetEntry(), 30, 30, -18, 0) + itemTemplate->Name1, 0, item->GetEntry(), "", 0);
            }
        }
        else
        {
            for (uint8 i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; ++i)
            {
                if (Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                    if (item->GetEntry() == action)
                    {
                        player->DestroyItem(INVENTORY_SLOT_BAG_0, i, true);
                        CharacterDatabase.Execute("INSERT INTO mod_collector_items (PlayerGUID, ItemEntry) VALUES ({}, {})", player->GetGUID().GetCounter(), item->GetEntry());
                        CloseGossipMenuFor(player);
                        return true;
                    }
            }
        }

        player->PlayerTalkClass->SendGossipMenu(1, creature->GetGUID());
        return true;
    }

    std::string GetItemIcon(uint32 entry, uint32 width, uint32 height, int x, int y) const
    {
        std::ostringstream ss;
        ss << "|TInterface";
        const ItemTemplate* temp = sObjectMgr->GetItemTemplate(entry);
        const ItemDisplayInfoEntry* dispInfo = NULL;
        if (temp)
        {
            dispInfo = sItemDisplayInfoStore.LookupEntry(temp->DisplayInfoID);
            if (dispInfo)
                ss << "/ICONS/" << dispInfo->inventoryIcon;
        }
        if (!dispInfo)
            ss << "/InventoryItems/WoWUnknownItem01";
        ss << ":" << width << ":" << height << ":" << x << ":" << y << "|t";
        return ss.str();
    }
};

void AddModHoardCollectorScripts()
{
    new npc_hoard_the_collector();
};
