-- Azsuna, They Came From the Sea (37256), fix round 2026-11-27: the quest had no map POI. Blobs and points from the
-- retail TrinityCore data (TDB 1210), in this core's columns: WorldMapAreaId 1015 (Azsuna, as 37257 here) for the retail
-- UiMapID 630, no Z on the points, SpawnTrackingID 0 (the retail ids are later than 7.3.5).
-- Blob 0 the turn-in (Idri at 286,6683), 1 Zarrin's head (185,6866), 2 the naga (an eight-point area), 3 the start.
DELETE FROM `quest_poi` WHERE `QuestID` = 37256;
INSERT INTO `quest_poi` (`QuestID`, `BlobIndex`, `Idx1`, `ObjectiveIndex`, `QuestObjectiveID`, `QuestObjectID`, `MapID`, `WorldMapAreaId`, `Floor`, `Priority`, `Flags`, `WorldEffectID`, `PlayerConditionID`, `SpawnTrackingID`, `AlwaysAllowMergingBlobs`, `VerifiedBuild`) VALUES
(37256, 0, 0, -1,      0,      0, 1220, 1015, 0, 0, 0, 0, 0, 0, 0, 0),
(37256, 0, 1,  0, 276231, 120079, 1220, 1015, 0, 0, 0, 0, 0, 0, 0, 0),
(37256, 0, 2,  1, 276761,  88084, 1220, 1015, 0, 0, 0, 0, 0, 0, 0, 0),
(37256, 0, 3, 32,      0,      0, 1220, 1015, 0, 0, 0, 0, 0, 0, 0, 0);
DELETE FROM `quest_poi_points` WHERE `QuestID` = 37256;
INSERT INTO `quest_poi_points` (`QuestID`, `Idx1`, `Idx2`, `X`, `Y`, `VerifiedBuild`) VALUES
(37256, 0, 0, 286, 6683, 0),
(37256, 1, 0, 185, 6866, 0),
(37256, 2, 0, 179, 6535, 0),
(37256, 2, 1, 290, 6775, 0),
(37256, 2, 2, 269, 6798, 0),
(37256, 2, 3, 200, 6842, 0),
(37256, 2, 4,  83, 6850, 0),
(37256, 2, 5,  32, 6784, 0),
(37256, 2, 6, -24, 6658, 0),
(37256, 2, 7, -24, 6647, 0),
(37256, 3, 0, 286, 6683, 0);
