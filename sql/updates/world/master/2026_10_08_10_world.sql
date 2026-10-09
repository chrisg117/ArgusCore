-- Broken Shore scenario (map 1460): King Varian Wrynn (90713) stood unarmed. He holds his sword ("Monster - 1H
-- Sword - Varian's Blade"), which is the item the 7.3.5 client's own Creature.db2 lists for him.
DELETE FROM `creature_equip_template` WHERE `CreatureID`=90713;
INSERT INTO `creature_equip_template` (`CreatureID`, `ID`, `ItemID1`, `AppearanceModID1`, `ItemVisual1`, `ItemID2`, `AppearanceModID2`, `ItemVisual2`, `ItemID3`, `AppearanceModID3`, `ItemVisual3`, `VerifiedBuild`) VALUES
(90713, 1, 45899, 0, 0, 0, 0, 0, 0, 0, 0, 0);

UPDATE `creature` SET `equipment_id`=1 WHERE `guid`=14600300;
