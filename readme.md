
# ESPBased LCD Display Frontend for Currently Playing Spotify Tracks
 > A piece from two component (esp+hd44780, flask) project   
 
 ### Github Repo URL for this project:
 - [Python Flask Middleware](https://github.com/black0803/spotify-flask-middleware-api "Middleware API for Spotify Based on Python Flask")
 - [ESP+hd44780 Frontend](https://github.com/black0803/spotify-esp-display "Front end tier of the spotify display project using ESP and hd44780 16x2 LCD")

### Application Architecture
 ![Application Architecture Diagram](https://github.com/black0803/spotify-esp-display/blob/main/img/app-architecture-design.png?raw=true)  
 The application is split into 3 roughly three components, which will be stated as frontend, middleware, and (real) backend. Frontend is covered by the ESP and hd44780 LCD, while the middleware is the whole flask deployment together with mongodb as the database, and spotify (+google translate, which is a bonus actually) as its true backend. This python application provides you an endpoint to hit spotify API using custom identities, with simplified output as well to make parsing data in ESP easier.

 ![Application Instructions Diagram](https://github.com/black0803/spotify-esp-display/blob/main/img/app-flow-design.png?raw=true)  
 The expected flow of the application when you are using ESP32 to display is as follow:  
 - ESP sends a HTTP POST Request to /get, with "Content-Type=application/x-www-form-urlencoded" and payload "uid=<username>"
   > curl representation:
   > ``curl -H "Content-Type=application/x-www-form-urlencoded" -d uid=nobita http://192.168.1.225:5000/get``
 - Python Flask fetch access token that are saved on MongoDB
 - Using the access token from mongodb, Python hit POST message to https://api.spotify.com/v1/me/player, and wait for the json file sent by spotify API to be processed for Flask's response
 - Python flask parse the received json, and send only the necessary part for the ESP to use.
 - ESP receive the response from Python Flask, and then display the result to the LCD.


 ### How to use:
 - Copy ``env.h`` file from ``env-example.h``, and adjust the variables inside (``SSID``, ``PASSWORD``, ``WEB_SERVER``, and ``USER``) to suit your need.
 - If you are using ESP32, you will be using ``WiFi.h`` and ``HTTPClient.h``. However, ESP8266 users will be using ``ESP8266WiFi.h`` and ``ESP8266HTTPClient.h`` instead. Comment and uncomment based on your microcontroller.
 ![ESP wifi library](https://github.com/black0803/spotify-esp-display/blob/main/img/library-option.PNG?raw=true) 
 - Compile and Upload binary to ESP32 or ESP8266 based on your microcontroller.
 - Adjust LCD to be on connected to the I2C interface.
 - Start the ESP

 ### Wokwi Preview:
 You can view the ESP32 mockup via: https://wokwi.com/projects/346513317585486418

 Querying for user ``nobita``, you can see my currently playing song on the wokwi simulation (if I am playing one). Some minor change on the lcd library to be compatible with what wokwi provides.

 ### Product Preview:
 ![LCD Display](https://github.com/black0803/spotify-esp-display/blob/main/img/display.jpg?raw=true)  
 This is how the display will look if the ESP32 is connected to the middleware.

  ### (Planned) Future Improvements:
 - Showing the song duration and current progress of the song. I haven't consider how things will be shown on the LCD as currently it already occupies the whole 16x2 slot as is. However, it is possible to implement this and I will try to do it as well.

  ### Known Issues:
 - hd44780 LCD is a very small LCD with limitations that restricts the outputted text to only ASCII letters. This means that non-ASCII based letters (arabic, korean, japanese, which utilizes unicode[UTF-8 or UTF-16]) makes it undisplayable to the screen. To circumvent this, I actually applied a Google Translate library provided as a method to pass the load of translating the title and artist name. This straightforward approach means some names do not get translated properly, which makes it sort of bad. If you have any recommendation on what to use to merely transcribe the letters into plaintext, that will be helpful. Using TFT LCD is also a consideration, but I probably need to learn the display first.