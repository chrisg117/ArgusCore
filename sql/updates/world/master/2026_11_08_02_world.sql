-- Warlock S1: The Sixth (40716) and the hall founding.
--
-- The data gives The Sixth no starter. The 7.0 Order Campaign aura in Dalaran (224240, zone 7502) already casts
-- 204858 Summon Risstyn's Portal (103505) for a warlock who has not taken it (zone_dalaran_broken_isle.cpp). Staging
-- from Wowhead's "Destruction Warlock Artifact Acquisition Preview" (youtube uG0AT8FnkDM):
--   0:06  Ritssyn, out of his portal in the street, greets the warlock (broadcast 106111);
--   0:13  he offers The Sixth;
--   0:20  after the accept: "Join us at the Circle of Wills ... My portal will take you there directly" (104179);
--   0:26  he is gone; the warlock clicks the red portal;
--   0:32  "Discovered The Underbelly", Circle of Wills: the grated arena floor.
-- So: the portal's script summons Ritssyn 103506 (as the warlock's summon; the data's 204860 cannot find the portal on
-- this core); that Ritssyn offers the quest (fork data), says his two lines and walks back into the portal; a click on the portal casts
-- 204857 Risstyn's Portal, whose spell_target_position (TDB) is the Circle of Wills, -856.05, 4453.25, 696.67.
-- The quest is turned in to Ritssyn Flamescowl 101456 there; its reward spell 210908 triggers 192190
-- (spell_order_hall_founding: a non-DH founds site 161 through 185506) and 210839 (solo-experience prompt).
--
-- Circle of Wills floor: 204857's destination puts it at z 696.7; the navmesh at Ritssyn's spot (the quest's turn-in
-- POI, -842, 4447) has the matching layer at 697.05 (zone 8392, the Underbelly's top level; the Dalaran streets are
-- the 735-745 layers above). Facing: towards the portal's landing.
--
-- lint: texts pending 40716 offer text from the user's texts file (advisor ruling 2026-11-08: empty until then)

-- Warlocks only (no quest_template_addon row in the data)
DELETE FROM `quest_template_addon` WHERE `ID`=40716;
INSERT INTO `quest_template_addon` (`ID`, `AllowableClasses`) VALUES
(40716, 256);

-- The portal and the Ritssyn who steps out of it (both are only ever these summons: no spawns of either entry).
-- The portal takes clicks (this core needs UNIT_NPC_FLAG_SPELLCLICK in the template as well as the spellclick row)
UPDATE `creature_template` SET `ScriptName`='npc_risstyns_portal', `npcflag`=`npcflag`|16777216 WHERE `entry`=103505;
UPDATE `creature_template` SET `ScriptName`='npc_ritssyn_flamescowl_the_sixth', `npcflag`=`npcflag`|2 WHERE `entry`=103506;

DELETE FROM `npc_spellclick_spells` WHERE `npc_entry`=103505;
INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(103505, 204857, 3, 0); -- Demonic Portal: Risstyn's Portal, to the Circle of Wills

-- Ritssyn 103506 offers The Sixth (fork data); 101456 already ends it (creature_questender, TDB)
DELETE FROM `creature_queststarter` WHERE `id`=103506 AND `quest`=40716;
INSERT INTO `creature_queststarter` (`id`, `quest`, `VerifiedBuild`) VALUES
(103506, 40716, 0); -- Ritssyn Flamescowl (portal copy): The Sixth

-- His two lines (broadcast texts from the client data)
DELETE FROM `creature_text` WHERE `CreatureID`=103506;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundPlayType`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 103506, 0, 0, `Text`, 12, 0, 100, 0, 0, 0, 0, `ID`, 0, 'Ritssyn Flamescowl - The Sixth greeting' FROM `hotfixes`.`broadcast_text` WHERE `ID`=106111;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundPlayType`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 103506, 1, 0, `Text`, 12, 0, 100, 0, 0, 0, 0, `ID`, 0, 'Ritssyn Flamescowl - The Sixth accepted' FROM `hotfixes`.`broadcast_text` WHERE `ID`=104179;

-- The portal's click only while The Sixth is in the log. (204860 Summon Risstyn is not used: a self-range spell, its
-- DEST_NEARBY_ENTRY search has range 0 on this core and never finds the portal; the portal's script summons Ritssyn.)
DELETE FROM `conditions` WHERE (`SourceTypeOrReferenceId`=13 AND `SourceEntry`=204860) OR (`SourceTypeOrReferenceId`=18 AND `SourceGroup`=103505);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `Comment`) VALUES
(18, 103505, 204857, 0, 0, 9, 0, 40716, 0, 0, 0, 'Risstyn''s Portal: only with The Sixth in the log');

-- Ritssyn Flamescowl in the Circle of Wills (Dalaran fork block 12205000-099: warlock)
DELETE FROM `creature` WHERE `guid`=12205000;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `PhaseId`, `PhaseGroup`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `VerifiedBuild`) VALUES
(12205000, 101456, 1220, 0, 0, '0', 0, 0, 0, 0, -842.00, 4447.00, 697.05, 2.72, 300, 0, 0, 0, NULL, NULL, NULL, NULL, 0); -- Ritssyn Flamescowl
