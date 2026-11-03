-- Retake Faronaar (36811): the Legion Portal stands open (startOpen) to show its fel swirl, but the client will not start
-- the "Attacking" unlock (lock 99, type 14) on an object it sees as open, so the portal takes no lock and a click uses it.
UPDATE `gameobject_template` SET `Data0`=0 WHERE `entry`=239676;
