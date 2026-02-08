/*


00:49:19.316 -> rst:0x1 (POWERON_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
00:49:19.317 -> configsip: 0, SPIWP:0xee
00:49:19.317 -> clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
00:49:19.317 -> mode:DIO, clock div:2
00:49:19.317 -> load:0x3fff0030,len:4
00:49:19.317 -> load:0x3fff0034,len:6968
00:49:19.317 -> load:0x40078000,len:13072
00:49:19.317 -> ho 0 tail 12 room 4
00:49:19.317 -> load:0x40080400,len:3896
00:49:19.317 -> entry 0x40080688
00:49:19.351 -> [0;32mI (30) boot: ESP-IDF v4.1-dirty 2nd stage bootloader[0m
00:49:19.351 -> [0;32mI (30) boot: compile time 16:15:01[0m
00:49:19.351 -> [0;32mI (30) boot: chip revision: 1[0m
00:49:19.351 -> [0;32mI (34) boot_comm: chip revision: 1, min. bootloader chip revision: 0[0m
00:49:19.351 -> [0;32mI (41) boot.esp32: SPI Speed      : 40MHz[0m
00:49:19.351 -> [0;32mI (45) boot.esp32: SPI Mode       : DIO[0m
00:49:19.351 -> [0;32mI (50) boot.esp32: SPI Flash Size : 4MB[0m
00:49:19.384 -> [0;32mI (54) boot: Enabling RNG early entropy source...[0m
00:49:19.384 -> [0;32mI (60) boot: Partition Table:[0m
00:49:19.384 -> [0;32mI (63) boot: ## Label            Usage          Type ST Offset   Length[0m
00:49:19.384 -> [0;32mI (71) boot:  0 nvs              WiFi data        01 02 00009000 00005000[0m
00:49:19.384 -> [0;32mI (78) boot:  1 otadata          OTA data         01 00 0000e000 00002000[0m
00:49:19.384 -> [0;32mI (86) boot:  2 app0             OTA app          00 10 00010000 00300000[0m
00:49:19.417 -> [0;32mI (93) boot:  3 spiffs           Unknown data     01 82 00310000 000f0000[0m
00:49:19.418 -> [0;32mI (101) boot: End of partition table[0m
00:49:19.418 -> [0;32mI (105) boot_comm: chip revision: 1, min. application chip revision: 0[0m
00:49:19.418 -> [0;32mI (112) esp_image: segment 0: paddr=0x00010020 vaddr=0x3f400020 size=0x1d2048 (1908808) map[0m
00:49:20.148 -> [0;32mI (848) esp_image: segment 1: paddr=0x001e2070 vaddr=0x3ffbdb60 size=0x04d3c ( 19772) load[0m
00:49:20.149 -> [0;32mI (856) esp_image: segment 2: paddr=0x001e6db4 vaddr=0x40080000 size=0x00400 (  1024) load[0m
00:49:20.182 -> [0;32mI (857) esp_image: segment 3: paddr=0x001e71bc vaddr=0x40080400 size=0x08e54 ( 36436) load[0m
00:49:20.182 -> [0;32mI (879) esp_image: segment 4: paddr=0x001f0018 vaddr=0x400d0018 size=0x9df74 (647028) map[0m
00:49:20.428 -> [0;32mI (1126) esp_image: segment 5: paddr=0x0028df94 vaddr=0x40089254 size=0x0b6a0 ( 46752) load[0m
00:49:20.462 -> [0;32mI (1158) boot: Loaded app from partition at offset 0x10000[0m
00:49:20.462 -> [0;32mI (1158) boot: Disabling RNG early entropy source...[0m
00:49:20.462 -> E (59) psram: PSRAM ID read error: 0xffffffff
00:49:20.496 -> 
00:49:20.530 -> E (116) sdmmc_common: sdmmc_init_ocr: send_op_cond (1) returned 0x107
00:49:20.530 -> Card Mount Failed
00:49:20.530 -> PSRAM Initialization Failure
00:49:20.739 -> [E][camera.c:1113] camera_probe(): Detected camera not supported.
00:49:20.739 -> [E][camera.c:1379] esp_camera_init(): Camera probe failed with error 0x20004
00:49:20.739 -> Camera init failed with error 0x20004





// Define the GPIO pins you want to control
int outputPins[TOTAL_O_PINS] = {16, 17, 18, 19, 21 }; //16,17,18,19 for the 4 channel relay. 21 for the piezo buzzer
int inputPins[TOTAL_I_PINS] = {13,14}; //13 and 14 for the two level sensors.

*/

#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

const char* ssid = "b";
const char* password = "n";

// use first channel of 16 channels (started from zero)
#define LEDC_CHANNEL_0     0

// use 13 bit precission for LEDC timer
#define LEDC_TIMER_13_BIT  13

// use 5000 Hz as a LEDC base frequency
#define LEDC_BASE_FREQ     5000

// fade LED PIN (replace with LED_BUILTIN constant for built-in LED)
#define LED_PIN           21



// Arduino like analogWrite
// value has to be between 0 and valueMax
void ledcAnalogWrite(uint8_t channel, uint32_t value, uint32_t valueMax = 255) {
  // calculate duty, 8191 from 2 ^ 13 - 1
  uint32_t duty = (8191 / valueMax) * min(value, valueMax);

  // write duty to LEDC
  ledcWrite(channel, duty);
}



void setup() {
  Serial.begin(115200);
  Serial.println("Booting");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  // Port defaults to 3232
  // ArduinoOTA.setPort(3232);

  // Hostname defaults to esp3232-[MAC]
  // ArduinoOTA.setHostname("myesp32");

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

  ArduinoOTA.begin();

  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

    ledcSetup(LEDC_CHANNEL_0, LEDC_BASE_FREQ, LEDC_TIMER_13_BIT);
  ledcAttachPin(LED_PIN, LEDC_CHANNEL_0);
}

void loop() {
  ArduinoOTA.handle();
    // set the pitch of the buzzer on LEDC channel 0
  ledcAnalogWrite(LEDC_CHANNEL_0, 127);

 
}
