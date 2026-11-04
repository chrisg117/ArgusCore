-- The Call of Icecrown (38990), its first steps: the Death Gate from Dalaran and the arrival on Acherus.
-- A Pact of Necessity's turn-in offers it (190092); Lankral offers it again to a frost death knight who declined.
-- The quest's own gate summon aims at a target type the core lacks, so quest_the_call_of_icecrown places it.
-- Acherus has no spawns in this world DB; the positions are the quest POIs, the heights the decks' floors.
DELETE FROM `creature_queststarter` WHERE `quest`=38990;
INSERT INTO `creature_queststarter` (`id`, `quest`, `VerifiedBuild`) VALUES
(101441, 38990, 0);

DELETE FROM `quest_template_addon` WHERE `ID`=38990;
INSERT INTO `quest_template_addon` (`ID`, `AllowableClasses`, `PrevQuestID`, `ScriptName`) VALUES
(38990, 32, 40715, 'quest_the_call_of_icecrown');

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=19 AND `SourceEntry`=38990;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(19, 0, 38990, 0, 0, 47, 0, 40723, 64, 0, 0, 0, 0, '', 'The Call of Icecrown: Frost chosen');

-- The summoned gate takes the player to Acherus (187753, routed by spell_dk_death_gate_teleport)
DELETE FROM `npc_spellclick_spells` WHERE `npc_entry`=95854;
INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(95854, 187753, 3, 0);

-- The gate on Acherus that opens Icecrown Citadel: 102959 is the client's second Death Gate
DELETE FROM `npc_spellclick_spells` WHERE `npc_entry`=102959;
INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(102959, 200428, 3, 0);

DELETE FROM `spell_script_names` WHERE `ScriptName` IN ('spell_dk_the_call_of_icecrown_death_gate', 'spell_dk_death_gate_icecrown');
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(187753, 'spell_dk_the_call_of_icecrown_death_gate'),
(200428, 'spell_dk_death_gate_icecrown');

-- Mograine meets the player where the Death Gate lands (113695, lower deck) and takes the turn-in on the upper deck
-- (93437) once the blades are won; the Icecrown gate stands by the landing while the quest is under way
DELETE FROM `creature` WHERE `guid` BETWEEN 12207000 AND 12207002;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `PhaseId`, `PhaseGroup`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `VerifiedBuild`) VALUES
(12207000, 113695, 1220, 7679, 7679, '0', 0, 0, 0, 0, -1439.00, 1161.00, 275.38, 3.68, 300, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Highlord Darion Mograine (arrival)
(12207001, 93437, 1220, 7679, 7679, '0', 0, 0, 0, 0, -1507.00, 1048.00, 299.22, 0.00, 300, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Highlord Darion Mograine (hall)
(12207002, 102959, 1220, 7679, 7679, '0', 0, 0, 0, 0, -1446.00, 1166.00, 275.38, 4.16, 300, 0, 0, 0, NULL, NULL, NULL, NULL, 0); -- Death Gate (to Icecrown)

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=32 AND `SourceGroup`=3 AND `SourceEntry` IN (113695, 93437, 102959);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(32, 3, 113695, 0, 0, 47, 0, 38990, 8, 0, 0, 0, 0, '', 'Mograine (arrival): The Call of Icecrown under way'),
(32, 3, 102959, 0, 0, 47, 0, 38990, 8, 0, 0, 0, 0, '', 'Death Gate (to Icecrown): The Call of Icecrown under way'),
(32, 3, 93437, 0, 0, 47, 0, 38990, 66, 0, 0, 0, 0, '', 'Mograine (hall): The Call of Icecrown complete or done');

-- The client offers a spellclick only on a unit with the spellclick flag; the core does not set it from the rows above
UPDATE `creature_template` SET `npcflag`=`npcflag`|16777216 WHERE `entry` IN (95854, 102959);
