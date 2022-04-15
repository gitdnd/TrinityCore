ALTER TABLE `item_generator_legendary_template`
	ADD COLUMN `socketMod` SMALLINT(3) NOT NULL DEFAULT '0' AFTER `secondaryStatModifier`,
	ADD COLUMN `prismaticSocket` SMALLINT(3) NOT NULL DEFAULT '0' AFTER `socketMod`,
	ADD COLUMN `primaryStatCountMod` SMALLINT(3) NOT NULL DEFAULT '0' AFTER `prismaticSocket`,
	ADD COLUMN `secondaryStatCountMod` SMALLINT(3) NOT NULL DEFAULT '0' AFTER `primaryStatCountMod`;
