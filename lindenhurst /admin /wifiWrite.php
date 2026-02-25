<?php
$ssid = $_GET['ssid'];
$password = $_GET['password'];
$myfile = fopen("/etc/network/interfaces", "wr") or die("Unable to open file!");
$txt = "source-directory /etc/network/interfaces.d

auto lo
iface lo inet loopback

iface eth0 inet manual

allow-hotplug wlan0
iface wlan0 inet dhcp
        wpa-ssid ".'"'.$ssid.'"'."
        wpa-psk ".'"'.$password.'"'; 

fwrite($myfile, $txt);
fclose($myfile);
