-- Archmage Khadgar 113986 had only UNIT_NPC_FLAG_QUESTGIVER, so a click sent CMSG_QUESTGIVER_HELLO, which shows quests
-- and never menu 20457. In the Blink of an Eye (44663 / 44184) needs that menu to start the city's teleportation.
UPDATE `creature_template` SET `npcflag` = `npcflag` | 1 WHERE `entry` = 113986;
