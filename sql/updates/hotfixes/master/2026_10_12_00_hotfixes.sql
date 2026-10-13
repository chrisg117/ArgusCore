-- GarrType.db2: one row per garrison type (2 = Warlords garrison, 3 = Legion class hall).
-- The core reads it to tell which type a garrison site belongs to. The table only holds hotfix
-- overrides; the rows themselves come from the client file.
CREATE TABLE IF NOT EXISTS `garr_type` (
  `ID` int unsigned NOT NULL DEFAULT '0',
  `Flags` int unsigned NOT NULL DEFAULT '0',
  `PrimaryCurrencyTypeID` int unsigned NOT NULL DEFAULT '0',
  `SecondaryCurrencyTypeID` int unsigned NOT NULL DEFAULT '0',
  `ExpansionID` int unsigned NOT NULL DEFAULT '0',
  `MapIDs1` int NOT NULL DEFAULT '0',
  `MapIDs2` int NOT NULL DEFAULT '0',
  `VerifiedBuild` int NOT NULL DEFAULT '0',
  PRIMARY KEY (`ID`,`VerifiedBuild`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
