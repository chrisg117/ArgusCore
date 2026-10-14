-- GarrFollowerQuality.db2: how many abilities and traits a follower has at each quality.
-- GarrFollowerType.db2: the limits of each follower type (active followers, item level).
-- The core reads both for the followers of the Legion class hall.
-- Both tables only hold hotfix overrides; the rows themselves come from the client files.
CREATE TABLE IF NOT EXISTS `garr_follower_quality` (
  `ID` int unsigned NOT NULL DEFAULT '0',
  `XpToNextQuality` int unsigned NOT NULL DEFAULT '0',
  `ShipmentXP` smallint unsigned NOT NULL DEFAULT '0',
  `Quality` tinyint unsigned NOT NULL DEFAULT '0',
  `AbilityCount` tinyint unsigned NOT NULL DEFAULT '0',
  `TraitCount` tinyint unsigned NOT NULL DEFAULT '0',
  `GarrFollowerTypeID` tinyint unsigned NOT NULL DEFAULT '0',
  `ClassSpecID` int unsigned NOT NULL DEFAULT '0',
  `VerifiedBuild` int NOT NULL DEFAULT '0',
  PRIMARY KEY (`ID`,`VerifiedBuild`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS `garr_follower_type` (
  `ID` int unsigned NOT NULL DEFAULT '0',
  `MaxItemLevel` smallint unsigned NOT NULL DEFAULT '0',
  `MaxFollowers` tinyint unsigned NOT NULL DEFAULT '0',
  `MaxFollowerBuildingType` tinyint unsigned NOT NULL DEFAULT '0',
  `GarrTypeID` tinyint unsigned NOT NULL DEFAULT '0',
  `LevelRangeBias` tinyint unsigned NOT NULL DEFAULT '0',
  `ItemLevelRangeBias` tinyint unsigned NOT NULL DEFAULT '0',
  `Flags` tinyint unsigned NOT NULL DEFAULT '0',
  `VerifiedBuild` int NOT NULL DEFAULT '0',
  PRIMARY KEY (`ID`,`VerifiedBuild`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
