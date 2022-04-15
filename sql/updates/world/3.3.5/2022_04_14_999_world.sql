ALTER TABLE `item_generator_legendary_template`
	ADD COLUMN `socketMod` SMALLINT(3) NOT NULL DEFAULT '0' AFTER `secondaryStatModifier`,
	ADD COLUMN `prismaticSocket` SMALLINT(3) NOT NULL DEFAULT '0' AFTER `socketMod`,
	ADD COLUMN `primaryStatCountMod` SMALLINT(3) NOT NULL DEFAULT '0' AFTER `prismaticSocket`,
	ADD COLUMN `secondaryStatCountMod` SMALLINT(3) NOT NULL DEFAULT '0' AFTER `primaryStatCountMod`;
	
ALTER TABLE `item_generator_legendary_template`
	CHANGE COLUMN `socketMod` `socketMod` SMALLINT(5) UNSIGNED NOT NULL DEFAULT '0' AFTER `secondaryStatModifier`,
	CHANGE COLUMN `prismaticSocket` `prismaticSocket` SMALLINT(5) UNSIGNED NOT NULL DEFAULT '0' AFTER `socketMod`,
	CHANGE COLUMN `primaryStatCountMod` `primaryStatCountMod` SMALLINT(5) UNSIGNED NOT NULL DEFAULT '0' AFTER `prismaticSocket`,
	CHANGE COLUMN `secondaryStatCountMod` `secondaryStatCountMod` SMALLINT(5) UNSIGNED NOT NULL DEFAULT '0' AFTER `primaryStatCountMod`;
