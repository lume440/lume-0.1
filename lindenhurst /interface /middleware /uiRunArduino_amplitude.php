<?php
exec('sudo python /var/lume/lindenhurst/arduino/amplitude/sketchRun_amplitude.py');
header('Location: http://localhost/lumebot.com/public_html/lindenhurst/inside/');
?>
