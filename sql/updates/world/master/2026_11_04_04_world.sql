-- Azsuna, Faronaar: Kor'vas Bloodthorn follows the player through Fel Machinations (37656) and Saving Stellagosa (37450)
-- and fights beside her; Overseer Lykill (86535) gets his lines and two abilities; the first Fel Lock (239455) gives the
-- hidden "First chain unlocked" credit (105635). The scripts are in zone_azsuna.cpp.

-- 90624, an unspawned Kor'vas template (display 66159, scaling 98..110), is the summoned companion
UPDATE `creature_template` SET `ScriptName` = 'npc_faronaar_korvas' WHERE `entry` = 90624;
-- its level was 1 (no other row uses it); 110 like the spawned Kor'vas 90255, so the 98..110 scaling applies
UPDATE `creature_template_difficulty` SET `MinLevel` = 110, `MaxLevel` = 110 WHERE `Entry` = 90624 AND `DifficultyID` = 0;
-- her warglaives, as her other Kor'vas Bloodthorn entries (98292 and on)
DELETE FROM `creature_equip_template` WHERE `CreatureID` = 90624;
INSERT INTO `creature_equip_template` (`CreatureID`, `ID`, `ItemID1`, `AppearanceModID1`, `ItemVisual1`, `ItemID2`, `AppearanceModID2`, `ItemVisual2`, `ItemID3`, `AppearanceModID3`, `ItemVisual3`, `VerifiedBuild`) VALUES
(90624, 1, 128359, 0, 0, 128371, 0, 0, 0, 0, 0, 0);

UPDATE `creature_template` SET `ScriptName` = 'npc_overseer_lykill' WHERE `entry` = 86535;
UPDATE `gameobject_template` SET `ScriptName` = 'go_saving_stellagosa_fel_lock' WHERE `entry` = 239455;

-- on retail Saving Stellagosa and Fel Machinations are turned in to the Kor'vas following you, and Dark Revelations is
-- taken from her (WoW-Pro 7.3.5 Azsuna, near Traitor's Overlook); the camp 90474 and Faronaar 90255 keep their rows
UPDATE `creature_template` SET `npcflag` = `npcflag` | 2 WHERE `entry` = 90624;
DELETE FROM `creature_questender` WHERE `id` = 90624 AND `quest` IN (37450, 37656);
INSERT INTO `creature_questender` (`id`, `quest`, `VerifiedBuild`) VALUES
(90624, 37450, 0),
(90624, 37656, 0);
DELETE FROM `creature_queststarter` WHERE `id` = 90624 AND `quest` = 37449;
INSERT INTO `creature_queststarter` (`id`, `quest`, `VerifiedBuild`) VALUES
(90624, 37449, 0);

-- between the two quests the follower offers Saving Stellagosa herself
DELETE FROM `creature_queststarter` WHERE `id` = 90624 AND `quest` = 37450;
INSERT INTO `creature_queststarter` (`id`, `quest`, `VerifiedBuild`) VALUES
(90624, 37450, 0);

-- one Kor'vas at a time: the static one in Faronaar (12201032, phase 4356) shows only once the follower is gone, i.e.
-- Fel Machinations rewarded and Dark Revelations taken; before, she was shown from the moment 37656 was taken, next to
-- the follower. Vanquish Point (area 7356) keeps its rows: she is hidden there once 37656 is taken.
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 26 AND `SourceGroup` = 4356 AND `SourceEntry` IN (7349, 7354);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(26, 4356, 7349, 0, 0, 47, 0, 37656, 64, 0, 0, 0, 0, '', 'Apply Phase 4356 in Faronaar once Quest 37656 is rewarded'),
(26, 4356, 7349, 0, 0, 47, 0, 37449, 106, 0, 0, 0, 0, '', 'and Quest 37449 is taken (the follower Kor''vas has left)'),
(26, 4356, 7354, 0, 0, 47, 0, 37656, 64, 0, 0, 0, 0, '', 'Apply Phase 4356 in Faronaar once Quest 37656 is rewarded'),
(26, 4356, 7354, 0, 0, 47, 0, 37449, 106, 0, 0, 0, 0, '', 'and Quest 37449 is taken (the follower Kor''vas has left)');
