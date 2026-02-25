#this script starts chromium with the Lume interface
import os
import time
#the command to start chromium with flags
#time.sleep(7)
os.system("sudo SDL_VIDEODRIVER=fbcon SDL_FBDEV=/dev/fb1 mplayer -vo sdl -framedrop /var/lume/lindenhurst/assets/sys/bootLong.mp4")
time.sleep(3)
os.system('su maos -c "startx -- -nocursor"')
#time.sleep(10)
#os.system("chromium-browser --noerrdialogs --no-sandbox --user-data-dir --kiosk --incognito /var/www/html/lumebot.com/public_html/semantic/inside/lindy4X.html")
