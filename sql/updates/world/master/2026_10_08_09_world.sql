-- Broken Shore scenario (map 1460), Alliance side: Lady Jaina Proudmoore and Genn Greymane, who lead the
-- landing force and fight alongside players on the beach. They are a spawn group of their own, spawned by
-- the instance script when "Storm the Beach" starts, because they stay when that stage ends.
--
-- They stand a few yards up the beach from where players land, short of the demons. Their positions are a
-- choice: Wowhead's map pins for Genn only show where he was seen fighting, and Jaina's are on another map.

UPDATE `creature_template` SET `ScriptName`='npc_broken_shore_alliance_leader' WHERE `entry` IN (90714, 90717);

-- What Jaina holds: her staff ("Monster - Staff, Ornate Mage Staff (Frost Enchant)") and a book in her off hand
-- ("Monster - Item, Book - B02 Blue Glowing Offhand"). These are the items the 7.3.5 client's own Creature.db2
-- lists for her; it lists none for Genn.
DELETE FROM `creature_equip_template` WHERE `CreatureID`=90714;
INSERT INTO `creature_equip_template` (`CreatureID`, `ID`, `ItemID1`, `AppearanceModID1`, `ItemVisual1`, `ItemID2`, `AppearanceModID2`, `ItemVisual2`, `ItemID3`, `AppearanceModID3`, `ItemVisual3`, `VerifiedBuild`) VALUES
(90714, 1, 139131, 0, 0, 12869, 0, 0, 0, 0, 0, 0);

-- What they yell on the beach. Text, emote and sound are those of each line's broadcast text; Jaina's lines
-- are the female text of theirs. Group 0 is the same for both: one of three yells when a fight starts.
DELETE FROM `creature_text` WHERE `CreatureID` IN (90714, 90717);
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundPlayType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(90714, 0, 0, 'Laying down a blizzard!', 14, 0, 100, 0, 0, 53356, 0, 94593, 0, 'Lady Jaina Proudmoore - in a fight'),
(90714, 0, 1, 'Putting this one on ice!', 14, 0, 100, 0, 0, 53357, 0, 94594, 0, 'Lady Jaina Proudmoore - in a fight'),
(90714, 0, 2, 'My magic will tear you apart!', 14, 0, 100, 0, 0, 53358, 0, 94595, 0, 'Lady Jaina Proudmoore - in a fight'),
(90714, 1, 0, 'The third fleet reinforcements!', 14, 0, 100, 603, 0, 53345, 0, 94240, 0, 'Lady Jaina Proudmoore - greets the reinforcements'),
(90714, 2, 0, 'It''s now or never, Genn!', 14, 0, 100, 5, 0, 53347, 0, 94242, 0, 'Lady Jaina Proudmoore - before the charge'),
(90714, 3, 0, 'Those crystals appear to be anchoring the structures to our dimension. Take them out, and I bet the whole structure would crumble!', 14, 0, 100, 0, 0, 53362, 0, 94632, 0, 'Lady Jaina Proudmoore - on the crystals'),
(90714, 4, 0, 'It worked!', 14, 0, 100, 0, 0, 53364, 0, 94635, 0, 'Lady Jaina Proudmoore - first spire destroyed'),
(90717, 0, 0, 'Side by side, don''t let them break through!', 14, 0, 100, 0, 0, 53353, 0, 94590, 0, 'Genn Greymane - in a fight'),
(90717, 0, 1, 'Go for the throat!', 14, 0, 100, 0, 0, 53354, 0, 94591, 0, 'Genn Greymane - in a fight'),
(90717, 0, 2, 'This one''s mine!', 14, 0, 100, 0, 0, 53355, 0, 94592, 0, 'Genn Greymane - in a fight'),
(90717, 1, 0, 'Just in time. We haven''t been able to break their line, now we may have a chance.', 14, 0, 100, 0, 0, 53346, 0, 94241, 0, 'Genn Greymane - greets the reinforcements'),
(90717, 2, 0, 'There''s only one way out of this, Alliance, and it''s through that line of demons! Cannons, lay down covering fire! All forces, CHARGE!', 14, 0, 100, 0, 0, 53348, 0, 94585, 0, 'Genn Greymane - orders the charge'),
(90717, 3, 0, 'Let''s hope you''re right.', 14, 0, 100, 0, 0, 53363, 0, 94633, 0, 'Genn Greymane - answers Jaina on the crystals'),
(90717, 4, 0, 'One down, two to go.', 14, 0, 100, 0, 0, 53365, 0, 94636, 0, 'Genn Greymane - first spire destroyed'),
(90717, 5, 0, 'One more and the shore is ours!', 14, 0, 100, 0, 0, 53367, 0, 94638, 0, 'Genn Greymane - second spire destroyed');

DELETE FROM `spawn_group_template` WHERE `groupId`=14611;
INSERT INTO `spawn_group_template` (`groupId`, `groupName`, `groupFlags`) VALUES
(14611, 'Broken Shore Scenario - Alliance Leaders', 4);

DELETE FROM `creature` WHERE `guid` IN (14601100, 14601101);
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curHealthPct`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `VerifiedBuild`) VALUES
(14601100, 90714, 1460, 7534, 8290, '12', 0, 0, 0, -1, 0, 1, 457.00, 2084.00, 0.70, 0.3036, 7200, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0), -- Lady Jaina Proudmoore
(14601101, 90717, 1460, 7534, 8290, '12', 0, 0, 0, -1, 0, 0, 458.00, 2068.00, 0.25, 0.4734, 7200, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 0); -- Genn Greymane

DELETE FROM `spawn_group` WHERE `groupId`=14611;
INSERT INTO `spawn_group` (`groupId`, `spawnType`, `spawnId`) VALUES
(14611, 0, 14601100),
(14611, 0, 14601101);
