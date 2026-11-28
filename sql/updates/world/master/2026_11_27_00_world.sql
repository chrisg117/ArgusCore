-- Azsuna, Their Dying Breaths (42271), fix round 2026-11-27.
-- 1. Two of the five Cracked Ley Crystals (240267) could not be reached: 12202102 stood 23 yd under the hillside (the
--    navmesh at 669,6571 is at z 88), and 12202105 stood on a ledge with no route from the whelplings. Both move to the
--    nearest clear point that has a route (navplace, at least 4 yd from walls).
-- 2. The Grievously Wounded Whelpling (122292) comes from Create Whelpling (180527: "Revive a whelpling using the Dim Ley
--    Crystal", CREATE_ITEM 122292), cast by a fifth click on a whelpling once four are revived (the guide's "And one
--    more whelpling", WoW-Pro 7.3.5C Azsuna). It was handed out by the script on the fourth revive. The click casts it
--    on the clicker (cast_flags 3): its CREATE_ITEM goes to the spell target, and the whelpling as target gets nothing.
UPDATE `gameobject` SET `position_x` = 669.71, `position_y` = 6570.39, `position_z` = 88.81 WHERE `guid` = 12202102;
UPDATE `gameobject` SET `position_x` = 683.92, `position_y` = 6540.74, `position_z` = 64.58 WHERE `guid` = 12202105;

DELETE FROM `npc_spellclick_spells` WHERE `npc_entry` = 90880 AND `spell_id` = 180527;
INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(90880, 180527, 3, 0);

-- Resuscitating until four are revived; then Create Whelpling, once (not with the item already in the bags)
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 18 AND `SourceGroup` = 90880 AND `SourceEntry` IN (180713, 180527);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `ConditionStringValue1`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(18, 90880, 180713, 0, 0,  9, 0,  42271, 0, 0, '', 0, 0, 0, '', 'Mana-Drained Whelpling - Resuscitating while on Their Dying Breaths'),
(18, 90880, 180713, 0, 0, 48, 0, 284015, 0, 4, '', 1, 0, 0, '', 'and fewer than four whelplings are revived'),
(18, 90880, 180527, 0, 0,  9, 0,  42271, 0, 0, '', 0, 0, 0, '', 'Mana-Drained Whelpling - Create Whelpling while on Their Dying Breaths'),
(18, 90880, 180527, 0, 0, 48, 0, 284015, 0, 4, '', 0, 0, 0, '', 'once four whelplings are revived'),
(18, 90880, 180527, 0, 0,  2, 0, 122292, 1, 0, '', 1, 0, 0, '', 'and the Grievously Wounded Whelpling is not in the bags');
