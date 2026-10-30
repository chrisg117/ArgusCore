-- Actions on Azeroth: the end of the priest campaign is the zone choice on the Command Map.
-- The map is the client's adventure map, and AdventureMapPOI.db2 offers each zone as a quest. Each of those quests
-- is flagged to cast a spell on accept, and the zone's Chosen spell is the one that credits 41019.
DELETE FROM `creature_queststarter` WHERE `id`=102655 AND `quest`=41019;
INSERT INTO `creature_queststarter` (`id`, `quest`, `VerifiedBuild`) VALUES
(102655, 41019, 0);

DELETE FROM `quest_template_addon` WHERE `ID` IN (41019, 39718, 39731, 39735, 39864, 39733);
INSERT INTO `quest_template_addon` (`ID`, `AllowableClasses`, `PrevQuestID`, `SourceSpellID`) VALUES
(41019, 16, 41017, 0),
(39718, 0, 0, 191808), -- Paradise Lost: Azsuna Chosen
(39731, 0, 0, 191809), -- The Tranquil Forest: Val'Sharah Chosen
(39735, 0, 0, 191812), -- Stormheim (Alliance): Stormheim Chosen
(39864, 0, 0, 191812), -- Stormheim (Horde): Stormheim Chosen
(39733, 0, 0, 191813); -- The Lone Mountain: Highmountain Chosen

-- A priest who already took a zone quest has nothing left to pick on the map, and this spell is that credit.
UPDATE `creature_template` SET `AIName`='SmartAI' WHERE `entry`=102655;
DELETE FROM `smart_scripts` WHERE `entryorguid`=102655 AND `source_type`=0;
INSERT INTO `smart_scripts` (`entryorguid`, `source_type`, `id`, `link`, `event_type`, `event_param1`, `action_type`, `action_param1`, `action_param2`, `target_type`, `comment`) VALUES
(102655, 0, 0, 0, 19, 41019, 85, 215269, 2, 7, 'Alonsus Faol - On Actions on Azeroth accepted - Invoker casts Zone Breadcrumb Already Obtained');

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=22 AND `SourceGroup`=1 AND `SourceEntry`=102655 AND `SourceId`=0;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `Comment`) VALUES
(22, 1, 102655, 0, 0, 47, 0, 39718, 74, 'Paradise Lost taken, complete or rewarded'),
(22, 1, 102655, 0, 1, 47, 0, 39731, 74, 'The Tranquil Forest taken, complete or rewarded'),
(22, 1, 102655, 0, 2, 47, 0, 39735, 74, 'Stormheim (Alliance) taken, complete or rewarded'),
(22, 1, 102655, 0, 3, 47, 0, 39864, 74, 'Stormheim (Horde) taken, complete or rewarded'),
(22, 1, 102655, 0, 4, 47, 0, 39733, 74, 'The Lone Mountain taken, complete or rewarded');
