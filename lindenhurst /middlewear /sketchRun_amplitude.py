#!/usr/bin/python
#this file is called by the interface to upload the desired sketch to the arduino
#due to the requirements of the ino build tool
import os
#the directory where the actual sketch lives
os.chdir("/var/lume/lindenhurst/arduino/amplitude/")
#the command to upload
os.system("ino upload -m uno")
