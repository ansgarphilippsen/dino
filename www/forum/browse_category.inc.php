<?php
  $query = "SELECT * FROM category";
  $result = mysql_query($query) or die(mysql_error());

  echo "<TABLE BORDER=0 CELLPADDING=0 CELLSPACING=5 WIDTH=100%>\n";

  while($line=mysql_fetch_array($result)) {
    // get number of threads in category
    $query = "SELECT id FROM thread WHERE cat_id=$line[id]";
    $r=mysql_query($query);
    $tc=mysql_num_rows($r);

    // formating
    echo "<TR><TD>\n";

    echo "<TABLE BORDER=0 CELLSPACING=0 CELLPADDING=3 BGCOLOR=$col_cat_line WIDTH=100%>\n";
    echo "<TR><TD>\n";
    echo "<A HREF='$link_base?action=view_cat&id=$line[id]'>$line[name]</A>\n";
    echo "<BR><FONT SIZE=-1>$line[desc]</FONT>\n";
    echo "</TD><TD ALIGN=RIGHT>\n";
    echo "<FONT SIZE=-1>$tc threads</FONT>\n";
    echo "</TD></TR>\n";
    echo "</TABLE>\n";

    echo "</TD></TR>\n";
  }

  echo "</TABLE>\n";

?>
