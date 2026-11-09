-- Warlock S5: Ritual Ruination (43254), Gul'dan's ritual on the Broken Shore.
--
-- The data gives 43254 no starter; Calydus 109838 at Krasus' Landing offers it after An Eye for a Scepter (43153)
-- (Wowhead's acquisition video, 37:05; fork data). A Fel Bat by him takes the warlock to the ritual (a teleport for now,
-- credit 110523; the flight is a fidelity item). Gul'dan 110533 takes the warlock for Jergosh's stand-in and sets them
-- against Allaris Narassin 110556; the Scepter of Sargeras 110561 lies on the altar (a spellclick on 219839, the
-- client's own spell: credit, item 128941; its scene 1365 is held back until previewed); the Scepter's Dimensional Rift,
-- or a click on the Legion Portal, overloads the portal (219920, credit 110606); Calydus opens the way to Dalaran (219705,
-- credit 110607; its 221318 teleports, spell_target_position below). The turn-in, Calydus 101097 in the Circle, is data.
-- POIs: Gul'dan -654,2584, the Scepter -671,2601, the portal -674,2607, the escape -693,2647, Dalaran -829,4374.
-- The Scepter's look: display 71354 (Scepter on the altar) on the spawn; 10812, the template's other model, is a
-- local-only lineup stand-in for the user's final pass. The Legion Portal's display 27772 is unpreviewed (ledger).
--
-- lint: z 12205006 the Scepter sits on the altar: dev target gives the altar top 84.38 (the navmesh has only the ground, 82.18)
-- lint: texts pending 43254 offer/request texts from the user's texts file (advisor ruling 2026-11-08)

DELETE FROM `quest_template_addon` WHERE `ID`=43254;
INSERT INTO `quest_template_addon` (`ID`, `AllowableClasses`, `PrevQuestID`) VALUES
(43254, 256, 43153);

DELETE FROM `creature_queststarter` WHERE `id`=109838 AND `quest`=43254;
INSERT INTO `creature_queststarter` (`id`, `quest`, `VerifiedBuild`) VALUES
(109838, 43254, 0); -- Calydus (Krasus' Landing): Ritual Ruination

-- Scripts and flags
UPDATE `creature_template` SET `ScriptName`='npc_fel_bat_ritual_ruination', `npcflag`=`npcflag`|1 WHERE `entry`=110480;
UPDATE `creature_template` SET `ScriptName`='npc_guldan_ritual_ruination' WHERE `entry`=110533;
UPDATE `creature_template` SET `ScriptName`='npc_allaris_ritual_ruination' WHERE `entry`=110556;
UPDATE `creature_template` SET `npcflag`=`npcflag`|16777216 WHERE `entry`=110561; -- UNIT_NPC_FLAG_SPELLCLICK
UPDATE `gameobject_template` SET `ScriptName`='go_ritual_ruination_portal', `Data0`=0 WHERE `entry`=251763; -- no lock: the click is the fallback

DELETE FROM `spell_script_names` WHERE `spell_id` IN (219839, 196586, 219923);
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(219839, 'spell_warlock_take_the_scepter'),
(196586, 'spell_warlock_dimensional_rift_ritual'),
(219923, 'spell_warlock_portal_overload');

-- The Scepter on the altar: the clicker casts 219839, once Allaris is dead and while the Scepter is not yet taken
DELETE FROM `npc_spellclick_spells` WHERE `npc_entry`=110561;
INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(110561, 219839, 1, 0);
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=18 AND `SourceGroup`=110561;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `Comment`) VALUES
(18, 110561, 219839, 0, 0, 48, 0, 285267, 0, 1, 0, 'Scepter of Sargeras: Allaris slain'),
(18, 110561, 219839, 0, 0, 48, 0, 285268, 0, 0, 0, 'Scepter of Sargeras: not yet taken');

-- 221318 Demonic Portal to Dalaran (TARGET_DEST_DB): the escape POI in Dalaran
DELETE FROM `spell_target_position` WHERE `ID`=221318;
INSERT INTO `spell_target_position` (`ID`, `EffectIndex`, `OrderIndex`, `MapID`, `PositionX`, `PositionY`, `PositionZ`, `Orientation`, `VerifiedBuild`) VALUES
(221318, 0, 0, 1220, -829.00, 4374.00, 738.96, 3.93, 0);

-- Map 1220 fork series 12200000 + n (creatures from 12205003, gameobjects from 12205003)
DELETE FROM `creature` WHERE `guid` IN (12205003, 12205004, 12205005, 12205006);
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `PhaseId`, `PhaseGroup`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `VerifiedBuild`) VALUES
(12205003, 110480, 1220, 0, 0, '0', 0, 0, 0, 0, -819.00, 4256.00, 745.85, 3.14, 120, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Fel Bat, Krasus' Landing (the ride's POI)
(12205004, 110533, 1220, 0, 0, '0', 0, 0, 0, 0, -654.00, 2584.00, 82.14, 2.14, 300, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Gul'dan (Listen to Gul'dan POI)
(12205005, 110556, 1220, 0, 0, '0', 0, 0, 0, 0, -660.00, 2592.00, 82.24, 2.30, 300, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Allaris Narassin
(12205006, 110561, 1220, 0, 0, '0', 0, 0, 71354, 0, -671.00, 2601.00, 84.40, 2.14, 60, 0, 0, 0, NULL, NULL, NULL, NULL, 0); -- Scepter of Sargeras on the altar (the Scepter POI)
DELETE FROM `gameobject` WHERE `guid` IN (12205003, 12205004);
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(12205003, 251770, 1220, 0, 0, '0', 0, 0, 0, -1, -671.00, 2601.00, 82.18, 0, 0, 0, 0, 1, 120, 255, 1, '', NULL, 0), -- the altar
(12205004, 251763, 1220, 0, 0, '0', 0, 0, 0, -1, -674.00, 2607.00, 81.53, 5.176, 0, 0, 0.525731, -0.850651, 120, 255, 1, '', NULL, 0); -- the ritual's Legion Portal (Ruin the ritual POI)

-- Seen only on the quest (visibility by entry: these are the entries' only spawns)
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=32 AND `SourceGroup` IN (3, 5) AND `SourceEntry` IN (110480, 110533, 110556, 110561, 251770, 251763);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `Comment`) VALUES
(32, 3, 110480, 0, 0, 47, 0, 43254, 8, 0, 0, 'Fel Bat (Krasus'' Landing): Ritual Ruination incomplete'),
(32, 3, 110533, 0, 0, 47, 0, 43254, 8, 0, 0, 'Gul''dan (the ritual): Ritual Ruination incomplete'),
(32, 3, 110556, 0, 0, 47, 0, 43254, 8, 0, 0, 'Allaris Narassin: Ritual Ruination incomplete'),
(32, 3, 110561, 0, 0, 47, 0, 43254, 8, 0, 0, 'Scepter of Sargeras (the altar): Ritual Ruination incomplete'),
(32, 5, 251770, 0, 0, 47, 0, 43254, 8, 0, 0, 'Legion altar: Ritual Ruination incomplete'),
(32, 5, 251763, 0, 0, 47, 0, 43254, 8, 0, 0, 'Legion Portal (the ritual): Ritual Ruination incomplete');

-- Lines (broadcast texts)
DELETE FROM `creature_text` WHERE `CreatureID` IN (110533, 110556) OR (`CreatureID`=109838 AND `GroupID`=0);
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundPlayType`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT c.`cid`, c.`grp`, 0, b.`Text`, 12, 0, 100, 0, 0, 0, 0, b.`ID`, 0, c.`cmt` FROM `hotfixes`.`broadcast_text` b JOIN (
  SELECT 110533 AS cid, 0 AS grp, 116381 AS bid, 'Gul''dan - reinforcements at last' AS cmt
  UNION ALL SELECT 110533, 1, 116382, 'Gul''dan - Jergosh did not make it'
  UNION ALL SELECT 110533, 2, 117566, 'Gul''dan - amplify this portal'
  UNION ALL SELECT 110533, 3, 117567, 'Gul''dan - an anchor on Argus'
  UNION ALL SELECT 110533, 4, 117576, 'Gul''dan - only one of you'
  UNION ALL SELECT 110533, 5, 117590, 'Gul''dan - you came out the victor'
  UNION ALL SELECT 110533, 6, 117591, 'Gul''dan - the time has come'
  UNION ALL SELECT 110556, 0, 117568, 'Allaris Narassin - which of us wields the Scepter'
  UNION ALL SELECT 109838, 0, 117620, 'Calydus - time to go, friend'
) c ON c.`bid` = b.`ID`;
