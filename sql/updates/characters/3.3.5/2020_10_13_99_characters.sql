ALTER TABLE `item_template_virtual`
	ADD COLUMN `socketSeed` INT(11) UNSIGNED NOT NULL AFTER `seed`,
	ADD COLUMN `qualitySeed` INT(11) UNSIGNED NOT NULL AFTER `socketSeed`,
	ADD COLUMN `statSeed` INT(11) UNSIGNED NOT NULL AFTER `qualitySeed`,
	ADD COLUMN `nameSeed` INT(11) UNSIGNED NOT NULL AFTER `statSeed`,
	ADD COLUMN `displaySeed` INT(11) UNSIGNED NOT NULL AFTER `nameSeed`,
	ADD COLUMN `spellSeed` INT(11) UNSIGNED NOT NULL AFTER `displaySeed`,
	ADD COLUMN `statValueSeed` INT(11) UNSIGNED NOT NULL AFTER `spellSeed`;
