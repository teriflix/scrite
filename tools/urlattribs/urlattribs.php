<?php
require_once('OpenGraph.php');
$url = $_POST['url'];
$graph = OpenGraph::fetch($url);
$json = array();
foreach ($graph as $key => $value)
    $json[$key] = $value;
echo json_encode($json, JSON_PRETTY_PRINT);
?>
