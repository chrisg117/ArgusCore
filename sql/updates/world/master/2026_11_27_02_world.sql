-- Azsuna, Ael'Yith (90267), fix round 2026-11-27: faction 2635 (hostile Nightborne), the retail value (TDB 1210); here it
-- was 35, so the target of The Nightborne Prince (37861, "Slay Ael'Yith") could not be attacked. Who sees him already
-- matches retail: phase 6803 in Ley-Ruins of Zarkhenar (7361) until 37861 is complete or rewarded.
-- 108721 (also Ael'Yith) stays faction 35, as in retail: a separate, friendly entry with no spawn.
UPDATE `creature_template` SET `faction` = 2635 WHERE `entry` = 90267;
