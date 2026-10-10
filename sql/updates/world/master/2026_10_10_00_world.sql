-- Broken Shore scenario (map 1460): the cutscene that plays when the Black City has been razed.
--
-- It is scene 1373, which the spell "Stage 3 Scene" (181926) plays. Its script takes the player to the edge
-- of the city when it ends. Its flags, which were sniffed, say that it cannot be skipped (0x04), but a video
-- of the retail scenario shows a player skip it, as every other cutscene of the scenario, so that flag is
-- taken off.
UPDATE `scene_template` SET `Flags`=`Flags`&~0x04, `ScriptName`='scene_broken_shore_alliance_stage_3' WHERE `SceneId`=1373;
