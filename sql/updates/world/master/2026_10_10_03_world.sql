-- The priest's first quests of the order campaign: "Priestly Matters" (40705), "A Legend You Can Hold" (40706)
-- and, for the shadow weapon, "Blade in Twilight" (40710).

-- The choice of the weapon had no script, so that choosing did nothing. The script was there already
-- (zone_tirisfal_glades.cpp): it casts the spell of the chosen weapon, which gives the quest's credit and
-- completes the hidden quest that remembers the choice (40707 shadow, 40708 holy, 40709 discipline).
UPDATE `playerchoice` SET `ScriptName`='playerchoice_a_weapon_you_can_hold_priest' WHERE `ChoiceId`=248;

-- Alonsus Faol at Faol's Rest gives "Blade in Twilight", which had no giver
DELETE FROM `creature_queststarter` WHERE `id`=101314 AND `quest`=40710;
INSERT INTO `creature_queststarter` (`id`, `quest`, `VerifiedBuild`) VALUES
(101314, 40710, 0);

-- The three are for priests, and "Blade in Twilight" follows "A Legend You Can Hold"
DELETE FROM `quest_template_addon` WHERE `ID` IN (40705, 40710);
INSERT INTO `quest_template_addon` (`ID`, `AllowableClasses`, `PrevQuestID`) VALUES
(40705, 16, 0),
(40710, 16, 40706);

UPDATE `quest_template_addon` SET `AllowableClasses`=16 WHERE `ID`=40706;

-- and is offered to those who chose the shadow weapon
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=19 AND `SourceGroup`=0 AND `SourceEntry`=40710;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `Comment`) VALUES
(19, 0, 40710, 0, 0, 8, 0, 40707, 0, 0, 0, 'Blade in Twilight (40710) is available if A Legend You Can Hold: Choose Shadow weapon (40707) is rewarded');
