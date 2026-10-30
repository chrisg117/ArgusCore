-- The Fallen Lion (40517): nothing in the throne room could deliver Varian's letter. King Anduin (100429), who ends the
-- quest, had no spawn; nothing cast the credits for "Hear Leaders Mourning" or "Deliver Varian's letter and listen to
-- King Anduin"; and none of the quest's conversations existed. As in the retail video ("[Quest 40517] - The Fallen
-- Lion", Phoenix Gaming), whose chat log names each speaker and times each line:
-- - Taking the quest from Genn on the Skybreaker plays his send-off (lines 1853, 3947: custom conversation 1519002).
-- - Entering the throne room plays the court's mourning (conversation 926: Jaina announces the player, then Anduin,
--   Velen, Muradin, Falstad) and counts "Hear Leaders Mourning"; the letter is taken as the council begins 13 s later
--   (lines 1876-5015: custom conversation 1519001, Anduin, Velen, Tyrande, Jaina); its end counts the letter
--   (at_the_call_of_war_throne_room, conversation_the_fallen_lion_mourning, conversation_the_fallen_lion_council).
-- - The court (Anduin, Jaina, Velen, Tyrande, Malfurion, Muradin, Gelbin, Moira, Falstad) stands before the throne while
--   the quest is in the log; the throne's usual Anduin (107574) steps aside meanwhile. Positions are the Wowhead pins
--   converted with the client's Stormwind map bounds, heights measured in game, everyone facing Anduin.
-- The conversations' ids for the council and the send-off are not known (no spell names them), so they are custom.
-- Not built: the optional gryphon ride with Tess and Crowley (lines 2643-2645, 2650-2651), the mourners along the way.

-- The court, before the throne
DELETE FROM `creature` WHERE `guid` BETWEEN 15190020 AND 15190028;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `PhaseId`, `PhaseGroup`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `VerifiedBuild`) VALUES
(15190020, 100429, 0, 1519, 6292, '0', 0, 0, 0, 0, -8363.30, 232.60, 157.07, 2.25, 300, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Anduin Wrynn
(15190021, 100428, 0, 1519, 6292, '0', 0, 0, 0, 0, -8373.45, 235.57, 156.59, 5.99, 300, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Lady Jaina Proudmoore
(15190022, 100458, 0, 1519, 6292, '0', 0, 0, 0, 0, -8373.45, 242.52, 155.35, 5.51, 300, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Tyrande Whisperwind
(15190023, 100457, 0, 1519, 6292, '0', 0, 0, 0, 0, -8378.08, 242.52, 155.35, 5.69, 300, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Malfurion Stormrage
(15190024, 100460, 0, 1519, 6292, '0', 0, 0, 0, 0, -8368.82, 246.05, 155.35, 5.11, 300, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Prophet Velen
(15190025, 100454, 0, 1519, 6292, '0', 0, 0, 0, 0, -8368.82, 249.52, 155.35, 5.03, 300, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Muradin Bronzebeard
(15190026, 100471, 0, 1519, 6292, '0', 0, 0, 0, 0, -8366.50, 239.10, 155.87, 5.16, 300, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Gelbin Mekkatorque
(15190027, 100455, 0, 1519, 6292, '0', 0, 0, 0, 0, -8375.77, 253.00, 155.35, 5.27, 300, 0, 0, 0, NULL, NULL, NULL, NULL, 0), -- Moira Thaurissan
(15190028, 100456, 0, 1519, 6292, '0', 0, 0, 0, 0, -8371.14, 253.00, 155.35, 5.08, 300, 0, 0, 0, NULL, NULL, NULL, NULL, 0); -- Falstad Wildhammer

-- Who sees the court: while The Fallen Lion is in the log; the usual Anduin not meanwhile
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 32 AND `SourceGroup` = 3 AND `SourceEntry` IN (100429, 100428, 100458, 100457, 100460, 100454, 100471, 100455, 100456);
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 32 AND `SourceGroup` = 3 AND `SourceEntry` = 107574 AND `ConditionValue1` = 40517;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `ConditionStringValue1`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(32, 3, 100429, 0, 0, 47, 0, 40517, 10, 0, '', 0, 0, 0, '', 'Anduin (court): while The Fallen Lion is in the log'),
(32, 3, 100428, 0, 0, 47, 0, 40517, 10, 0, '', 0, 0, 0, '', 'Jaina (court): while The Fallen Lion is in the log'),
(32, 3, 100458, 0, 0, 47, 0, 40517, 10, 0, '', 0, 0, 0, '', 'Tyrande (court): while The Fallen Lion is in the log'),
(32, 3, 100457, 0, 0, 47, 0, 40517, 10, 0, '', 0, 0, 0, '', 'Malfurion (court): while The Fallen Lion is in the log'),
(32, 3, 100460, 0, 0, 47, 0, 40517, 10, 0, '', 0, 0, 0, '', 'Velen (court): while The Fallen Lion is in the log'),
(32, 3, 100454, 0, 0, 47, 0, 40517, 10, 0, '', 0, 0, 0, '', 'Muradin (court): while The Fallen Lion is in the log'),
(32, 3, 100471, 0, 0, 47, 0, 40517, 10, 0, '', 0, 0, 0, '', 'Gelbin (court): while The Fallen Lion is in the log'),
(32, 3, 100455, 0, 0, 47, 0, 40517, 10, 0, '', 0, 0, 0, '', 'Moira (court): while The Fallen Lion is in the log'),
(32, 3, 100456, 0, 0, 47, 0, 40517, 10, 0, '', 0, 0, 0, '', 'Falstad (court): while The Fallen Lion is in the log'),
(32, 3, 107574, 0, 0, 47, 0, 40517, 10, 0, '', 1, 0, 0, '', 'Anduin (usual): not while the court is there');

-- Genn's send-off on the Skybreaker
DELETE FROM `conversation_template` WHERE `Id` IN (926, 1519001, 1519002);
INSERT INTO `conversation_template` (`Id`, `FirstLineId`, `LastLineEndTime`, `Flags`, `ScriptName`, `VerifiedBuild`) VALUES
(1519002, 1853, 15000, 0, '', 0),
(926, 2301, 16000, 0, 'conversation_the_fallen_lion_mourning', 0),
(1519001, 1876, 93000, 0, 'conversation_the_fallen_lion_council', 0);

DELETE FROM `conversation_actors` WHERE `ConversationId` IN (926, 1519001, 1519002);
INSERT INTO `conversation_actors` (`ConversationId`, `ConversationActorId`, `ConversationActorGuid`, `Idx`, `CreatureId`, `CreatureDisplayInfoId`, `NoActorObject`, `ActivePlayerObject`, `VerifiedBuild`) VALUES
(1519002, 0, 14600900, 0, 0, 0, 0, 0, 0), -- Genn Greymane (Skybreaker)
(926, 0, 15190021, 0, 0, 0, 0, 0, 0), -- Lady Jaina Proudmoore
(926, 0, 15190020, 1, 0, 0, 0, 0, 0), -- Anduin Wrynn
(926, 0, 15190024, 2, 0, 0, 0, 0, 0), -- Prophet Velen
(926, 0, 15190025, 3, 0, 0, 0, 0, 0), -- Muradin Bronzebeard
(926, 0, 15190028, 4, 0, 0, 0, 0, 0), -- Falstad Wildhammer
(1519001, 0, 15190020, 0, 0, 0, 0, 0, 0), -- Anduin Wrynn
(1519001, 0, 15190024, 1, 0, 0, 0, 0, 0), -- Prophet Velen
(1519001, 0, 15190022, 2, 0, 0, 0, 0, 0), -- Tyrande Whisperwind
(1519001, 0, 15190021, 3, 0, 0, 0, 0, 0); -- Lady Jaina Proudmoore

DELETE FROM `conversation_line_template` WHERE `Id` IN (1853, 3947, 2301, 6555, 2300, 2298, 2299, 1876, 1877, 1883, 1878, 1879, 1880, 1881, 1884, 1885, 1968, 1969, 5014, 5015);
INSERT INTO `conversation_line_template` (`Id`, `StartTime`, `UiCameraID`, `ActorIdx`, `Flags`, `VerifiedBuild`) VALUES
(1853, 0, 0, 0, 0, 0), -- Genn: Tess and Commander Crowley will escort you to the Keep...
(3947, 10000, 0, 0, 0, 0), -- Genn: When you deliver the letter, please give Prin... King Anduin my condolences.
(2301, 0, 0, 0, 0, 0), -- Jaina: King Anduin, the hero of the Broken Shore approaches.
(6555, 3000, 0, 1, 0, 0), -- Anduin: At last, some good news on this day of sorrow.
(2300, 5000, 0, 2, 0, 0), -- Velen: His valor inspired us all, even in death...
(2298, 10000, 0, 3, 0, 0), -- Muradin: I can't believe he's really gone.
(2299, 12000, 0, 4, 0, 0), -- Falstad: He died with honor, protecting his people. He was a true king.
(1876, 0, 0, 0, 0, 0), -- Anduin: My father was a seasoned strategist...
(1877, 8000, 0, 1, 0, 0), -- Velen: You are not alone in this, My King. We stand behind you.
(1883, 13000, 0, 2, 0, 0), -- Tyrande: And with the return of the Illidari...
(1878, 22000, 0, 3, 0, 0), -- Jaina: Have you forgotten the Horde's treachery?...
(1879, 29000, 0, 1, 0, 0), -- Velen: To fight two wars at once would be folly...
(1880, 36000, 0, 0, 0, 0), -- Anduin: Velen is right. We must stand beside the demon hunters...
(1881, 43000, 0, 3, 0, 0), -- Jaina: The Horde betrayed your father and left us to die!...
(1884, 51000, 0, 1, 0, 0), -- Velen: My people know the price of facing the demons as a world divided...
(1885, 59000, 0, 0, 0, 0), -- Anduin: Agreed. I'm sorry, Jaina, but vengeance must wait...
(1968, 67000, 0, 3, 0, 0), -- Jaina: Dalaran will help protect the Eastern Kingdoms...
(1969, 74000, 0, 0, 0, 0), -- Anduin: We must not splinter like this... not now. Please, Jaina!
(5014, 79000, 0, 3, 0, 0), -- Jaina: You disappoint me, King of Stormwind...
(5015, 87000, 0, 0, 0, 0); -- Anduin: My father dead... disaster on the Broken Shore...

-- The send-off plays when the quest is taken
INSERT INTO `quest_template_addon` (`ID`, `ScriptName`) VALUES (40517, 'quest_the_fallen_lion')
ON DUPLICATE KEY UPDATE `ScriptName` = 'quest_the_fallen_lion';
