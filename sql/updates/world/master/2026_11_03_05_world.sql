-- Athissa, Tidemistress of the Hatecoil (88091): hostile on Wowhead and the kill target of Nar'thalas Still Suffers
-- (37486), but friendly (faction 35) in the database, so no player could attack her. Faction 16 as her Hatecoil
-- Raiders (88084).
UPDATE `creature_template` SET `faction` = 16 WHERE `entry` = 88091;

-- Ancient Tome (89834), the books of Hit the Books (37678): hostile on Wowhead, faction 35 here
UPDATE `creature_template` SET `faction` = 16 WHERE `entry` = 89834;

-- Elya Azuremoon (88275), who carries the Tidestone Vault Key of The Headmistress' Keys (37730), and the three drawings
-- inside Sythorne's books (Study Hall: Combat Research, 42371): kill targets, faction 35 here
UPDATE `creature_template` SET `faction` = 16 WHERE `entry` IN (88275, 107299, 107300, 107301);
