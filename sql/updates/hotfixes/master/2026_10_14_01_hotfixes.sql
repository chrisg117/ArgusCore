-- The mission encounter and threat tables: which enemies a mission has, the threats
-- (mechanics) each brings, what an ability does against them, and the sets an encounter or
-- threat is drawn from. The class hall's missions read them to work out a team's chance.
-- The tables only hold hotfix overrides; the rows themselves come from the client files.
CREATE TABLE IF NOT EXISTS `garr_ability_effect` (
  `CombatWeightBase` float NOT NULL DEFAULT '0',
  `CombatWeightMax` float NOT NULL DEFAULT '0',
  `ActionValueFlat` float NOT NULL DEFAULT '0',
  `ActionRecordID` int NOT NULL DEFAULT '0',
  `GarrAbilityID` smallint unsigned NOT NULL DEFAULT '0',
  `AbilityAction` tinyint unsigned NOT NULL DEFAULT '0',
  `AbilityTargetType` tinyint unsigned NOT NULL DEFAULT '0',
  `GarrMechanicTypeID` tinyint unsigned NOT NULL DEFAULT '0',
  `Flags` tinyint unsigned NOT NULL DEFAULT '0',
  `ActionRace` tinyint unsigned NOT NULL DEFAULT '0',
  `ActionHours` tinyint unsigned NOT NULL DEFAULT '0',
  `ID` int unsigned NOT NULL DEFAULT '0',
  `VerifiedBuild` int NOT NULL DEFAULT '0',
  PRIMARY KEY (`ID`,`VerifiedBuild`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS `garr_encounter` (
  `Name` text,
  `CreatureID` int NOT NULL DEFAULT '0',
  `UiAnimScale` float NOT NULL DEFAULT '0',
  `UiAnimHeight` float NOT NULL DEFAULT '0',
  `PortraitFileDataID` int NOT NULL DEFAULT '0',
  `ID` int unsigned NOT NULL DEFAULT '0',
  `UiTextureKitID` int unsigned NOT NULL DEFAULT '0',
  `VerifiedBuild` int NOT NULL DEFAULT '0',
  PRIMARY KEY (`ID`,`VerifiedBuild`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS `garr_encounter_locale` (
  `ID` int unsigned NOT NULL DEFAULT '0',
  `locale` varchar(4) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  `Name_lang` text CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci,
  `VerifiedBuild` int NOT NULL DEFAULT '0',
  PRIMARY KEY (`ID`,`locale`,`VerifiedBuild`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci
/*!50500 PARTITION BY LIST  COLUMNS(locale)
( PARTITION deDE VALUES IN ('deDE') ENGINE = InnoDB,
  PARTITION esES VALUES IN ('esES') ENGINE = InnoDB,
  PARTITION esMX VALUES IN ('esMX') ENGINE = InnoDB,
  PARTITION frFR VALUES IN ('frFR') ENGINE = InnoDB,
  PARTITION itIT VALUES IN ('itIT') ENGINE = InnoDB,
  PARTITION koKR VALUES IN ('koKR') ENGINE = InnoDB,
  PARTITION ptBR VALUES IN ('ptBR') ENGINE = InnoDB,
  PARTITION ruRU VALUES IN ('ruRU') ENGINE = InnoDB,
  PARTITION zhCN VALUES IN ('zhCN') ENGINE = InnoDB,
  PARTITION zhTW VALUES IN ('zhTW') ENGINE = InnoDB) */;

CREATE TABLE IF NOT EXISTS `garr_encounter_set_x_encounter` (
  `ID` int unsigned NOT NULL DEFAULT '0',
  `GarrEncounterID` int unsigned NOT NULL DEFAULT '0',
  `GarrEncounterSetID` int unsigned NOT NULL DEFAULT '0',
  `VerifiedBuild` int NOT NULL DEFAULT '0',
  PRIMARY KEY (`ID`,`VerifiedBuild`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS `garr_encounter_x_mechanic` (
  `ID` int unsigned NOT NULL DEFAULT '0',
  `GarrMechanicID` tinyint unsigned NOT NULL DEFAULT '0',
  `GarrMechanicSetID` tinyint unsigned NOT NULL DEFAULT '0',
  `GarrEncounterID` smallint unsigned NOT NULL DEFAULT '0',
  `VerifiedBuild` int NOT NULL DEFAULT '0',
  PRIMARY KEY (`ID`,`VerifiedBuild`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS `garr_mechanic` (
  `ID` int unsigned NOT NULL DEFAULT '0',
  `Factor` float NOT NULL DEFAULT '0',
  `GarrMechanicTypeID` tinyint unsigned NOT NULL DEFAULT '0',
  `GarrAbilityID` int NOT NULL DEFAULT '0',
  `VerifiedBuild` int NOT NULL DEFAULT '0',
  PRIMARY KEY (`ID`,`VerifiedBuild`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS `garr_mechanic_set_x_mechanic` (
  `GarrMechanicID` tinyint unsigned NOT NULL DEFAULT '0',
  `ID` int unsigned NOT NULL DEFAULT '0',
  `GarrMechanicSetID` int unsigned NOT NULL DEFAULT '0',
  `VerifiedBuild` int NOT NULL DEFAULT '0',
  PRIMARY KEY (`ID`,`VerifiedBuild`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS `garr_mechanic_type` (
  `Name` text,
  `Description` text,
  `IconFileDataID` int NOT NULL DEFAULT '0',
  `Category` tinyint unsigned NOT NULL DEFAULT '0',
  `ID` int unsigned NOT NULL DEFAULT '0',
  `VerifiedBuild` int NOT NULL DEFAULT '0',
  PRIMARY KEY (`ID`,`VerifiedBuild`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS `garr_mechanic_type_locale` (
  `ID` int unsigned NOT NULL DEFAULT '0',
  `locale` varchar(4) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  `Name_lang` text CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci,
  `Description_lang` text CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci,
  `VerifiedBuild` int NOT NULL DEFAULT '0',
  PRIMARY KEY (`ID`,`locale`,`VerifiedBuild`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci
/*!50500 PARTITION BY LIST  COLUMNS(locale)
( PARTITION deDE VALUES IN ('deDE') ENGINE = InnoDB,
  PARTITION esES VALUES IN ('esES') ENGINE = InnoDB,
  PARTITION esMX VALUES IN ('esMX') ENGINE = InnoDB,
  PARTITION frFR VALUES IN ('frFR') ENGINE = InnoDB,
  PARTITION itIT VALUES IN ('itIT') ENGINE = InnoDB,
  PARTITION koKR VALUES IN ('koKR') ENGINE = InnoDB,
  PARTITION ptBR VALUES IN ('ptBR') ENGINE = InnoDB,
  PARTITION ruRU VALUES IN ('ruRU') ENGINE = InnoDB,
  PARTITION zhCN VALUES IN ('zhCN') ENGINE = InnoDB,
  PARTITION zhTW VALUES IN ('zhTW') ENGINE = InnoDB) */;

CREATE TABLE IF NOT EXISTS `garr_mission_x_encounter` (
  `OrderIndex` tinyint unsigned NOT NULL DEFAULT '0',
  `ID` int unsigned NOT NULL DEFAULT '0',
  `GarrEncounterID` int unsigned NOT NULL DEFAULT '0',
  `GarrEncounterSetID` int unsigned NOT NULL DEFAULT '0',
  `GarrMissionID` int unsigned NOT NULL DEFAULT '0',
  `VerifiedBuild` int NOT NULL DEFAULT '0',
  PRIMARY KEY (`ID`,`VerifiedBuild`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
