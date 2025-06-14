SET @Entry = 200005;
SET @Name = "Mythic Plus";
SET @Subname = "Master";

DELETE FROM `creature_template_model` WHERE `CreatureID` = @Entry;
DELETE FROM `creature_template` WHERE `entry` = @Entry;

INSERT INTO `creature_template` (`entry`, `name`, `subname`, `IconName`, `gossip_menu_id`, `minlevel`, `maxlevel`, `exp`, `faction`, `npcflag`, `scale`, `rank`, `dmgschool`, `baseattacktime`, `rangeattacktime`, `unit_class`, `unit_flags`, `type`, `type_flags`, `lootid`, `pickpocketloot`, `skinloot`, `AIName`, `MovementType`, `HoverHeight`, `RacialLeader`, `movementId`, `RegenHealth`, `mechanic_immune_mask`, `flags_extra`, `ScriptName`) VALUES
(@Entry, @Name, @Subname, null, 0, 80, 80, 2, 35, 1, 1, 0, 0, 2000, 0, 1, 2147483648, 7, 138936390, 0, 0, 0, '', 0, 1, 0, 0, 1, 0, 0, 'mod_mythic_plus_npc');
INSERT INTO `creature_template_model` (`CreatureID`, `Idx`, `CreatureDisplayID`, `DisplayScale`, `Probability`, `VerifiedBuild`) VALUES (@Entry, 0, 19646, 1, 1, 0);