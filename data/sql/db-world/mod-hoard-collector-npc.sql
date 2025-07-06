SET
@Entry := 70000,
@Name := "Hoard the Collector",
@Model := 18324,
@Scale := 1;

DELETE FROM `creature_template` WHERE `entry` = @Entry;
INSERT INTO `creature_template` (`entry`, `name`, `subname`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, `exp`, `faction`, `npcflag`, `scale`, `rank`, `dmgschool`, `baseattacktime`, `rangeattacktime`, `unit_class`, `unit_flags`, `type`, `type_flags`, `lootid`, `pickpocketloot`, `skinloot`, `AIName`, `MovementType`, `HoverHeight`, `RacialLeader`, `movementId`, `RegenHealth`, `mechanic_immune_mask`, `flags_extra`, `ScriptName`) VALUES
(@Entry, @Name, NULL, NULL, 0, 1, 1, 2, 35, 1, @Scale, 0, 0, 2000, 0, 1, 0, 7, 0, 0, 0, 0, '', 0, 1, 0, 0, 1, 0, 0, 'npc_hoard_the_collector');

DELETE FROM `creature_template_model` WHERE `CreatureID` = @Entry;
INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`) VALUES
(@Entry, 0, @Model, @Scale, 1, 0);

DELETE FROM `npc_text` WHERE `ID` IN (70000, 70001, 70002, 70003, 70004);
INSERT INTO `npc_text` (`ID`, `text0_0`) VALUES
(70000, 'Ah, greetings, traveler. I am Hoard the Collector — seer of the stars, warden of what must not be lost. Place your treasures in my care, be they cursed, blessed, or simply beloved. I forget nothing... and what I guard is guarded forever.'),
(70001, 'There are no items left in the storage.'),
(70002, 'Hmm… I see no treasures worthy of safekeeping. Either your pack is light, or what you carry is not mine to guard. Return when you bear something of value.\n\nThere are no valid items to store in your backpack.'),
(70003, 'Curious… I searched the threads of fate, but found nothing matching your words. Check your spelling, or perhaps the item you seek is yet to be claimed.\n\nNo items found matching your search criteria.'),
(70004, 'Know this, traveler: not every item can be entrusted to my care. The vault obeys ancient laws — only certain objects, those attuned to the weave of order and soulbinding, may be stored. Attempting to place what lies outside these bounds would risk unraveling the balance I am sworn to protect. Choose wisely what you bring before me.\n\nIn order to be stored, items must be:\n- Bind on Pickup\n- Not stackable\n\nYou are able to unlock up to 3 storage slots, holding 150 items each.');
