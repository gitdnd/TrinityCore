DELIMITER ;;
CREATE PROCEDURE support_gem ()
BEGIN
    DECLARE CONTINUE HANDLER FOR 1050 BEGIN END;	
	CREATE TABLE hot_support_gem (
    guid INT UNSIGNED NOT NULL,
    spell INT UNSIGNED NOT NULL,
    support_type INT UNSIGNED NOT NULL,
    second_data INT UNSIGNED NOT NULL,
	which_socket TINYINT UNSIGNED NOT NULL,
	cast_phase INT UNSIGNED NOT NULL
   ); 
END;;
CALL support_gem();;
DROP PROCEDURE  support_gem;; 	

