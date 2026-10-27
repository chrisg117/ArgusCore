-- QuestV2CliTask.db2: the bonus objective / world quest eligibility filters the client checks.
-- The tables only hold hotfix overrides; the rows themselves come from the client file.
CREATE TABLE IF NOT EXISTS `quest_v2_cli_task` (
  `FiltRaces` bigint NOT NULL DEFAULT 0,
  `QuestTitle` text CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci,
  `BulletText` text CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci,
  `StartItem` int NOT NULL DEFAULT 0,
  `UniqueBitFlag` smallint unsigned NOT NULL DEFAULT 0,
  `ConditionID` smallint unsigned NOT NULL DEFAULT 0,
  `FiltClasses` smallint NOT NULL DEFAULT 0,
  `FiltCompletedQuest1` smallint unsigned NOT NULL DEFAULT 0,
  `FiltCompletedQuest2` smallint unsigned NOT NULL DEFAULT 0,
  `FiltCompletedQuest3` smallint unsigned NOT NULL DEFAULT 0,
  `FiltMinSkillID` smallint unsigned NOT NULL DEFAULT 0,
  `WorldStateExpressionID` smallint NOT NULL DEFAULT 0,
  `FiltActiveQuest` tinyint unsigned NOT NULL DEFAULT 0,
  `FiltCompletedQuestLogic` tinyint unsigned NOT NULL DEFAULT 0,
  `FiltMaxFactionID` tinyint unsigned NOT NULL DEFAULT 0,
  `FiltMaxFactionValue` tinyint unsigned NOT NULL DEFAULT 0,
  `FiltMaxLevel` tinyint unsigned NOT NULL DEFAULT 0,
  `FiltMinFactionID` tinyint unsigned NOT NULL DEFAULT 0,
  `FiltMinFactionValue` tinyint unsigned NOT NULL DEFAULT 0,
  `FiltMinLevel` tinyint unsigned NOT NULL DEFAULT 0,
  `FiltMinSkillValue` tinyint unsigned NOT NULL DEFAULT 0,
  `FiltNonActiveQuest` tinyint unsigned NOT NULL DEFAULT 0,
  `ID` int unsigned NOT NULL DEFAULT 0,
  `BreadCrumbID` int unsigned NOT NULL DEFAULT 0,
  `QuestInfoID` int unsigned NOT NULL DEFAULT 0,
  `SandboxScalingID` int NOT NULL DEFAULT 0,
  `VerifiedBuild` int NOT NULL DEFAULT 0,
  PRIMARY KEY (`ID`,`VerifiedBuild`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

CREATE TABLE IF NOT EXISTS `quest_v2_cli_task_locale` (
  `ID` int unsigned NOT NULL DEFAULT 0,
  `locale` varchar(4) CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci NOT NULL,
  `QuestTitle_lang` text CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci,
  `BulletText_lang` text CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci,
  `VerifiedBuild` int NOT NULL DEFAULT 0,
  PRIMARY KEY (`ID`,`locale`,`VerifiedBuild`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
