/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU AGPL v3 license: https://github.com/azerothcore/azerothcore-wotlk/blob/master/LICENSE-AGPL3
 */

#include "HoardCollector.h"

HoardCollector* HoardCollector::instance()
{
    static HoardCollector instance;
    return &instance;
}

void HoardCollector::LoadCollectionItems(uint32 guid)
{
    // Load collected items from the database
    QueryResult result = CharacterDatabase.Query("SELECT PlayerGuid, StorageID, ItemEntry FROM mod_collector_items WHERE PlayerGuid = {}", guid);
    if (!result)
        return;
    do
    {
        Field* fields = result->Fetch();
        ObjectGuid playerGuid = ObjectGuid::Create<HighGuid::Player>(fields[0].Get<uint32>());
        uint8 storageId = fields[1].Get<uint8>();
        uint32 itemId = fields[2].Get<uint32>();
        AddItemToCollection(playerGuid, storageId, itemId);
    } while (result->NextRow());
}

bool HoardCollector::IsItemValid(Item* item) const
{
    // Check if the item is valid for collection
    if (!item || !item->GetTemplate())
        return false;

    if (item->GetUInt32Value(ITEM_FIELD_DURATION))
        return false;

    if (item->IsNotEmptyBag())
        return false;

    if (item->GetCount() > 1)
        return false;

    // Check if the item is already in the collection
    if (HasItemInAnyCollection(item->GetOwnerGUID(), item->GetEntry()))
        return false;

    return true;
}
