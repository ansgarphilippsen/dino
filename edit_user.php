<?php
include "m_init.inc.php";
//mysql_select_db($dbu_name) or die(mysql_error());

if(validate()) {
  $id = auth_id();
  $query = "SELECT * FROM $tb_usr WHERE id=$id";
  $result = mysql_query($query,$db_link) or die(mysql_error());
  $cuser = mysql_fetch_array($result);
  $p_username = $cuser[username];
	$p_email = $cuser[email];
	$p_fullname = $cuser[fullname];
	$p_restype = $cuser[restype];
	$p_org = $cuser[org];
	$p_country = $cuser[country];
} else {
  $id=0;
}

if($action=="new_pw") {
  $crypt0 = md5($pw0);
  $crypt1 = md5($pw1);
  if($cuser[passw]==$crypt0) {
    if($pw1 == $pw2) {
      if(strlen($pw1)<6) {
        $err_mesg = "New password is too short (min 6 char)!";
        unset($action);
      } else {
        $query = "UPDATE $tb_usr SET passw='$crypt1' WHERE id=$id LIMIT 1";        $result = mysql_query($query,$db_link) or die(mysql_error());
        echo "<b>Password successfully updated</b>";
      }
    } else {
      $err_mesg = "The two new passwords do not match!";
      unset($action);
    }
  } else {
    $err_mesg = "The old password is incorrect!";
    unset($action);
  }
} else if($action=="mod") {
  if(strlen($p_name)==0) {
    $err_mesg = "you must supply a name";
		unset($action);	
	} else {
	  $query = "UPDATE $tb_usr SET name='$p_name' WHERE id=$id";
		mysql_query($query) or die(mysql_error());
	  $query = "UPDATE $tb_usr SET restype='$p_restype' WHERE id=$id";
		mysql_query($query) or die(mysql_error());
	  $query = "UPDATE $tb_usr SET org='$p_org' WHERE id=$id";
		mysql_query($query) or die(mysql_error());
	  $query = "UPDATE $tb_usr SET country='$p_country' WHERE id=$id";
		mysql_query($query) or die(mysql_error());

	  header("Location: edit_user.php");
	}
} else if($action=="new_email") {
  if(strlen($p_email)==0) {
	  header("Location: edit_user.php");
	} else {
    if(ereg("(.+)@(.+)\.(..+)",$p_email)) {
			$query = "UPDATE $tb_usr SET email='$p_email' WHERE id=$id";
			$result = mysql_query($query) or die(mysql_error());
		  header("Location: edit_user.php");
		} else {
		  $err_mesg = "The new email adress is malformed";
			unset($action);
		}	
	}
} else if($action=="new") {
  // new user request
	
  $err_mesg = "";
	if(check_new_user($p_username,$p_email,$pw1,$pw2,$p_realname)) {
	  echo "OK\n";
	} else {
	  unset($action);
	}
	
} else {
  unset($action);
}


if($id==0) {
	$title = "<b>Register</b>\n";
} else {
	$title = "<b>Edit User Settings</b>\n";
}

include "m_header.inc.php";

if(!isset($action)) {

  if(strlen($err_mesg)>0) {
		echo "<HR NOSHADE SIZE=1>\n";
    echo "<FONT COLOR=#ff0000>error: $err_mesg</FONT>\n";
		echo "<HR NOSHADE SIZE=1>\n";
  }

  echo "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%>\n";
  echo "<TR><TD COLSPAN=2>\n";

if($id>0) {
  echo "<H4>Modify Info</H4>\n";
  echo "<FORM ACTION='edit_user.php?action=mod' METHOD=POST>\n";
} else {
	echo "<H4>Enter your data</H4>\n";
  echo "<FORM ACTION='edit_user.php?action=new' METHOD=POST>\n";
}
  echo "<TABLE BORDER=0 CELLSPACING=1 CELLPADDING=4>\n";

	echo "<TR><TD><B>Login Name</B><BR>\n";
if($id==0) {
  echo "<INPUT TYPE=TEXT NAME='p_username' VALUE='$p_username' SIZE=24 MAXSIZE=24>\n";
} else {
  echo "$p_username\n";	
}
	echo "</TD></TR>\n";

//if($id==0) {
//  echo "<TR><TD><B>Enter Password twice</B><BR>\n";
//  echo "<INPUT TYPE=PASSWORD SIZE=24 MAXSIZE=24 NAME='pw1'><BR>\n";
//  echo "<INPUT TYPE=PASSWORD SIZE=24 MAXSIZE=24 NAME='pw2'>\n";
//	echo "</TD></TR>\n";
//}

	echo "<TR><TD><B>Real Name</B> <SMALL> This will appear in the forum</SMALL><BR>\n";
  echo "<INPUT TYPE=TEXT NAME='p_fullname' VALUE='$p_fullname' SIZE=60>\n";
	echo "</TD></TR>\n";

if($id==0) {
	echo "<TR><TD><B>Email</B><BR>";
	echo "<SMALL>This is your contact	address to which the password will be mailed</SMALL><BR>\n";
  echo "<INPUT TYPE=TEXT NAME='p_email' VALUE='$p_email' SIZE=60>\n";
} else {
	echo "<TR><TD><B>Email</B><BR>";
  echo "$cuser[email]";
}
	echo "</TD></TR>\n";


	echo "<TR><TD><B>Type of Research</B><BR>\n";
  echo "<INPUT TYPE=TEXT NAME='p_restype' VALUE='$p_restype' SIZE=60>\n";
	echo "</TD></TR>\n";

	echo "<TR><TD><B>Organization</B><BR>\n";
  echo "<INPUT TYPE=TEXT NAME='p_org' VALUE='$p_org' SIZE=60>\n";
	echo "</TD></TR>\n";

	echo "<TR><TD><B>Country</B><BR>\n";
  echo "<INPUT TYPE=TEXT NAME='p_country' VALUE='$p_country' SIZE=60>\n";
	echo "</TD></TR>\n";

  echo "<TR><TD ALIGN=CENTER>";
	echo "<INPUT TYPE=SUBMIT NAME='submit' VALUE='Submit'>\n";
	echo "</TD></TR>\n";

  echo "</TABLE>\n";
  echo "</FORM>\n";

  echo "</TD></TR><TR><TD COLSPAN=2>\n";
	
  echo "<HR NOSHADE SIZE=1>\n";

  echo "</TD></TR>\n";
	
	echo "<TR><TD VALIGN=TOP>\n";

if($id>0) {
  echo "<H4>Change Password</H4>\n";
  echo "<FORM ACTION='edit_user.php?action=new_pw' METHOD=POST>\n";
  echo "<TABLE BORDER=0 CELLSPACING=1 CELLSPACING=1>\n";
  echo "<TR><TD ALIGN=RIGHT>Enter old Password</TD>\n";
  echo "<TD><INPUT TYPE=PASSWORD NAME='pw0'></TD></TR>\n";
  echo "<TR><TD ALIGN=RIGHT>Enter new Password</TD>\n";
  echo "<TD><INPUT TYPE=PASSWORD NAME='pw1'></TD></TR>\n";
  echo "<TR><TD ALIGN=RIGHT>Enter new one again</TD>\n";
  echo "<TD><INPUT TYPE=PASSWORD NAME='pw2'></TD></TR>\n";
  echo "<TR><TD COLSPAN=2 ALIGN=CENTER><INPUT TYPE=SUBMIT NAME='submit' VALUE='Submit'></TD></TR>\n";
  echo "</TABLE>\n";
  echo "</FORM>\n";

  echo "</TD><TD VALIGN=TOP>\n";

  echo "<H4>Change Email</H4>\n";
  echo "<FORM ACTION='edit_user.php?action=new_email' METHOD=POST>\n";
  echo "<TABLE BORDER=0 CELLSPACING=1 CELLSPACING=1>\n";
  echo "<TR><TD><INPUT TYPE=TEXT NAME='p_email' VALUE='$p_email' SIZE=40></TD></TR>\n";
	echo "<TR><TD ALIGN=CENTER><SMALL>This is you login and your contact	address.<BR>Please check for typos!</SMALL></TD></TR>\n";
  echo "<TR><TD ALIGN=CENTER><INPUT TYPE=SUBMIT VALUE='Submit'></TD></TR>\n";
	echo "</TABLE>\n";	
  echo "</FORM>\n";  
  echo "</TD></TR>\n";
}

  echo "</TABLE>\n";
	
	echo "<HR NOSHADE SIZE=1>\n";
}


include "m_footer.inc.php";

function check_new_user($un,$em,$p1,$p2,$rn) {
	global $err_mesg;
	global $tb_usr;

	// username length
	if(strlen($un)<5) {
	  $err_mesg = "Username too short (min 5 char)";
		return 0;
	}

	// unique user ?
	$query = "SELECT id FROM $tb_usr WHERE username = '$un'";
	$result = mysql_query($query) or die(mysql_error());
	if(mysql_num_rows($result)) {
	  $err_mesg = "This username is already taken!";
		return 0;
	}

	// email format ok ?
  if(!ereg("(.+)@(.+)\.(..+)",$em)) {
		$err_mesg = "Please supply a valid email address";
		return 0;
	}

	// unique email ?
	$query = "SELECT id FROM $tb_usr WHERE email = '$em'";
 	$result = mysql_query($query) or die(mysql_error());
	if(mysql_num_rows($result)) {
		$err_mesg = "The email you supplied is already registered!";
		return 0;
	}
	return 1;
}

?>
