<?php
include "init.inc.php";

$title = "<B>Viewing thread</B>\n";

include "f_header.inc.php";

$query = "SELECT * FROM $tb_ent WHERE thread_id=$id ORDER BY ts_create ASC";
$result = mysql_query($query) or die(mysql_error());

echo "<TABLE BORDER=0 CELLPADDING=5 CELLSPACING=10 WIDTH=100% BGCOLOR=$col_thread_line>\n";
  
while($line=mysql_fetch_array($result)) {
  echo "<TR><TD>\n";
  $uname="anonymous";
  echo "<TABLE BORDER=0 CELLPADDING=3 CELLSPACING=0 WIDTH=100%>\n";
  echo "<TR BGCOLOR=$col_eview1>\n";
  echo "<TD ALIGN=LEFT><i>$uname</i></TD><TD ALIGN=RIGHT>$line[ts_create]</TD>\n";
  echo "</TR><TR>\n";
  echo "<TD COLSPAN=2 BGCOLOR=$col_eview2>$line[content]</TD>\n";
  echo "</TR>\n";
  echo "</TABLE\n";

  echo "</TD></TR>\n";
}

echo "</TABLE>\n";
echo "<HR NOSHADE SIZE=1>\n";
echo "<CENTER>\n";
$query = "SELECT state FROM $tb_thr WHERE id=$id";
$result = mysql_query($query);
$line = mysql_fetch_array($result);
if($line[state]=="open") {
  echo "<A HREF='new_entry.php?id=$id&cid=$cid'>Add Comment</A>&nbsp;|&nbsp;\n";  
} else {
  echo "<i>This thread is closed, comments can no longer be added</i><BR>\n";
}
echo "<A HREF='view_cat.php?id=$cid'>back to thread overview</A>\n";
echo "</CENTER>\n";
echo "<HR NOSHADE SIZE=1>\n";

include "f_footer.inc.php";
?>
