# Giant-Clock

This is my code to control an ESP32 controller for Ivan Miranda's [Giant Clock](https://www.youtube.com/watch?v=4z82I-032zs)

*Please note*: My segment `Segment::A` is different from Ivan's and likely different from yours.  My segments go from top-left, and proceed clockwise:
- top-left - `Segment::A`
- top - `Segment::B`
- top-right - `Segment::C`
- bottom-right - `Segment::D`
- bottom - `Segment::E`
- bottom-left - `Segment::F`
- center - `Segment::G`

If you would like to use this code for your ESP32, please create an `include/secret.h` with proper `const char*` for:
- `SSID`
- `WIFI_PASSWORD`
- `HOSTNAME`

To update using OTA, follow the PlatformIO documentation for such a process, and set in your platformio.ini:
- `upload_protocol`
- `upload_port`

The clock will read (in order):
- LoAd
- run
- SEt
- [the time - once set from NTP]

Enjoy your giant clock!
