ALTER TABLE `item_generator_legendary_template`
	ADD COLUMN `statGroupOverride` SMALLINT(5) UNSIGNED NOT NULL DEFAULT '0' AFTER `secondaryStatCountMod`;
