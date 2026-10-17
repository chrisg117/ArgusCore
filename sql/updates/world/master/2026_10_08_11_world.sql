-- Broken Shore scenario (map 1460): the Anchoring Crystals (91704) and Shielded Anchors (101667) healed a
-- third of their health every two seconds while they were being attacked. They have NullCreatureAI so that
-- they stand still and do not fight back, and a creature with that AI never counts as engaged, so the core
-- kept regenerating them as if nothing was attacking. They are objects to be destroyed, so their health
-- regeneration is turned off, as it is for other creatures of this kind (Antipersonnel Cannon, Battleground
-- Demolisher).
UPDATE `creature_template` SET `RegenHealth`=0 WHERE `entry` IN (91704, 101667);
