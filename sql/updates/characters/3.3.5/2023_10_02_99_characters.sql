ALTER TABLE `characters`
	ADD COLUMN `lootPref` TINYINT(3) UNSIGNED NOT NULL DEFAULT '0' AFTER `grantableLevels`;
