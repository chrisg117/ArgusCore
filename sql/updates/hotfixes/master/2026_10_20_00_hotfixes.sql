-- Necrotic Venom (215449, 218831) and its damage aura (215460) showed on the player as buffs: the engine rates
-- a periodic trigger aimed at any target as positive, and a right-click cancelled the mark, skipping the payoff.
-- Hotfix override of the effect rows with the client's IsHarmful effect attribute (0x1000) set, so the auras
-- are debuffs. Every other column keeps the client file value.
DELETE FROM `spell_effect` WHERE `ID` IN (320188,320206,320207,325891,330844,330938,330939,344690,344691);
INSERT INTO `spell_effect` (`ID`, `Effect`, `EffectBasePoints`, `EffectIndex`, `EffectAura`, `DifficultyID`, `EffectAmplitude`, `EffectAuraPeriod`, `EffectBonusCoefficient`, `EffectChainAmplitude`, `EffectChainTargets`, `EffectDieSides`, `EffectItemType`, `EffectMechanic`, `EffectPointsPerResource`, `EffectRealPointsPerLevel`, `EffectTriggerSpell`, `EffectPosFacing`, `EffectAttributes`, `BonusCoefficientFromAP`, `PvpMultiplier`, `Coefficient`, `Variance`, `ResourceCoefficient`, `GroupSizeBasePointsCoefficient`, `EffectSpellClassMask1`, `EffectSpellClassMask2`, `EffectSpellClassMask3`, `EffectSpellClassMask4`, `EffectMiscValue1`, `EffectMiscValue2`, `EffectRadiusIndex1`, `EffectRadiusIndex2`, `ImplicitTarget1`, `ImplicitTarget2`, `SpellID`, `VerifiedBuild`) VALUES
(320188, 6, 215460, 0, 23, 0, 0.0, 250, 0.0, 1.0, 0, 0, 0, 0, 0.0, 0.0, 215451, 0.0, 4096, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0, 0, 0, 0, 0, 0, 0, 0, 25, 0, 215449, 26972),
(320206, 6, 185249, 0, 23, 0, 0.0, 1000, 0.0, 1.0, 0, 19501, 0, 0, 0.0, 0.0, 233434, 0.0, 4096, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 215460, 26972),
(320207, 6, 0, 1, 23, 0, 0.0, 1000, 0.0, 1.0, 0, 0, 0, 0, 0.0, 0.0, 215459, 0.0, 4097, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 215460, 26972),
(325891, 6, 215464, 0, 23, 0, 0.0, 250, 0.0, 1.0, 0, 0, 0, 0, 0.0, 0.0, 215451, 0.0, 4096, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0, 0, 0, 0, 0, 0, 0, 0, 25, 0, 218831, 26972),
(330844, 6, 0, 2, 23, 0, 0.0, 1000, 0.0, 1.0, 0, 0, 0, 0, 0.0, 0.0, 225519, 0.0, 4097, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 215460, 26972),
(330938, 6, 92624, 0, 23, 17, 0.0, 1000, 0.0, 1.0, 0, 9751, 0, 0, 0.0, 0.0, 233434, 0.0, 4096, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 215460, 26972),
(330939, 6, 250799, 0, 23, 16, 0.0, 1000, 0.0, 1.0, 0, 26401, 0, 0, 0.0, 0.0, 233434, 0.0, 4096, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 215460, 26972),
(344690, 6, 185249, 0, 23, 15, 0.0, 1000, 0.0, 1.0, 0, 19501, 0, 0, 0.0, 0.0, 233434, 0.0, 4096, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 215460, 26972),
(344691, 6, 195000, 0, 23, 14, 0.0, 1000, 0.0, 1.0, 0, 0, 0, 0, 0.0, 0.0, 233434, 0.0, 4096, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 215460, 26972);
