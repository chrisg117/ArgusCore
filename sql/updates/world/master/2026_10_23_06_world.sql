-- Rise of the Illidari (38690) and Fel Infusion (38689), the two quests after Breaking Out, had nothing to start or do
-- them with. Kayn Sunfury (92980) and Altruis the Sufferer (92986) give them at the Illidari Ward's entrance; Kayn
-- (96665) and Altruis end them in the Tomb of the Penitent, from their Wowhead pins (floor z -281.4, checked with the
-- agent character).
--
-- The ward's cells: the Vault model repeats the ward's cell block every 33.3 yd downwards, and has its crystal cell doors
-- as model doodads only in the copies below the walkable one, where retail has objects instead. Every cell of the
-- walkable copy (48, on its two levels) is the doodad of the copy below moved 33.3 yd up: 8 clickable Warden Cells
-- (244588), each on the stack of a Wowhead pin, at the level the retail video frees it from, holding one of the named
-- Illidari of the quest's optional objectives; the other 40 dormant (244596). A cell frees its prisoner once per player
-- (go_rise_of_the_illidari_warden_cell), who answers with their retail line. The demons stand on the ward's lower floor
-- at their Wowhead pins; demons slain in the Vault fill Fel Infusion's bar (quest_fel_infusion, player_fel_infusion).
-- Freed Illidari (96656) fight the demons beside them there without end (npc_mardum_squad_fighter), and Legion Portals
-- (99501) ring the floor, both at their Wowhead pins.
--
-- The Vault's bonus objective, Vault of the Wardens (39742), is given on entering the Vault and rewarded once its bar is
-- full (player_ / quest_vault_of_the_wardens_bonus_objective); its demons give its kill credits. The Legion Portals
-- stand open (emote state 559: their frame's looping animation 149) and are closed by clicking them, which channels
-- Destroying Legion Portal (202064) and destroys the portal as the player's kill, as in the retail Fel Infusion video.

DELETE FROM `creature_queststarter` WHERE (`id` = 92980 AND `quest` = 38690) OR (`id` = 92986 AND `quest` = 38689);
INSERT INTO `creature_queststarter` (`id`, `quest`, `VerifiedBuild`) VALUES
(92980, 38690, 0),
(92986, 38689, 0);

DELETE FROM `quest_template_addon` WHERE `ID` IN (38690, 38689, 39742);
INSERT INTO `quest_template_addon` (`ID`, `PrevQuestID`, `ScriptName`) VALUES
(38690, 38672, ''),
(38689, 38672, 'quest_fel_infusion'),
(39742, 0, 'quest_vault_of_the_wardens_bonus_objective');

-- Legion Portal: clicked to close it (a normal enemy of the bonus objective, a guess: the video shows its little XP)
UPDATE `creature_template` SET `npcflag` = 16777216, `KillCredit1` = 97969 WHERE `entry` = 99501;
DELETE FROM `creature_template_addon` WHERE `entry` = 99501;
INSERT INTO `creature_template_addon` (`entry`, `emote`) VALUES
(99501, 559);
DELETE FROM `npc_spellclick_spells` WHERE `npc_entry` = 99501;
INSERT INTO `npc_spellclick_spells` (`npc_entry`, `spell_id`, `cast_flags`, `user_type`) VALUES
(99501, 202064, 1, 0);
DELETE FROM `spell_script_names` WHERE `spell_id` = 202064;
INSERT INTO `spell_script_names` (`spell_id`, `ScriptName`) VALUES
(202064, 'spell_vault_of_the_wardens_destroying_legion_portal');

-- Kayn has to show he has a quest
UPDATE `creature_template` SET `npcflag` = 2 WHERE `entry` = 92980;
-- The clickable cell belongs to the quest (goober questID), so it glows like a quest object for players on it, and
-- stops for a player once their cell is open
UPDATE `gameobject_template` SET `Data1` = 38690, `ScriptName` = 'go_rise_of_the_illidari_warden_cell' WHERE `entry` = 244588;

DELETE FROM `creature` WHERE `guid` BETWEEN 14680008 AND 14680199;
INSERT INTO `creature` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `modelid`, `equipment_id`, `position_x`, `position_y`, `position_z`, `orientation`, `spawntimesecs`, `wander_distance`, `currentwaypoint`, `curHealthPct`, `MovementType`, `npcflag`, `unit_flags`, `unit_flags2`, `unit_flags3`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(14680008, 92980, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4324.50, -548.00, -281.40, 1.571, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Kayn, ward entrance
(14680009, 92986, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4328.50, -548.00, -281.50, 1.571, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Altruis, ward entrance
(14680040, 96665, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4239.80, -292.30, -281.39, 5.970, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Kayn, Tomb of the Penitent
(14680041, 92986, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4245.30, -296.40, -281.20, 5.970, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Altruis, Tomb of the Penitent
(14680042, 96847, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4327.00, -525.30, -287.30, 1.571, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Drelanim Whisperwind, holding the ward
(14680010, 92776, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4332.48, -472.16, -288.36, 4.418, 60, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0),
(14680011, 92776, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4313.40, -478.29, -288.24, 5.404, 60, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 'npc_mardum_squad_fighter', 'IllidariWardSquad', 0), -- squad
(14680012, 92776, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4354.27, -482.38, -288.53, 3.516, 60, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0),
(14680013, 92776, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4321.57, -486.47, -288.57, 5.274, 60, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0),
(14680014, 92776, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4373.35, -492.60, -282.96, 3.161, 60, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0),
(14680015, 92776, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4291.60, -492.60, -289.38, 6.257, 60, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0),
(14680016, 92776, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4329.75, -494.64, -288.35, 2.846, 60, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0),
(14680017, 92776, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4347.46, -500.77, -287.97, 2.815, 60, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0),
(14680018, 92776, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4343.38, -504.86, -288.31, 2.563, 60, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 'npc_mardum_squad_fighter', 'IllidariWardSquad', 0), -- squad
(14680019, 92776, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4310.68, -506.90, -288.57, 0.719, 60, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0),
(14680020, 92776, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4333.84, -508.95, -288.52, 2.040, 60, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 'npc_mardum_squad_fighter', 'IllidariWardSquad', 0), -- squad
(14680021, 92776, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4325.66, -517.12, -287.21, 1.557, 60, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 'npc_mardum_squad_fighter', 'IllidariWardSquad', 0), -- squad
(14680022, 92782, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4312.04, -449.68, -289.61, 5.021, 60, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0),
(14680023, 92782, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4299.77, -455.81, -287.77, 5.320, 60, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0),
(14680024, 92782, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4361.09, -463.98, -287.23, 3.841, 60, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0),
(14680025, 92782, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4282.06, -466.03, -287.67, 5.724, 60, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0),
(14680026, 92782, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4377.44, -472.16, -277.81, 3.535, 60, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0),
(14680027, 92782, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4331.11, -474.20, -288.34, 4.453, 60, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 'npc_mardum_squad_fighter', 'IllidariWardSquad', 0), -- squad
(14680028, 92782, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4313.40, -484.42, -288.41, 5.659, 60, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 'npc_mardum_squad_fighter', 'IllidariWardSquad', 0), -- squad
(14680029, 92782, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4339.29, -488.51, -288.42, 3.501, 60, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 'npc_mardum_squad_fighter', 'IllidariWardSquad', 0), -- squad
(14680030, 92782, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4386.98, -490.55, -289.64, 3.190, 60, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0),
(14680031, 92782, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4287.51, -492.60, -288.68, 6.260, 60, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0),
(14680032, 92782, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4260.26, -494.64, -288.81, 0.017, 60, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 'npc_mardum_squad_fighter', 'IllidariWardSquad', 0), -- squad
(14680033, 92782, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4337.93, -502.82, -288.49, 2.478, 60, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 'npc_mardum_squad_fighter', 'IllidariWardSquad', 0), -- squad
(14680034, 92782, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4272.52, -506.90, -280.96, 0.246, 60, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0),
(14680035, 92782, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4325.66, -513.03, -288.32, 1.554, 60, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 'npc_mardum_squad_fighter', 'IllidariWardSquad', 0), -- squad
(14680036, 92782, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4362.45, -521.21, -287.22, 2.492, 60, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0),
(14680037, 92782, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4327.02, -523.25, -287.47, 1.605, 60, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0),
(14680110, 96656, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 1, 4327.02, -476.25, -288.38, 4.653, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 'npc_mardum_squad_fighter', 'IllidariWardSquad', 0), -- Freed Illidari, squad
(14680111, 96656, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 1, 4307.95, -476.25, -288.18, 5.520, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 'npc_mardum_squad_fighter', 'IllidariWardSquad', 0), -- Freed Illidari, squad
(14680112, 96656, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 1, 4322.94, -478.29, -288.46, 4.911, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 'npc_mardum_squad_fighter', 'IllidariWardSquad', 0), -- Freed Illidari, squad
(14680113, 96656, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 1, 4312.04, -486.47, -288.46, 5.817, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 'npc_mardum_squad_fighter', 'IllidariWardSquad', 0), -- Freed Illidari, squad
(14680114, 96656, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 1, 4332.48, -488.51, -288.57, 3.798, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 'npc_mardum_squad_fighter', 'IllidariWardSquad', 0), -- Freed Illidari, squad
(14680115, 96656, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 1, 4268.44, -490.55, -289.27, 6.232, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 'npc_mardum_squad_fighter', 'IllidariWardSquad', 0), -- Freed Illidari, squad
(14680116, 96656, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 1, 4339.29, -492.60, -288.45, 3.209, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 'npc_mardum_squad_fighter', 'IllidariWardSquad', 0), -- Freed Illidari, squad
(14680117, 96656, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 1, 4264.35, -492.60, -288.85, 6.269, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 'npc_mardum_squad_fighter', 'IllidariWardSquad', 0), -- Freed Illidari, squad
(14680118, 96656, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 1, 4313.40, -496.68, -288.57, 0.248, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 'npc_mardum_squad_fighter', 'IllidariWardSquad', 0), -- Freed Illidari, squad
(14680119, 96656, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 1, 4339.29, -506.90, -288.40, 2.352, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 'npc_mardum_squad_fighter', 'IllidariWardSquad', 0), -- Freed Illidari, squad
(14680120, 96656, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 1, 4317.49, -508.95, -288.57, 1.067, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 'npc_mardum_squad_fighter', 'IllidariWardSquad', 0), -- Freed Illidari, squad
(14680121, 96656, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 1, 4328.39, -515.08, -287.71, 1.681, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 'npc_mardum_squad_fighter', 'IllidariWardSquad', 0), -- Freed Illidari, squad
(14680122, 96656, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 1, 4294.32, -523.25, -287.59, 0.754, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, 'npc_mardum_squad_fighter', 'IllidariWardSquad', 0), -- Freed Illidari, squad
(14680150, 99501, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4344.74, -446.61, -289.55, 4.332, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Legion Portal
(14680151, 99501, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4309.99, -449.68, -289.61, 5.063, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Legion Portal
(14680152, 99501, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4284.11, -470.12, -289.64, 5.774, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Legion Portal
(14680153, 99501, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4366.54, -479.31, -289.55, 3.478, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Legion Portal
(14680154, 99501, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4252.09, -493.62, -281.67, 0.002, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Legion Portal
(14680155, 99501, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4367.22, -509.97, -289.57, 2.761, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Legion Portal
(14680156, 99501, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4282.74, -509.97, -289.64, 0.364, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Legion Portal
(14680090, 97617, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4306.59, -556.79, -281.77, 1.273, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Asha Ravensong in a Warden Cell
(14680091, 97622, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4359.92, -527.30, -274.89, 2.358, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Kor'vas Bloodthorn in a Warden Cell
(14680092, 97618, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4279.98, -526.20, -275.01, 0.618, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Falara Nightsong in a Warden Cell
(14680093, 97616, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4389.41, -512.66, -281.77, 2.848, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Cyana Nightglaive in a Warden Cell
(14680094, 97614, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4279.88, -460.71, -275.01, 5.665, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Allari the Souleater in a Warden Cell
(14680095, 97615, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4291.96, -459.34, -274.89, 5.496, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Jace Darkweaver in a Warden Cell
(14680096, 97620, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4359.92, -459.34, -274.89, 3.931, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0), -- Lyana Darksorrow in a Warden Cell
(14680097, 97619, 1468, 7814, 0, '0', 0, 0, 0, -1, 0, 0, 4345.28, -429.85, -281.77, 4.418, 120, 0, 0, 100, 0, NULL, NULL, NULL, NULL, '', NULL, 0); -- Marius Felbane in a Warden Cell

DELETE FROM `gameobject` WHERE `guid` BETWEEN 14680002 AND 14680099;
INSERT INTO `gameobject` (`guid`, `id`, `map`, `zoneId`, `areaId`, `spawnDifficulties`, `phaseUseFlags`, `PhaseId`, `PhaseGroup`, `terrainSwapMap`, `position_x`, `position_y`, `position_z`, `orientation`, `rotation0`, `rotation1`, `rotation2`, `rotation3`, `spawntimesecs`, `animprogress`, `state`, `ScriptName`, `StringId`, `VerifiedBuild`) VALUES
(14680002, 244588, 1468, 7814, 0, '0', 0, 0, 0, -1, 4306.74, -556.31, -281.77, 1.2758, 0, 0, 0.595524, 0.803337, 60, 255, 1, '', NULL, 0), -- clickable, lower level
(14680003, 244588, 1468, 7814, 0, '0', 0, 0, 0, -1, 4359.56, -526.94, -274.89, 2.3562, 0, 0, 0.923880, 0.382683, 60, 255, 1, '', NULL, 0), -- clickable, upper level
(14680004, 244588, 1468, 7814, 0, '0', 0, 0, 0, -1, 4280.38, -525.91, -275.01, 1.6773, 0, 0, 0.743728, 0.668482, 60, 255, 1, '', NULL, 0), -- clickable, upper level
(14680005, 244588, 1468, 7814, 0, '0', 0, 0, 0, -1, 4388.93, -512.52, -281.77, 2.8466, 0, 0, 0.989144, 0.146946, 60, 255, 1, '', NULL, 0), -- clickable, lower level
(14680006, 244588, 1468, 7814, 0, '0', 0, 0, 0, -1, 4280.28, -461.00, -275.01, 4.6269, 0, 0, 0.736687, -0.676233, 60, 255, 1, '', NULL, 0), -- clickable, upper level
(14680007, 244588, 1468, 7814, 0, '0', 0, 0, 0, -1, 4292.32, -459.70, -274.89, 5.4978, 0, 0, 0.382683, -0.923880, 60, 255, 1, '', NULL, 0), -- clickable, upper level
(14680008, 244588, 1468, 7814, 0, '0', 0, 0, 0, -1, 4359.56, -459.70, -274.89, 3.9270, 0, 0, 0.923880, -0.382683, 60, 255, 1, '', NULL, 0), -- clickable, upper level
(14680009, 244588, 1468, 7814, 0, '0', 0, 0, 0, -1, 4345.14, -430.33, -281.77, 4.4174, 0, 0, 0.803337, -0.595524, 60, 255, 1, '', NULL, 0), -- clickable, lower level
(14680050, 244596, 1468, 7814, 0, '0', 0, 0, 0, -1, 4253.49, -502.85, -281.60, 1.5376, 0, 0, 0.695286, 0.718733, 60, 255, 1, '', NULL, 0),
(14680051, 244596, 1468, 7814, 0, '0', 0, 0, 0, -1, 4253.36, -483.79, -281.62, 4.7229, 0, 0, 0.703395, -0.710799, 60, 255, 1, '', NULL, 0),
(14680052, 244596, 1468, 7814, 0, '0', 0, 0, 0, -1, 4262.99, -512.63, -270.34, 0.2985, 0, 0, 0.148672, 0.988886, 60, 255, 1, '', NULL, 0),
(14680053, 244596, 1468, 7814, 0, '0', 0, 0, 0, -1, 4262.99, -512.63, -281.77, 0.2985, 0, 0, 0.148672, 0.988886, 60, 255, 1, '', NULL, 0),
(14680054, 244596, 1468, 7814, 0, '0', 0, 0, 0, -1, 4262.95, -474.12, -270.34, 5.9882, 0, 0, 0.146946, -0.989144, 60, 255, 1, '', NULL, 0),
(14680055, 244596, 1468, 7814, 0, '0', 0, 0, 0, -1, 4262.95, -474.12, -281.77, 5.9882, 0, 0, 0.146946, -0.989144, 60, 255, 1, '', NULL, 0),
(14680056, 244596, 1468, 7814, 0, '0', 0, 0, 0, -1, 4280.38, -525.91, -287.73, 1.6773, 0, 0, 0.743728, 0.668482, 60, 255, 1, '', NULL, 0),
(14680057, 244596, 1468, 7814, 0, '0', 0, 0, 0, -1, 4280.28, -461.00, -287.73, 4.6269, 0, 0, 0.736687, -0.676233, 60, 255, 1, '', NULL, 0),
(14680058, 244596, 1468, 7814, 0, '0', 0, 0, 0, -1, 4292.32, -526.94, -274.89, 0.7854, 0, 0, 0.382683, 0.923880, 60, 255, 1, '', NULL, 0),
(14680059, 244596, 1468, 7814, 0, '0', 0, 0, 0, -1, 4292.23, -527.03, -287.57, 0.7854, 0, 0, 0.382683, 0.923880, 60, 255, 1, '', NULL, 0),
(14680060, 244596, 1468, 7814, 0, '0', 0, 0, 0, -1, 4292.23, -459.61, -287.57, 5.4978, 0, 0, 0.382683, -0.923880, 60, 255, 1, '', NULL, 0),
(14680061, 244596, 1468, 7814, 0, '0', 0, 0, 0, -1, 4293.35, -447.75, -275.01, 0.1065, 0, 0, 0.053207, 0.998583, 60, 255, 1, '', NULL, 0),
(14680062, 244596, 1468, 7814, 0, '0', 0, 0, 0, -1, 4293.35, -447.75, -287.73, 0.1065, 0, 0, 0.053207, 0.998583, 60, 255, 1, '', NULL, 0),
(14680063, 244596, 1468, 7814, 0, '0', 0, 0, 0, -1, 4293.63, -538.99, -275.01, 6.1977, 0, 0, 0.042748, -0.999086, 60, 255, 1, '', NULL, 0),
(14680064, 244596, 1468, 7814, 0, '0', 0, 0, 0, -1, 4293.63, -538.99, -287.73, 6.1977, 0, 0, 0.042748, -0.999086, 60, 255, 1, '', NULL, 0),
(14680065, 244596, 1468, 7814, 0, '0', 0, 0, 0, -1, 4306.74, -556.31, -270.34, 1.2758, 0, 0, 0.595524, 0.803337, 60, 255, 1, '', NULL, 0),
(14680066, 244596, 1468, 7814, 0, '0', 0, 0, 0, -1, 4306.63, -430.36, -270.34, 5.0108, 0, 0, 0.594121, -0.804376, 60, 255, 1, '', NULL, 0),
(14680067, 244596, 1468, 7814, 0, '0', 0, 0, 0, -1, 4306.63, -430.36, -281.77, 5.0108, 0, 0, 0.594121, -0.804376, 60, 255, 1, '', NULL, 0),
(14680068, 244596, 1468, 7814, 0, '0', 0, 0, 0, -1, 4316.41, -565.90, -281.62, 0.0105, 0, 0, 0.005236, 0.999986, 60, 255, 1, '', NULL, 0),
(14680069, 244596, 1468, 7814, 0, '0', 0, 0, 0, -1, 4316.41, -420.86, -281.60, 6.2500, 0, 0, 0.016580, -0.999863, 60, 255, 1, '', NULL, 0),
(14680070, 244596, 1468, 7814, 0, '0', 0, 0, 0, -1, 4335.48, -565.77, -281.60, 3.1084, 0, 0, 0.999863, 0.016580, 60, 255, 1, '', NULL, 0),
(14680071, 244596, 1468, 7814, 0, '0', 0, 0, 0, -1, 4335.47, -420.74, -281.62, 3.1521, 0, 0, 0.999986, -0.005236, 60, 255, 1, '', NULL, 0),
(14680072, 244596, 1468, 7814, 0, '0', 0, 0, 0, -1, 4345.14, -430.33, -270.34, 4.4174, 0, 0, 0.803337, -0.595524, 60, 255, 1, '', NULL, 0),
(14680073, 244596, 1468, 7814, 0, '0', 0, 0, 0, -1, 4345.25, -556.28, -270.34, 1.8692, 0, 0, 0.804376, 0.594121, 60, 255, 1, '', NULL, 0),
(14680074, 244596, 1468, 7814, 0, '0', 0, 0, 0, -1, 4345.25, -556.28, -281.77, 1.8692, 0, 0, 0.804376, 0.594121, 60, 255, 1, '', NULL, 0),
(14680075, 244596, 1468, 7814, 0, '0', 0, 0, 0, -1, 4358.26, -447.65, -275.01, 3.0561, 0, 0, 0.999086, 0.042748, 60, 255, 1, '', NULL, 0),
(14680076, 244596, 1468, 7814, 0, '0', 0, 0, 0, -1, 4358.26, -447.65, -287.73, 3.0561, 0, 0, 0.999086, 0.042748, 60, 255, 1, '', NULL, 0),
(14680077, 244596, 1468, 7814, 0, '0', 0, 0, 0, -1, 4358.54, -538.89, -275.01, 3.2481, 0, 0, 0.998583, -0.053207, 60, 255, 1, '', NULL, 0),
(14680078, 244596, 1468, 7814, 0, '0', 0, 0, 0, -1, 4358.54, -538.89, -287.73, 3.2481, 0, 0, 0.998583, -0.053207, 60, 255, 1, '', NULL, 0),
(14680079, 244596, 1468, 7814, 0, '0', 0, 0, 0, -1, 4359.66, -527.03, -287.57, 2.3562, 0, 0, 0.923880, 0.382683, 60, 255, 1, '', NULL, 0),
(14680080, 244596, 1468, 7814, 0, '0', 0, 0, 0, -1, 4359.66, -459.61, -287.57, 3.9270, 0, 0, 0.923880, -0.382683, 60, 255, 1, '', NULL, 0),
(14680081, 244596, 1468, 7814, 0, '0', 0, 0, 0, -1, 4371.51, -460.73, -275.01, 4.8189, 0, 0, 0.668482, -0.743728, 60, 255, 1, '', NULL, 0),
(14680082, 244596, 1468, 7814, 0, '0', 0, 0, 0, -1, 4371.51, -460.73, -287.73, 4.8189, 0, 0, 0.668482, -0.743728, 60, 255, 1, '', NULL, 0),
(14680083, 244596, 1468, 7814, 0, '0', 0, 0, 0, -1, 4371.61, -525.63, -275.01, 1.4853, 0, 0, 0.676233, 0.736687, 60, 255, 1, '', NULL, 0),
(14680084, 244596, 1468, 7814, 0, '0', 0, 0, 0, -1, 4371.61, -525.63, -287.73, 1.4853, 0, 0, 0.676233, 0.736687, 60, 255, 1, '', NULL, 0),
(14680085, 244596, 1468, 7814, 0, '0', 0, 0, 0, -1, 4388.93, -512.52, -270.34, 2.8466, 0, 0, 0.989144, 0.146946, 60, 255, 1, '', NULL, 0),
(14680086, 244596, 1468, 7814, 0, '0', 0, 0, 0, -1, 4388.90, -474.01, -270.34, 3.4400, 0, 0, 0.988886, -0.148672, 60, 255, 1, '', NULL, 0),
(14680087, 244596, 1468, 7814, 0, '0', 0, 0, 0, -1, 4388.90, -474.01, -281.77, 3.4400, 0, 0, 0.988886, -0.148672, 60, 255, 1, '', NULL, 0),
(14680088, 244596, 1468, 7814, 0, '0', 0, 0, 0, -1, 4398.40, -483.79, -281.60, 4.6792, 0, 0, 0.718733, -0.695286, 60, 255, 1, '', NULL, 0),
(14680089, 244596, 1468, 7814, 0, '0', 0, 0, 0, -1, 4398.52, -502.85, -281.62, 1.5813, 0, 0, 0.710799, 0.703395, 60, 255, 1, '', NULL, 0);

DELETE FROM `creature_text` WHERE `CreatureID` IN (97614, 97615, 97616, 97617, 97618, 97619, 97620, 97622);
INSERT INTO `creature_text` (`CreatureID`, `GroupID`, `ID`, `Text`, `Type`, `Language`, `Probability`, `Emote`, `Duration`, `Sound`, `SoundPlayType`, `BroadcastTextId`, `TextRange`, `comment`) VALUES
(97614, 0, 0, 'The Wardens seek our help? Things must be desperate.', 12, 0, 100, 0, 0, 57334, 0, 98708, 0, 'Allari the Souleater - freed'),
(97615, 0, 0, 'I will go ahead and secure the elevators.', 12, 0, 100, 0, 0, 57292, 0, 99504, 0, 'Jace Darkweaver - freed'),
(97616, 0, 0, 'Years and years with only the voices in my head. I thought I would go insane.', 12, 0, 100, 0, 0, 57290, 0, 98712, 0, 'Cyana Nightglaive - freed'),
(97617, 0, 0, 'Ahhhh, it feels good to MOVE again. Thank you, $n.', 12, 0, 100, 0, 0, 57287, 0, 98709, 0, 'Asha Ravensong - freed'),
(97618, 0, 0, 'We will need weapons and supplies if we are to break out of here. I am on it, $n.', 12, 0, 100, 0, 0, 57289, 0, 98711, 0, 'Falara Nightsong - freed'),
(97619, 0, 0, 'I sense demons nearby. After all that time in stasis, I am ready to hunt.', 12, 0, 100, 0, 0, 57344, 0, 99505, 0, 'Marius Felbane - freed'),
(97620, 0, 0, 'My blades are ready, $n. I will check on the situation ahead.', 12, 0, 100, 0, 0, 57293, 0, 99506, 0, 'Lyana Darksorrow - freed'),
(97622, 0, 0, '$n, it has been a long time since Mardum. I am ready for another fight.', 12, 0, 100, 0, 0, 57288, 0, 98710, 0, 'Kor''vas Bloodthorn - freed');

-- The freed Kayn and Altruis stand in several rooms: by the lever during Breaking Out, at the ward's entrance once it is
-- done, and Altruis also in the Tomb of the Penitent. Visibility conditions go by entry, so each spawn is told apart by
-- what stands beside it (the lever, or Drelanim holding the ward).
DELETE FROM `conditions` WHERE `SourceTypeOrReferenceId` = 32 AND `SourceGroup` = 3 AND `SourceEntry` IN (92980, 92986, 96665);
INSERT INTO `conditions` (`SourceTypeOrReferenceId`, `SourceGroup`, `SourceEntry`, `SourceId`, `ElseGroup`, `ConditionTypeOrReference`, `ConditionTarget`, `ConditionValue1`, `ConditionValue2`, `ConditionValue3`, `ConditionStringValue1`, `NegativeCondition`, `ErrorType`, `ErrorTextId`, `ScriptName`, `Comment`) VALUES
(32, 3, 92980, 0, 0, 30, 1, 244923, 15, 0, '', 0, 0, 0, '', 'Kayn (freed): by the lever'),
(32, 3, 92980, 0, 0, 48, 0, 278275, 0, 1, '', 0, 0, 0, '', 'Kayn (freed): by the lever once freed'),
(32, 3, 92980, 0, 1, 30, 1, 244923, 15, 0, '', 0, 0, 0, '', 'Kayn (freed): by the lever'),
(32, 3, 92980, 0, 1, 47, 0, 38672, 2, 0, '', 0, 0, 0, '', 'Kayn (freed): by the lever while Breaking Out is complete'),
(32, 3, 92980, 0, 2, 29, 1, 96847, 30, 0, '', 0, 0, 0, '', 'Kayn (freed): at the ward'),
(32, 3, 92980, 0, 2, 47, 0, 38672, 64, 0, '', 0, 0, 0, '', 'Kayn (freed): at the ward once Breaking Out is done'),
(32, 3, 92980, 0, 2, 47, 0, 38690, 66, 0, '', 1, 0, 0, '', 'Kayn (freed): at the ward until Rise of the Illidari is complete'),
(32, 3, 92986, 0, 0, 30, 1, 244923, 15, 0, '', 0, 0, 0, '', 'Altruis (freed): by the lever'),
(32, 3, 92986, 0, 0, 48, 0, 278274, 0, 1, '', 0, 0, 0, '', 'Altruis (freed): by the lever once freed'),
(32, 3, 92986, 0, 1, 30, 1, 244923, 15, 0, '', 0, 0, 0, '', 'Altruis (freed): by the lever'),
(32, 3, 92986, 0, 1, 47, 0, 38672, 2, 0, '', 0, 0, 0, '', 'Altruis (freed): by the lever while Breaking Out is complete'),
(32, 3, 92986, 0, 2, 29, 1, 96847, 30, 0, '', 0, 0, 0, '', 'Altruis (freed): at the ward'),
(32, 3, 92986, 0, 2, 47, 0, 38672, 64, 0, '', 0, 0, 0, '', 'Altruis (freed): at the ward once Breaking Out is done'),
(32, 3, 92986, 0, 2, 47, 0, 38689, 66, 0, '', 1, 0, 0, '', 'Altruis (freed): at the ward until Fel Infusion is complete'),
(32, 3, 92986, 0, 3, 30, 1, 244923, 15, 0, '', 1, 0, 0, '', 'Altruis (freed): in the Tomb of the Penitent (not by the lever)'),
(32, 3, 92986, 0, 3, 29, 1, 96847, 30, 0, '', 1, 0, 0, '', 'Altruis (freed): in the Tomb of the Penitent (not at the ward)'),
(32, 3, 92986, 0, 3, 47, 0, 38672, 64, 0, '', 0, 0, 0, '', 'Altruis (freed): in the Tomb of the Penitent once Breaking Out is done'),
(32, 3, 96665, 0, 0, 47, 0, 38672, 64, 0, '', 0, 0, 0, '', 'Kayn (Tomb of the Penitent): once Breaking Out is done');

-- The gold minimap dot on each caged Illidari: the 7.3.5 client gives a creature a quest dot when an objective of a
-- quest the player is on asks for it and has flag 0x1 (tracked on minimap). The eight named prisoners are such
-- objectives (optional, hidden), so they get the flag; the dot goes once that prisoner is freed.
UPDATE `quest_objectives` SET `Flags` = `Flags` | 1 WHERE `QuestID` = 38690 AND `ID` BETWEEN 280065 AND 280072;

