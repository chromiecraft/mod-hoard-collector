#ifndef DEF_HOARDCOLLECTOR_H
#define DEF_HOARDCOLLECTOR_H

#include "Player.h"
#include "Config.h"
#include "InstanceScript.h"
#include "ScriptMgr.h"
#include "ScriptedGossip.h"

enum HoarderStorages
{
    STORAGE_ONE,
    STORAGE_TWO,
    STORAGE_THREE,
    MAX_HOARDER_STORAGES
};

const std::string ModName = "mhc";
const std::string SubsModName = "acore_cms_subscriptions";

class HoardCollector
{
private:
    std::unordered_map<ObjectGuid, std::unordered_map<uint8, std::vector<uint32>>> HoarderCollectedItems;
    std::unordered_map<ObjectGuid, uint8> HoarderStorageSelectionCache;
    uint32 HoarderNpcId;
    bool Enabled;

public:
    static HoardCollector* instance();
    std::unordered_map<ObjectGuid, std::unordered_map<uint8, std::vector<uint32>>>& GetCollectedItems() { return HoarderCollectedItems; }

    void LoadCollectionItems(uint32 guid);

    void AddItemToCollection(ObjectGuid playerGuid, uint8 storageId, uint32 itemId)
    {
        HoarderCollectedItems[playerGuid][storageId].push_back(itemId);
    }

    void RemoveItemFromCollection(ObjectGuid playerGuid, uint8 storageId, uint32 itemId)
    {
        auto& items = HoarderCollectedItems[playerGuid][storageId];
        items.erase(std::remove(items.begin(), items.end(), itemId), items.end());
    }

    bool HasItemInCollection(ObjectGuid playerGuid, uint8 storageId, uint32 itemId) const
    {
        auto playerIt = HoarderCollectedItems.find(playerGuid);
        if (playerIt != HoarderCollectedItems.end())
        {
            const auto& storageMap = playerIt->second;
            auto storageIt = storageMap.find(storageId);
            if (storageIt != storageMap.end())
            {
                const auto& items = storageIt->second;
                return std::find(items.begin(), items.end(), itemId) != items.end();
            }
        }
        return false;
    }

    bool HasItemInAnyCollection(ObjectGuid playerGuid, uint32 itemId) const
    {
        auto playerIt = HoarderCollectedItems.find(playerGuid);
        if (playerIt != HoarderCollectedItems.end())
        {
            const auto& storageMap = playerIt->second;
            for (const auto& [storageId, items] : storageMap)
            {
                if (std::find(items.begin(), items.end(), itemId) != items.end())
                    return true;
            }
        }
        return false;
    }

    void ClearCollection(ObjectGuid playerGuid)
    {
        HoarderCollectedItems.erase(playerGuid);
    }

    void SetStorageSelection(ObjectGuid playerGuid, uint8 storageId)
    {
        HoarderStorageSelectionCache[playerGuid] = storageId;
    }

    [[nodiscard]] uint8 GetStorageSelection(ObjectGuid playerGuid) const
    {
        auto it = HoarderStorageSelectionCache.find(playerGuid);
        return (it != HoarderStorageSelectionCache.end()) ? it->second : 0;
    }

    void ClearStorageSelection(ObjectGuid playerGuid)
    {
        HoarderStorageSelectionCache.erase(playerGuid);
    }

    void SetHoarderNpcId(uint32 npcId) { HoarderNpcId = npcId; }
    void SetEnabled(bool enabled) { Enabled = enabled; }
    [[nodiscard]] bool IsEnabled() const { return Enabled; }
    [[nodiscard]] bool IsCollectionEmpty(ObjectGuid playerGuid) { return HoarderCollectedItems[playerGuid].empty(); }

    [[nodiscard]] uint32 GetHoarderNpcId() const { return HoarderNpcId; }
    [[nodiscard]] bool IsItemValid(Item* item) const;
};

#define sCollector HoardCollector::instance()

#endif
