<?php

function oxygenmark_render_template($template_path, $_params) {
    if(!isset($_params) || count($_params) == 0) $params = array(
        "" => ""
    );
    else {
        $params = array();
        foreach($_params as $k => $v) {
            $params[(string) $k] = (string) $v;
        }
    }

    $ch = curl_init();
    curl_setopt($ch, CURLOPT_URL, "http://127.0.0.1:7022/render");
    curl_setopt($ch, CURLOPT_POST, 1);
    curl_setopt($ch, CURLOPT_HEADER, 0);
    curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1);
    curl_setopt($ch, CURLOPT_POSTFIELDS, json_encode(array(
        "type" => "template",
        "path" => $template_path,
        "params" => $params
    )));

    $result = curl_exec($ch);
    curl_close($ch);

    return $result;
}
