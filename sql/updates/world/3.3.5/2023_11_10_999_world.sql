ALTER TABLE `item_generator_legendary_template`
	ADD COLUMN `itemLimitCatagory` SMALLINT(5) NOT NULL DEFAULT '0' AFTER `statGroupOverride`;
