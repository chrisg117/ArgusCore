-- DH campaign, the Illidari Redoubt: Eternal Vigil (42869), Securing the Way (42872), Return to Mardum (41033 Altruis / 41221 Kayn).
-- From the client: the Redoubt's area, quest POIs, the actors' entries, Sentence to Death's button (OverrideSpellData 737),
-- Reality Tear on the gateway object, hall creation 192191. Givers from the Warcraft Wiki: Kor'vas, Jace, Malevolence. No sniff
-- exists: spawn positions other than the POIs, and Kor'vas's line, are ours.
DELETE FROM `creature_queststarter` WHERE `quest` IN (42869, 42872, 41033, 41221);
INSERT INTO `creature_queststarter` (`id`, `quest`, `VerifiedBuild`) VALUES
(102799, 42869, 0),
(109196, 42872, 0),
(109240, 41033, 0),
(109240, 41221, 0);

DELETE FROM `quest_template_addon` WHERE `ID` IN (42869, 42872, 41033, 41221);
INSERT INTO `quest_template_addon` (`ID`, `AllowableClasses`, `PrevQuestID`) VALUES
(42869, 2048, 0),
(42872, 2048, 42869),
(41033, 2048, 42872),
(41221, 2048, 42872);

UPDATE `creature_template` SET `npcflag`=`npcflag`|2 WHERE `entry`=109240;

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=19 AND `SourceEntry` IN (42869, 41033, 41221);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `Comment`) VALUES
(19, 0, 42869, 0, 0, 47, 0, 39247, 64, 0, 0, 'Eternal Vigil: after The Hunt (Kayn)'),
(19, 0, 42869, 0, 1, 47, 0, 41119, 64, 0, 0, 'Eternal Vigil: after The Hunt (Altruis)'),
(19, 0, 41033, 0, 0, 47, 0, 40375, 64, 0, 0, 'Return to Mardum (Altruis): Altruis chosen'),
(19, 0, 41221, 0, 0, 47, 0, 40374, 64, 0, 0, 'Return to Mardum (Kayn): Kayn chosen');

-- Kor'vas at Krasus' Landing now also gives Eternal Vigil, and stays until it is done
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=32 AND `SourceGroup`=3 AND `SourceEntry`=102799;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `Comment`) VALUES
(32, 3, 102799, 0, 0, 47, 0, 39247, 74, 0, 0, 'Kor''vas at Krasus'' Landing: The Hunt (Kayn) taken'),
(32, 3, 102799, 0, 0, 47, 0, 42869, 11, 0, 0, 'Kor''vas at Krasus'' Landing: until Eternal Vigil is done'),
(32, 3, 102799, 0, 1, 47, 0, 41119, 74, 0, 0, 'Kor''vas at Krasus'' Landing: The Hunt (Altruis) taken'),
(32, 3, 102799, 0, 1, 47, 0, 42869, 11, 0, 0, 'Kor''vas at Krasus'' Landing: until Eternal Vigil is done');

-- The Redoubt, floor at 648, and the Fel Hammer's first spawns. Its ways back and forth are here so that nobody boards and is stuck.
DELETE FROM `creature` WHERE `guid` BETWEEN 12204129 AND 12204141;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `PhaseId`, `PhaseGroup`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `VerifiedBuild`) VALUES
(12204129, 109196, 1220, 0, 0, '0', 0, 0, 0, 0, -945.00, 4090.00, 648.40, 4.00, 120, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Jace
(12204130, 109240, 1220, 0, 0, '0', 0, 0, 0, 0, -940.00, 4085.00, 648.40, 3.60, 120, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Malevolence
(12204131, 109239, 1220, 0, 0, '0', 0, 0, 0, 0, -958.00, 4058.00, 648.22, 2.20, 120, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Kor'vas
(12204132, 109246, 1220, 0, 0, '0', 0, 0, 0, 0, -970.00, 4075.00, 648.28, 5.30, 120, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Belath
(12204133, 109273, 1220, 0, 0, '0', 0, 0, 0, 0, -964.00, 4066.00, 648.25, 0.00, 120, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Soul Font
(12204134, 109247, 1220, 0, 0, '0', 0, 0, 0, 0, -972.00, 4058.00, 648.20, 0.80, 30, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Captured Wrathguard
(12204135, 109252, 1220, 0, 0, '0', 0, 0, 0, 0, -972.00, 4068.00, 648.28, 6.00, 30, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Imprisoned Imps
(12204136, 109252, 1220, 0, 0, '0', 0, 0, 0, 0, -965.00, 4056.00, 648.20, 1.50, 30, 0, 0, 0, NULL, NULL, NULL, NULL, 0),
(12204137, 109252, 1220, 0, 0, '0', 0, 0, 0, 0, -956.00, 4065.00, 648.28, 3.10, 30, 0, 0, 0, NULL, NULL, NULL, NULL, 0),
(12204138, 95237, 1519, 0, 0, '0', 0, 0, 0, 0, 1525.00, 1418.00, 244.56, 3.14, 120, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Altruis
(12204139, 95240, 1519, 0, 0, '0', 0, 0, 0, 0, 1525.00, 1418.00, 244.56, 3.14, 120, 0, 0, 0, NULL, NULL, NULL, NULL, 0); -- Kayn

DELETE FROM `gameobject` WHERE `guid` BETWEEN 12204004 AND 12204006;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `PhaseId`, `PhaseGroup`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `VerifiedBuild`) VALUES
(12204004, 251579, 1220, 0, 0, '0', 0, 0, -964.00, 4066.00, 648.25, 0, 0, 0, 0, 1, 120, 255, 1, 0), -- what Reality Tear is cast at
(12204005, 251286, 1220, 0, 0, '0', 0, 0, -951.00, 4077.00, 648.40, 0, 0, 0, 0, 1, 120, 255, 1, 0), -- the gateway, to the Fel Hammer
(12204006, 247007, 1519, 0, 0, '0', 0, 0, 1465.00, 1411.00, 244.61, 0, 0, 0, 0, 1, 120, 255, 1, 0); -- and back to Dalaran

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=32 AND `SourceGroup`=3 AND `SourceEntry` IN (109196, 109240, 109239, 109246, 109273, 109247, 109252, 95237, 95240);
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=32 AND `SourceGroup`=5 AND `SourceEntry` IN (251579, 251286);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `Comment`) VALUES
(32, 3, 109196, 0, 0, 47, 0, 42869, 74, 0, 0, 'Jace at the Redoubt: once Eternal Vigil is taken'),
(32, 3, 109240, 0, 0, 47, 0, 42869, 74, 0, 0, 'Malevolence at the Redoubt: once Eternal Vigil is taken'),
(32, 3, 109239, 0, 0, 47, 0, 42869, 74, 0, 0, 'Kor''vas at the Redoubt: once Eternal Vigil is taken'),
(32, 3, 109246, 0, 0, 47, 0, 42869, 74, 0, 0, 'Belath at the Redoubt: once Eternal Vigil is taken'),
(32, 3, 109273, 0, 0, 47, 0, 42869, 74, 0, 0, 'Soul Font at the Redoubt: once Eternal Vigil is taken'),
(32, 3, 109247, 0, 0, 47, 0, 42872, 1, 0, 0, 'Captured Wrathguard: Securing the Way not taken yet'),
(32, 3, 109247, 0, 0, 47, 0, 42869, 74, 0, 0, 'Captured Wrathguard: once Eternal Vigil is taken'),
(32, 3, 109247, 0, 1, 48, 0, 284853, 0, 0, 0, 'Captured Wrathguard: or until the player sentenced it'),
(32, 3, 109252, 0, 0, 47, 0, 42872, 1, 0, 0, 'Imprisoned Imp: Securing the Way not taken yet'),
(32, 3, 109252, 0, 0, 47, 0, 42869, 74, 0, 0, 'Imprisoned Imp: once Eternal Vigil is taken'),
(32, 3, 109252, 0, 1, 48, 0, 284853, 0, 0, 0, 'Imprisoned Imp: or until the player sentenced it'),
(32, 3, 95237, 0, 0, 47, 0, 40375, 64, 0, 0, 'Altruis on the Fel Hammer: Altruis chosen'),
(32, 3, 95240, 0, 0, 47, 0, 40374, 64, 0, 0, 'Kayn on the Fel Hammer: Kayn chosen'),
(32, 5, 251579, 0, 0, 47, 0, 42872, 8, 0, 0, 'Reality Tear''s object: Securing the Way in progress'),
(32, 5, 251579, 0, 0, 48, 0, 284853, 0, 1, 0, 'Reality Tear''s object: after the sentence'),
(32, 5, 251286, 0, 0, 47, 0, 42872, 66, 0, 0, 'Gateway to the Fel Hammer: once it is ripped open');

-- Sentence to Death's button, while the quest wants it. The aura's own removal rules drop it on any teleport, so it comes back with the area.
DELETE FROM `spell_area` WHERE `spell`=217376 AND `area`=8284;
INSERT INTO `spell_area` (`spell`, `area`, `quest_start`, `quest_end`, `aura_spell`, `racemask`, `gender`, `flags`, `quest_start_status`, `quest_end_status`) VALUES
(217376, 8284, 42872, 0, 0, 0, 2, 3, 8, 11);
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=17 AND `SourceEntry`=217376;
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=13 AND `SourceEntry` IN (217458, 217386);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `Comment`) VALUES
(17, 0, 217376, 0, 0, 48, 0, 284853, 0, 0, 0, 'Sentence to Death button: not once sentenced'),
(13, 1, 217458, 0, 0, 31, 0, 3, 109273, 0, 0, 'Reality Tear: at the Soul Font'),
(13, 1, 217386, 0, 0, 31, 0, 3, 109273, 0, 0, 'Soul Missiles: at the Soul Font');

DELETE FROM `spell_script_names` WHERE `spell_id` IN (192190, 217377, 217458);
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(192190, 'spell_dh_create_order_hall'),
(217377, 'spell_dh_sentence_to_death'),
(217458, 'spell_dh_reality_tear');

DELETE FROM `creature_text` WHERE `CreatureID`=109239;
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(109239, 0, 0, 'Your souls are forfeit!', 12, 0, 100, 0, 0, 0, 0, 0, 'Kor''vas Bloodthorn to the prisoners');

-- The characters of this chain are level 110
UPDATE `creature_template_difficulty` SET `MinLevel`=110, `MaxLevel`=110 WHERE `DifficultyID`=0 AND `Entry` IN (102799, 109196, 109239, 109240, 109246, 109247, 109252, 95237, 95240);
