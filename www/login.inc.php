<?php
echo "<TABLE BORDER=0 CELLPADDING=1 CELLSPACING=0>\n";
if(!validate()) {
  echo "<small><i>not logged in</i></small><br>\n";
	echo "<FORM ACTION='auth.php?action=login' METHOD=POST>\n";
	echo "<TR><TD ALIGN=RIGHT>\n";
	echo "<small>name</small>\n";
	echo "</TD><TD ALIGN=LEFT>\n";
	echo "<INPUT TYPE=TEXT NAME='p_name' VALUE='$p_name' SIZE=16 MAXSIZE=30>\n";
	echo "</TD></TR>\n";
	echo "<TR><TD ALIGN=RIGHT>\n";
	echo "<small>pass</small>\n";
	echo "</TD><TD ALIGN=LEFT>\n";
	echo "<INPUT TYPE=PASSWORD NAME='p_passw' VALUE='' SIZE=16	MAXSIZE=30>\n";
	echo "</TD></TR>\n";
	echo "<TR><TD ALIGN=CENTER COLSPAN=2>\n";
	echo "<INPUT TYPE=SUBMIT NAME='submit' VALUE='login'\n";
	echo "</TD></TR>\n";
	echo "<TR><TD COLSPAN=2 ALIGN=CENTER>\n";
	echo "<A HREF='auth.php?action=lostpw'><small><i>forgot&nbsp;password?</i></small>\n";
	echo "</TD></TR>\n";
	echo "<TR><TD COLSPAN=2 ALIGN=CENTER>\n";
	echo "</TD></TR>\n";
	echo "<TR><TD COLSPAN=2 ALIGN=CENTER>\n";
	echo "<A HREF='edit_user.php'>Register\n";
	echo "</TD></TR>\n";
	echo "</FORM>\n";
}	else {
	$un = auth_user();
	echo "<TR><TD ALIGN=CENTER>\n";
  echo "<small>logged in as <i>$un</i></small><br>\n";
	echo "</TD></TR>\n";
	echo "<TR><TD>\n";
	echo "<A HREF='edit_user.php'>user settings</A>\n";
	echo "</TD></TR>\n";
	echo "<TR><TD>\n";
	echo "<A HREF='auth.php?action=logout'>logout</A>\n";
	echo "</TD></TR>\n";

}
echo "</TABLE>\n";
?>
