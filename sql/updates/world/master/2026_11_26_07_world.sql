-- Eye of Azshara intro: Warlord Parjesh's two intro lines (spec "Intermissions and RP").
-- 101992 "Hmph. Intruders. They will not get far." when the first player lands (instance script).
-- 101991 "Guardians of the ritual, prepare yourselves! She will not be disappointed!" when a player comes near.
-- Yells heard across the zone (TextRange 2): he stands inland, away from the zone-in.
DELETE FROM `creature_text` WHERE `CreatureID` = 91784 AND `GroupID` IN (10, 11);
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundPlayType`, `BroadcastTextId`, `TextRange`, `comment`)
SELECT 91784, c.`grp`, 0, b.`Text`, 14, 0, 100, 0, 0, b.`SoundKitID1`, 0, b.`ID`, 2, c.`cmt` FROM `hotfixes`.`broadcast_text` b JOIN (
  SELECT 10 AS grp, 101992 AS bid, 'Warlord Parjesh - intro, the first player lands' AS cmt
  UNION ALL SELECT 11, 101991, 'Warlord Parjesh - intro, a player comes near'
) c ON b.`ID` = c.`bid`;
