DROP TABLE IF EXISTS `mythic_plus_affix`;
CREATE TABLE `mythic_plus_affix`(
	`lvl` int unsigned NOT NULL,
	`affixtype` smallint unsigned NOT NULL,
	`val1` float,
	`val2` float
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

INSERT INTO `mythic_plus_affix` (`lvl`, `affixtype`, `val1`) VALUES (1, 1, 0.15);
INSERT INTO `mythic_plus_affix` (`lvl`, `affixtype`, `val1`) VALUES (2, 1, 0.15);
INSERT INTO `mythic_plus_affix` (`lvl`, `affixtype`, `val1`) VALUES (2, 2, 0.1);
INSERT INTO `mythic_plus_affix` (`lvl`, `affixtype`, `val1`) VALUES (2, 4, 10);

INSERT INTO `mythic_plus_affix` (`lvl`, `affixtype`, `val1`) VALUES (3, 1, 0.2);
INSERT INTO `mythic_plus_affix` (`lvl`, `affixtype`, `val1`) VALUES (3, 2, 0.15);
INSERT INTO `mythic_plus_affix` (`lvl`, `affixtype`, `val1`) VALUES (3, 3, 15);
INSERT INTO `mythic_plus_affix` (`lvl`, `affixtype`, `val1`) VALUES (3, 4, 20);

INSERT INTO `mythic_plus_affix` (`lvl`, `affixtype`, `val1`) VALUES (4, 1, 0.2);
INSERT INTO `mythic_plus_affix` (`lvl`, `affixtype`, `val1`) VALUES (4, 2, 0.15);
INSERT INTO `mythic_plus_affix` (`lvl`, `affixtype`, `val1`) VALUES (4, 3, 30);
INSERT INTO `mythic_plus_affix` (`lvl`, `affixtype`, `val1`) VALUES (4, 4, 30);

INSERT INTO `mythic_plus_affix` (`lvl`, `affixtype`, `val1`) VALUES (5, 1, 0.2);
INSERT INTO `mythic_plus_affix` (`lvl`, `affixtype`, `val1`) VALUES (5, 2, 0.15);
INSERT INTO `mythic_plus_affix` (`lvl`, `affixtype`, `val1`) VALUES (5, 3, 30);
INSERT INTO `mythic_plus_affix` (`lvl`, `affixtype`, `val1`) VALUES (5, 4, 30);
INSERT INTO `mythic_plus_affix` (`lvl`, `affixtype`, `val1`) VALUES (5, 5, null);
