<?php

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
