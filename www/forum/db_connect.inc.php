<?php
session_start();

// open persistent mysql connection
$db_link = mysql_pconnect($db_host,$db_user,$db_pass)
        or die(mysql_error());

// use dino database
mysql_select_db($db_name) or die(mysql_error());


?>
