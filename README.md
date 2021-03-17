# Toshiba AC - ESP8266 + Blynk
Remotely control an Air Conditioner (Toshiba in this case) with an ESP8266 board (WeMos D1 mini) and Blynk app.

I have used a Wemos D1 mini board (ESP8266 built-in).

Written by Luca Urbinati with some useful libraries: WiFiManager, Blynk library, Adafruit DHT sensor libraries, Toshiba AC library, from Mattia Rossi (link in the code).

In collaboration with FabLab Romagna: http://fablabromagna.org/.

* **Toshiba_ac_blynk.ino**
  In the code you can learn how to:
    * connect your Blynk app to an ESP8266 board, such as Wemos D1 mini,
    * send commands from your Blynk app to the board to handle a Toshiba Air Conditioning remotely via IR signals,
    * receive notifications to your email address or to your Blynk app about the status of the temperature in the environment of the board, reading a DHT11 temperature sensor.

* **Toshiba_ac_blynk.jpg**
  This is the picture that shows the configuration of Blynk App blocks used in this project.
