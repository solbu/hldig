<?

include("htdig.phph");

if(!extension_loaded('htdig')) {
	dl('htdigphp.so');
}
$module = 'htdig';
$functions = get_extension_funcs($module);
echo "Functions available in the test extension:<br>\n";
foreach($functions as $func) {
    echo $func."<br>\n";
}
echo "<br>\n";
echo "\n<HR>\n\n";

$htsearch_parms = array("configFile" => "/home/httpd/cgi-bin/nealr.db/archive/archive.conf",
                        "logFile" => " ", 
                        "debug" => 0);

echo "htsearch_parms array:\n";
foreach ($htsearch_parms as $key => $value)
{
    echo "key[$key] => value[$value]\n";
}
echo "\n";

$error = htsearch_open($htsearch_parms);

if ($error == 0)
{

    $p_query = "claim";

    echo "p_query[$p_query]\n";

    $htsearch_query = array("raw_query" => "$p_query",
                        "algorithm" => HTSEARCH_ALG_AND_STR,
                        "sortby" => HTSEARCH_SORT_SCORE_STR,
                        "format" => HTSEARCH_FORMAT_LONG_STR,
                        "time_format" => "%Y-%m-%d");

    echo "htsearch_query array:\n";
    foreach ($htsearch_query as $key => $value)
    {
        echo "key[$key] => value[$value]\n";
    }
    echo "\n";

    $num_results = htsearch_query($htsearch_query);

    if ($num_results > 0)
    {
        for($i = 0; $i < $num_results; $i++)
        {
            echo "-------------------------------------------------------\n";
            echo "[$i]\n";
            $htsearch_match = htsearch_get_nth_match($i);
            
            foreach ($htsearch_match as $key => $value)
            {
                echo "key[$key] => value[$value]\n";
            }

        }
        
        echo "Num Results: ".$num_results."\n";
    }
    else
    {
        echo "No Results or Error\n";
    }


}
else
{   
    echo "Error htsearch_open [$error]\n";
}

htsearch_close();


?>
