<?php
if(!isset($action)) {
  include "browse_category.inc.php";
} else if($action=="view_cat") {
  include "view_cat.inc.php";
} else if($action=="view_thread") {
  include "view_thread.inc.php";
} else if($action=="new_thread") {
  include "new_thread.inc.php";
} else {
  echo "<i>an internal error occured!</i>\n";
}
?>
