DROP TABLE IF EXISTS `mythic_plus_dungeon_snapshot`;
CREATE TABLE `mythic_plus_dungeon_snapshot`(
	`id` int unsigned NOT NULL,
	`map` smallint unsigned NOT NULL,
	`mapdifficulty` smallint unsigned NOT NULL,
	`starttime` bigint unsigned NOT NULL,
	`snaptime` bigint unsigned NOT NULL,
	`combattime` int unsigned NOT NULL,
	`char_guid` int unsigned NOT NULL,
	`char_name` varchar(12) NOT NULL,
	`mythiclevel` smallint unsigned NOT NULL,
	`creature_entry` int unsigned NOT NULL,
	`creature_final_boss` tinyint unsigned NOT NULL DEFAULT '0',
	`rewarded` tinyint unsigned NOT NULL DEFAULT '0'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;