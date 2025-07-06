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
    HOARDER_ACTION_STORAGE_1         = 0,
    HOARDER_ACTION_STORAGE_2         = 1,
    HOARDER_ACTION_STORAGE_3         = 2,
    HOARDER_ACTION_BACKPACK_ITEMS    = 3,
    HOARDER_ACTION_SEARCH_ITEMS      = 4,
    HOARDER_ACTION_HELP              = 5,
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

        sCollector->ClearStorageSelection(player->GetGUID());
        sCollector->GetCollectedItems()[player->GetGUID()][STORAGE_SEARCH].clear();

        uint8 menuIndex = 0;

        player->PlayerTalkClass->GetGossipMenu().AddMenuItem(menuIndex, GOSSIP_ICON_CHAT, "Tell me, how does this work?", 0, HOARDER_ACTION_HELP, "", 0);
        player->PlayerTalkClass->GetGossipMenu().AddMenuItem(++menuIndex, GOSSIP_ICON_VENDOR, "Store items from my main backpack.", 0, HOARDER_ACTION_BACKPACK_ITEMS, "", 0);
        player->PlayerTalkClass->GetGossipMenu().AddMenuItem(++menuIndex, GOSSIP_ICON_INTERACT_1, "Search for items.", GOSSIP_SENDER_MAIN, HOARDER_ACTION_SEARCH_ITEMS, "", 0, true);

        for (uint8 index = 0; index < MAX_HOARDER_STORAGES; ++index)
        {
            if (player->GetPlayerSetting(ModName, index).IsEnabled() || player->IsGameMaster())
                player->PlayerTalkClass->GetGossipMenu().AddMenuItem(++menuIndex, GOSSIP_ICON_MONEY_BAG, "View items stored in storage " + std::to_string(index + 1) + ".", 0, HOARDER_ACTION_STORAGE_1 + index, "", 0);
        }

        player->PlayerTalkClass->SendGossipMenu(70000, creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
    {
        player->PlayerTalkClass->ClearMenus();

        uint32 menuItem = 0;

        switch (action)
        {
            case HOARDER_ACTION_STORAGE_1:
            case HOARDER_ACTION_STORAGE_2:
            case HOARDER_ACTION_STORAGE_3:
                if (sCollector->GetStorageSelection(player->GetGUID()) != HOARDER_ACTION_BACKPACK_ITEMS)
                {
                    // Player is not selecting a storage to store an item, display vendor interface.
                    sCollector->SetStorageSelection(player->GetGUID(), action);
                    ShowItemsInFakeVendor(player, creature, action);
                }
                else
                {
                    // Player is selecting a storage to store items.
                    // Update the current storage selected and display items.
                    sCollector->SetStorageSelection(player->GetGUID(), action);

                    for (uint8 i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; ++i)
                    {
                        if (Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                            if (ItemTemplate const* itemTemplate = item->GetTemplate())
                                if (sCollector->IsItemValid(item))
                                    player->PlayerTalkClass->GetGossipMenu().AddMenuItem(menuItem++, 0, GetItemIcon(item->GetEntry(), 30, 30, -18, 0) + itemTemplate->Name1, 0, item->GetEntry(), "", 0);
                    }

                    if (!menuItem)
                        player->PlayerTalkClass->SendGossipMenu(70002, creature->GetGUID());
                    else
                        player->PlayerTalkClass->SendGossipMenu(70000, creature->GetGUID());
                }
                return true;
            case HOARDER_ACTION_BACKPACK_ITEMS:
            {
                uint8 menuIndex = 0;
                for (uint8 index = 0; index < MAX_HOARDER_STORAGES; ++index)
                {
                    if (player->GetPlayerSetting(ModName, index).IsEnabled() || player->IsGameMaster())
                        player->PlayerTalkClass->GetGossipMenu().AddMenuItem(++menuIndex, GOSSIP_ICON_MONEY_BAG, "Store item in storage " + std::to_string(index + 1) + ".", 0, HOARDER_ACTION_STORAGE_1 + index, "", 0);
                }

                sCollector->SetStorageSelection(player->GetGUID(), HOARDER_ACTION_BACKPACK_ITEMS);
                break;
            }
            case HOARDER_ACTION_HELP:
                player->PlayerTalkClass->SendGossipMenu(70004, creature->GetGUID());
                return true;
            default:
                // In this case, the action is an item entry
                // Process the action of storing an item
                for (uint8 i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; ++i)
                {
                    if (Item* item = player->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                        if (item->GetEntry() == action)
                        {
                            if (!sCollector->IsItemValid(item))
                            {
                                player->SendSystemMessage("This item cannot be stored with the collector.");
                                CloseGossipMenuFor(player);
                                return true;
                            }

                            CharacterDatabase.Execute("INSERT INTO mod_collector_items (PlayerGUID, ItemEntry) VALUES ({}, {})", player->GetGUID().GetCounter(), item->GetEntry());
                            sCollector->AddItemToCollection(player->GetGUID(), sCollector->GetStorageSelection(player->GetGUID()), item->GetEntry());
                            player->DestroyItem(INVENTORY_SLOT_BAG_0, i, true);
                            CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();
                            player->SaveInventoryAndGoldToDB(trans);
                            CharacterDatabase.CommitTransaction(trans);
                            CloseGossipMenuFor(player);
                            return true;
                        }
                }
                break;
        }

        player->PlayerTalkClass->SendGossipMenu(70000, creature->GetGUID());
        return true;
    }

    bool OnGossipSelectCode(Player* player, Creature* creature, uint32 sender, uint32 action, const char* code) override
    {
        player->PlayerTalkClass->ClearMenus();

        if (sender != GOSSIP_SENDER_MAIN)
            return true;

        std::string searchInput(code);
        std::string normalizedInput = searchInput;

        bool capitalizeNext = true;
        for (char& c : normalizedInput)
        {
            if (std::isspace(static_cast<unsigned char>(c)))
                capitalizeNext = true;
            else if (capitalizeNext)
            {
                c = std::toupper(static_cast<unsigned char>(c));
                capitalizeNext = false;
            }
        }

        std::vector<uint32> allItems;

        if (auto it = sCollector->GetCollectedItems().find(player->GetGUID()); it != sCollector->GetCollectedItems().end())
            for (const auto& [_, items] : it->second)
                for (uint32 itemId : items)
                    allItems.emplace_back(itemId);

        uint32 matchedCount = 0;
        std::vector<uint32> foundItems;

        for (const auto& item : allItems)
        {
            if (matchedCount >= MAX_VENDOR_ITEMS)
                break;

            if (ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(item))
            {
                std::string name = itemTemplate->Name1;
                if (name.find(searchInput) != std::string::npos || name.find(normalizedInput) != std::string::npos)
                {
                    sCollector->AddItemToCollection(player->GetGUID(), STORAGE_SEARCH, item);
                    ++matchedCount;
                }
            }
        }

        if (!matchedCount)
            player->PlayerTalkClass->SendGossipMenu(70003, creature->GetGUID());
        else
        {
            sCollector->SetStorageSelection(player->GetGUID(), STORAGE_SEARCH);
            npc_hoard_the_collector::ShowItemsInFakeVendor(player, creature, STORAGE_SEARCH);
        }

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

    static void ShowItemsInFakeVendor(Player* player, Creature* creature, uint8 storage)
    {
        std::vector<uint32> itemList = sCollector->GetCollectedItems()[player->GetGUID()][storage];

        uint32 itemCount = itemList.size();

        if (!itemCount)
        {
            player->PlayerTalkClass->SendGossipMenu(70001, creature->GetGUID());
            return;
        }

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

class mod_hoard_collector_playerscript : public PlayerScript
{
public:
    mod_hoard_collector_playerscript() : PlayerScript("mod_hoard_collector_playerscipt", {
        PLAYERHOOK_ON_LOGOUT,
        PLAYERHOOK_ON_BEFORE_BUY_ITEM_FROM_VENDOR
        }) {
    }

    void OnPlayerLogout(Player* player) override
    {
        sCollector->ClearCollection(player->GetGUID());
        sCollector->ClearStorageSelection(player->GetGUID());
    }

    void OnPlayerBeforeBuyItemFromVendor(Player* player, ObjectGuid vendorguid, uint32 /*vendorslot*/, uint32& itemEntry, uint8 /*count*/, uint8 /*bag*/, uint8 /*slot*/) override
    {
        Creature* vendor = player->GetMap()->GetCreature(vendorguid);
        if (!vendor)
            return;

        if (sCollector->GetHoarderNpcId() != vendor->GetEntry())
            return;

        uint8 storageId = sCollector->GetStorageSelection(player->GetGUID());

        if (storageId == STORAGE_SEARCH && sCollector->HasItemInAnyCollection(player->GetGUID(), itemEntry))
        {
            player->AddItem(itemEntry, 1); // Add the item to the player's inventory

            for (uint8 storage = 0; storage < MAX_HOARDER_STORAGES; ++storage)
                sCollector->RemoveItemFromCollection(player->GetGUID(), storage, itemEntry); // Remove the item from the collector's collection

            sCollector->RemoveItemFromCollection(player->GetGUID(), storageId, itemEntry);
            CharacterDatabase.Execute("DELETE FROM mod_collector_items WHERE PlayerGuid = {} AND ItemEntry = {}", player->GetGUID().GetCounter(), itemEntry);

            CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();
            player->SaveInventoryAndGoldToDB(trans);
            CharacterDatabase.CommitTransaction(trans);
        } else if (sCollector->HasItemInCollection(player->GetGUID(), storageId, itemEntry))
        {
            player->AddItem(itemEntry, 1); // Add the item to the player's inventory
            sCollector->RemoveItemFromCollection(player->GetGUID(), storageId, itemEntry); // Remove the item from the collector's collection
            CharacterDatabase.Execute("DELETE FROM mod_collector_items WHERE PlayerGuid = {} AND ItemEntry = {}", player->GetGUID().GetCounter(), itemEntry);

            CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();
            player->SaveInventoryAndGoldToDB(trans);
            CharacterDatabase.CommitTransaction(trans);
        }

        npc_hoard_the_collector::ShowItemsInFakeVendor(player, vendor, storageId); //Refresh menu
        itemEntry = 0; //Prevents the handler from proceeding to core vendor handling
    }
};

class mod_hoard_collector_worldscript : public WorldScript
{
public:
    mod_hoard_collector_worldscript() : WorldScript("mod_hoard_collector_worldscript", {
        WORLDHOOK_ON_AFTER_CONFIG_LOAD,
        }) {
    }

    void OnAfterConfigLoad(bool reload) override
    {
        sCollector->SetEnabled(sConfigMgr->GetOption<bool>("ModHoardCollector.Enable", false));
        sCollector->SetHoarderNpcId(sConfigMgr->GetOption<uint32>("ModHoardCollector.NpcID", 70000));
    }
};

void AddModHoardCollectorScripts()
{
    new npc_hoard_the_collector();
    new mod_hoard_collector_playerscript();
    new mod_hoard_collector_worldscript();
};
