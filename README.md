# wiLED
Web-Inspired LED Controller for ESP8266.  

drled_arduino contains an Arduino application.  drled_arduino.ino is the Arduino sketch.  The rest of the application is in .h files the sketch includes.  
To build the sketch
1. Copy lib/net/wifi_credentials.h.example to lib/net/wifi_credentials.h and configure your SSID and password
2. Logging and unit tests can be enabled/disabled in env.h.
3. Build & upload with an Arduino too.  I use the Arduino IDE (version 1.8.16) and vscode Arduino extension (v0.4.11).

webadmin is an html application to manage drled_arduino controllers. There are no server-side functions other than serving html, js, and css files.  To run
1. Edit html/env-localhost.js to define a viewable name and a host name (or IP) for all of you controllers.
2. Change the name "env-localhost" to your servername if you use something other than localhost.  Or modifiy html/leds.html to load any env.js filename you want
3. Point your webserver to the webadmin directory
4. browse to https://localhost/html/leds.html or your server if you don't use localhost.


