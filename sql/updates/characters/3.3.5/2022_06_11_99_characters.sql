ALTER TABLE `item_template_virtual`
	ADD COLUMN `generatedMagicFind` FLOAT UNSIGNED NOT NULL DEFAULT 0 AFTER `legendaryId`;
