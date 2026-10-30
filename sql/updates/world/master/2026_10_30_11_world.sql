-- Illidari Stand: starters taken from the quests' start POIs (quest_poi ObjectiveIndex 32), which land on these NPCs.
-- Into the Fray 44137 and From Within 40815/44140 stay without a starter: nothing in the data separates them from 38834
-- and 36920, and the phases test those two.
DELETE FROM `creature_queststarter` WHERE (`id`=89362 AND `quest`=36920) OR (`id`=89398 AND `quest` IN (37653,37660)) OR (`id`=90317 AND `quest`=37658);
INSERT INTO `creature_queststarter` (`id`, `quest`, `VerifiedBuild`) VALUES
(89362, 36920, 0), -- Kayn Sunfury: From Within
(89398, 37653, 0), -- Allari the Souleater: Demon Souls
(89398, 37660, 0), -- Allari the Souleater: The Scythe of Souls
(90317, 37658, 0); -- Jace Darkweaver: Reignite the Wards

INSERT INTO `quest_template_addon` (`ID`, `PrevQuestID`) VALUES
(38834, 41220),
(36920, 38834),
(37653, 38834),
(37658, 38834),
(37660, 37653)
ON DUPLICATE KEY UPDATE `PrevQuestID`=VALUES(`PrevQuestID`);
