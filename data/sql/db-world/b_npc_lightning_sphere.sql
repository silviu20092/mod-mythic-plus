SET @Entry = 200006;
SET @Name = "Lightning";
SET @Subname = "Sphere";

DELETE FROM `creature_template_model` WHERE `CreatureID` = @Entry;
DELETE FROM `creature_template` WHERE `entry` = @Entry;

INSERT INTO `creature_template` (`entry`, `name`, `subname`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, `exp`, `faction`, `npcflag`, `scale`, `rank`, `dmgschool`, `baseattacktime`, `rangeattacktime`, `unit_class`, `unit_flags`, `type`, `type_flags`, `lootid`, `pickpocketloot`, `skinloot`, `AIName`, `MovementType`, `HoverHeight`, `RacialLeader`, `movementId`, `RegenHealth`, `mechanic_immune_mask`, `flags_extra`, `HealthModifier`, `ScriptName`) VALUES
(@Entry, @Name, @Subname, null, 0, 80, 80, 2, 14, 0, 1, 1, 0, 2000, 0, 8, 131076, 10, 0, 0, 0, 0, '', 0, 1, 0, 0, 1, 0, 0, 3, 'npc_lightning_sphere');
INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`) VALUES (@Entry, 0, 25144, 1, 1, 0);