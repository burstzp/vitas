<?php
$mc = new memcache();
$i = 0;
$t1=microtime(true);
$mc->connect("127.0.0.1", 7777);
while($i++< 10000) {
  var_dump($i);
  $s = str_repeat("中跟".mt_rand(1000,999999999999), 1000);
  var_dump(strlen($s));
  $ret = ($mc->set("test".$i, $s));
  var_dump($ret);
//  var_dump($mc->get("test"));
//i
var_dump($mc->get("test".$i));
  break;
  if ($ret===false)
    die;
}

var_dump(microtime(true)-$t1);
?>
