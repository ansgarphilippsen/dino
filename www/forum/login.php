<?php
if($action=="logout") {
	session_destroy();
	header("Location: index.php");
	exit;
}

include "init.inc.php";
mysql_select_db($dbu_name) or die(mysql_error());

if($submit=="login") {
	// user and password given
	if($p_email!="" && $p_passw!="") {
		$crpt=md5($p_passw);
		$query = "SELECT * FROM reguser";
		$result = mysql_query($query) or die(mysql_error());
			
		while ($line = mysql_fetch_array($result)) {
			if($line[email]==$p_email && $line[passw]==$crpt) {
				$auth_user=$line[name];
				$auth_id=$line[id];
				$auth_level=explode(",",$line[priv]);
			
	      register($auth_id,$auth_user,$auth_level);
				break;
			}		
		}
		if(!validate()) {
		  $mesg = "login incorrect - please try again";
			unset($submit);
		} else {
		  header("Location: index.php");
		}
	} else {
	  unset($submit);
	}
}

$title = "<b>Login</b>";

include "f_header.inc.php";
echo "<CENTER>\n";

if($submit=="send new password") {
  $query = "SELECT id FROM reguser WHERE email='$p_email'";
	$result = mysql_query($query) or die(mysql_error());
	$res = mysql_num_rows($result);
	if(!ereg("(.+)@(.+)\.(..+)",$p_email) || $res==0) {
	  $mesg = "Please supply a valid email adress to resend the password to";
		unset($submit);
	} else {
	  $new_pw="";
	  for($i=0;$i<8;$i++) {
		  $new_pw .= chr(ord("a")+rand(0,25));
		}
	  $m_to = $p_email;
		$m_subj  = "Reset Request from the DINO Website";
		$m_mesg  = "You (or someone entering your email) have requested a\n";
		$m_mesg .= "new password to be generated for your account at \n";
		$m_mesg .= "www.dino3d.org. It was reset to $new_pw\n\n";
    $m_mesg .= "You may login and change it in the user preferences section\n";
		$m_head  = "From: dino@dino3d.org\r\n";

		if(mail($m_to,$m_subj,$m_mesg,$m_head)) {
		  $new_crypt = md5($new_pw);
		  $query = "UPDATE reguser SET passw='$new_crypt' WHERE email='$p_email'";
			$result = mysql_query($query) or die(mysql_error());
  	  echo "<P>A new password has been send to <tt>$p_email</tt>";
			echo "<P><A HREF='login.php'>Back to login page</A>\n";
		} else {
			die("unexpected error while sending mail");
		}
	}
}

if(!isset($submit)) {
 	if(strlen($mesg)>0) {
	  echo "$mesg\n";
	}
	echo "<FORM ACTION='login.php' METHOD=POST>\n";
	echo "<TABLE BORDER=0 CELLPADDING=3 CELLSPACING=0>\n";
	echo "<TR><TD ALIGN=CENTER>\n";
	echo "<tt>email </tt><INPUT TYPE=TEXT NAME='p_email' VALUE='$p_email' SIZE=40 MAXSIZE=50>\n";
	echo "</TD></TR>\n";
	echo "<TR><TD ALIGN=CENTER>\n";
	echo "<tt>passw </tt><INPUT TYPE=PASSWORD NAME='p_passw' VALUE='' SIZE=40	MAXSIZE=20>\n";
	echo "</TD></TR>\n";
	echo "<TR><TD ALIGN=CENTER>\n";
	echo "<INPUT TYPE=SUBMIT NAME='submit' VALUE='login'\n";
	echo "</TD></TR>\n";
	echo "<TR><TD ALIGN=CENTER>\n";
	echo "
	<SMALL>
If you have forgotten your password,<BR>
enter your email above and select the<BR>
button below to create a new password,<BR>
which is then send to you.
	</SMALL>\n";
	echo "</TD></TR>\n";
	echo "<TR><TD ALIGN=CENTER>\n";
	echo "<INPUT TYPE=SUBMIT NAME='submit' VALUE='send new password'\n";
	echo "</TD></TR>\n";
	echo "</TABLE>\n";
	echo "</FORM>\n";	
}

echo "</CENTER>\n";
include "f_footer.inc.php";

?>
