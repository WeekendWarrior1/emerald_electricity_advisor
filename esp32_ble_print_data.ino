/**
 * A BLE client example that is rich in capabilities.
 * There is a lot new capabilities implemented.
 * author unknown
 * updated by chegewara
 * modified by WeekendWarrior1 to specifically connect to an Emerald Electricity Advisor
 */

#include "BLEDevice.h"

static char *BLE_address("30:1b:97:00:00:00");
static uint32_t emerald_pass_key = 123123; //123123
static float pulses_per_kw = 1000;

// assuming getting data 30s data packets from emerald energy advisor, which is standard
static float pulse_multiplier = (2*60) / pulses_per_kw; //0.12

static BLEUUID SERVICE_TIME_UUID("00001910-0000-1000-8000-00805f9b34fb");
static BLEUUID CHAR_TIME_READ_UUID("00002b10-0000-1000-8000-00805f9b34fb");  // read, notify
static BLEUUID CHAR_TIME_WRITE_UUID("00002b11-0000-1000-8000-00805f9b34fb"); // read, write-without-response

static boolean doConnect = false;
static boolean connected = false;
static boolean doScan = false;

static BLEAdvertisedDevice *myDevice;
static BLEClient *pClient;

static BLERemoteCharacteristic *pRemoteCharacteristic_time_read;
static BLERemoteCharacteristic *pRemoteCharacteristic_time_write;

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

static void notifyCallback(
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

class MyClientCallback : public BLEClientCallbacks
{
    void onConnect(BLEClient *pclient)
    {
        connected = true;
        Serial.println("onConnect");

        Serial.print("MTU:");
        Serial.println(pclient->getMTU());
        // pclient->getMTU();
    }

    void onDisconnect(BLEClient *pclient)
    {
        connected = false;
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
        }
        else
        {
            Serial.println("auth_cmpl.failed");
            Serial.print("fail_reason: ");
            Serial.println(auth_cmpl.fail_reason);
        }
    }
};

bool connectToServer()
{
    Serial.print("Forming a connection to ");
    Serial.println(BLE_address);

    // security
    BLESecurity *pSecurity = new BLESecurity();
    pSecurity->setAuthenticationMode(ESP_BLE_SEC_ENCRYPT_MITM);
    pSecurity->setCapability(ESP_IO_CAP_KBDISP);
    pSecurity->setRespEncryptionKey(16);

    BLEDevice::setSecurityCallbacks(new MySecurityCallback());

    pClient = BLEDevice::createClient();
    Serial.println(" - Created client");
    pClient->setClientCallbacks(new MyClientCallback());

    // Connect to the remote BLE Server.
    pClient->connect(myDevice);
    Serial.println(" - Connected to server");

    // service time
    Serial.println("Attempting to get time service...");
    BLERemoteService *pRemoteService_time = pClient->getService(SERVICE_TIME_UUID);
    if (pRemoteService_time == nullptr)
    {
        Serial.print("Failed to find our time service UUID: ");
        Serial.println(SERVICE_TIME_UUID.toString().c_str());
        pClient->disconnect();
        return false;
    }
    // time char notify
    pRemoteCharacteristic_time_read = pRemoteService_time->getCharacteristic(CHAR_TIME_READ_UUID);
    if (pRemoteCharacteristic_time_read == nullptr)
    {
        Serial.print("Failed to find our characteristic UUID: ");
        Serial.println(CHAR_TIME_READ_UUID.toString().c_str());
        pClient->disconnect();
        return false;
    }
    else
    {
        // if (pRemoteCharacteristic->canNotify())
        pRemoteCharacteristic_time_read->registerForNotify(notifyCallback);
    }

    pRemoteCharacteristic_time_write = pRemoteService_time->getCharacteristic(CHAR_TIME_WRITE_UUID);
    if (pRemoteCharacteristic_time_write == nullptr)
    {
        Serial.print("Failed to find our characteristic UUID: ");
        Serial.println(CHAR_TIME_WRITE_UUID.toString().c_str());
        pClient->disconnect();
        return false;
    } else {
        Serial.println("Writing getUpdatedPowerCmd to time_write");
        // pRemoteCharacteristic_time_write->writeValue(writeRequest, true);
        pRemoteCharacteristic_time_write->writeValue(getUpdatedPowerCmd, false);
    }
    Serial.println("Write succeeded");

    connected = true;
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

    if (connected)
    {
        // currently doing nothing here, because we are instead using a notify callback
    }
    else if (doScan)
    {
        BLEDevice::getScan()->start(0); // this is just example to start scan after disconnect, most likely there is better way to do it in arduino
    }

    delay(1000); // Delay a second between loops.
} // End of loop