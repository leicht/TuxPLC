<html>
<head>
	<title>Chart</title>
	<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
</head>
<body bgcolor="#b8c8fe" leftmargin="6" >

<?
function btn($ran,$btnrange)
{
	if ($ran==$btnrange) 
		return $btnrange.'d.png';
	else
		return $btnrange.'u.png';
}

if (!isset($_GET['largeur']) OR !isset($_GET['hauteur'])) {
  echo "<script type=\"text/javascript\">\n";
  echo "  location.href=\"${_SERVER['SCRIPT_NAME']}?${_SERVER['QUERY_STRING']}"
          . "&largeur=\" + screen.width + \"&hauteur=\" + screen.height;\n";
  echo "</script>\n";
}

$MyPenColor = array ("#ff0000","#0000ff","#00ff00","#ff00ff","#00ffff","#ff8000","#008080","#800080");
   mysql_connect("localhost", "histosql", "histosql") or die("Impossible de se connecter : " . mysql_error());
   mysql_select_db("histosql") or die("Impossible de selectionner : " . mysql_error());

if (isset($title)) {
	if (strcmp($query, '') == 0) 
		$query .= '?';
	else
		$query .= '&';
	$query .="title=".$title;	
}
if (isset($tag0)) {
	if (strcmp($query, '') == 0) 
		$query .= '?';
	else
		$query .= '&';
	$query .="tag0=".$tag0;	
	$tag[0]=$tag0;
}
if (isset($tag1)) {
	if (strcmp($query, '') == 0) 
		$query .= '?';
	else
		$query .= '&';
	$query .="tag1=".$tag1;
	$tag[1]=$tag1;
}
if (isset($tag2)) {
	if (strcmp($query, '') == 0) 
		$query .= '?';
	else
		$query .= '&';
	$query .="tag2=".$tag2;
	$tag[2]=$tag2;
}
if (isset($tag3)) {
	if (strcmp($query, '') == 0) 
		$query .= '?';
	else
		$query .= '&';
	$query .="tag3=".$tag3;
	$tag[3]=$tag3;
}

if (isset($tag4)) {
	if (strcmp($query, '') == 0) 
		$query .= '?';
	else
		$query .= '&';
	$query .="tag4=".$tag4;
	$tag[4]=$tag4;
}
if (isset($tag5)) {
	if (strcmp($query, '') == 0) 
		$query .= '?';
	else
		$query .= '&';
	$query .="tag5=".$tag5;
	$tag[5]=$tag5;
}
if (isset($tag6)) {
	if (strcmp($query, '') == 0) 
		$query .= '?';
	else
		$query .= '&';
	$query .="tag6=".$tag6;
	$tag[6]=$tag6;
}
if (isset($tag7)) {
	if (strcmp($query, '') == 0) 
		$query .= '?';
	else
		$query .= '&';
	$query .="tag7=".$tag7;
	$tag[7]=$tag7;
}
if (isset($range)) {
	if (strcmp($query, '') == 0) 
		$query .= '?';
	else
		$query .= '&';
	$query .="range=".$range;
	$ran = $range;
} else {$ran=24;}
if (!isset($lag)) {
	$lag=0;
}
if (strcmp($query, '') == 0) 
	$query .= '?';
else
	$query .= '&';
$query .="lag=".$lag;
if (isset($_GET['largeur']) AND isset($_GET['hauteur'])) {
	$width=$largeur-140;
	$nbrTag=0;
	for ($i = 0; $i < 8; $i++) {
		if (isset($tag[$i])) {$nbrTag++;}
	}
	
	$height=$hauteur-280-($nbrTag*20);
	$query .="&width=".$width."&height=".$height;

}
echo "<p align=center><font face='Arial, Helvetica, sans-serif' size=4 color=#000066><b>".$title."</b></font><br>";

$Titre="<font face='Verdana, Arial, Helvetica, sans-serif' size=2 color=#FFFF00><b>";

$fl="<font face='Verdana, Arial, Helvetica, sans-serif' size=2>";

echo '	<a href="/php/chart.php'.$query.'&range=72"><img src="'.btn($ran,72).'" border="0"></a>
	<a href="/php/chart.php'.$query.'&range=48"><img src="'.btn($ran,48).'" border="0"></a>
	<a href="/php/chart.php'.$query.'&range=24"><img src="'.btn($ran,24).'" border="0"></a>
	<a href="/php/chart.php'.$query.'&range=12"><img src="'.btn($ran,12).'" border="0"></a>
	<a href="/php/chart.php'.$query.'&range=8"><img src="'.btn($ran,8).'" border="0"></a>
	<a href="/php/chart.php'.$query.'&range=4"><img src="'.btn($ran,4).'" border="0"></a>
	<a href="/php/chart.php'.$query.'&range=2"><img src="'.btn($ran,2).'" border="0"></a>
	<a href="/php/chart.php'.$query.'&range=1"><img src="'.btn($ran,1).'" border="0"></a>
	<br>';
echo '<img src="/cgi/histochart.cgi'.$query.'" border="0"><br>';
echo '<a href="/php/chart.php'.$query.'&lag='.($lag+1).'"><img src="backward.png" border="0"></a>&nbsp;';
echo '<a href="/php/chart.php'.$query.'&lag='.($lag-1).'"><img src="forward.png" border="0"></a></p>';



echo "<table cellspacing=0 cellpadding=0 border=0 align=center>";
for ($i = 0; $i < 8; $i++) {
	$Q = "Select * from DEFINITION where TAGNAME='$tag[$i]'";
	$query=mysql_query($Q) or die ("Erreur");
	while ($row = mysql_fetch_object($query)) {
		echo '<tr valign=middle>
			<td bgcolor='.$MyPenColor[$i].'><a class=lien2 href="/php/chart.php?tag'.$i.'='.$tag[$i].'&lag='.($lag).'&range='.($ran).'">'.$fl.'&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;</a></td>
			<td>'.$fl.'&nbsp;&nbsp;</td>
			<td> <a class=lien2 href="/php/chart.php?tag'.$i.'='.$tag[$i].'&lag='.($lag).'&range='.($ran).'">'.$fl.$row->TAGNAME.'</a></td>
			<td>'.$fl.'&nbsp;:&nbsp;</td>
			<td>'.$fl.$row->TAG_DEFINITION.'</td>
			</tr>';

	}
}
echo "</table>";
mysql_free_result($query);
mysql_close();

?>



</body>
</html>
