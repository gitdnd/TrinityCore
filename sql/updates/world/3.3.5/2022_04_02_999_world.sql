CREATE TABLE `item_generator_legendary_template` (
	`Id` INT(10) UNSIGNED NOT NULL,
	`minItemLevel` INT(10) NOT NULL,
	`maxItemLevel` INT(10) NOT NULL,
	`itemClass` SMALLINT(6) NOT NULL DEFAULT '0',
	`itemSubClass` SMALLINT(6) NOT NULL DEFAULT '0',
	`itemInventoryType` SMALLINT(6) NOT NULL DEFAULT '0',
	`itemStatGroup` SMALLINT(6) NOT NULL DEFAULT '0',
	`primaryStatModifier` FLOAT UNSIGNED NOT NULL DEFAULT '0',
	`secondaryStatModifier` FLOAT UNSIGNED NOT NULL DEFAULT '0',
	PRIMARY KEY (`Id`) USING BTREE
)
COLLATE='latin1_swedish_ci'
ENGINE=InnoDB
;

CREATE TABLE `item_generator_legendary_spell_entry` (
	`legendaryIndex` INT(10) UNSIGNED NOT NULL,
	`spellIndex` SMALLINT(5) UNSIGNED NOT NULL DEFAULT '0',
	`SpellId` INT(10) UNSIGNED NOT NULL,
	`SpellTrigger` INT(10) UNSIGNED NOT NULL,
	`SpellCharges` INT(10) NOT NULL,
	`SpellPPMRate` FLOAT NOT NULL DEFAULT '0',
	`SpellCooldown` INT(10) NOT NULL,
	`SpellCategory` INT(10) UNSIGNED NOT NULL,
	`SpellCategoryCooldown` INT(10) NOT NULL,
	PRIMARY KEY (`legendaryIndex`, `spellIndex`) USING BTREE
)
COLLATE='latin1_swedish_ci'
ENGINE=InnoDB
;
