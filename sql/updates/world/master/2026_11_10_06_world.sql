-- Warlock re-do, fix round after the verification run (warlock re-do, 2026-11-10).
-- The freed council speak to the whole area: they are party members in the video, and their lines reached the player
-- there even with the group spread out (archive talk, Shinfel in Jagganoth's grip).
UPDATE `creature_text` SET `TextRange` = 1 WHERE `CreatureID` IN (99147, 99145, 99146);

-- Jagganoth's three lines on the way out play as talking heads, as in the video (12:13, 12:30, 13:03): one-line
-- conversations with the client's own ConversationLine ids and a TalkingHead actor (NoActorObject 0, no spawn guid, a
-- creature id). Camera 82: of the cameras the data's talking-head actors use (296, 119, 82, 748, 680, 130), the one whose
-- framing of the pit lord comes closest to the video's portrait (head and the fel flames above it; captures in the
-- re-do notes). The creature_text rows they replace are dropped.
DELETE FROM `creature_text` WHERE `CreatureID` = 98831 AND `GroupID` IN (5, 6, 7);
DELETE FROM `conversation_template` WHERE `Id` BETWEEN 15220001 AND 15220003;
INSERT INTO `conversation_template` (`Id`, `FirstLineId`, `LastLineEndTime`, `Flags`, `ScriptName`, `VerifiedBuild`) VALUES
(15220001, 2238, 8000, 0, '', 0),
(15220002, 2240, 8000, 0, '', 0),
(15220003, 2241, 8000, 0, '', 0);
DELETE FROM `conversation_actors` WHERE `ConversationId` BETWEEN 15220001 AND 15220003;
INSERT INTO `conversation_actors` (`ConversationId`, `ConversationActorId`, `ConversationActorGuid`, `Idx`, `CreatureId`, `CreatureDisplayInfoId`, `NoActorObject`, `ActivePlayerObject`, `VerifiedBuild`) VALUES
(15220001, 0, 0, 0, 98831, 73397, 0, 0, 0),
(15220002, 0, 0, 0, 98831, 73397, 0, 0, 0),
(15220003, 0, 0, 0, 98831, 73397, 0, 0, 0);
DELETE FROM `conversation_line_template` WHERE `Id` IN (2238, 2240, 2241);
INSERT INTO `conversation_line_template` (`Id`, `StartTime`, `UiCameraID`, `ActorIdx`, `Flags`, `VerifiedBuild`) VALUES
(2238, 0, 82, 0, 0, 0), -- Jagganoth: a skull trophy (103363)
(2240, 0, 82, 0, 0, 0), -- Jagganoth: burn to cinders (103365)
(2241, 0, 82, 0, 0, 0); -- Jagganoth: this is not over (103366)
