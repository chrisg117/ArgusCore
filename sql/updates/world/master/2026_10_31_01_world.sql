-- Azsuna client run: the client never drew the Fel Machinations demon hunter at his first rough spot; this one it does.
-- The demon hunters of Faronaar scale with the zone instead of standing at level 1.
UPDATE `creature` SET `position_x`=-300.00, `position_y`=7334.00, `position_z`=18.26, `orientation`=0 WHERE `guid`=12201001;
UPDATE `creature_template_difficulty` SET `MinLevel`=110, `MaxLevel`=110, `LevelScalingMin`=98, `LevelScalingMax`=110 WHERE `Entry`=90474 AND `DifficultyID`=0;
