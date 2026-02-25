#this file is called by the interface to upload the desired sketch to the arduino
#due to the requirements of the ino build tool
import os
#the directory where the actual scketch lives
os.chdir("/home/maos/ino/tuner/")
the command to upload
os.system("ino upload -m mega2560")
