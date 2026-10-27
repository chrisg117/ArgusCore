-- Illidari, We Are Leaving (39689 Alliance / 39690 Horde), after Between Us and Freedom, had no one to give it, and the
-- Alliance version's teleport and hearth bind had nowhere to go. As in the retail video (youtu.be/asGMB9e9A3Y):
-- - Archmage Khadgar, who arrives outside the Vault's exit, gives and ends it (Wowhead), once Between Us and Freedom is
--   turned in.
-- - Turning it in casts Teleport to Stormwind / Orgrimmar (the quest's reward spell), which also binds the hearthstone and
--   switches to the primary specialization. The Horde destinations are sniffed; the Alliance ones mirror them: in front
--   of Anduin Wrynn in Stormwind Keep, facing him (the spell's own facing, 5.35, is his facing turned round), and the
--   Gilded Rose inn in the Trade District, as the Horde bind is Orgrimmar's Valley of Strength inn.

DELETE FROM `creature_queststarter` WHERE `quest` IN (39689, 39690);
INSERT INTO `creature_queststarter` (`id`, `quest`, `VerifiedBuild`) VALUES
(97978, 39689, 0),
(97978, 39690, 0);

DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 19 AND `SourceEntry` IN (39689, 39690);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `ConditionStringValue1`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(19, 0, 39689, 0, 0, 8, 0, 39688, 0, 0, '', 0, 0, 0, '', 'Illidari, We Are Leaving (A): Between Us and Freedom (Havoc) done'),
(19, 0, 39689, 0, 1, 8, 0, 40255, 0, 0, '', 0, 0, 0, '', 'Illidari, We Are Leaving (A): or Between Us and Freedom (Vengeance) done'),
(19, 0, 39690, 0, 0, 8, 0, 39694, 0, 0, '', 0, 0, 0, '', 'Illidari, We Are Leaving (H): Between Us and Freedom (Havoc) done'),
(19, 0, 39690, 0, 1, 8, 0, 40256, 0, 0, '', 0, 0, 0, '', 'Illidari, We Are Leaving (H): or Between Us and Freedom (Vengeance) done');

DELETE FROM `spell_target_position` WHERE `ID` IN (192757, 197364);
INSERT INTO `spell_target_position` (`ID`, `EffectIndex`, `OrderIndex`, `MapID`, `PositionX`, `PositionY`, `PositionZ`, `Orientation`, `VerifiedBuild`) VALUES
(192757, 0, 0, 0, -8367.06, 237.22, 157.07, NULL, 0), -- Teleport to Stormwind: Stormwind Keep, before Anduin
(197364, 0, 0, 0, -8866.38, 671.02, 97.99, NULL, 0);  -- Bind to Stormwind: the Gilded Rose
