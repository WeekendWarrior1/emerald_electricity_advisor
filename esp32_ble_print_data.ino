/**
 * A BLE client example that is rich in capabilities.
 * There is a lot new capabilities implemented.
 * author unknown
 * updated by chegewara
 * modified by WeekendWarrior1 to specifically connect to an Emerald Electricity Advisor
 */

#include "BLEDevice.h"

static char *BLE_address("30:1b:97:00:00:00"); // lowercase only or else will fail to match
// if your pairing pin starts with 0, eg "024024", set the emerald_pass_key as 24024
static uint32_t emerald_pass_key = 123123;
static float pulses_per_kw = 1000;

// assuming getting data 30s data packets from emerald energy advisor, which is standard
static float pulse_multiplier = (2*60.0) / pulses_per_kw; //0.12

static BLEUUID SERVICE_DEVICE_INFO_UUID("0000180A-0000-1000-8000-00805f9b34fb");
static BLEUUID CHAR_DEVICE_MANUFACTUER_UUID("00002A29-0000-1000-8000-00805f9b34fb");
static BLEUUID CHAR_DEVICE_SERIAL_UUID("00002A25-0000-1000-8000-00805f9b34fb");
static BLEUUID CHAR_DEVICE_FIRMWARE_UUID("00002A26-0000-1000-8000-00805f9b34fb");

static BLEUUID SERVICE_TIME_UUID("00001910-0000-1000-8000-00805f9b34fb");
static BLEUUID CHAR_TIME_READ_UUID("00002b10-0000-1000-8000-00805f9b34fb");  // read, notify
static BLEUUID CHAR_TIME_WRITE_UUID("00002b11-0000-1000-8000-00805f9b34fb"); // read, write-without-response

static BLEUUID SERVICE_BATTERY_UUID("0000180f-0000-1000-8000-00805f9b34fb");
static BLEUUID CHAR_BATTERY_READ_UUID("00002a19-0000-1000-8000-00805f9b34fb");  // read, notify

static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;
static boolean authenticated = false;
static boolean subscribed = false;

static BLEAdvertisedDevice *myDevice;
static BLEClient *pClient;

static BLERemoteCharacteristic *pRemoteCharacteristic_time_read;
static BLERemoteCharacteristic *pRemoteCharacteristic_time_write;
static BLERemoteCharacteristic *pRemoteCharacteristic_battery_read;

static std::string getImpulseCmd =                              "0001010500";
static std::string getPairingCodeCmd =                          "0001030100";
static std::string getEvery30sPowerConsumptionCmd =             "0001020306";
static std::string getDeviceTimeCmd =                           "0001010200";
static std::string getUpdatedPowerCmd =                         "0001020100";
static std::string getEvery30sPowerConsumptionCmdWitninHours =  "0001021308";

static std::string setImpulseCmd =          "0001010402";
static std::string startGettingHistoryCmd = "0001020400";
static std::string endGettingHistoryCmd =   "0001020600";
static std::string setDeviceTimeCmd =       "0001010104";
static std::string resetCmd =               "0001010a00";
static std::string setAutoUploadStatusCmd = "0001020b01";

static uint32_t return30sPowerConsumptionCmd =      0x0001020a06;
static uint32_t returnUpdatedPowerCmd =             0x0001020204;
static uint32_t returnEvery30sPowerConsumptionCmd = 0x000102050e;
static uint32_t returnImpulseCmd =                  0x0001010602;
static uint32_t returnPairingCodeCmd =              0x0001030206;
static uint32_t returnDeviceTimeCmd =               0x0001010304;

// returns integer value of BLE command header
// atm safely fits into uint32_t because first byte is always 00
uint32_t getCommandFromBLENotify(uint8_t *pData) {
    uint32_t commandHeader = 0;
    for (int i = 0;  i < 5; i++) {
        commandHeader += (pData[i] << (8*(4-i)));
    }
    return commandHeader;
}

uint32_t getDateFromBLENotify(uint8_t *pData) {
    uint32_t commandDateBin = 0;
    for (int i = 5;  i < 9; i++) {
        commandDateBin += (pData[i] << (8*(8-i)));
    }
    return commandDateBin;
}

// currently just prints out date in iso format
void decodeEmeraldDate(uint32_t commandDateBin) {
    // (6)year + (4)month + (5)days + (5)hours(locale adjusted) + (6)minutes + (6)seconds
    uint16_t year = 2000 + (commandDateBin >> 26);
    uint8_t month = ((commandDateBin >> 22) & 0b1111);
    uint8_t days = ((commandDateBin >> 17) & 0b11111);
    uint8_t hours = ((commandDateBin >> 12) & 0b11111);
    uint8_t minutes = ((commandDateBin >> 6) & 0b111111);
    uint8_t seconds = commandDateBin & 0b111111;

    Serial.print("Date: "); Serial.print(year); Serial.print("-");
    if (month < 10) { Serial.print("0"); } Serial.print(month); Serial.print("-");
    if (days < 10) { Serial.print("0"); } Serial.print(days); Serial.print(" ");
    if (hours < 10) { Serial.print("0"); } Serial.print(hours); Serial.print(":");
    if (minutes < 10) { Serial.print("0"); } Serial.print(minutes); Serial.print(":");
    if (seconds < 10) { Serial.print("0"); } Serial.print(seconds); Serial.println(".000");
}

uint32_t buildEmeraldDate(struct tm timeinfo) {
    uint32_t dateBin = 0;
    // dateBin += (timeinfo.tm_sec & 0b111111); // Skip seconds, as Emerald expects a 0 here
    dateBin += ((timeinfo.tm_min & 0b111111) << 6);
    dateBin += ((timeinfo.tm_hour & 0b11111) << 12);
    dateBin += ((timeinfo.tm_mday & 0b11111) << 17);
    dateBin += (((timeinfo.tm_mon + 1) & 0b1111) << 22); // Emerald expects month + 1
    dateBin += (((timeinfo.tm_year - 100) & 0b111111) << 26); //tm_year is years since 1900, so - 100 from it to get years since 2000
    return dateBin;
}

uint8_t* buildEmeraldCalenderStartEnd(struct tm timeinfo) {
    uint8_t startYear = (timeinfo.tm_year - 100);
    uint8_t startMonth = (timeinfo.tm_mon + 1);
    uint8_t startDay = timeinfo.tm_mday;
    uint8_t startHour = timeinfo.tm_hour;

    uint8_t endYear = (timeinfo.tm_year - 100);
    uint8_t endMonth = (timeinfo.tm_mon + 1);
    uint8_t endDay = timeinfo.tm_mday;
    uint8_t endHour = 23;

    static uint8_t startEndBin[8] = { startYear,startMonth,startDay,startHour, endYear,endMonth,endDay,endHour };
    Serial.print(startEndBin[0],HEX); 
    Serial.print(startEndBin[1],HEX); 
    Serial.print(startEndBin[2],HEX); 
    Serial.print(startEndBin[3],HEX); 
    Serial.print(startEndBin[4],HEX); 
    Serial.print(startEndBin[5],HEX); 
    Serial.print(startEndBin[6],HEX); 
    Serial.print(startEndBin[7],HEX); 
    Serial.println("");
    return startEndBin;
}

static void emeraldCommandCallback(
    BLERemoteCharacteristic *pBLERemoteCharacteristic,
    uint8_t *pData,
    size_t length,
    bool isNotify)
{
    Serial.print("Notify callback for characteristic ");
    Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
    Serial.print(" of data length ");
    Serial.println(length);
    Serial.print("data: ");
    for (int i = 0; i < length; i++) {
        Serial.print(" ");
        Serial.print(pData[i]);
    }
    Serial.println("");

    // 5 bytes is the command header
    if (length >= 5) {
        uint32_t commandHeader = getCommandFromBLENotify(pData);
        // Serial.print("Command Header: ");
        // Serial.println(commandHeader);
        Serial.print("Command Header HEX: ");
        Serial.println(commandHeader, HEX);

        if (commandHeader == return30sPowerConsumptionCmd) {
            Serial.println("return30sPowerConsumptionCmd:");
            if (length == 11) {

                // get date binary
                uint32_t commandDateBin = getDateFromBLENotify(pData);
                decodeEmeraldDate(commandDateBin);

                // // get power used
                uint16_t msb = pData[length - 2] << 8;
                uint16_t total_pulses = msb + pData[length - 1];
                float total_kw = total_pulses * pulse_multiplier;
                Serial.print(total_kw);
                Serial.println(" kW");
            } else {
                Serial.println("but length != 11, mangled data received");
            }
        } else if (commandHeader == returnUpdatedPowerCmd) {
            Serial.println("matches returnUpdatedPowerCmd");
        } else if (commandHeader == returnEvery30sPowerConsumptionCmd) {
            Serial.println("matches returnEvery30sPowerConsumptionCmd");
        }
    } else {
        Serial.print("Recived callback data doesn't include command header");
    }
    Serial.println("");
}

static void emeraldBatteryCallback(
    BLERemoteCharacteristic *pBLERemoteCharacteristic,
    uint8_t *pData,
    size_t length,
    bool isNotify)
{
    Serial.print("Notify callback for characteristic ");
    Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
    Serial.print(" of data length ");
    Serial.println(length);
    Serial.print("data: ");
    for (int i = 0; i < length; i++) {
        Serial.print(" ");
        Serial.print(pData[i]);
    }
    Serial.println("");
}

class MyClientCallback : public BLEClientCallbacks
{
    void onConnect(BLEClient *pclient)
    {
        connected = true;
        Serial.println("onConnect");

        // Serial.print("MTU:");
        // Serial.println(pclient->getMTU());
        // pclient->getMTU();
    }

    void onDisconnect(BLEClient *pclient)
    {
        connected = false;
        authenticated = false;
        subscribed = false;
        Serial.println("onDisconnect");
    }

    // void onWrite(BLECharacteristic *pCharacteristic)
    // {
    //     std::string value = pCharacteristic->getValue();

    //     if (value.length() > 0)
    //     {
    //         Serial.println("*********");
    //         Serial.print("New value: ");
    //         for (int i = 0; i < value.length(); i++)
    //             Serial.print(value[i]);

    //         Serial.println();
    //         Serial.println("*********");
    //     }
    // }
};

class MySecurityCallback : public BLESecurityCallbacks
{
    /**
     * @brief Its request from peer device to input authentication pin code displayed on peer device.
     * It requires that our device is capable to input 6-digits code by end user
     * @return Return 6-digits integer value from input device
     */
    uint32_t onPassKeyRequest()
    {
        Serial.println("onPassKeyRequest");
        return emerald_pass_key;
    }

    bool onConfirmPIN(uint32_t pin)
    {
        Serial.print("onConfirmPIN: ");
        Serial.println(pin);
        return true;
    }

    /**
     * @brief Provide us 6-digits code to perform authentication.
     * It requires that our device is capable to display this code to end user
     * @param
     */
    void onPassKeyNotify(uint32_t pass_key)
    {
        Serial.print("onPassKeyNotify: ");
        Serial.println(pass_key);
    }

    /**
     * @brief Here we can make decision if we want to let negotiate authorization with peer device or not
     * return Return true if we accept this peer device request
     */
    bool onSecurityRequest()
    {
        Serial.println("onSecurityRequest");
        return true; //?
    }

    /**
     * Provide us information when authentication process is completed
     */
    void onAuthenticationComplete(esp_ble_auth_cmpl_t auth_cmpl)
    {
        Serial.println("onAuthenticationComplete");
        if (auth_cmpl.success)
        {
            Serial.println("auth_cmpl.success");
            authenticated = true;
        }
        else
        {
            Serial.println("auth_cmpl.failed");
            Serial.print("fail_reason: ");
            Serial.println(auth_cmpl.fail_reason);
            authenticated = false;
        }
    }
};

bool connectToServer()
{
    Serial.print("Forming a connection to ");
    Serial.println(BLE_address);

    // security
    BLESecurity *pSecurity = new BLESecurity();
    esp_ble_io_cap_t iocap = ESP_IO_CAP_KBDISP;
    esp_ble_gap_set_security_param(ESP_BLE_SM_IOCAP_MODE, &iocap, sizeof(esp_ble_io_cap_t));

    uint8_t key_size = 16;      //the key size should be 7~16 bytes
    esp_ble_gap_set_security_param(ESP_BLE_SM_MAX_KEY_SIZE, &key_size, sizeof(uint8_t));

    esp_ble_auth_req_t auth_req = ESP_LE_AUTH_REQ_SC_MITM_BOND; //bonding with peer device after authentication
    esp_ble_gap_set_security_param(ESP_BLE_SM_AUTHEN_REQ_MODE, &auth_req, sizeof(esp_ble_auth_req_t));


    BLEDevice::setSecurityCallbacks(new MySecurityCallback());

    pClient = BLEDevice::createClient();
    Serial.println(" - Created client");
    pClient->setClientCallbacks(new MyClientCallback());

    // Connect to the remote BLE Server.
    pClient->connect(myDevice);
    Serial.println(" - Connected to server");

    connected = true;
    return true;
}

bool subscribeToCharacteristics() {
    BLERemoteService *pRemoteService_time = pClient->getService(SERVICE_TIME_UUID);
    pRemoteCharacteristic_time_read = pRemoteService_time->getCharacteristic(CHAR_TIME_READ_UUID);
    pRemoteCharacteristic_time_write = pRemoteService_time->getCharacteristic(CHAR_TIME_WRITE_UUID);
    pRemoteCharacteristic_time_read->registerForNotify(emeraldCommandCallback);
    delay(500);
    uint8_t set_auto_upload[] = {0x00, 0x01, 0x02, 0x0b, 0x01, 0x01};
    pRemoteCharacteristic_time_write->writeValue(set_auto_upload, false);
    return true;
}

/**
 * Scan for BLE servers and find the first one that advertises the service we are looking for.
 */
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
    /**
     * Called for each advertising BLE server.
     */
    void onResult(BLEAdvertisedDevice advertisedDevice)
    {
        Serial.print("BLE Advertised Device found: ");
        Serial.println(advertisedDevice.toString().c_str());

        if (advertisedDevice.getAddress().toString() == BLE_address)
        {
            Serial.println("Attempting to connect to device...");

            BLEDevice::getScan()->stop();
            myDevice = new BLEAdvertisedDevice(advertisedDevice);
            doConnect = true;
            doScan = true;

            Serial.println("Found our device");
        }
        else
        {
            Serial.println("Skipping, isn't correct device");
        }
    } // onResult
};    // MyAdvertisedDeviceCallbacks

void setup()
{
    Serial.begin(115200);
    Serial.println("Starting Arduino BLE Client application...");

    BLEDevice::init("");

    // Retrieve a Scanner and set the callback we want to use to be informed when we
    // have detected a new device.  Specify that we want active scanning and start the
    // scan to run for 5 seconds.
    BLEScan *pBLEScan = BLEDevice::getScan();
    pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
    pBLEScan->setInterval(1349);
    pBLEScan->setWindow(449);
    pBLEScan->setActiveScan(true);
    pBLEScan->start(5, false);
} // End of setup.

// This is the Arduino main loop function.
void loop()
{

    // If the flag "doConnect" is true then we have scanned for and found the desired
    // BLE Server with which we wish to connect.  Now we connect to it.  Once we are
    // connected we set the connected flag to be true.
    if (doConnect == true)
    {
        if (connectToServer())
        {
            Serial.println("We are now connected to the BLE Server.");
        }
        else
        {
            Serial.println("We have failed to connect to the server; there is nothin more we will do.");
        }
        doConnect = false;
    }

    if (connected && authenticated && !subscribed)
    {
        // set impulses on device, 00010104020C80
        Serial.print("Pulses: ");
        Serial.println(pulses_per_kw,HEX);
                
        // writeCurrentTime(6 hex)

        
        subscribed = subscribeToCharacteristics();
        if (subscribed) {
            Serial.println("Succesfully completed subscribe function");
        }
    }
    else if (doScan && !connected)
    {
        BLEDevice::getScan()->start(0); // this is just example to start scan after disconnect, most likely there is better way to do it in arduino
    }

    delay(1000); // Delay a second between loops.
} // End of loop