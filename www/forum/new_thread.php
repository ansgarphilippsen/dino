<?php
include "init.inc.php";

if($id<1) {
  header("Location: $base_link");
}

if($submit=="Cancel") {
  header("Location: view_cat.php?id=$id");
  exit;  
} else if($submit=="Submit" || $submit=="Preview") {

  $e_content2 = check_tags(stripslashes($e_content));

  if(strlen($t_name)<3) {
    $err_mesg = "thread title missing or too short";
    unset($submit);
  } else if(strlen($e_content)==0) {
    $err_mesg = "entry content empty";
    unset($submit);
  } else {
    if($submit=="Submit") {
      // add to db
      $query = "INSERT INTO $tb_thr VALUES ('','$id','$t_name','0',NOW(),NOW(),'open')";
      $result = mysql_query($query) or die(mysql_error());
      $tid = mysql_insert_id();
      
      $query = "INSERT INTO $tb_ent VALUES ('','$tid','0',NOW(),NOW(),'$e_content2')";
      $result = mysql_query($query) or die(mysql_error());

      header("Location: view_cat.php?id=$id");
      exit;
    } else {
      //let it pass through below
    }
  }
}

include "f_header.inc.php";

echo "<FORM ACTION='new_thread.php?id=$id' METHOD=POST>\n";
echo "<TABLE BORDER=0 CELLPADDING=5 CELLSPACING=0 WIDTH=100%>\n";
echo "<TR><TD>\n";
if(strlen($err_mesg)>0) {
  echo "<FONT COLOR=#ff0000>error: $err_mesg</FONT>\n";
  echo "</TD></TR><TR><TD>\n";
}
echo "<B>Thread Title</B>\n";
echo "</TD></TR><TR><TD>\n";
echo "<INPUT TYPE=TEXT NAME='t_name' VALUE='$t_name' SIZE=80>\n";
echo "</TD></TR><TR><TD>\n";
echo "<B>First Entry</B>\n";
echo "</TD></TR><TR><TD>\n";
echo "<TEXTAREA COLS=80 ROWS=20 NAME='e_content'>";
echo "$e_content";
echo "</TEXTAREA>\n";
echo "</TD></TR><TR><TD>\n";
echo "<i>\n
You may use the tags <tt>&lt;b&gt;</tt>, <tt>&lt;i&gt;</tt>, 
<tt>&lt;tt&gt;</tt>, <tt>&lt;p&gt;</tt>, and <tt>&lt;u&gt;</tt>.
An empty line will be converted into a paragraph, and a word beginning with
<tt>http://</tt>, <tt>https://</tt> or <tt>ftp://</tt> will be converted into
a hyperlink.
</i>\n";
if($submit=="Preview") {
echo "<i>
Please check you entry for errors before submitting.
</i>\n";
} else {
echo "<i>
You must preview your entry at least once before you can submit it.
</i>\n";
}
echo "</TD></TR><TR><TD>\n";
if($submit=="Preview") {
  echo "<INPUT TYPE=SUBMIT NAME='submit' VALUE='Submit'>\n";
} else {
  echo "<INPUT TYPE=SUBMIT NAME='submit' VALUE='Submit' DISABLED>\n";
}
echo "<INPUT TYPE=SUBMIT NAME='submit' VALUE='Preview'>\n";
echo "<INPUT TYPE=SUBMIT NAME='submit' VALUE='Cancel'>\n";
echo "</TD></TR>\n";
echo "</TABLE>\n";
echo "</FORM>\n";

if($submit=="Preview") {
 echo "<i>This is how the entry will appear</i>\n";
  echo "<HR SIZE=1 NOSHADE>\n";
  echo "$e_content2";
  echo "<HR SIZE=1 NOSHADE>\n";
}

include "f_footer.inc.php";

?>
