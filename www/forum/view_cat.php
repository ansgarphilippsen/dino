<?php
include "f_init.inc.php";

$query = "SELECT * FROM $tb_cat WHERE id=$id";
$result = mysql_query($query) or die(mysql_error());
$line=mysql_fetch_array($result);

$title = "<b>Category $line[name]</b><BR><SMALL>$line[desc]</SMALL\n";

include "f_header.inc.php";

echo "<TABLE BORDER=0 CELLPADDING=10 CELLSPACING=0 WIDTH=100%>\n";

echo "<TR BGCOLOR=$col_cview2><TD ALIGN=CENTER>\n";

echo "<A HREF='new_thread.php?id=$id'>Add New Thread</A>\n";
echo "&nbsp;|&nbsp;\n";
echo "<A HREF='$base_link'>Back to Category Overview</A>\n";


$query = "SELECT * FROM $tb_thr WHERE cat_id=$id ORDER BY ts_update DESC";
$result = mysql_query($query) or die(mysql_error());

echo "<TR BGCOLOR=$col_cview3><TD>\n";
if(mysql_num_rows($result)<1) {
 echo "<i>no threads in this category</i>\n";
} else {
  echo "<i>Current Threads</i>\n";
  echo "</TD></TR><TR BGCOLOR=$col_cview3><TD>\n";

  echo "<TABLE BORDER=1 CELLSPACING=0 CELLPADDING=2 BGCOLOR=$col_thread_line WIDTH=100%>\n";
  echo "<TR>\n";
  echo "<TH WIDTH=20>state</TH>";
  echo "<TH WIDTH=100%>name</TH>";
  echo "<TH>comments</TH>";
  echo "<TH>last&nbsp;update</TH>";
  echo "</TR>\n";
  while($line=mysql_fetch_array($result)) {
    // get number of entries in thread
    $query = "SELECT id FROM $tb_ent WHERE thread_id=$line[id]";
    $r=mysql_query($query);
    $te=mysql_num_rows($r);
    // formating
    echo "<TR><TD ALIGN=CENTER>\n";
    if($line[state]=="open") {
      echo "<IMG SRC='l_green.gif'>\n";      
    } else {
      echo "<IMG SRC='l_red.gif'>\n";      
    }
    echo "</TD>\n";
    echo "<TD><A HREF='view_thread.php?id=$line[id]&cid=$id'>$line[title]</A></TD>\n";
    echo "<TD ALIGN=CENTER>$te</TD>\n";
    echo "<TD ALIGN=RIGHT>$line[ts_update]</TD>\n";
    echo "</TR>\n";
  }
  echo "</TABLE>\n";
}

echo "</TABLE>\n";

include "f_footer.inc.php";
?>
