<?php
include "init.inc.php";

mysql_select_db($dbu_name) or die(mysql_error());

if($action=="logout") {

session_destroy();

} else if($action="resend_pw") {

} else {

// user and password given
if($user!="" && $passw!="") {
	$crpt=md5($passw);
	$query = "SELECT * FROM reguser";
	$result = mysql_query($query) or die(mysql_error());
			
	while ($line = mysql_fetch_array($result)) {
		if($line[email]==$user && $line[passw]==$crpt) {
			$auth_user=$line[name];
			$auth_id=$line[id];
			$auth_level=explode(",",$line[priv]);
			
			//session_register("auth_user");
			//session_register("auth_id");
			//session_register("auth_level");
      register($auth_id,$auth_user,$auth_level);
 			break;
		}		
	}
}

}

header("Location: index.php");
?>
