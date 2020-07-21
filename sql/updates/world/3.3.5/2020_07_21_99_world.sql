ALTER TABLE `item_generator_spells`
	ADD COLUMN `minItemLevel` INT NOT NULL AFTER `statGroup`,
	ADD COLUMN `maxItemLevel` INT NOT NULL AFTER `minItemLevel`;
