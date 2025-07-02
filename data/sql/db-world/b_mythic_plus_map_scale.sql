DROP TABLE IF EXISTS `mythic_plus_map_scale`;
CREATE TABLE `mythic_plus_map_scale`(
	`map` smallint unsigned NOT NULL,
	`mapdifficulty` smallint unsigned NOT NULL,
	`dmg_scale_trash` float NOT NULL default '1',
	`dmg_scale_boss` float NOT NULL default '1'
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

insert into `mythic_plus_map_scale`(`map`, `mapdifficulty`, `dmg_scale_trash`, `dmg_scale_boss`) values (389, 0, 4, 4.2);
insert into `mythic_plus_map_scale`(`map`, `mapdifficulty`, `dmg_scale_trash`, `dmg_scale_boss`) values (47, 0, 4, 4.2);
