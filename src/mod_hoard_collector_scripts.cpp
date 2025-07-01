/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 */

#include "ScriptMgr.h"
#include "Player.h"
#include "HoardCollector.h"
#include "Config.h"
#include "Chat.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"

enum HoarderActions
{
    HOARDER_ACTION_BACKPACK_ITEMS    = 1,
    HOARDER_ACTION_VIEW_STORED_ITEMS = 2
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
        if (sCollector->IsCollectionEmpty(player->GetGUID()))
            sCollector->LoadCollectionItems(player->GetGUID().GetCounter());

        player->PlayerTalkClass->GetGossipMenu().AddMenuItem(0, 0, "Store items from my main backpack.", 0, HOARDER_ACTION_BACKPACK_ITEMS, "", 0);
        player->PlayerTalkClass->GetGossipMenu().AddMenuItem(1, 0, "View stored items.", 0, HOARDER_ACTION_VIEW_STORED_ITEMS, "", 0);
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
        else if (action == HOARDER_ACTION_VIEW_STORED_ITEMS)
        {
            ShowItemsInFakeVendor(player, creature);
            return true;
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
                        sCollector->AddItemToCollection(player->GetGUID(), item->GetEntry());
                        CloseGossipMenuFor(player);
                        return true;
                    }
            }
        }

        player->PlayerTalkClass->SendGossipMenu(1, creature->GetGUID());
        return true;
    }

    // @brief Returns the icon for an item with specified entry, width, height, x and y offsets.
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

    static void EncodeItemToPacket(WorldPacket& data, ItemTemplate const* proto, uint8& slot, uint32 price)
    {
        data << uint32(slot + 1);
        data << uint32(proto->ItemId);
        data << uint32(proto->DisplayInfoID);
        data << int32(1); //Infinite Stock
        data << uint32(price);
        data << uint32(proto->MaxDurability);
        data << uint32(1);  //Buy Count of 1
        data << uint32(0);
        slot++;
    }

    static void ShowItemsInFakeVendor(Player* player, Creature* creature)
    {
        std::vector<uint32> itemList = sCollector->GetCollectedItems()[player->GetGUID()];

        uint32 itemCount = itemList.size();

        WorldPacket data(SMSG_LIST_INVENTORY, 8 + 1 + itemCount * 8 * 4);
        data << uint64(creature->GetGUID().GetRawValue());

        uint8 count = 0;
        size_t count_pos = data.wpos();
        data << uint8(count);

        for (uint32 i = 0; i < itemCount && count < MAX_VENDOR_ITEMS; ++i)
        {
            if (ItemTemplate const* itemtemplate = sObjectMgr->GetItemTemplate(itemList[i]))
                EncodeItemToPacket(data, itemtemplate, count, 0); // Assuming no price for simplicity
        }

        data.put(count_pos, count);
        player->GetSession()->SendPacket(&data);
    }
};

void AddModHoardCollectorScripts()
{
    new npc_hoard_the_collector();
};
