-- The enders offer what follows their zone quest: RewardNextQuest is only offered by a giver that has it in its quest list.
DELETE FROM `creature_queststarter` WHERE (`id`,`quest`) IN ((86563,41220),(91172,39861),(96644,38035),(96683,38307),(97666,38907));
INSERT INTO `creature_queststarter` (`id`, `quest`, `VerifiedBuild`) VALUES
(86563, 41220, 0), -- Archmage Khadgar: Down to Azsuna
(91172, 39861, 0), -- Archmage Khadgar: Tying Up Loose Ends
(96644, 38035, 0), -- Sky Admiral Rogers: A Royal Summons
(96683, 38307, 0), -- Nathanos Blightcaller: The Warchief Beckons
(97666, 38907, 0); -- Warbrave Oro: Keepers of the Hammer
