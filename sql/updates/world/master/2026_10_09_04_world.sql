-- Broken Shore scenario (map 1460): the forces that are with King Varian Wrynn.
--
-- Gelbin Mekkatorque (90716) and Varian (90713) fight as the Alliance's other leaders do. The Stormwind
-- Guards (92123), Gnomeregan Tinkerers (92122) and Alliance Priests (92074) fight as its soldiers do. The
-- instance script puts them at Varian's camp; none of them was sniffed in place.
UPDATE `creature_template` SET `ScriptName`='npc_broken_shore_alliance_leader' WHERE `entry` IN (90713, 90716);
UPDATE `creature_template` SET `ScriptName`='npc_broken_shore_alliance_soldier' WHERE `entry` IN (92074, 92122, 92123);

-- The Alliance Priest has faction 35 and the Stormwind Guard 2141 in this database, which is not the faction
-- of the scenario's other Alliance forces (2879), so demons and they would not fight each other.
UPDATE `creature_template` SET `faction`=2879 WHERE `entry` IN (92074, 92123);
