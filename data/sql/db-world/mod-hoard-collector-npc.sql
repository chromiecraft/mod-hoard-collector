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

DELETE FROM `npc_text` WHERE `ID` IN (70000, 70001);
INSERT INTO `npc_text` (`ID`, `text0_0`) VALUES
(70000, 'Ah, greetings, traveler. I am Hoard the Collector — seer of the stars, warden of what must not be lost. Place your treasures in my care, be they cursed, blessed, or simply beloved. I forget nothing... and what I guard is guarded forever.'),
(70001, 'There are no items left in the storage.');
