# NMEA0183-WiFi
Pumping Data From Serial to Wifi on the cheap with an esp8266 using the arduino core. 

Just copy credentials.h.example to credentials.h and fill in the blanks.

Beware: You're on your own here. This is for 5V logic level working with a NASA Clipper sumlog. If it doesn't work with your setup then don't blame me.

I like to keep things snappy, so the sourcecode is as small as possible, as is the circuit. It just takes data in and sends everyline which begins with a "$" via udp to the host setup in the settings. Technically it's not even using NMEA-0183, it's just forwarding data.

No weird magic, no smoke&mirrors, just standard libraries. The only exeption to this is the watchdog implementation. Since the arduino core uses the esp watchdog and I don't want to mess with that we resort to a soft watchdog. The logic behind the watchdog is easy: If we haven't followed the happy path once in 30s then reboot. Not the the most sophisticated error handling but effective.

Due to the TTL to RS232 conversion we need to use this Softserial library because the esp8266 UART can't do inverted logic. 

https://github.com/plerup/espsoftwareserial

Further versions probably will include a level shifter instead of the voltage devider and softserial.   
