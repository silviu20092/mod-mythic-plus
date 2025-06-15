DROP TABLE IF EXISTS `mythic_plus_level_rewards`;
CREATE TABLE `mythic_plus_level_rewards`(
	`lvl` int unsigned NOT NULL,
	`rewardtype` smallint unsigned NOT NULL,
	`val1` int unsigned NOT NULL,
	`val2` int unsigned
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

INSERT INTO `mythic_plus_level_rewards` (`lvl`, `rewardtype`, `val1`, `val2`) VALUES (1, 0, 1000000, null);
INSERT INTO `mythic_plus_level_rewards` (`lvl`, `rewardtype`, `val1`, `val2`) VALUES (1, 1, 29434, 1);
INSERT INTO `mythic_plus_level_rewards` (`lvl`, `rewardtype`, `val1`, `val2`) VALUES (2, 0, 4000000, null);
INSERT INTO `mythic_plus_level_rewards` (`lvl`, `rewardtype`, `val1`, `val2`) VALUES (2, 1, 29434, 2);
INSERT INTO `mythic_plus_level_rewards` (`lvl`, `rewardtype`, `val1`, `val2`) VALUES (3, 0, 8000000, null);
INSERT INTO `mythic_plus_level_rewards` (`lvl`, `rewardtype`, `val1`, `val2`) VALUES (3, 1, 29434, 5);
INSERT INTO `mythic_plus_level_rewards` (`lvl`, `rewardtype`, `val1`, `val2`) VALUES (4, 0, 10000000, null);
INSERT INTO `mythic_plus_level_rewards` (`lvl`, `rewardtype`, `val1`, `val2`) VALUES (4, 1, 29434, 10);
INSERT INTO `mythic_plus_level_rewards` (`lvl`, `rewardtype`, `val1`, `val2`) VALUES (5, 0, 15000000, null);
INSERT INTO `mythic_plus_level_rewards` (`lvl`, `rewardtype`, `val1`, `val2`) VALUES (5, 1, 29434, 20);
