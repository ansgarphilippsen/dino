<?php
session_start();

include "m_settings.inc.php";
include "funcs.inc.php";

// open persistent mysql connection
$db_link = mysql_pconnect($db_host, $db_name, $db_pass)
        or die(mysql_error());

// use dino database
mysql_select_db($db_name) or die(mysql_error());


?>
