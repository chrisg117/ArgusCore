-- Faronaar: Fel Machinations and Saving Stellagosa. Nothing here was sniffed. Everyone stands at the quests' POIs, heights
-- from the terrain, positions rough. The order is Wowhead's storyline: From Within, Fel Machinations, Saving Stellagosa.
INSERT INTO `quest_template_addon` (`ID`, `PrevQuestID`) VALUES
(37656, 36920),
(37450, 37656),
(37449, 37450)
ON DUPLICATE KEY UPDATE `PrevQuestID`=VALUES(`PrevQuestID`);

DELETE FROM `creature_queststarter` WHERE `id`=90474 AND `quest` IN (37656, 37449);
INSERT INTO `creature_queststarter` (`id`, `quest`, `VerifiedBuild`) VALUES
(90474, 37656, 0), -- Demon Hunter: Fel Machinations
(90474, 37449, 0); -- Demon Hunter: Dark Revelations, offered on turning in Saving Stellagosa

-- Soul Harvesters hold the hostages: destroying one frees its demon hunter
UPDATE `creature_template` SET `faction`=2856, `AIName`='SmartAI' WHERE `entry`=94745;
UPDATE `creature_template_difficulty` SET `MinLevel`=110, `MaxLevel`=110, `LevelScalingMin`=98, `LevelScalingMax`=110 WHERE `Entry`=94745 AND `DifficultyID`=0;
DELETE FROM `smart_scripts` WHERE `entryorguid`=94745 AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `action_type`, `action_param1`, `target_type`, `comment`) VALUES
(94745, 0, 0, 0, 25, 8, 0, 1, 'Soul Harvester - On reset - Set passive'),
(94745, 0, 1, 0, 6, 33, 90487, 7, 'Soul Harvester - On death - Credit Demon Hunters freed');

-- Overseer Lykill carries the key to the Fel Locks
UPDATE `creature_template` SET `faction`=2856 WHERE `entry`=86535;
UPDATE `creature_template_difficulty` SET `LootID`=86535 WHERE `Entry`=86535 AND `DifficultyID`=0;
DELETE FROM `creature_loot_template` WHERE `Entry`=86535 AND `Item`=120359;
INSERT INTO `creature_loot_template` (`Entry`, `ItemType`, `Item`, `Chance`, `QuestRequired`, `LootMode`, `GroupId`, `MinCount`, `MaxCount`, `Comment`) VALUES
(86535, 0, 120359, 100, 1, 1, 0, 1, 1, 'Overseer Lykill - Lykill''s Key');

UPDATE `creature_template` SET `ScriptName`='npc_saving_stellagosa_stellagosa' WHERE `entry`=90546;

DELETE FROM `creature` WHERE `guid` BETWEEN 12201001 AND 12201030;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `PhaseId`, `PhaseGroup`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `VerifiedBuild`) VALUES
(12201001, 90474, 1220, 7334, 7349, '0', 0, 0, 0, 0, -294.00, 7330.00, 16.20, 3.14, 300, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Fel Machinations giver and ender
(12201002, 90474, 1220, 7334, 7349, '0', 0, 0, 0, 0, -624.00, 7251.00, 26.70, 3.14, 300, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Saving Stellagosa giver
(12201003, 90474, 1220, 7334, 7351, '0', 0, 0, 0, 0, -722.00, 7337.00, 14.90, 3.14, 300, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Saving Stellagosa ender
(12201004, 86535, 1220, 7334, 7351, '0', 0, 0, 0, 0, -670.00, 7159.00, 28.00, 1.57, 300, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Overseer Lykill
(12201005, 90662, 1220, 7334, 7349, '0', 0, 0, 0, 0, -290.00, 7420.00, 45.50, 4.71, 600, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Imp Mother Fecunda, the quest text's imp mother
(12201006, 94745, 1220, 7334, 7349, '0', 0, 0, 0, 0, -250.00, 7290.00, 9.00, 0, 60, 0, 0, 0, NULL, NULL, NULL, NULL, 0),
(12201007, 94745, 1220, 7334, 7349, '0', 0, 0, 0, 0, -220.00, 7300.00, 11.10, 0, 60, 0, 0, 0, NULL, NULL, NULL, NULL, 0),
(12201008, 94745, 1220, 7334, 7349, '0', 0, 0, 0, 0, -260.00, 7340.00, 15.80, 0, 60, 0, 0, 0, NULL, NULL, NULL, NULL, 0),
(12201009, 94745, 1220, 7334, 7349, '0', 0, 0, 0, 0, -300.00, 7380.00, 29.60, 0, 60, 0, 0, 0, NULL, NULL, NULL, NULL, 0),
(12201010, 94745, 1220, 7334, 7349, '0', 0, 0, 0, 0, -330.00, 7400.00, 31.70, 0, 60, 0, 0, 0, NULL, NULL, NULL, NULL, 0),
(12201011, 94745, 1220, 7334, 7349, '0', 0, 0, 0, 0, -340.00, 7340.00, 31.70, 0, 60, 0, 0, 0, NULL, NULL, NULL, NULL, 0),
(12201012, 94745, 1220, 7334, 7349, '0', 0, 0, 0, 0, -290.00, 7300.00, 14.00, 0, 60, 0, 0, 0, NULL, NULL, NULL, NULL, 0),
(12201013, 90487, 1220, 7334, 7349, '0', 0, 0, 0, 0, -247.00, 7290.00, 9.00, 3.14, 60, 0, 0, 0, NULL, NULL, NULL, NULL, 0),
(12201014, 90487, 1220, 7334, 7349, '0', 0, 0, 0, 0, -217.00, 7300.00, 11.10, 3.14, 60, 0, 0, 0, NULL, NULL, NULL, NULL, 0),
(12201015, 90487, 1220, 7334, 7349, '0', 0, 0, 0, 0, -257.00, 7340.00, 15.80, 3.14, 60, 0, 0, 0, NULL, NULL, NULL, NULL, 0),
(12201016, 90487, 1220, 7334, 7349, '0', 0, 0, 0, 0, -297.00, 7380.00, 29.60, 3.14, 60, 0, 0, 0, NULL, NULL, NULL, NULL, 0),
(12201017, 90487, 1220, 7334, 7349, '0', 0, 0, 0, 0, -327.00, 7400.00, 31.70, 3.14, 60, 0, 0, 0, NULL, NULL, NULL, NULL, 0),
(12201018, 90487, 1220, 7334, 7349, '0', 0, 0, 0, 0, -337.00, 7340.00, 31.70, 3.14, 60, 0, 0, 0, NULL, NULL, NULL, NULL, 0),
(12201019, 90487, 1220, 7334, 7349, '0', 0, 0, 0, 0, -287.00, 7300.00, 14.00, 3.14, 60, 0, 0, 0, NULL, NULL, NULL, NULL, 0),
(12201020, 101943, 1220, 7334, 7349, '0', 0, 0, 0, 0, -245.00, 7283.00, 9.00, 1.57, 120, 5, 0, 1, NULL, NULL, NULL, NULL, 0),
(12201021, 101943, 1220, 7334, 7349, '0', 0, 0, 0, 0, -305.00, 7375.00, 29.50, 1.57, 120, 5, 0, 1, NULL, NULL, NULL, NULL, 0),
(12201022, 101943, 1220, 7334, 7349, '0', 0, 0, 0, 0, -335.00, 7345.00, 29.70, 1.57, 120, 5, 0, 1, NULL, NULL, NULL, NULL, 0),
(12201023, 101943, 1220, 7334, 7351, '0', 0, 0, 0, 0, -670.00, 7150.00, 28.60, 1.57, 120, 5, 0, 1, NULL, NULL, NULL, NULL, 0);

-- The hostages kneel
DELETE FROM `creature_addon` WHERE `guid` BETWEEN 12201013 AND 12201019;
INSERT INTO `creature_addon` (`guid`, `PathId`, `mount`, `MountCreatureID`, `StandState`, `AnimTier`, `VisFlags`, `SheathState`, `PvPFlags`, `emote`, `aiAnimKit`, `movementAnimKit`, `meleeAnimKit`, `visibilityDistanceType`, `auras`) VALUES
(12201013, 0, 0, 0, 8, 0, 0, 1, 0, 0, 0, 0, 0, 0, ''),
(12201014, 0, 0, 0, 8, 0, 0, 1, 0, 0, 0, 0, 0, 0, ''),
(12201015, 0, 0, 0, 8, 0, 0, 1, 0, 0, 0, 0, 0, 0, ''),
(12201016, 0, 0, 0, 8, 0, 0, 1, 0, 0, 0, 0, 0, 0, ''),
(12201017, 0, 0, 0, 8, 0, 0, 1, 0, 0, 0, 0, 0, 0, ''),
(12201018, 0, 0, 0, 8, 0, 0, 1, 0, 0, 0, 0, 0, 0, ''),
(12201019, 0, 0, 0, 8, 0, 0, 1, 0, 0, 0, 0, 0, 0, '');

-- The Fel Locks on the chains: the chain bunnies' spots, and lock 2358 takes Lykill's Key
DELETE FROM `gameobject` WHERE `guid` BETWEEN 12201102 AND 12201104;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `PhaseId`, `PhaseGroup`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `VerifiedBuild`) VALUES
(12201102, 239455, 1220, 7334, 7351, '0', 0, 0, -725.30, 7310.90, 14.20, 0, 0, 0, 0, 1, 30, 255, 1, 0),
(12201103, 239455, 1220, 7334, 7351, '0', 0, 0, -723.90, 7349.00, 15.30, 0, 0, 0, 0, 1, 30, 255, 1, 0),
(12201104, 239455, 1220, 7334, 7351, '0', 0, 0, -758.10, 7326.30, 14.50, 0, 0, 0, 0, 1, 30, 255, 1, 0);
