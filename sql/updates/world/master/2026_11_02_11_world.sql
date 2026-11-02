-- One Battlelord Gaardoun at a time: at the hub until he leads the way (203676's aura), then the one who walks, then the one at the
-- spoils until The Cursed Forge is done, then the hub's again as the recruiter. State masks: 1 none, 2 complete, 8 in progress, 64 rewarded.
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=32 AND `SourceGroup`=3 AND `SourceEntry` IN (103068, 103025);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `Comment`) VALUES
(32, 3, 103068, 0, 0, 47, 0, 41037, 66, 0, 0, 'Gaardoun at the hub: once the Fel Hammer is active (Kayn)'),
(32, 3, 103068, 0, 0, 47, 0, 41062, 9, 0, 0, 'Gaardoun at the hub: until he has shown the spoils (Kayn)'),
(32, 3, 103068, 0, 0, 1, 0, 203676, 2, 0, 1, 'Gaardoun at the hub: not while he leads the way (Kayn)'),
(32, 3, 103068, 0, 1, 47, 0, 41060, 66, 0, 0, 'Gaardoun at the hub: once the Fel Hammer is active (Altruis)'),
(32, 3, 103068, 0, 1, 47, 0, 41070, 9, 0, 0, 'Gaardoun at the hub: until he has shown the spoils (Altruis)'),
(32, 3, 103068, 0, 1, 1, 0, 203676, 2, 0, 1, 'Gaardoun at the hub: not while he leads the way (Altruis)'),
(32, 3, 103068, 0, 2, 47, 0, 41064, 64, 0, 0, 'Gaardoun at the hub: the recruiter, after The Cursed Forge'),
(32, 3, 103025, 0, 0, 47, 0, 41062, 66, 0, 0, 'Gaardoun below: once he has shown the spoils (Kayn)'),
(32, 3, 103025, 0, 0, 47, 0, 41064, 64, 0, 1, 'Gaardoun below: until The Cursed Forge is done (Kayn)'),
(32, 3, 103025, 0, 1, 47, 0, 41070, 66, 0, 0, 'Gaardoun below: once he has shown the spoils (Altruis)'),
(32, 3, 103025, 0, 1, 47, 0, 41064, 64, 0, 1, 'Gaardoun below: until The Cursed Forge is done (Altruis)');
