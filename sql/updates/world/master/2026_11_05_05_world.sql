-- DK campaign: The Call of Icecrown, scenario 901 step 1810, Spire trash placement after the third pass (Task 3b-2b).
-- Pass 3 cleared the step with no GM help and the user rated the pulls balanced; the one complaint was fighting
-- Gavinrad together with his knight and swordsman.

-- Gavinrad's pack is its own pull, moved about 40 yd east onto the walkway toward Halahk, so a player fights the pair,
-- rests, then takes Gavinrad alone. Fidelity compromise for a fresh 100 in quest gear: the recording pulls them together
DELETE FROM `creature_formations` WHERE `leaderGUID`=14800004;
UPDATE `creature` SET `position_x`=4402.00, `position_y`=2767.00, `position_z`=357.70, `orientation`=0 WHERE `guid`=14800016; -- Spectral Knight
UPDATE `creature` SET `position_x`=4399.00, `position_y`=2766.00, `position_z`=357.70, `orientation`=0 WHERE `guid`=14800017; -- Phantom Swordsman

-- Halahk's knights move 12 to 15 yd west, up the ramp onto the walked floor of her approach. From the sunken spot they
-- saw her only inside 8 yd, so their Charge never fired
UPDATE `creature` SET `position_x`=4387.20, `position_y`=2672.40, `position_z`=348.90, `orientation`=3.58 WHERE `guid`=14800011;
UPDATE `creature` SET `position_x`=4389.30, `position_y`=2672.90, `position_z`=348.60, `orientation`=3.58 WHERE `guid`=14800012;
