<?php
  echo "<TABLE BORDER=0 CELLPADDING=10 CELLSPACING=0 WIDTH=100%>\n";

  $query = "SELECT * FROM category WHERE id=$id";
  $result = mysql_query($query) or die(mysql_error());
  $line=mysql_fetch_array($result);
  echo "<TR BGCOLOR=$col_cview1><TD>\n";
  echo "<b>$line[name]</b><BR><FONT SIZE=-1>$line[desc]</FONT>\n";
  echo "</TD></TR><TR BGCOLOR=$col_cview2><TD ALIGN=CENTER>\n";
  
  echo "<A HREF='$base_link?action=new_thread&id=$id'>Add New Thread</A>\n";
  echo "&nbsp;|&nbsp;\n";
  echo "<A HREF='$base_link'>Back to Category Overview</A>\n";


  $query = "SELECT * FROM thread WHERE cat_id=$id";
  $result = mysql_query($query) or die(mysql_error());

  echo "<TR BGCOLOR=$col_cview3><TD>\n";
  if(mysql_num_rows($result)<1) {
    echo "<i>no threads in this category</i>\n";
  } else {
    echo "<i>Current Threads</i>\n";
    echo "</TD></TR><TR BGCOLOR=$col_cview3><TD>\n";

    echo "<TABLE BORDER=1 CELLSPACING=0 CELLPADDING=3 BGCOLOR=$col_thread_line WIDTH=100%>\n";
    while($line=mysql_fetch_array($result)) {
      // get number of entries in thread
      $query = "SELECT id FROM 	entry WHERE thread_id=$line[id]";
      $r=mysql_query($query);
      $te=mysql_num_rows($r);

      // formating

      echo "<TR><TD WIDTH=20 ALIGN=CENTER>\n";
      if($line[state]=="open") {
        echo "<IMG SRC='l_green.gif'>\n";      
      } else {
        echo "<IMG SRC='l_red.gif'>\n";      
      }
      echo "</TD>\n";
      echo "<TD>\n";
      echo "<A HREF='$link_base?action=view_thread&id=$line[id]'>$line[title]</A>\n";
      echo "</TD><TD ALIGN=RIGHT>\n";
      echo "<FONT SIZE=-1>$te entries</FONT>\n";
      echo "</TD></TR>\n";

    }
    echo "</TABLE>\n";
  }

  echo "</TABLE>\n";
?>
