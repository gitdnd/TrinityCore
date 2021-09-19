CREATE TABLE `item_spell_gem_desc` (
	`SpellId` INT UNSIGNED NOT NULL,
	`Description` VARCHAR(16000) NOT NULL DEFAULT '',
	PRIMARY KEY (`SpellId`)
)
COLLATE='utf8mb4_0900_ai_ci'
;
