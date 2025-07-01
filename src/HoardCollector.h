#ifndef DEF_HOARDCOLLECTOR_H
#define DEF_HOARDCOLLECTOR_H

#include "Player.h"
#include "Config.h"
#include "InstanceScript.h"
#include "ScriptMgr.h"
#include "ScriptedGossip.h"

const std::string ModName = "mod_hoard_collector";
const std::string SubsModName = "acore_cms_subscriptions";

class HoardCollector
{
private:
    std::unordered_map<ObjectGuid, std::vector<uint32>> HoarderCollectedItems;

public:
    static HoardCollector* instance();
    std::unordered_map<ObjectGuid, std::vector<uint32>>& GetCollectedItems() { return HoarderCollectedItems; }

    void LoadCollectionItems(uint32 guid);

    void AddItemToCollection(ObjectGuid playerGuid, uint32 itemId)
    {
        HoarderCollectedItems[playerGuid].push_back(itemId);
    }

    void RemoveItemFromCollection(ObjectGuid playerGuid, uint32 itemId)
    {
        auto& items = HoarderCollectedItems[playerGuid];
        items.erase(std::remove(items.begin(), items.end(), itemId), items.end());
    }

    bool HasItemInCollection(ObjectGuid playerGuid, uint32 itemId) const
    {
        auto it = HoarderCollectedItems.find(playerGuid);
        if (it != HoarderCollectedItems.end())
        {
            const auto& items = it->second;
            return std::find(items.begin(), items.end(), itemId) != items.end();
        }
        return false;
    }

    void ClearCollection(ObjectGuid playerGuid)
    {
        HoarderCollectedItems.erase(playerGuid);
    }

    [[nodiscard]] bool IsCollectionEmpty(ObjectGuid playerGuid) { return HoarderCollectedItems[playerGuid].empty(); }
};

#define sCollector HoardCollector::instance()

#endif
