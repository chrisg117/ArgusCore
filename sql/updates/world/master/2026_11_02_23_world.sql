-- Stellagosa Chain Bunny - All Phases (90578) anchors Stellagosa's chains at Faronaar but could be targeted: it is a
-- trigger and not selectable.
UPDATE `creature_template` SET `unit_flags`=`unit_flags`|33554432, `flags_extra`=`flags_extra`|128 WHERE `entry`=90578;
