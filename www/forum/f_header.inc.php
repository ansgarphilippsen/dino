<?php
echo "<HTML><HEADER><TITLE>DINO Forum</TITLE></HEADER>\n";
echo "<BODY BGCOLOR=$col_page_bg>\n";

echo "<TABLE BORDER=0 CELLPADDING=5 CELLSPACING=0 WIDTH=100% BGCOLOR=$col_table>\n";

echo "<TR><TD BGCOLOR=$col_page_bg>\n";
echo "<CENTER><b>DINO Forum</b></CENTER>\n";
echo "</TD></TR>\n";

echo "<TR><TD BGCOLOR=$col_header>\n";
echo "<TABLE BORDER=0 CELLSPACING=0 CELLPADDING=5 WIDTH=100%>\n";
echo "<TR><TD ALIGN=LEFT>\n";
echo "$title\n";
echo "</TD><TD ALIGN=RIGHT>\n";
if(validate()) {
  $u=auth_name();
  echo "Logged in as $u<BR>";
	echo "<A HREF='edit_user.php'>manage account</A>\n";
	echo "&nbsp;|&nbsp;";
	echo "<A HREF='auth.php?action=logout'>logout</A>\n";
} else {
  echo "Browsing as guest<BR>";
	echo "<A HREF='login.php'>login</A>\n";
}

echo "</TD></TR>\n";
echo "</TABLE>\n";
echo "</TD></TR>\n";

echo "<TR><TD BGCOLOR=$col_main>\n";

?>
