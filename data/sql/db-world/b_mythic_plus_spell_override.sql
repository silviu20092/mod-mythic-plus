DROP TABLE IF EXISTS `mythic_plus_spell_override`;
CREATE TABLE `mythic_plus_spell_override`(
	`spellid` int unsigned NOT NULL,
	`map` int unsigned NOT NULL,
	`modpct` float default '-1',
	`dotmodpct` float default '-1',
	`comment` varchar(255),
	PRIMARY KEY (`spellid`, `map`)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci;

INSERT INTO `mythic_plus_spell_override` (`spellid`, `map`, `modpct`, `dotmodpct`, `comment`) VALUES (9532,389,3.75, -1,'Ragefire Shaman - Lightning Bolt');
INSERT INTO `mythic_plus_spell_override` (`spellid`, `map`, `modpct`, `dotmodpct`, `comment`) VALUES (11968,389,18,-1,'Molten Elemental - Fire Shield');
INSERT INTO `mythic_plus_spell_override` (`spellid`, `map`, `modpct`, `dotmodpct`, `comment`) VALUES (18266,389,-1,8,'Searing Blade Cultist - Curse of Agony');
INSERT INTO `mythic_plus_spell_override` (`spellid`, `map`, `modpct`, `dotmodpct`, `comment`) VALUES (20800,389,5,12,'Jergosh the Invoker - Immolate DOT');
