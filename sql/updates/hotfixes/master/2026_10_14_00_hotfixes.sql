-- CharShipment.db2 and CharShipmentContainer.db2: the orders (work orders, troop orders) a
-- garrison or class hall can place, and the containers that group them. The class hall reads
-- what an order costs, how long it takes and which follower it gives from them.
-- The tables only hold hotfix overrides; the rows themselves come from the client files.
CREATE TABLE IF NOT EXISTS `char_shipment` (
  `ID` int unsigned NOT NULL DEFAULT '0',
  `TreasureID` int unsigned NOT NULL DEFAULT '0',
  `Duration` int unsigned NOT NULL DEFAULT '0',
  `SpellID` int NOT NULL DEFAULT '0',
  `DummyItemID` int NOT NULL DEFAULT '0',
  `OnCompleteSpellID` int NOT NULL DEFAULT '0',
  `ContainerID` smallint unsigned NOT NULL DEFAULT '0',
  `GarrFollowerID` smallint unsigned NOT NULL DEFAULT '0',
  `MaxShipments` tinyint unsigned NOT NULL DEFAULT '0',
  `Flags` tinyint unsigned NOT NULL DEFAULT '0',
  `VerifiedBuild` int NOT NULL DEFAULT '0',
  PRIMARY KEY (`ID`,`VerifiedBuild`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS `char_shipment_container` (
  `ID` int unsigned NOT NULL DEFAULT '0',
  `PendingText` text,
  `Description` text,
  `WorkingSpellVisualID` int unsigned NOT NULL DEFAULT '0',
  `UiTextureKitID` smallint unsigned NOT NULL DEFAULT '0',
  `WorkingDisplayInfoID` smallint unsigned NOT NULL DEFAULT '0',
  `SmallDisplayInfoID` smallint unsigned NOT NULL DEFAULT '0',
  `MediumDisplayInfoID` smallint unsigned NOT NULL DEFAULT '0',
  `LargeDisplayInfoID` smallint unsigned NOT NULL DEFAULT '0',
  `CrossFactionID` smallint unsigned NOT NULL DEFAULT '0',
  `BaseCapacity` tinyint unsigned NOT NULL DEFAULT '0',
  `GarrBuildingType` tinyint unsigned NOT NULL DEFAULT '0',
  `GarrTypeID` tinyint unsigned NOT NULL DEFAULT '0',
  `MediumThreshold` tinyint unsigned NOT NULL DEFAULT '0',
  `LargeThreshold` tinyint unsigned NOT NULL DEFAULT '0',
  `Faction` tinyint NOT NULL DEFAULT '0',
  `CompleteSpellVisualID` int unsigned NOT NULL DEFAULT '0',
  `VerifiedBuild` int NOT NULL DEFAULT '0',
  PRIMARY KEY (`ID`,`VerifiedBuild`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS `char_shipment_container_locale` (
  `ID` int unsigned NOT NULL DEFAULT '0',
  `locale` varchar(4) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  `PendingText_lang` text CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci,
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
