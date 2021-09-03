ALTER TABLE `instance_template`
	ADD COLUMN `maxPlayerOverride` INT(10) UNSIGNED NOT NULL DEFAULT 0 AFTER `allowMount`;