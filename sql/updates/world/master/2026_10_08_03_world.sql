-- Unattended Cannon (100959), the vehicle players fight from in the Black City of the Broken Shore scenario
-- (map 1460), was never sniffed: it has no vehicle id, no spellclick, no spells and level 1.
-- Its spells are named for it in the client: "Pilot Cannon" (199980) is the aura that controls it, and
-- "Fel Cannonball" (199993) and "Fel Furnace" (199402) are the two buttons a player has while piloting it.
-- The vehicle id is a guess. Nothing in the client ties one to the cannon, so it gets the one nearest to where
-- its own should be among those of the same period whose only seat lets its passenger drive and cast (4585).
-- Level 98-110 is the level scaling range stored for it.
UPDATE `creature_template` SET `npcflag`=`npcflag`|0x01000000, `VehicleId`=4585 WHERE `entry`=100959;

UPDATE `creature_template_difficulty` SET `MinLevel`=98, `MaxLevel`=110 WHERE `DifficultyID`=0 AND `Entry`=100959;

DELETE FROM `npc_spellclick_spells` WHERE `npc_entry`=100959;
INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(100959, 199980, 1, 0);

DELETE FROM `creature_template_spell` WHERE `CreatureID`=100959;
INSERT INTO `creature_template_spell` (`CreatureID`, `Index`, `Spell`, `VerifiedBuild`) VALUES
(100959, 0, 199993, 0),
(100959, 1, 199402, 0);
