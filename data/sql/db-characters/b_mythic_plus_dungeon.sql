DROP TABLE IF EXISTS `mythic_plus_dungeon`;
CREATE TABLE `mythic_plus_dungeon`(
	`id` int unsigned NOT NULL,
	`map` smallint unsigned NOT NULL,
	`timelimit` int unsigned NOT NULL,
	`starttime` bigint unsigned NOT NULL DEFAULT '0',
	`mythiclevel` smallint unsigned NOT NULL DEFAULT '0',
	`done` tinyint unsigned NOT NULL DEFAULT '0',
	`ismythic` tinyint unsigned NOT NULL DEFAULT '1',
	PRIMARY KEY (`id`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;