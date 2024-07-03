ALTER TABLE `characters`
	ADD COLUMN `bonusTalents` INT(10) UNSIGNED NOT NULL DEFAULT '0' AFTER `talentLevel`;