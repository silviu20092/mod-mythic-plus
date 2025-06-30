ALTER TABLE `mythic_plus_level` ADD `random_affix_count` int unsigned DEFAULT '0';

INSERT INTO `mythic_plus_level` (`lvl`, `timelimit`, `random_affix_count`) VALUES (7, 60*40, 1);

INSERT INTO `mythic_plus_affix` (`lvl`, `affixtype`, `val1`) VALUES (7, 1, 0.2);
INSERT INTO `mythic_plus_affix` (`lvl`, `affixtype`, `val1`) VALUES (7, 2, 0.15);
INSERT INTO `mythic_plus_affix` (`lvl`, `affixtype`, `val1`) VALUES (7, 3, 30);
INSERT INTO `mythic_plus_affix` (`lvl`, `affixtype`, `val1`) VALUES (7, 4, 30);
INSERT INTO `mythic_plus_affix` (`lvl`, `affixtype`, `val1`, `val2`) VALUES (7, 6, 25000, 65);

INSERT INTO `mythic_plus_level_rewards` (`lvl`, `rewardtype`, `val1`, `val2`) VALUES (7, 0, 30000000, null);
INSERT INTO `mythic_plus_level_rewards` (`lvl`, `rewardtype`, `val1`, `val2`) VALUES (7, 1, 29434, 30);
