<?php

require "OxygenMarkRenderer.php";

echo oxygenmark_render_template(dirname(__FILE__) . "/server_info.omc", $_SERVER);

echo "\n";
