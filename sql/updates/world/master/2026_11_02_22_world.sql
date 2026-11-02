-- Retake Faronaar (36811): the core now opens startOpen goobers itself, so the Legion Portals go back to the normal
-- spawn state; spawned open they offered no interaction.
UPDATE `gameobject` SET `state`=1 WHERE `guid` IN (12201201, 12201203);
