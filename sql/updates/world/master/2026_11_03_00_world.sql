-- Retake Faronaar (36811): the Legion Portal takes its lock 99 (Open Attacking) back, reverting 2026_11_02_24, now that
-- the server starts the "Attacking" cast (8386) when the client reports a click on the open portal.
UPDATE `gameobject_template` SET `Data0`=99 WHERE `entry`=239676;
