<?php

$prefix = "1o5jd74m_";
	
$v_uid = $prefix . "auth_id";
$v_uname = $prefix . "auth_user";
$v_ulevel = $prefix . "auth_level";

function validate($key="") {
  global $v_ulevel;
  if(session_is_registered($v_ulevel)) {
    if(strlen($key)==0) {
      return 1;
    } else {
		  $ulevel=auth_level();
      if(in_array($key,$ulevel)) {
        return 1;
      }
    }
  }
  return 0;
}

function register($uid,$uname,$ulevel) {
  global $v_uid;
	global $v_uname;
	global $v_ulevel;

  global ${$v_uid};
	global ${$v_uname};
	global ${$v_ulevel};

  ${$v_uid} = $uid;
  ${$v_uname} = $uname;
  ${$v_ulevel} = $ulevel;

  session_register($v_uid);
	session_register($v_uname);
	session_register($v_ulevel);

  return 0;
}

function auth_id() {
  global $v_uid;
  global ${$v_uid};
	return ${$v_uid};
}

function auth_name() {
  global $v_uname;
	global ${$v_uname};
	return ${$v_uname};
}

function auth_level() {
  global $v_ulevel;
	global ${$v_ulevel};
	return ${$v_ulevel};
}

function check_tags($string)
{
	$sr = array (
//		"'<'",
//		"'>'",
		"'[\r\f]'",
		"'(https?://\S+)'",
		"'(\n\s*\n)'"

	);
	$rp = array (
//		"&lt;",
//		"&gt;",
		"",
		"<a href=\\1>\\1</a>",
		"<p>"
	);
	$string = strip_tags($string,'<b><tt><i><p><br><u>');
	$string =	preg_replace($sr,$rp,$string);
	return $string;	
}


?>
