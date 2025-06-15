DROP TABLE IF EXISTS `mythic_plus_level`;
CREATE TABLE `mythic_plus_level`(
	`lvl` int unsigned NOT NULL,
	`timelimit` int unsigned NOT NULL,
	PRIMARY KEY (`lvl`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

INSERT INTO `mythic_plus_level` (`lvl`, `timelimit`) VALUES (1, 60*45);
INSERT INTO `mythic_plus_level` (`lvl`, `timelimit`) VALUES (2, 60*45);
INSERT INTO `mythic_plus_level` (`lvl`, `timelimit`) VALUES (3, 60*40);
INSERT INTO `mythic_plus_level` (`lvl`, `timelimit`) VALUES (4, 60*40);
INSERT INTO `mythic_plus_level` (`lvl`, `timelimit`) VALUES (5, 60*40);
