<?php
// open persistent mysql connection
$db_link = mysql_pconnect("cobra.mih.unibas.ch", "dino", "dino13")
        or die(mysql_error());

// use dino database
mysql_select_db("dino") or die(mysql_error());
?>
