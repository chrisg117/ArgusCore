-- GarrTalent.db2: the talents (hall upgrades) of the Legion class hall research trees.
-- The core reads cost, duration, row and unlock condition from it when a talent is researched.
-- Both tables only hold hotfix overrides; the rows themselves come from the client file.
CREATE TABLE IF NOT EXISTS `garr_talent` (
  `Name` text,
  `Description` text,
  `IconFileDataID` int NOT NULL DEFAULT '0',
  `ResearchDurationSecs` int NOT NULL DEFAULT '0',
  `Tier` tinyint NOT NULL DEFAULT '0',
  `UiOrder` tinyint NOT NULL DEFAULT '0',
  `Flags` tinyint NOT NULL DEFAULT '0',
  `ID` int unsigned NOT NULL DEFAULT '0',
  `GarrTalentTreeID` int unsigned NOT NULL DEFAULT '0',
  `GarrAbilityID` int unsigned NOT NULL DEFAULT '0',
  `PlayerConditionID` int unsigned NOT NULL DEFAULT '0',
  `ResearchCost` int NOT NULL DEFAULT '0',
  `ResearchCostCurrencyTypesID` int unsigned NOT NULL DEFAULT '0',
  `ResearchGoldCost` int NOT NULL DEFAULT '0',
  `PerkSpellID` int unsigned NOT NULL DEFAULT '0',
  `PerkPlayerConditionID` int unsigned NOT NULL DEFAULT '0',
  `RespecCost` int NOT NULL DEFAULT '0',
  `RespecCostCurrencyTypesID` int unsigned NOT NULL DEFAULT '0',
  `RespecDurationSecs` int NOT NULL DEFAULT '0',
  `RespecGoldCost` int NOT NULL DEFAULT '0',
  `VerifiedBuild` int NOT NULL DEFAULT '0',
  PRIMARY KEY (`ID`,`VerifiedBuild`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS `garr_talent_locale` (
  `ID` int unsigned NOT NULL DEFAULT '0',
  `locale` varchar(4) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  `Name_lang` text CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci,
  `Description_lang` text CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci,
  `VerifiedBuild` int NOT NULL DEFAULT '0',
  PRIMARY KEY (`ID`,`locale`,`VerifiedBuild`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci
/*!50500 PARTITION BY LIST  COLUMNS(locale)
(PARTITION deDE VALUES IN ('deDE') ENGINE = InnoDB,
 PARTITION esES VALUES IN ('esES') ENGINE = InnoDB,
 PARTITION esMX VALUES IN ('esMX') ENGINE = InnoDB,
 PARTITION frFR VALUES IN ('frFR') ENGINE = InnoDB,
 PARTITION itIT VALUES IN ('itIT') ENGINE = InnoDB,
 PARTITION koKR VALUES IN ('koKR') ENGINE = InnoDB,
 PARTITION ptBR VALUES IN ('ptBR') ENGINE = InnoDB,
 PARTITION ruRU VALUES IN ('ruRU') ENGINE = InnoDB,
 PARTITION zhCN VALUES IN ('zhCN') ENGINE = InnoDB,
 PARTITION zhTW VALUES IN ('zhTW') ENGINE = InnoDB) */;
