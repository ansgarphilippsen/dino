<?php
include "init.inc.php";
mysql_select_db($dbu_name) or die(mysql_error());

if(validate()) {
  $id = auth_id();
  $query = "SELECT * FROM $tb_usr WHERE id=$id";
  $result = mysql_query($query,$db_link) or die(mysql_error());
  $cuser = mysql_fetch_array($result);
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
}



$title = "<b>Edit User Settings</b>\n";

include "f_header.inc.php";

if(!isset($action)) {

	echo "<HR NOSHADE SIZE=1>\n";
	echo "<CENTER>\n";
	echo "<A HREF='index.php'>Back to forum</A>\n";
	echo "</CENTER>\n";
	echo "<HR NOSHADE SIZE=1>\n";

  if(strlen($err_mesg)>0) {
    echo "<FONT COLOR=#ff0000>$err_mesg</FONT>\n";
		echo "<HR NOSHADE SIZE=1>\n";
  }

  echo "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 WIDTH=100%>\n";
  echo "<TR><TD COLSPAN=2>\n";

  echo "<H4>Modify Info</H4>\n";
  echo "<FORM ACTION='edit_user.php?action=mod' METHOD=POST>\n";
  echo "<TABLE BORDER=0 CELLSPACING=1 CELLPADDING=4>\n";

	echo "<TR><TD><B>Name</B> <SMALL> This will appear in the forum</SMALL><BR>\n";
  echo "<INPUT TYPE=TEXT NAME='p_name' VALUE='$cuser[name]' SIZE=60>\n";
	echo "</TD></TR>\n";

	echo "<TR><TD><B>Email</B><BR>";
  echo "$cuser[email]";
	echo "</TD></TR>\n";

	echo "<TR><TD><B>Type of Research</B><BR>\n";
  echo "<INPUT TYPE=TEXT NAME='p_restype' VALUE='$cuser[restype]' SIZE=60>\n";
	echo "</TD></TR>\n";

	echo "<TR><TD><B>Organization</B><BR>\n";
  echo "<INPUT TYPE=TEXT NAME='p_org' VALUE='$cuser[org]' SIZE=60>\n";
	echo "</TD></TR>\n";

	echo "<TR><TD><B>Country</B><BR>\n";
  echo "<INPUT TYPE=TEXT NAME='p_country' VALUE='$cuser[country]' SIZE=60>\n";
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

  echo "</TABLE>\n";
	
	echo "<HR NOSHADE SIZE=1>\n";
	echo "<CENTER>\n";
	echo "<A HREF='index.php'>Back to forum</A>\n";
	echo "</CENTER>\n";
	echo "<HR NOSHADE SIZE=1>\n";
}


include "f_footer.inc.php";
?>
