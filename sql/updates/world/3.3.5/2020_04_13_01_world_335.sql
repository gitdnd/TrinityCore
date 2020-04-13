CREATE TABLE `item_generator_spells` (
	`spellId` INT UNSIGNED NOT NULL,
	`quality` INT UNSIGNED NOT NULL,
	`itemClass` INT NOT NULL,
	`subClass` INT NOT NULL,
	`inventoryType` INT NOT NULL,
	`SpellTrigger` INT UNSIGNED NOT NULL,
	`SpellCharges` INT NOT NULL,
	`SpellPPMRate` FLOAT NOT NULL DEFAULT 0,
	`SpellCooldown` INT NOT NULL,
	`SpellCategory` INT UNSIGNED NOT NULL,
	`SpellCategoryCooldown` INT NOT NULL,
	UNIQUE INDEX `spellId` (`spellId`, `quality`, `itemClass`, `subClass`, `inventoryType`)
)
COLLATE='latin1_swedish_ci'
;
