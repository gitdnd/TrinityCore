ALTER TABLE `instance_template`
	ADD COLUMN `mapXPRate` FLOAT UNSIGNED NOT NULL DEFAULT 1 AFTER `maxPlayerOverride`;
