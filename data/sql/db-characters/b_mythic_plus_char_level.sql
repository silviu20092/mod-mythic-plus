DROP TABLE IF EXISTS `mythic_plus_char_level`;
CREATE TABLE `mythic_plus_char_level`(
	`guid` int unsigned NOT NULL,
	`mythiclevel` int unsigned NOT NULL,
	PRIMARY KEY (`guid`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;