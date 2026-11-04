-- DK campaign: The Call of Icecrown's arrival. The intro scene's end launches the Icecrown Citadel scenario (190794
-- sends 45596, which casts 188406 Launch The Blades of the Fallen Prince Scenario); in Acherus with the quest under
-- way a death knight is sent straight in. On 1480 Mograine waits at the landing, walks up the ramp and speaks.

-- The Death Gate 102959 is a Niskara object: it does not belong in Acherus, and the scene's end replaces it
DELETE FROM `creature` WHERE `guid`=12207002;
DELETE FROM `npc_spellclick_spells` WHERE `npc_entry`=102959;
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=32 AND `SourceGroup`=3 AND `SourceEntry`=102959;
DELETE FROM `spell_script_names` WHERE `spell_id`=200428 AND `ScriptName`='spell_dk_death_gate_icecrown';
UPDATE `creature_template` SET `npcflag`=`npcflag`&~16777216 WHERE `entry`=102959;

-- The scene's end launches the scenario
DELETE FROM `event_script_names` WHERE `Id`=45596;
INSERT INTO `event_script_names` (`Id`, `ScriptName`) VALUES
(45596, 'event_dk_the_blades_of_the_fallen_prince_scene_end');
DELETE FROM `spell_script_names` WHERE `spell_id`=188406;
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(188406, 'spell_dk_launch_the_blades_of_the_fallen_prince_scenario');

-- Highlord Darion Mograine (95193) at the landing beside the citadel (hand-placed: no sniff)
UPDATE `creature_template` SET `ScriptName`='npc_icc_scenario_darion_mograine' WHERE `entry`=95193;
DELETE FROM `creature` WHERE `guid`=14800000;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `PhaseId`, `PhaseGroup`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `VerifiedBuild`) VALUES
(14800000, 95193, 1480, 0, 0, '12', 0, 0, 0, 0, -729.00, 2205.10, 535.80, 0.00, 300, 0, 0, 0, NULL, NULL, NULL, NULL, 0); -- Highlord Darion Mograine

-- 744 Frost DK - Entrance (196966): Mograine's five lines, client lines 8322-8326; timings from the line lengths
DELETE FROM `conversation_template` WHERE `Id` IN (744, 14800001);
INSERT INTO `conversation_template` (`Id`, `FirstLineId`, `LastLineEndTime`, `Flags`, `ScriptName`, `VerifiedBuild`) VALUES
(744, 8322, 41300, 0, '', 0),
(14800001, 1773, 6000, 0, '', 0);
DELETE FROM `conversation_actors` WHERE `ConversationId` IN (744, 14800001);
INSERT INTO `conversation_actors` (`ConversationId`, `ConversationActorId`, `ConversationActorGuid`, `Idx`, `CreatureId`, `CreatureDisplayInfoId`, `NoActorObject`, `ActivePlayerObject`, `VerifiedBuild`) VALUES
(744, 0, 14800000, 0, 0, 0, 0, 0, 0), -- Highlord Darion Mograine
(14800001, 0, 0, 0, 94369, 64011, 1, 0, 0); -- The Lich King (talking head)
DELETE FROM `conversation_line_template` WHERE `Id` IN (8322, 8323, 8324, 8325, 8326, 1773);
INSERT INTO `conversation_line_template` (`Id`, `StartTime`, `UiCameraID`, `ActorIdx`, `Flags`, `VerifiedBuild`) VALUES
(8322, 0, 0, 0, 0, 0), -- Ah, $n. So the Lich King chose you to be his hand?
(8323, 7900, 0, 0, 0, 0), -- Our forces have gathered from all across Northrend.
(8324, 19200, 0, 0, 0, 0), -- Make no mistake, we are not bound to this new Lich King.
(8325, 27700, 0, 0, 0, 0), -- For now, our goals align with his.
(8326, 36500, 0, 0, 0, 0), -- The gates of Icecrown lie to the north. Do not keep our ally waiting!
(1773, 0, 296, 0, 0, 0); -- Enter, $n, there is much to be done.
