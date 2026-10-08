-- Two creatures of the Alliance demon portal in the Broken Shore scenario (map 1460) were never sniffed and
-- kept the placeholder faction 35 and level 1: Eredar Chaos Guard (90525) and Shielded Anchor (101667).
-- Give them the values already stored for the Mo'arg Painbringer (92564) of the same portal:
-- faction 2780 and level 98-110, which is also the level scaling range stored for both.
UPDATE `creature_template` SET `faction`=2780 WHERE `entry` IN (90525, 101667);

UPDATE `creature_template_difficulty` SET `MinLevel`=98, `MaxLevel`=110 WHERE `DifficultyID`=0 AND `Entry` IN (90525, 101667);
