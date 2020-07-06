ALTER TABLE `item_template_virtual`
	ADD COLUMN `statGroup` TINYINT(3) UNSIGNED NOT NULL DEFAULT 0 AFTER `seed`;
