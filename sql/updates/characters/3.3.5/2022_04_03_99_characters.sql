ALTER TABLE `item_template_virtual`
	ADD COLUMN `legendarySeed` INT(10) UNSIGNED NOT NULL DEFAULT '0' AFTER `sheath`,
	ADD COLUMN `legendaryId` INT(10) UNSIGNED NOT NULL DEFAULT '0' AFTER `legendarySeed`;
