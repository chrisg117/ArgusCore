-- Both Senegos showed at once: the phase of the healed one (4439) was also true for everyone who still sees him in pain (4438).
-- He shows once The Nightborne Prince is rewarded, where the one in pain leaves. Until then the one in pain gives Those Who Remember, at its start POI.
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=26 AND `SourceGroup`=4439 AND `SourceEntry`=7334 AND `ElseGroup`=0 AND `ConditionValue1`=37861;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `Comment`) VALUES
(26, 4439, 7334, 0, 0, 47, 0, 37861, 64, 0, 0, 'Apply Phase 4439 if Quest 37861 is rewarded');

DELETE FROM `creature_queststarter` WHERE `id`=89975 AND `quest`=37690;
INSERT INTO `creature_queststarter` (`id`, `quest`, `VerifiedBuild`) VALUES
(89975, 37690, 0);
