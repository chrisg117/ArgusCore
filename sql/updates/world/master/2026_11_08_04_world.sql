-- Warlock S3: Finding the Scepter (43100), Caer Darrow.
--
-- The data gives 43100 no starter: its start POI is the Circle of Wills, where Calydus 101097 stands (Wowhead's
-- acquisition video, 14:20: taken right after the weapon choice), so he offers it to a warlock who chose Destruction
-- (fork data). On Caer Darrow (area 2298): the island credit, the Shadow Council's three papers (no templates in the
-- data: fork ones under the objectives' own entries), Jergosh the Invoker with the Book of Medivh (video 18:30, his line
-- is broadcast 115699), and Calydus 109698, who takes the turn-in and gives the Guise of Jergosh (218545).
-- The papers' look is a lineup (local-only preview file); until the pick, display 14799 (Shadow Council Codex) at size 2,
-- unpreviewed. Places: Wowhead's map data for 251696-8, 109608 and 109698 (Western Plaguelands, uiMap 22), converted
-- with WorldMapArea 22's bounds (Calydus's 69.2, 79.0 converts to 1101.9, -2558.9, beside his turn-in POI).
--
-- lint: texts pending 43100 offer/request texts from the user's texts file (advisor ruling 2026-11-08)

DELETE FROM `quest_template_addon` WHERE `ID`=43100;
INSERT INTO `quest_template_addon` (`ID`, `AllowableClasses`, `PrevQuestID`) VALUES
(43100, 256, 40684);

-- Calydus in the Circle offers it once Destruction is chosen (40687)
DELETE FROM `creature_queststarter` WHERE `id`=101097 AND `quest`=43100;
INSERT INTO `creature_queststarter` (`id`, `quest`, `VerifiedBuild`) VALUES
(101097, 43100, 0); -- Calydus: Finding the Scepter
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=19 AND `SourceEntry`=43100;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `Comment`) VALUES
(19, 0, 43100, 0, 0, 8, 0, 40687, 0, 0, 0, 'Finding the Scepter: Destruction chosen');

-- Calydus on Caer Darrow (gossip: the Book, then the guise) and Jergosh
UPDATE `creature_template` SET `ScriptName`='npc_calydus_caer_darrow', `npcflag`=`npcflag`|1 WHERE `entry`=109698;
UPDATE `creature_template` SET `ScriptName`='npc_jergosh_the_invoker' WHERE `entry`=109608;

-- Map 0 fork series 10000000 + n (creatures from 10000020, gameobjects from 10000010)
DELETE FROM `creature` WHERE `guid` IN (10000020, 10000021);
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `PhaseId`, `PhaseGroup`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `VerifiedBuild`) VALUES
(10000020, 109698, 0, 0, 0, '0', 0, 0, 0, 0, 1097.00, -2557.00, 60.41, 0.10, 300, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Calydus, Caer Darrow (the turn-in POI)
(10000021, 109608, 0, 0, 0, '0', 0, 0, 0, 0, 1239.60, -2589.00, 90.84, 3.00, 60, 0, 0, 0, NULL, NULL, NULL, NULL, 0); -- Jergosh the Invoker (Wowhead 69.9, 74.2; his three sniffed spots average)

-- Seen only on the quest (visibility by entry: both are only these spawns). Calydus also while it waits for its turn-in
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=32 AND `SourceGroup` IN (3, 5) AND `SourceEntry` IN (109698, 109608, 251696, 251697, 251698);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `Comment`) VALUES
(32, 3, 109698, 0, 0, 47, 0, 43100, 74, 0, 0, 'Calydus (Caer Darrow): Finding the Scepter incomplete, complete or rewarded'),
(32, 3, 109608, 0, 0, 47, 0, 43100, 8, 0, 0, 'Jergosh the Invoker: Finding the Scepter incomplete'),
(32, 5, 251696, 0, 0, 47, 0, 43100, 8, 0, 0, 'Council Notice: Finding the Scepter incomplete'),
(32, 5, 251697, 0, 0, 47, 0, 43100, 8, 0, 0, 'Forgotten Letter: Finding the Scepter incomplete'),
(32, 5, 251698, 0, 0, 47, 0, 43100, 8, 0, 0, 'Logbook of Ur''dan: Finding the Scepter incomplete');

-- The Shadow Council's papers (fork templates under the objectives' entries; goobers, read through go_shadow_council_information)
DELETE FROM `gameobject_template` WHERE `entry` IN (251696, 251697, 251698);
INSERT INTO `gameobject_template` (`entry`, `type`, `displayId`, `name`, `IconName`, `castBarCaption`, `unk1`, `size`, `ScriptName`, `VerifiedBuild`) VALUES
(251696, 10, 14799, 'Council Notice', '', '', '', 2, 'go_shadow_council_information', 0),
(251697, 10, 14799, 'Forgotten Letter', '', '', '', 2, 'go_shadow_council_information', 0),
(251698, 10, 14799, 'Logbook of Ur''dan', '', '', '', 2, 'go_shadow_council_information', 0);
DELETE FROM `gameobject` WHERE `guid` IN (10000010, 10000011, 10000012);
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(10000010, 251696, 0, 0, 0, '0', 0, 0, 0, -1, 1087.60, -2554.60, 60.96, 0, 0, 0, 0, 1, 60, 255, 1, '', NULL, 0), -- Council Notice (Wowhead 69.1, 79.5)
(10000011, 251697, 0, 0, 0, '0', 0, 0, 0, -1, 1146.40, -2567.50, 60.99, 0, 0, 0, 0, 1, 60, 255, 1, '', NULL, 0), -- Forgotten Letter (Wowhead 69.4, 77.45)
(10000012, 251698, 0, 0, 0, '0', 0, 0, 0, -1, 1146.40, -2550.30, 60.14, 0, 0, 0, 0, 1, 60, 255, 1, '', NULL, 0); -- Logbook of Ur'dan (Wowhead 69.0, 77.45)

-- Lines (broadcast texts)
DELETE FROM `creature_text` WHERE `CreatureID` IN (109608, 109698);
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundPlayType`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT c.`cid`, 0, 0, b.`Text`, c.`type`, 0, 100, 0, 0, 0, 0, b.`ID`, 0, c.`cmt` FROM `hotfixes`.`broadcast_text` b JOIN (
  SELECT 109608 AS cid, 115699 AS bid, 14 AS type, 'Jergosh the Invoker - the Book of Medivh is ours' AS cmt
  UNION ALL SELECT 109698, 115708, 12, 'Calydus - Finding the Scepter turned in: trick Gul''dan'
) c ON c.`bid` = b.`ID`;
