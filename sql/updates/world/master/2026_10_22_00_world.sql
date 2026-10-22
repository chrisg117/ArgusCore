-- Hidden No More (39495): inside Cryptic Hollow three squads of Illidari allies stand against Felguard Butchers and
-- Eredar Sorcerers, but nothing made them fight, so all 20 live butchers attacked the player. They now really fight
-- each other (npc_cryptic_hollow_squad_fighter): damage between them stops at 30% health so the fights go on, the
-- player's damage counts as usual, and a fighting creature leaves the player alone unless attacked. Each side faces
-- the nearest enemy at spawn. The loose demons outside the squads are unchanged.

-- west squad
UPDATE `creature` SET `orientation` = 1.0148 WHERE `guid` = 6001022; -- Felguard Butcher
UPDATE `creature` SET `orientation` = 0.9128 WHERE `guid` = 6001012; -- Felguard Butcher
UPDATE `creature` SET `orientation` = 1.0476 WHERE `guid` = 6001016; -- Felguard Butcher
UPDATE `creature` SET `orientation` = 1.3939 WHERE `guid` = 6001021; -- Felguard Butcher
UPDATE `creature` SET `orientation` = 1.8889 WHERE `guid` = 6001024; -- Felguard Butcher
UPDATE `creature` SET `orientation` = 2.1591 WHERE `guid` = 6001023; -- Felguard Butcher
UPDATE `creature` SET `orientation` = 2.137 WHERE `guid` = 6001025; -- Felguard Butcher
UPDATE `creature` SET `orientation` = 1.5319 WHERE `guid` = 6001018; -- Eredar Sorcerer
UPDATE `creature` SET `orientation` = 3.5664 WHERE `guid` = 6001047; -- Jace Darkweaver
UPDATE `creature` SET `orientation` = 5.6221 WHERE `guid` = 6001044; -- Demon Hunter
UPDATE `creature` SET `orientation` = 4.0544 WHERE `guid` = 6001046; -- Ashtongue Warrior
UPDATE `creature` SET `orientation` = 5.2785 WHERE `guid` = 6001043; -- Coilskar Sea-Caller
-- north squad
UPDATE `creature` SET `orientation` = 0.2578 WHERE `guid` = 6001010; -- Felguard Butcher
UPDATE `creature` SET `orientation` = 6.1362 WHERE `guid` = 6001014; -- Felguard Butcher
UPDATE `creature` SET `orientation` = 5.8641 WHERE `guid` = 6001019; -- Felguard Butcher
UPDATE `creature` SET `orientation` = 1.2049 WHERE `guid` = 6001029; -- Felguard Butcher
UPDATE `creature` SET `orientation` = 0.6868 WHERE `guid` = 6001015; -- Felguard Butcher
UPDATE `creature` SET `orientation` = 5.2791 WHERE `guid` = 6001009; -- Felguard Butcher
UPDATE `creature` SET `orientation` = 5.9081 WHERE `guid` = 6001013; -- Eredar Sorcerer
UPDATE `creature` SET `orientation` = 2.7665 WHERE `guid` = 6001041; -- Demon Hunter
UPDATE `creature` SET `orientation` = 3.2261 WHERE `guid` = 6001036; -- Demon Hunter
UPDATE `creature` SET `orientation` = 2.9957 WHERE `guid` = 6001042; -- Ashtongue Mystic
UPDATE `creature` SET `orientation` = 3.2054 WHERE `guid` = 6001038; -- Coilskar Myrmidon
-- east squad
UPDATE `creature` SET `orientation` = 6.2406 WHERE `guid` = 6001007; -- Felguard Butcher
UPDATE `creature` SET `orientation` = 0.376 WHERE `guid` = 6001006; -- Felguard Butcher
UPDATE `creature` SET `orientation` = 5.5678 WHERE `guid` = 6001002; -- Felguard Butcher
UPDATE `creature` SET `orientation` = 0.7078 WHERE `guid` = 6001003; -- Felguard Butcher
UPDATE `creature` SET `orientation` = 5.0312 WHERE `guid` = 6001005; -- Felguard Butcher
UPDATE `creature` SET `orientation` = 0.1737 WHERE `guid` = 6001004; -- Eredar Sorcerer
UPDATE `creature` SET `orientation` = 3.3153 WHERE `guid` = 6001035; -- Demon Hunter
UPDATE `creature` SET `orientation` = 2.96 WHERE `guid` = 6001034; -- Ashtongue Warrior
UPDATE `creature` SET `orientation` = 2.5774 WHERE `guid` = 6001033; -- Coilskar Myrmidon

UPDATE `creature` SET `StringId` = 'CrypticHollowSquad', `ScriptName` = 'npc_cryptic_hollow_squad_fighter' WHERE `guid` IN (6001022, 6001012, 6001016, 6001021, 6001024, 6001023, 6001025, 6001018, 6001047, 6001044, 6001046, 6001043, 6001010, 6001014, 6001019, 6001029, 6001015, 6001009, 6001013, 6001041, 6001036, 6001042, 6001038, 6001007, 6001006, 6001002, 6001003, 6001005, 6001004, 6001035, 6001034, 6001033);

-- Undo the emote-only version of this change (looping attack emotes and SmartAI react states)
UPDATE `creature_addon` SET `emote` = 0 WHERE `guid` IN (6001022, 6001012, 6001016, 6001021, 6001024, 6001023, 6001025, 6001018, 6001047, 6001044, 6001046, 6001043, 6001010, 6001014, 6001019, 6001029, 6001015, 6001009, 6001013, 6001041, 6001036, 6001042, 6001038, 6001007, 6001006, 6001002, 6001003, 6001005, 6001004, 6001035, 6001034, 6001033) AND `emote` IN (474, 567, 596);
DELETE FROM `smart_scripts` WHERE `source_type` = 0 AND `entryorguid` IN (-6001022, -6001012, -6001016, -6001021, -6001024, -6001023, -6001025, -6001018, -6001047, -6001044, -6001046, -6001043, -6001010, -6001014, -6001019, -6001029, -6001015, -6001009, -6001013, -6001041, -6001036, -6001042, -6001038, -6001007, -6001006, -6001002, -6001003, -6001005, -6001004, -6001035, -6001034, -6001033);
UPDATE `creature_template` SET `AIName` = '' WHERE `entry` IN (96494, 96473, 102726) AND `AIName` = 'SmartAI';
