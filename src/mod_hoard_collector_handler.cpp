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
    QueryResult result = CharacterDatabase.Query("SELECT PlayerGuid, ItemEntry FROM mod_collector_items WHERE PlayerGuid = {}", guid);
    if (!result)
        return;
    do
    {
        Field* fields = result->Fetch();
        ObjectGuid playerGuid = ObjectGuid::Create<HighGuid::Player>(fields[0].Get<uint32>());
        uint32 itemId = fields[1].Get<uint32>();
        AddItemToCollection(playerGuid, itemId);
    } while (result->NextRow());
}
