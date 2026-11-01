-- DH campaign, Dalaran and the Violet Hold: Making Arrangements (41120 Altruis / 40819 Kayn) and By Any Means (41121 / 39051).
-- From the client: gossip menus 19303/19304, every line of the scene, the door's kill credit and teleport 188163, the way
-- out 189688, Taldath's three spells. No sniff of the scene exists: its order, the spell timers and every position inside
-- map 1494 (apart from the quest POI and the spells' destinations) are ours.
DELETE FROM `creature_queststarter` WHERE `quest` IN (41120, 40819, 41121, 39051);
INSERT INTO `creature_queststarter` (`id`, `quest`, `VerifiedBuild`) VALUES
(99254, 41120, 0),
(99247, 40819, 0),
(103156, 41121, 0),
(94902, 39051, 0);

DELETE FROM `quest_template_addon` WHERE `ID` IN (41120, 40819, 41121, 39051);
INSERT INTO `quest_template_addon` (`ID`, `AllowableClasses`, `PrevQuestID`) VALUES
(41120, 2048, 40816),
(40819, 2048, 40814),
(41121, 2048, 41120),
(39051, 2048, 40819);

UPDATE `creature_template` SET `ScriptName`='npc_illidari_leader_violet_hold' WHERE `entry` IN (94902, 103156);
UPDATE `creature_template` SET `npcflag`=`npcflag`|1, `ScriptName`='npc_warden_alturas_dalaran' WHERE `entry`=96313;
UPDATE `creature_template` SET `ScriptName`='npc_warden_alturas_violet_hold' WHERE `entry`=99473;
UPDATE `creature_template` SET `ScriptName`='npc_taldath_the_destroyer' WHERE `entry`=94900;
-- The template's health modifier of 30 comes to 20 million health, for a player alone: 4 is our guess at a fight of a couple of minutes
UPDATE `creature_template_difficulty` SET `MinLevel`=98, `MaxLevel`=110, `LevelScalingMin`=98, `LevelScalingMax`=110, `HealthModifier`=4 WHERE `DifficultyID`=0 AND `Entry`=94900;

-- The sniffed menus lost their links: the threat leads on to his answer
UPDATE `gossip_menu_option` SET `ActionMenuID`=19304 WHERE `MenuID`=19303 AND `OptionID`=1;
DELETE FROM `creature_template_gossip` WHERE `CreatureID`=96313;
INSERT INTO `creature_template_gossip` (`CreatureID`, `MenuID`, `VerifiedBuild`) VALUES
(96313, 19303, 0);

-- Map 1494 "AcquisitionVioletHold", the hold of this one quest
DELETE FROM `instance_template` WHERE `map`=1494;
INSERT INTO `instance_template` (`map`, `parent`, `script`) VALUES
(1494, 0, '');

-- Altruis and Kayn stand on the same spot: a player only ever sees the leader they chose
DELETE FROM `creature` WHERE `guid` BETWEEN 12204101 AND 12204107;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `PhaseId`, `PhaseGroup`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `VerifiedBuild`) VALUES
(12204101, 103156, 1220, 7502, 7505, '0', 0, 0, 0, 0, -953.29, 4334.03, 740.18, 4.02, 300, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Altruis, outside the hold
(12204102, 94902, 1220, 7502, 7505, '0', 0, 0, 0, 0, -953.29, 4334.03, 740.18, 4.02, 300, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Kayn, outside the hold
(12204103, 96313, 1220, 7502, 7505, '0', 0, 0, 0, 0, -958.00, 4329.00, 740.16, 0.89, 300, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Warden Alturas, at the door
(12204104, 99473, 1494, 0, 0, '12', 0, 0, 0, 0, 1300.00, -262.00, 44.28, 3.14, 86400, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Warden Alturas, inside
(12204105, 96308, 1494, 0, 0, '12', 0, 0, 0, 0, 1312.00, -270.00, 44.09, 3.14, 86400, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Kirin Tor Protector
(12204106, 96308, 1494, 0, 0, '12', 0, 0, 0, 0, 1312.00, -254.00, 44.10, 3.14, 86400, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Kirin Tor Protector
(12204107, 94900, 1494, 0, 0, '12', 0, 0, 0, 0, 1426.00, -264.00, 52.50, 3.14, 86400, 0, 0, 0, NULL, NULL, NULL, NULL, 0); -- Taldath

-- Safe location 5374 is the client's for this map: dying in the hold sends the player back to its door
DELETE FROM `graveyard_zone` WHERE `ID`=5374 AND `GhostZone`=7777;
INSERT INTO `graveyard_zone` (`ID`, `GhostZone`, `Comment`) VALUES
(5374, 7777, 'The Violet Hold (By Any Means) - at its door in Dalaran');

DELETE FROM `creature_text` WHERE `CreatureID` IN (96313, 99473, 94900, 94902, 103156, 96308);
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundPlayType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(96313, 0, 0, 'If you don''t have time to explain, then I don''t have time to waste on you.', 12, 0, 100, 0, 0, 0, 0, 105840, 0, 'Warden Alturas: refuses'),
(96313, 1, 0, 'You''re fortunate that Khadgar trusts you. But make no mistake... I do not share his sentiment.', 12, 0, 100, 0, 0, 0, 0, 105851, 0, 'Warden Alturas: gives in'),
(96313, 2, 0, 'Fair enough. Let''s get this over with, then.', 12, 0, 100, 0, 0, 0, 0, 105852, 0, 'Warden Alturas: gives in'),
(96313, 3, 0, 'You may enter the hold, but I will not put the lives of my guards at risk for your mission. Is that clear?', 12, 0, 100, 0, 0, 0, 0, 97848, 0, 'Warden Alturas: terms'),
(96313, 4, 0, 'Very well. Do not make me regret trusting you.', 12, 0, 100, 0, 0, 0, 0, 97849, 0, 'Warden Alturas: lets them in'),
(99473, 0, 0, 'Lower the barrier and evacuate the hold. These Illidari wish to... exchange words... with the eredar.', 12, 0, 100, 0, 0, 0, 0, 97851, 0, 'Warden Alturas: evacuate'),
(99473, 1, 0, 'Follow me. I shall open the cell for you.', 12, 0, 100, 0, 0, 0, 0, 97859, 0, 'Warden Alturas: leads the way'),
(99473, 2, 0, 'Remember, demon hunter... you were never here.', 12, 0, 100, 0, 0, 0, 0, 97857, 0, 'Warden Alturas: at the cell'),
(96308, 0, 0, 'Kirin Tor Protector bows its head slightly in agreement.', 16, 0, 100, 0, 0, 0, 0, 97852, 0, 'Kirin Tor Protector: obeys'),
(94900, 0, 0, '$p?! You would stand before me after so many of your kin have fallen? Release me now and I may yet spare you.', 12, 0, 100, 0, 0, 0, 0, 97867, 0, 'Taldath: cell opened'),
(94900, 1, 0, 'Insolent whelps! Release me now and I may yet spare you.', 42, 0, 100, 0, 0, 0, 0, 96564, 0, 'Taldath: cell opened, portrait'),
(94900, 2, 0, 'FOOL! You dare face me alone?!', 14, 0, 100, 0, 0, 0, 0, 96565, 0, 'Taldath: aggro'),
(94900, 3, 0, 'Vermin! My lord wields power beyond your comprehension! He will end you!', 12, 0, 100, 0, 0, 0, 0, 96568, 0, 'Taldath: first answer'),
(94900, 4, 0, 'Your efforts mean NOTHING! Soon all of Suramar will bow to my master''s will!', 12, 0, 100, 0, 0, 0, 0, 105868, 0, 'Taldath: gives Suramar away'),
(94900, 5, 0, 'Varedis will hunt down every one of your disgusting ilk. Pity you won''t be around to witness it.', 12, 0, 100, 0, 0, 0, 0, 96566, 0, 'Taldath: gives Varedis away'),
(94900, 6, 0, 'The Twinblades... will be your end...', 12, 0, 100, 0, 0, 0, 0, 96567, 0, 'Taldath: death'),
(94900, 7, 0, 'The Twinblades... will destroy you...', 42, 0, 100, 0, 0, 0, 0, 97895, 0, 'Taldath: death, portrait'),
(94902, 0, 0, 'All we want is a few moments with the eredar... alone. He has information we need.', 12, 0, 100, 0, 0, 0, 0, 105849, 0, 'Kayn: at the door'),
(94902, 1, 0, 'As crystal, warden.', 12, 0, 100, 0, 0, 0, 0, 97850, 0, 'Kayn: at the door'),
(94902, 2, 0, 'After you, $p.', 12, 0, 100, 0, 0, 0, 0, 98167, 0, 'Kayn: inside'),
(94902, 3, 0, 'If you would do the honors, I will stand guard to ensure no one interferes.', 12, 0, 100, 0, 0, 0, 0, 97277, 0, 'Kayn: at the cell'),
(94902, 4, 0, 'Where is he?! Where is Varedis? Tell us or die.', 12, 0, 100, 0, 0, 0, 0, 97896, 0, 'Kayn: the fight'),
(94902, 5, 0, 'We have what we came for. Finish him!', 12, 0, 100, 0, 0, 0, 0, 97279, 0, 'Kayn: the fight'),
(94902, 6, 0, '$p, I will meet you at Krasus'' Landing and prepare a fel bat to take you to Suramar. Let the hunt begin!', 12, 0, 100, 0, 0, 0, 0, 97899, 0, 'Kayn: Taldath is dead'),
(94902, 7, 0, 'Meet Taldath, one of the most powerful eredar we''ve ever encountered. I''m sure you two will be fast friends.', 12, 0, 100, 0, 0, 0, 0, 97278, 0, 'Kayn: at the cell'),
(94902, 8, 0, 'Do not hold back, $n. Show him your wrath.', 42, 0, 100, 0, 0, 0, 0, 96560, 0, 'Kayn: the fight, portrait'),
(94902, 9, 0, 'Speak, demon. Who dares hunt the Illidari?', 42, 0, 100, 0, 0, 0, 0, 97276, 0, 'Kayn: the fight, portrait'),
(94902, 10, 0, 'Who is your master? Where is he hiding? SPEAK, DAMN YOU!', 42, 0, 100, 0, 0, 0, 0, 97280, 0, 'Kayn: the fight, portrait'),
(94902, 11, 0, 'Well done. The fool told us everything.', 42, 0, 100, 0, 0, 0, 0, 97281, 0, 'Kayn: the fight, portrait'),
(94902, 12, 0, 'The Twinblades of the Deceiver could be exactly what we need to strike down the most powerful Legion lords.', 12, 0, 100, 0, 0, 0, 0, 97897, 0, 'Kayn: Taldath is dead'),
(94902, 13, 0, 'Lord Illidan taught us to fight fire with fire, to use our enemies'' power against them. I believe we''ve found our fire.', 12, 0, 100, 0, 0, 0, 0, 97898, 0, 'Kayn: Taldath is dead'),
(103156, 0, 0, 'The eredar you''re holding possesses critical information. We need a few minutes alone with him.', 12, 0, 100, 0, 0, 0, 0, 105847, 0, 'Altruis: at the door'),
(103156, 1, 0, 'Of course.', 12, 0, 100, 0, 0, 0, 0, 105854, 0, 'Altruis: at the door'),
(103156, 3, 0, 'I will ensure no one interferes. The demon is all yours.', 12, 0, 100, 0, 0, 0, 0, 105875, 0, 'Altruis: at the cell'),
(103156, 4, 0, 'Do not test our patience. Tell us where your master is... or die!', 12, 0, 100, 0, 0, 0, 0, 105876, 0, 'Altruis: the fight'),
(103156, 5, 0, 'That''s all we needed. Finish him off.', 12, 0, 100, 0, 0, 0, 0, 105877, 0, 'Altruis: the fight'),
(103156, 6, 0, 'I will prepare a fel bat at Krasus'' Landing to take you to Suramar. Varedis'' time has come.', 12, 0, 100, 0, 0, 0, 0, 105880, 0, 'Altruis: Taldath is dead');

-- The door: a player whom Alturas has let in goes to this hold, everybody else to the dungeon through trigger 114
DELETE FROM `areatrigger_template` WHERE `Id`=1494001 AND `IsCustom`=1;
INSERT INTO `areatrigger_template` (`Id`, `IsCustom`, `Flags`, `ActionSetId`, `ActionSetFlags`, `VerifiedBuild`) VALUES
(1494001, 1, 1, 0, 0, 0);
DELETE FROM `areatrigger_template_actions` WHERE `AreaTriggerId`=1494001 AND `IsCustom`=1;
INSERT INTO `areatrigger_template_actions` (`AreaTriggerId`, `IsCustom`, `ActionType`, `ActionParam`, `TargetType`) VALUES
(1494001, 1, 0, 188163, 5);
DELETE FROM `areatrigger_create_properties` WHERE `Id`=1494001 AND `IsCustom`=1;
INSERT INTO `areatrigger_create_properties` (`Id`, `IsCustom`, `AreaTriggerId`, `IsAreatriggerCustom`, `Flags`, `MoveCurveId`, `ScaleCurveId`, `MorphCurveId`, `FacingCurveId`, `AnimId`, `AnimKitId`, `DecalPropertiesId`, `SpellForVisuals`, `TimeToTargetScale`, `Speed`, `Shape`, `ShapeData0`, `ShapeData1`, `ShapeData2`, `ShapeData3`, `ShapeData4`, `ShapeData5`, `ScriptName`, `VerifiedBuild`) VALUES
(1494001, 1, 1494001, 1, 0, 0, 0, 0, 0, -1, 0, 0, NULL, 0, 1, 0, 5, 5, 0, 0, 0, 0, '', 0);
DELETE FROM `areatrigger` WHERE `SpawnId`=1494001;
INSERT INTO `areatrigger` (`SpawnId`, `AreaTriggerCreatePropertiesId`, `IsCustom`, `MapId`, `SpawnDifficulties`, `PosX`, `PosY`, `PosZ`, `Orientation`, `PhaseUseFlags`, `PhaseId`, `PhaseGroup`, `ScriptName`, `Comment`, `VerifiedBuild`) VALUES
(1494001, 1494001, 1, 1220, '0', -968.47, 4315.12, 739.92, 0, 0, 0, 0, '', 'Dalaran - the Violet Hold door - into the hold of By Any Means', 0);

DELETE FROM `spell_script_names` WHERE `ScriptName`='spell_dh_by_any_means_teleport';
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(188163, 'spell_dh_by_any_means_teleport');

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=19 AND `SourceEntry` IN (41120, 40819, 41121, 39051);
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=32 AND `SourceGroup`=3 AND `SourceEntry` IN (103156, 94902, 96313);
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=15 AND `SourceGroup`=19303;
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=28 AND `SourceGroup` IN (114, 1494001) AND `SourceEntry`=1;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(19, 0, 41120, 0, 0, 47, 0, 40375, 64, 0, 0, 0, 0, '', 'Making Arrangements (Altruis): Altruis chosen'),
(19, 0, 41121, 0, 0, 47, 0, 40375, 64, 0, 0, 0, 0, '', 'By Any Means (Altruis): Altruis chosen'),
(19, 0, 40819, 0, 0, 47, 0, 40374, 64, 0, 0, 0, 0, '', 'Making Arrangements (Kayn): Kayn chosen'),
(19, 0, 39051, 0, 0, 47, 0, 40374, 64, 0, 0, 0, 0, '', 'By Any Means (Kayn): Kayn chosen'),
(32, 3, 103156, 0, 0, 47, 0, 40375, 64, 0, 0, 0, 0, '', 'Altruis (the Violet Hold): Altruis chosen'),
(32, 3, 103156, 0, 0, 15, 0, 2048, 0, 0, 0, 0, 0, '', 'Altruis (the Violet Hold): demon hunters only'),
(32, 3, 94902, 0, 0, 47, 0, 40374, 64, 0, 0, 0, 0, '', 'Kayn (the Violet Hold): Kayn chosen'),
(32, 3, 94902, 0, 0, 15, 0, 2048, 0, 0, 0, 0, 0, '', 'Kayn (the Violet Hold): demon hunters only'),
(32, 3, 96313, 0, 0, 47, 0, 41121, 10, 0, 0, 0, 0, '', 'Warden Alturas (the door): By Any Means (Altruis) in the log'),
(32, 3, 96313, 0, 1, 47, 0, 39051, 10, 0, 0, 0, 0, '', 'Warden Alturas (the door): By Any Means (Kayn) in the log'),
(15, 19303, 0, 0, 0, 48, 0, 281907, 0, 0, 0, 0, 0, '', 'No time to explain: not yet convinced (Altruis)'),
(15, 19303, 0, 0, 1, 48, 0, 281908, 0, 0, 0, 0, 0, '', 'No time to explain: not yet convinced (Kayn)'),
(15, 19303, 1, 0, 0, 48, 0, 281907, 0, 0, 0, 0, 0, '', 'Let us inside: not yet convinced (Altruis)'),
(15, 19303, 1, 0, 1, 48, 0, 281908, 0, 0, 0, 0, 0, '', 'Let us inside: not yet convinced (Kayn)'),
(28, 1494001, 1, 0, 0, 48, 0, 281907, 0, 1, 0, 0, 0, '', 'The hold of By Any Means: Alturas convinced (Altruis)'),
(28, 1494001, 1, 0, 1, 48, 0, 281908, 0, 1, 0, 0, 0, '', 'The hold of By Any Means: Alturas convinced (Kayn)'),
(28, 114, 1, 0, 0, 48, 0, 281907, 0, 1, 1, 0, 0, '', 'The dungeon: not for a player Alturas has let into the hold (Altruis)'),
(28, 114, 1, 0, 0, 48, 0, 281908, 0, 1, 1, 0, 0, '', 'The dungeon: not for a player Alturas has let into the hold (Kayn)');
