-- Retake Faronaar (36811): two Legion Portals and two Libram of Fel Knowledge books instead of fourteen objects (the
-- user's pass). The portal template is startOpen, which this core does not apply to goobers, so the portals spawn open
-- to show the fel swirl.
DELETE FROM `gameobject` WHERE `guid` IN (12201200, 12201202, 12201204, 12201205, 12201206, 12201207, 12201208, 12201211, 12201212, 12201213);
UPDATE `gameobject` SET `state`=0 WHERE `guid` IN (12201201, 12201203);
