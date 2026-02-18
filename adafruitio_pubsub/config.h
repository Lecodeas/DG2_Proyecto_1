/************************ Adafruit IO Config *******************************/

// visit io.adafruit.com if you need to create an account,
// or if you need your Adafruit IO key.
#define IO_USERNAME  "Dohrim"
#define IO_KEY       "aio_Hfmp94NvhBihgbhrKUDPOJ3XHbPZ"

#define WIFI_SSID "Ayumu"
#define WIFI_PASS "Saturno2021"

#include "AdafruitIO_WiFi.h"

AdafruitIO_WiFi io(IO_USERNAME, IO_KEY, WIFI_SSID, WIFI_PASS);
