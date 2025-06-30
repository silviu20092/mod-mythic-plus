DROP TABLE IF EXISTS `mythic_plus_keystone_timer`;
CREATE TABLE `mythic_plus_keystone_timer`(
	`guid` int unsigned NOT NULL,
	`buytime` bigint unsigned NOT NULL,
	PRIMARY KEY (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;