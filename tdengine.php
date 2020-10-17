<?php
$br = (php_sapi_name() == "cli")? "":"<br>";

if(!extension_loaded('tdengine')) {
	dl('tdengine.' . PHP_SHLIB_SUFFIX);
}
$module = 'tdengine';
$functions = get_extension_funcs($module);
echo "Functions available in the test extension:$br\n";
foreach($functions as $func) {
    echo $func."$br\n";
}
echo "$br\n";
$function = 'confirm_' . $module . '_compiled';
if (extension_loaded($module)) {
	$str = $function($module);
} else {
	$str = "Module $module is not compiled into PHP";
}
echo "$str\n";
$a = taos_connect("127.0.0.1","root","taosdata","demo",0);
// $q = taos_query($a,"CREATE TABLE meters (ts timestamp, current float, voltage int, phase float) TAGS (location binary(64), groupdId int);");
// $q1 = taos_query($a,"CREATE TABLE d1001 USING meters TAGS ('Beijing.Chaoyang', 2);");
$q = taos_query($a,"SELECT * FROM m1");
$q1 = taos_affected_rows($q);
$q2 = taos_fetch_all($q);
var_dump($a);
var_dump($q);
var_dump($q1);
var_dump($q2);
$w1 = taos_query($a,"insert into m1 values (1546301800000, 0, 0, 0, 0, 0.000000, 0.000000, 'hello')");
$w2 = taos_affected_rows($w1);
var_dump($w1);
var_dump($w2);
?>
