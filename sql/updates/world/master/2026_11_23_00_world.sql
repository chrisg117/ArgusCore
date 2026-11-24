-- spell_summon_replace: a summon spell cast by a player whose summon takes the place of the nearest creature of
-- ReplacedEntry within SearchRange yd (of the summoner) for that player only, by smooth phasing: the player stops seeing
-- the original while the summon lives, and after it when KeepReplacedHidden is 1 (until the original respawns). Every
-- other player keeps seeing the original. The summon becomes private to the player. Reload: .reload spell_summon_replace
DROP TABLE IF EXISTS `spell_summon_replace`;
CREATE TABLE `spell_summon_replace` (
  `SpellID` int unsigned NOT NULL,
  `ReplacedEntry` int unsigned NOT NULL,
  `SearchRange` float NOT NULL DEFAULT '10',
  `KeepReplacedHidden` tinyint unsigned NOT NULL DEFAULT '0',
  `Comment` varchar(255) NOT NULL DEFAULT '',
  PRIMARY KEY (`SpellID`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;
