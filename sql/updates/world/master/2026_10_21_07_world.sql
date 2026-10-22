-- Hidden No More (39495): the Rockslide (245045) at Cryptic Hollow stayed for everyone, so after the eye beams gave the
-- "Rockslide" credit (objective 280354) the player still could not enter the cave. Hide it from a player once that
-- objective is done or the quest is rewarded. TypeID 5 is TYPEID_GAMEOBJECT in this core.
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 32 AND `SourceGroup` = 5 AND `SourceEntry` = 245045;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `ConditionStringValue1`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(32, 5, 245045, 0, 0, 48, 0, 280354, 0, 1, '', 1, 0, 0, '', 'Rockslide is visible if objective 280354 (Rockslide) is not done'),
(32, 5, 245045, 0, 0, 47, 0, 39495, 64, 0, '', 1, 0, 0, '', 'Rockslide is visible if quest 39495 (Hidden No More) is not rewarded');
