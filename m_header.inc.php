<?php
echo "<HTML><HEAD><TITLE>DINO Homepage: $title</TITLE></HEAD>\n";
echo "<BODY BGCOLOR=#ffffff link=#0000ff alink=#0000ff vlink=#0000ff>\n";

echo "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=0 BGCOLOR=#ffff00 WIDTH=100%>\n";
echo "<TR>\n";
// top left corner
echo "<TD WIDTH=23 ALIGN=LEFT VALIGN=TOP><IMG SRC='wcorner_tl.gif'></TD>\n";
// top left side
echo "<TD ALIGN=LEFT><IMG SRC='logo1.jpg' ALT='logo'></TD>\n";
// midde
echo "<TD ALIGN=CENTER VALIGN=BOTTOM>\n";
echo "<FONT SIZE=+1><b><u>$title</u></b></FONT>\n";
echo "</TD>\n";
// top right corner
echo "<TD WIDTH=23 ALIGN=RIGHT VALIGN=TOP><IMG SRC='wcorner_tr.gif'></TD>\n";
echo "</TR>\n";
echo "<TR>\n";

// left menu
echo "<TD ALIGN=CENTER VALIGN=TOP COLSPAN=2>\n";

echo "<HR NOSHADE SIZE=1>\n";

include "menu.inc.php";

echo "<HR NOSHADE SIZE=1>\n";

include "login.inc.php";

echo "<HR NOSHADE SIZE=1>\n";

include "bannerlinks.inc.php";

echo "</TD>\n";
// main section
echo "<TD BGCOLOR=#ffffff WIDTH=100% ALIGN=LEFT VALIGN=TOP>\n";
?>
