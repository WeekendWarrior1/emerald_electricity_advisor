_This repository is not affiliated with Emerald._

# emerald_electricity_advisor
Collection of code, tools and documentation for data retrieval from your Emerald Electricity Advisor (all reverse engineered)

## Using the ESPHome Component

The ESPHome component hasn't been merged into esphome yet, but you can use it via `external_components`

#### Requirements:
- An ESP32
- A configured Emerald Electricity Advisor
- Electricity Advisor device information:
  - BLE MAC address (can be found on device sticker, by running sketch, or by using an app like nRF Connect)
  - Connection pairing pin (6 digits you input when setting up your device, also can be found on device sticker (last 6 digits of serial))
  - Your Smart meter pulse rate (eg. 1000 pulses = 1kW/h)

```yaml
external_components:
  - source: github://WeekendWarrior1/esphome@emerald_ble
    # requires ble_client and ble_tracker because I had to add some small features to authenticate properly
    components: [ ble_client, esp32_ble_tracker, emerald_ble ]

esp32_ble_tracker:

ble_client:
  - mac_address: 30:1B:97:01:02:03
    id: emerald_advisor

sensor:
  - platform: emerald_ble
    ble_client_id: emerald_advisor
    power:
      name: "Emerald Power"
    energy:
      name: "Emerald Total Energy"
    battery_level:
      name: "Emerald Battery"
    pairing_code: 123123
    pulses_per_kWh: 1000
```
You can also find a full config here: [emerald_ble.yaml](emerald_ble.yaml) 

## Using the Arduino sketch
This sketch simply prints the energy usage and time stamp for the updates sent by the Electricity Advisor, which should be received every 30s.

It's mainly useful to demonstrate that the Electricity Advisor is connectable by a third party device.

Of interest in the sketch is the security configuration required to pair/handshake with the Electricity Advisor, which may limit what BLE technologies, libraries and languages you can use to connect (not every library seems to support secure BLE connections)

#### Requirements:
- An ESP32
- A configured Emerald Electricity Advisor
- Electricity Advisor device information:
  - BLE MAC address (can be found on device sticker, by running sketch, or by using an app like nRF Connect)
  - Connection pairing pin (6 digits you input when setting up your device, also can be found on device sticker (last 6 digits of serial))
  - Your Smart meter pulse rate (eg. 1000 pulses = 1kW/h)


#### Fill in your details at top of esp32_ble_print_data.ino and upload:
```c++
static char *BLE_address("30:1b:97:00:00:00"); // lowercase only or else will fail to match
// if your pairing pin starts with 0, eg "024024", set the emerald_pass_key as 24024
static uint32_t emerald_pass_key = 123123;
static float pulses_per_kw = 1000;
```

#### Serial Monitor output:
![Serial Monitor Example Output](assets/arduino_serial_monitor_output.png)

## Using the Emerald API
If you would prefer to use the cloud api rather than retrieving real-time data from the Electricity Advisor, please head to [api_documentation](api_documentation.md) to learn how to authenticate and retrieve energy data.

Feel free to import the postman collection if you would like to have a play or to investigate the data you can retrieve:

`Import` (ctrl+o in postman), > `link` > `Enter a URL` `https://raw.githubusercontent.com/WeekendWarrior1/emerald_electricity_advisor/main/emerald-ems.postman_collection.json` > `Continue`