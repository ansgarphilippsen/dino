<?php
include "m_init.inc.php";

$title = "News";

include "m_header.inc.php";

// get news entries
$query = "SELECT DATE_FORMAT(ts,'%D %M %Y') AS dt,content,id FROM news ORDER BY id DESC";
$result = mysql_query($query,$db_link) or die("Invalid query ($query)");

echo "<TABLE BORDER=0 CELLSPACING=20 CELLPADDING=5 WIDTH=100%>\n";
while ($line = mysql_fetch_array($result)) {
	echo "<TR><TD bgcolor=#efefef>\n";
	echo "<p><b>$line[dt]</b></p>\n";
	echo "<p>$line[content]</p>\n";
	echo "</TD></TR>\n";
}

echo "</TABLE>\n";

include "m_footer.inc.php";
?>
