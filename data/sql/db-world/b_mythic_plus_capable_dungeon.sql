DROP TABLE IF EXISTS `mythic_plus_capable_dungeon`;
CREATE TABLE `mythic_plus_capable_dungeon`(
	`map` smallint unsigned NOT NULL,
	`mapdifficulty` smallint unsigned NOT NULL,
	`final_boss_entry` int unsigned NOT NULL,
	PRIMARY KEY (`map`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

INSERT INTO `mythic_plus_capable_dungeon` (`map`, `mapdifficulty`, `final_boss_entry`) VALUES (658, 0, 36658);
INSERT INTO `mythic_plus_capable_dungeon` (`map`, `mapdifficulty`, `final_boss_entry`) VALUES (632, 0, 36502);
INSERT INTO `mythic_plus_capable_dungeon` (`map`, `mapdifficulty`, `final_boss_entry`) VALUES (619, 0, 29311);
INSERT INTO `mythic_plus_capable_dungeon` (`map`, `mapdifficulty`, `final_boss_entry`) VALUES (601, 0, 29120);
INSERT INTO `mythic_plus_capable_dungeon` (`map`, `mapdifficulty`, `final_boss_entry`) VALUES (600, 0, 26632);
INSERT INTO `mythic_plus_capable_dungeon` (`map`, `mapdifficulty`, `final_boss_entry`) VALUES (604, 0, 29306);
INSERT INTO `mythic_plus_capable_dungeon` (`map`, `mapdifficulty`, `final_boss_entry`) VALUES (602, 0, 28923);
INSERT INTO `mythic_plus_capable_dungeon` (`map`, `mapdifficulty`, `final_boss_entry`) VALUES (599, 0, 27978);
INSERT INTO `mythic_plus_capable_dungeon` (`map`, `mapdifficulty`, `final_boss_entry`) VALUES (576, 0, 26723);
INSERT INTO `mythic_plus_capable_dungeon` (`map`, `mapdifficulty`, `final_boss_entry`) VALUES (578, 0, 27656);
INSERT INTO `mythic_plus_capable_dungeon` (`map`, `mapdifficulty`, `final_boss_entry`) VALUES (574, 0, 23954);
INSERT INTO `mythic_plus_capable_dungeon` (`map`, `mapdifficulty`, `final_boss_entry`) VALUES (575, 0, 26861);
INSERT INTO `mythic_plus_capable_dungeon` (`map`, `mapdifficulty`, `final_boss_entry`) VALUES (389, 0, 11519);
INSERT INTO `mythic_plus_capable_dungeon` (`map`, `mapdifficulty`, `final_boss_entry`) VALUES (47, 0, 4421);
