![alt text](https://github.com/Protonerd/FX-SaberOS/blob/master/README/22489987_10214390565742471_6678630774083988688_n.jpg)

# FX-SaberOS
System code for Arduino based Lightsaber replicas for DIYino family boards and Arduino based home brew systems.

This repository is currently in development.  The currently posted builds should be fully functional, but are ocassional updates and changes.  Please review the Config files for build notes prior to deployment.

Installation and configuration instructions are available in the Wiki https://github.com/Protonerd/FX-SaberOS/wiki

For Discussion, FAQ and Troubleshooting please visit us on Facebook at https://www.facebook.com/groups/FXSaberOS/

## Recent Important Changes:
* There are now two clash methods.  The original "POLL" method, and a new "INT" method which is less intensive but requires attaching INT on the MPU6050 to D2 of the controller board.
https://github.com/Protonerd/FX-SaberOS/wiki/Clash-Method

* Several libraries have been updated (I2Cdev, MPU6050, OneButton, DFPlayer).  If you're updating from a previous build, make sure to replace the old libraries with the new ones. In case of compilation errors when using Arduino IDE 2.x: close the IDE, clear the cache (on Windows by default under "C:\Users\\<your_username>\AppData\Local\Temp") and relaunch.
