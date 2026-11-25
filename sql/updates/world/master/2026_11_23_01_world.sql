-- Azsuna, Fel Machinations (37656): the freed demon hunter takes the caged one's place for the player who freed it
-- (spell_summon_replace), instead of the hostage fading out for everyone 1 s after the use. The hostage 90487 stays in
-- the world for other players and stays hidden for the freer once the freed one has left (8 s, its SAI).
DELETE FROM `spell_summon_replace` WHERE `SpellID` IN (178934, 180174, 180175, 180176);
INSERT INTO `spell_summon_replace` (`SpellID`, `ReplacedEntry`, `SearchRange`, `KeepReplacedHidden`, `Comment`) VALUES
(178934, 90487, 10, 1, 'Fel Machinations: Summon Demon Hunter - Blood Elf Female replaces the hostage'),
(180174, 90487, 10, 1, 'Fel Machinations: Summon Demon Hunter - Night Elf Female replaces the hostage'),
(180175, 90487, 10, 1, 'Fel Machinations: Summon Demon Hunter - Blood Elf Male replaces the hostage'),
(180176, 90487, 10, 1, 'Fel Machinations: Summon Demon Hunter - Night Elf Male replaces the hostage');

-- the Soul Harvesters no longer despawn the hostage (the summon replaces it); credit and the harvester's own despawn stay
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (240075, 240121, 240122, 240123) AND `source_type` = 1 AND `id` = 1;

-- the freed demon hunters (90532, 90534-90536) speak by rescue number with the follower Kor'vas
-- (npc_fel_machinations_freed_demon_hunter); its 8 s despawn replaces their SmartAI
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (90532, 90534, 90535, 90536) AND `source_type` = 0;
UPDATE `creature_template` SET `AIName` = '', `ScriptName` = 'npc_fel_machinations_freed_demon_hunter' WHERE `entry` IN (90532, 90534, 90535, 90536);

-- the rescue credit moves to the freed demon hunter (npc_fel_machinations_freed_demon_hunter): credited at the use, the sixth
-- credit completed the quest before the cage spell, whose quest check then refused the sixth rescue
DELETE FROM `smart_scripts` WHERE `entryorguid` IN (240075, 240121, 240122, 240123) AND `source_type` = 1 AND `id` = 0;
