<?php
// get all sections from db
$query = "SELECT * FROM site_sec";
$result = mysql_query($query,$db_link) or die(mysql_error());

echo "<TABLE BORDER=0 CELLSPACING=2 CELLPADDING=1 width=100%>\n";
echo "<TR><TH>Site Menu</TH></TR>\n";
while ($line = mysql_fetch_array($result)) {
	echo "<tr><td bgcolor=#dfdf0f>";
  echo "<a href='$line[link].php'>$line[name]</a>";
	echo "</td></tr>\n";
}

echo "</table>";
?>
