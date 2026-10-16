-- The follower growth tables: the experience each follower level needs, and the item level
-- upgrade rules armament items use. The tables only hold hotfix overrides; the rows
-- themselves come from the client files.
CREATE TABLE IF NOT EXISTS `garr_follower_level_xp` (
  `ID` int unsigned NOT NULL DEFAULT '0',
  `XpToNextLevel` smallint unsigned NOT NULL DEFAULT '0',
  `ShipmentXP` smallint unsigned NOT NULL DEFAULT '0',
  `FollowerLevel` tinyint unsigned NOT NULL DEFAULT '0',
  `GarrFollowerTypeID` tinyint unsigned NOT NULL DEFAULT '0',
  `VerifiedBuild` int NOT NULL DEFAULT '0',
  PRIMARY KEY (`ID`,`VerifiedBuild`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS `garr_item_level_upgrade_data` (
  `ID` int unsigned NOT NULL DEFAULT '0',
  `Operation` int NOT NULL DEFAULT '0',
  `MinItemLevel` int NOT NULL DEFAULT '0',
  `MaxItemLevel` int NOT NULL DEFAULT '0',
  `FollowerTypeID` int NOT NULL DEFAULT '0',
  `VerifiedBuild` int NOT NULL DEFAULT '0',
  PRIMARY KEY (`ID`,`VerifiedBuild`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
