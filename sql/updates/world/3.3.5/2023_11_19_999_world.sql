ALTER TABLE `instance_template`
	ADD COLUMN `minDungeonLevel` INT(10) UNSIGNED NOT NULL DEFAULT '0' AFTER `vLvlMod`;
