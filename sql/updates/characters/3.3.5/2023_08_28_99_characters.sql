ALTER TABLE `player_talents`
	ADD COLUMN `loadout` INT(10) UNSIGNED NOT NULL AFTER `node_index`,
	DROP PRIMARY KEY,
	ADD PRIMARY KEY (`guid`, `node_index`, `loadout`) USING BTREE;
