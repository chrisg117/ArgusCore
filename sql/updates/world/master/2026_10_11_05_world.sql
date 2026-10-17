-- "Blade in Twilight" (scenario 991): its last stage, "The True Death of Zakajz": the blade tells the
-- player to drain what is left of Zakajz, and when that is done Shadowlord Slaghammer comes and opens a
-- portal to Dalaran.
--
-- What happens is taken from a video of the retail scenario and from the client's data (the conversation
-- and the spells); where Slaghammer stands and where the portal opens was marked in game against the
-- video. The lines are Blizzard's (broadcast_text 104544, 104546, 104550 and 104552).

-- What the blade says of Zakajz: conversation 1146, which the spell "Conversation - Death to Zakajz"
-- (201917) starts. Its two lines are the client's (ConversationLine 2756 and 2757). As with the
-- conversation of the Deacon's death (2026_10_11_03_world.sql), the times are from the video, the view of
-- the speaker is the client's for this blade, and the id of the speaker is not known.
DELETE FROM `conversation_template` WHERE `Id`=1146;
INSERT INTO `conversation_template` (`Id`, `FirstLineId`, `LastLineEndTime`, `Flags`, `ScriptName`, `VerifiedBuild`) VALUES
(1146, 2756, 26000, 0, '', 0);

DELETE FROM `conversation_actors` WHERE `ConversationId`=1146;
INSERT INTO `conversation_actors` (`ConversationId`, `ConversationActorId`, `ConversationActorGuid`, `Idx`, `CreatureId`, `CreatureDisplayInfoId`, `NoActorObject`, `ActivePlayerObject`, `VerifiedBuild`) VALUES
(1146, 0, 0, 0, 102255, 67546, 0, 0, 0); -- Xal'atath, Blade of the Black Empire

DELETE FROM `conversation_line_template` WHERE `Id` IN (2756, 2757);
INSERT INTO `conversation_line_template` (`Id`, `StartTime`, `UiCameraID`, `ActorIdx`, `Flags`, `VerifiedBuild`) VALUES
(2756, 0, 748, 0, 0, 0), -- Yes, we are eager to feast on the fallen titan's minions. But first... Zakajz.
(2757, 11500, 748, 0, 0, 0); -- The cultists may try to stir him again, or he could awaken on his own...

-- "Dark Drain" is cast at a creature that the client's data does not name: Zakajz. What its end does is in
-- its script.
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId`=13 AND `SourceEntry`=201904;
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `ConditionStringValue1`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(13, 1, 201904, 0, 0, 31, 0, 3, 102039, 0, '', 0, 0, 0, '', 'Dark Drain - is cast at Zakajz the Corruptor');

DELETE FROM `spell_script_names` WHERE `spell_id`=201904;
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(201904, 'spell_blade_in_twilight_dark_drain');

-- What Slaghammer says when it is done (his other lines in the tomb are in 2026_10_11_01_world.sql to
-- 2026_10_11_03_world.sql)
DELETE FROM `creature_text` WHERE `CreatureID`=101899 AND `GroupID` IN (5, 6);
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundPlayType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(101899, 5, 0, 'Ye did it! The cultists ha''e run off! Ha ha! Ye should get back ta Faol. I''ll use this Twilight portal stone fer ye!', 12, 0, 100, 0, 0, 63455, 0, 104550, 0, 'Shadowlord Slaghammer - Zakajz is gone'),
(101899, 6, 0, 'Keep an eye on that blade, though. No tellin'' wha'' somethin'' that old be capable of.', 12, 0, 100, 0, 0, 63456, 0, 104552, 0, 'Shadowlord Slaghammer - the portal is open');

-- His portal is Blizzard's "Portal to Dalaran City" (247351), which casts "Teleport: Dalaran" (190598) on
-- who uses it. Where that spell takes one is not in the client's data: this is Krasus' Landing, where the
-- video's player arrives, at the place that the other teleports to it have.
DELETE FROM `spell_target_position` WHERE `ID`=190598;
INSERT INTO `spell_target_position` (`ID`, `EffectIndex`, `OrderIndex`, `MapID`, `PositionX`, `PositionY`, `PositionZ`, `Orientation`, `VerifiedBuild`) VALUES
(190598, 0, 0, 1220, -828.72, 4371.78, 738.64, NULL, 0);
