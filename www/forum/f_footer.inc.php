<?php
echo "</TD></TR>\n";
echo "<TR><TD BGCOLOR=$col_footer>\n";

echo "<TABLE BORDER=0 CELLSPACING=0 CELLPADDING=5 WIDTH=100%>\n";
echo "<TR><TD ALIGN=LEFT>\n";
echo "LEFT FOOTER";
echo "</TD><TD ALIGN=RIGHT>\n";
$query = "SELECT * FROM category";
$result = mysql_query($query) or die(mysql_error());
echo "<small>Quick Jump</small><BR>\n";
echo "<FORM ACTION='view_cat.php' METHOD=GET>\n";
echo "<SELECT NAME='id'>\n";
while($line=mysql_fetch_array($result)) {
  echo "<OPTION VALUE='$line[id]'>$line[name]</OPTION>\n";
}
echo "</SELECT>\n";
echo "<INPUT TYPE=SUBMIT VALUE='Go'>\n";
echo "</FORM>\n";
echo "</TD></TR>\n";
echo "</TABLE>\n";

echo "</TD></TR>\n";
echo "</TABLE>\n";
echo "</HTML>\n";
?>
