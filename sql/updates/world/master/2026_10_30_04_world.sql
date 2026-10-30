-- The follow-ups have no prerequisite of their own, so each ender would offer its quest to anyone passing by.
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=19 AND `SourceEntry` IN (41220, 39861, 38035, 38307, 38907);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `Comment`) VALUES
(19, 0, 41220, 0, 0, 8, 0, 39718, 'Down to Azsuna needs Paradise Lost rewarded'),
(19, 0, 39861, 0, 0, 8, 0, 39731, 'Tying Up Loose Ends needs The Tranquil Forest rewarded'),
(19, 0, 38035, 0, 0, 8, 0, 39735, 'A Royal Summons needs Stormheim (Alliance) rewarded'),
(19, 0, 38307, 0, 0, 8, 0, 39864, 'The Warchief Beckons needs Stormheim (Horde) rewarded'),
(19, 0, 38907, 0, 0, 8, 0, 39733, 'Keepers of the Hammer needs The Lone Mountain rewarded');
