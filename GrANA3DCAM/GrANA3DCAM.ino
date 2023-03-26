//Codigo disponivel no github desenvolvido por @SantoCyber
#include <WiFi.h>
#include "Arduino.h"
#include <StringArray.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <NTPClient.h>
#include <FS.h>
#include <WiFiClientSecure.h>
#include <time.h>
#include <ArduinoJson.h>
#include "EEPROM.h"
#include <ESPmDNS.h>
#include "FastLED.h"


//#######################################ATIVA FUNCOES


#define CAMERA 1
#define SDCARD 1
#define USEPIR 0


//#############################################################################Configura GPIO
#include <SD_MMC.h>
#include <SD.h>
#include "esp_vfs_fat.h"
#include "driver/sdmmc_host.h"
#include "sdmmc_cmd.h"


#define SD_MMC_CMD 38 //Please do not modify it.
#define SD_MMC_CLK 39 //Please do not modify it. 
#define SD_MMC_D0  40 //Please do not modify it.


#define LEDBUILTIN  48 
#define LED_PIN  47 //2
#define PIR 19



//################## IWDT
#include <ESP32Ping.h>
#include <esp_task_wdt.h>


#if (CAMERA == 1)

#include "soc/rtc_cntl_reg.h"  // Disable brownour problems
#include "soc/soc.h"           // Disable brownour problems
#include "sdkconfig.h"
#include "driver/rtc_io.h"
#include <esp_system.h>
#include <nvs_flash.h>
#include "esp_camera.h"
#endif


//################################################################################Task handle 
#include <stdio.h>
#include <driver/gpio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_attr.h>
#include "esp_heap_caps.h"

#define MBEDTLS_ERR_NET_RECV_FAILED                       -0x004C 



TaskHandle_t xHandle = NULL;


#define STACK_SIZE 10000

 // Structure that will hold the TCB of the task being created.
 StaticTask_t xTaskBuffer;

 // Buffer that the task being created will use as its stack.  Note this is
 // an array of StackType_t variables.  The size of StackType_t is dependent on
 // the RTOS port.
// StackType_t *xStack;

 StackType_t xStack[ STACK_SIZE ];



//############################################ Create AsyncWebServer object on port 80
AsyncWebServer server(80);

//#################
CRGB ledflash[1];

//####################################################Configura variaveis

bool motionSensorFlag = false;
String timeLapse;
String pirState;
String ledStateCAM = "OFF";
boolean led_state = true;


//Variables to save values from HTML form
String ssid;
String pass;
String nomedobot;
String tokentelegram;

String id,idX, welcome, from_name, text;//Váriaveis para armazenamento do ID e TEXTO gerado pelo Usuario
String msg;
String photo;


uint32_t notConnectedCounter = 0;
uint32_t ConnectedCounter = 0;
uint32_t PIRcount = 0;


// Search for parameter in HTTP POST request
const char* PARAM_INPUT_1 = "ssid";
const char* PARAM_INPUT_2 = "pass";
const char* PARAM_INPUT_5 = "tokentelegram";
const char* PARAM_INPUT_14 = "nomedobot";


// File paths to save input values permanently
const char* ssidPath = "/ssid.txt";
const char* passPath = "/pass.txt";
const char* nomedobotPath = "/nomedobot.txt";
const char* tokentelegramPath = "/tokentelegram.txt";
const char* FILE_PHOTO = "/photo.jpg";
const char* TIMELAPSE = "/timelapse.txt";
const char* PIRSAVE = "/pirsave.txt";


//##configura o millis
const long interval = 10000;  // interval to wait for Wi-Fi connection (milliseconds)
unsigned long previousMillis = 0;
unsigned long tempo = millis();
unsigned long tempoverifica = millis();
unsigned long tempo3 = millis();
unsigned long tempo4 = millis();
unsigned long tempo5 = millis();
unsigned long tempo6 = millis();
unsigned long tempo7 = millis();
unsigned long tempo8 = millis();
unsigned long tempo9 = millis();
unsigned long tempo10 = millis();
unsigned long tempoping = millis();
unsigned long tempotelegram = millis();
unsigned long tempotimelapse = millis();
unsigned long tempoumidade = millis();





//############CONFIG TIMER
#define NTP_OFFSET  -3  * 60 * 60 // In seconds
#define NTP_INTERVAL 60 * 1000    // In miliseconds
#define NTP_ADDRESS  "0.pool.ntp.org"
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, NTP_ADDRESS, NTP_OFFSET, NTP_INTERVAL);
const int udpPort = 3333;


//##################### Configura IP

IPAddress localIP;
//IPAddress localIP(192, 168, 1, 200); // hardcoded

// Set your Gateway IP address
IPAddress localGateway;
//IPAddress localGateway(192, 168, 1, 1); //hardcoded
IPAddress subnet(255, 255, 0, 0);
IPAddress primaryDNS(8, 8, 8, 8); //nao nao eh opcional nao, eh obrigatorio demorei mt tempo pra perceber q o relogio e o telegram n tava funcionando por causa do dns


//##########Configura cam
boolean takeNewPhoto = false;



#if (CAMERA == 1)
int max_frames = 20;
framesize_t configframesize = FRAMESIZE_VGA; // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
int frame_interval = 500;          // 0 = record at full speed, 100 = 100 ms delay between frames
float speed_up_factor = 1;          // 1 = play at realtime, 0.5 = slow motion, 10 = speedup 10x
int framesize = FRAMESIZE_VGA; //FRAMESIZE_HD;
int quality = 8;
int qualityconfig = 4;

struct tm timeinfo;
time_t now;



camera_fb_t * fb = NULL;
camera_fb_t * vid_fb = NULL;

TaskHandle_t the_camera_loop_task;
TaskHandle_t fototgtask;
TaskHandle_t savesdtask;
TaskHandle_t savespifftask;
void the_camera_loop(void* pvParameter) ;



bool video_ready = false;
bool picture_ready = false;
bool active_interupt = false;
bool pir_enabled = false;
bool avi_enabled = false;

int avi_buf_size = 0;
int idx_buf_size = 0;

bool isMoreDataAvailable();


////////////////////////////////  send photo as 512 byte blocks or jzblocksize 
int currentByte;
uint8_t* fb_buffer;
size_t fb_length;

bool isMoreDataAvailable() {
  return (fb_length - currentByte);
}

uint8_t getNextByte() {
  currentByte++;
  return (fb_buffer[currentByte - 1]);
}

////////////////////////////////  send avi as 512 byte blocks or jzblocksize 
int avi_ptr;
uint8_t* avi_buf;
size_t avi_len;

bool avi_more() {
  return (avi_len - avi_ptr);
}

uint8_t avi_next() {
  avi_ptr++;
  return (avi_buf[avi_ptr - 1]);
}

bool dataAvailable = false;


///////////////////////////////

uint8_t * psram_avi_buf = NULL;
uint8_t * psram_idx_buf = NULL;
uint8_t * psram_avi_ptr = 0;
uint8_t * psram_idx_ptr = 0;
char strftime_buf[64];


/// defined(CAMERA_MODEL_ESP32S3_EYE)
#define PWDN_GPIO_NUM -1
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 15
#define SIOD_GPIO_NUM 4
#define SIOC_GPIO_NUM 5

#define Y2_GPIO_NUM 11
#define Y3_GPIO_NUM 9
#define Y4_GPIO_NUM 8
#define Y5_GPIO_NUM 10
#define Y6_GPIO_NUM 12
#define Y7_GPIO_NUM 18
#define Y8_GPIO_NUM 17
#define Y9_GPIO_NUM 16

#define VSYNC_GPIO_NUM 6
#define HREF_GPIO_NUM 7
#define PCLK_GPIO_NUM 13


#endif


//############################################################### Arquivos 
// Initialize SPIFFS
void initSPIFFS() {
  if (!SPIFFS.begin(true)) {
    Serial.println("An error has occurred while mounting SPIFFS");
  }
  Serial.println("SPIFFS mounted successfully");
}



// Read  TOTAL File from SPIFFS
String readTotal(fs::FS &fs, const char * path){
  Serial.printf("Reading file: %s\r\n", path);

  File file = fs.open(path);
  if(!file || file.isDirectory()){
    Serial.println("- failed to open file for reading");
    return String();
  }
  
  String fileContent;
  while(file.available()){
    fileContent = file.readString();
    break;     
  }
    fileContent.trim();
 // fileContent.length() - 1;

  return fileContent;
}



// Read File from SPIFFS
String readFile(fs::FS &fs, const char * path){
  Serial.printf("Reading file: %s\r\n", path);

  File file = fs.open(path);
  if(!file || file.isDirectory()){
    Serial.println("- failed to open file for reading");
    return String();
  }
  
  String fileContent;
  while(file.available()){
    fileContent = file.readStringUntil('\n');
    break;     
  }
  fileContent.trim();
  fileContent.length();
  return fileContent;
}

 



// Write file to SPIFFS
void writeFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Writing file: %s\r\n", path);

  File file = fs.open(path, FILE_WRITE);
  if(!file){
    Serial.println("- failed to open file for writing");
    return;
  }
  if(file.print(message)){
    Serial.println("- file written");
  } else {
    Serial.println("- frite failed");
  }
}


// Adicinando no arquivo append file to SPIFFS
void addFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("ADD file: %s\r\n", path);

  File file = fs.open(path, FILE_APPEND);
  if(!file){
    Serial.println("- failed to open file for writing");
    return;
  }
  if(file.print(message)){
    Serial.println("- file adicionado");
    return;
  } else {
    Serial.println("- frite failed");
  }
}



//####################################### Configura  cartao SD

String readFileSD(fs::FS &fs, const char * path){
  Serial.printf("Reading file: %s\r\n", path);
    File root = SD_MMC.open(path);

  //File root = fs.open(path);
  if(!root || root.isDirectory()){
    Serial.println("- failed to open file for reading");
    return String();
  }
  
  String fileContent;
  while(root.available()){
    fileContent = root.readString();
    break;     
  }
  fileContent.trim();
  fileContent.length();
  return fileContent;
}


void deleteFile(fs::FS &fs, const char * path){
    Serial.printf("Deleting file: %s\n", path);
    if(fs.remove(path)){
        Serial.println("File deleted");
    } else {
        Serial.println("Delete failed");
    }
}


void writeFileSD(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Writing file: %s\n", path);
    //File root = SD_MMC.open(path, FILE_WRITE);

    File root = fs.open(path, FILE_WRITE);
    if(!root){
        Serial.println("Failed to open file for writing");
        return;
    }
   
}



void appendFileSD(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Appending to file: %s\n", path);

    File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial.println("Failed to open file for appending");
        return;
    }
    if(file.print(message)){
        Serial.println("Message appended");
    } else {
        Serial.println("Append failed");
    }
}



//###########################################################INICIA TELEGRAM TOKEN via web
String TOKEN(){
  initSPIFFS();
    tokentelegram = readFile (SPIFFS, tokentelegramPath);

String TTT = tokentelegram.c_str();
return TTT; 
}
//#include <UniversalTelegramBot.h>
#include "UniversalTelegramBot.h"
#define tokentele TOKEN()

WiFiClientSecure client;

UniversalTelegramBot bot(tokentele, client);



//######################################################################## Initialize WiFi
bool initWiFi() {
  if(ssid==""){
    Serial.println(" SSID indefindo");
    return false;
  }

  WiFi.mode(WIFI_STA);
  WiFi.setHostname(nomedobot.c_str());
  localIP;
  localGateway;


  if (!WiFi.config(localIP, localGateway, subnet,primaryDNS)){
    Serial.println("STA Failed to configure");
    return false;
  }
  WiFi.begin(ssid.c_str(), pass.c_str());
  
 

  
  Serial.println("Connecting to WiFi...");

  unsigned long currentMillis = millis();
  previousMillis = currentMillis;

  while(WiFi.status() != WL_CONNECTED) {
    currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      Serial.println("Failed to connect.");
      return false;
    }
  }



  if (!MDNS.begin(nomedobot.c_str())) {
    Serial.println("Error setting up MDNS responder!");
    return false;
  } else {
    MDNS.addService("http", "tcp", 80);
    Serial.printf("mDNS responder started '%s'\n", nomedobot.c_str());
    Serial.print("You can now connect to http://");
    Serial.print(nomedobot.c_str());
    Serial.println(".local");   

 }
  
 //##################Configura servicoes q precisa de conexao
  
    timeClient.begin();
    timeClient.update();

      
  Serial.println(WiFi.localIP());
  return true;
}




#if (CAMERA == 1)

bool setupCamera()
{
  camera_config_t config;
 config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
 // config.frame_size = FRAMESIZE_QVGA;
  config.pixel_format = PIXFORMAT_JPEG; // for streaming
  //config.pixel_format = PIXFORMAT_RGB565; // for face detection/recognition
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;
//  config.jpeg_quality = 12;
//  config.fb_count = 1;
  
  //init with high specs to pre-allocate larger buffers
  if (psramFound()) {
    config.frame_size = configframesize;
    config.jpeg_quality = qualityconfig;
    config.fb_count = 4; 
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

  Serial.printf("Internal Total heap %d, internal Free Heap %d\n", ESP.getHeapSize(), ESP.getFreeHeap());
  Serial.printf("SPIRam Total heap   %d, SPIRam Free Heap   %d\n", ESP.getPsramSize(), ESP.getFreePsram());

  static char * memtmp = (char *) malloc(32 * 1024);
  static char * memtmp2 = (char *) malloc(32 * 1024); //32767

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return false;
  }
 
  free(memtmp2);
  memtmp2 = NULL;
  free(memtmp);
  memtmp = NULL;


  sensor_t * s = esp_camera_sensor_get();

  //  drop down frame size for higher initial frame rate
  s->set_framesize(s, (framesize_t)framesize);
  s->set_quality(s, quality);
  delay(200);
  return true;
}

#endif

//######################## SETUP

void setup() {

    Serial.begin(115200);
    Serial.setDebugOutput(true);
    Serial.println();
    pinMode(LEDBUILTIN, OUTPUT);

//if (nvs_flash_init() != ESP_OK) {
//    printf("nvs_flash_init failed\r\n");
//  }
#if (SDCARD == 1)
  // Initialize SDMMC host driver
  sdmmc_host_t host = SDMMC_HOST_DEFAULT();
  sdmmc_slot_config_t slot_config = SDMMC_SLOT_CONFIG_DEFAULT();

  sdmmc_card_t* card;



    SD_MMC.setPins(SD_MMC_CLK, SD_MMC_CMD, SD_MMC_D0);
    if (!SD_MMC.begin("/sdcard", true, true, 20000, 5)) {
      Serial.println("Card Mount Failed");
     // return;
    }
    
    uint8_t cardType = SD_MMC.cardType();
    if(cardType == CARD_NONE){
        Serial.println("No SD_MMC card attached");
        //return;
    }
  

    Serial.print("SD_MMC Card Type: ");
    if(cardType == CARD_MMC){
        Serial.println("MMC");
    } else if(cardType == CARD_SD){
        Serial.println("SDSC");
    } else if(cardType == CARD_SDHC){
        Serial.println("SDHC");
    } else {
        Serial.println("UNKNOWN");
    }



    Serial.printf("Total space: %lluMB\r\n", SD_MMC.totalBytes() / (1024 * 1024));
    Serial.printf("Used space: %lluMB\r\n", SD_MMC.usedBytes() / (1024 * 1024));

#endif



//########### LED

    FastLED.addLeds<WS2812, LEDBUILTIN, GRB>(ledflash, 1).setCorrection(TypicalSMD5050);





  //desahabilita o watchdog configurando o timeout para 30 segundos

  esp_task_wdt_init(30, true);


//  heap_caps_malloc_extmem_enable(0);
  //Disable watchdog timers
  disableCore0WDT();
 // disableCore1WDT();
  disableLoopWDT();
  esp_task_wdt_delete(NULL);
  //rtc_wdt_protect_off();
  //rtc_wdt_disable();


    EEPROM.begin(512);

// Turn-off the 'brownout detector'
 // WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0);

//#########################################################inicia sensores tem umi pressao



#if (CAMERA == 1)
  // Camera init

  int avail_psram = ESP.getFreePsram();
  Serial.print("PSRAM size to store the video "); Serial.println(avail_psram);
  idx_buf_size = max_frames * 10 + 20;
//  if (hdcam) {
    avi_buf_size = avail_psram - 900 * 1024; //900 for hd, 500 for vga
    Serial.println("Camera to HD ");
  Serial.print("try to allocate "); Serial.println(avi_buf_size);
  psram_avi_buf = (uint8_t*)ps_malloc(avi_buf_size);
  if (psram_avi_buf == 0) Serial.printf("psram_avi allocation failed\n");
  psram_idx_buf = (uint8_t*)ps_malloc(idx_buf_size); // save file in psram
  if (psram_idx_buf == 0) Serial.printf("psram_idx allocation failed\n");


  
if (!setupCamera())
  {
    Serial.println("Camera Setup Failed!");
  }

#endif
 


 //########################Le arquivos gravados             
  //ja foi iniciado no telegram
  //initSPIFFS();
  
  // Load values saved in SPIFFS
  ssid = readFile(SPIFFS, ssidPath);
  pass = readFile(SPIFFS, passPath);
  nomedobot = readFile(SPIFFS, nomedobotPath);
  tokentelegram = readFile (SPIFFS, tokentelegramPath);
  photo = readFile (SPIFFS, FILE_PHOTO);
  timeLapse = readFile (SPIFFS, TIMELAPSE);
  pirState = readFile (SPIFFS, PIRSAVE);

#if (USEPIR == 1)

if (pirState == "pirON"){
motionSensorFlag = true;

Serial.println("pirON"); }
else{Serial.println("pirOFF");
motionSensorFlag = false;
}

#endif




  Serial.println(ssid);
  Serial.println(pass);
  Serial.println(tokentelegram);
  Serial.println(timeLapse);
  Serial.println(pirState);


  if(initWiFi()) {

   delay(200);
//client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
   client.setTimeout(2000);

   client.setInsecure();

delay(500);

delay(200);
       

    // Route for root / web page
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(SPIFFS, "/index.html", "text/html", false);
    });
    server.serveStatic("/", SPIFFS, "/");

    server.on("/lersd", HTTP_GET, listFilesOnWebPage);
    server.on("/lersdx/*", HTTP_GET, handleFile);
    server.on("/apagarsd", HTTP_GET, apagarsd);
    server.on("/flashcam", HTTP_GET, flashcam);
    server.on("/pir", HTTP_GET, pir);
    server.on("/timelapse", HTTP_GET, timelapse);


    server.on("/apagaconfig", HTTP_GET, [](AsyncWebServerRequest *request) {
     // ledState = "deletelog";
      writeFile(SPIFFS, ssidPath, "");
      writeFile(SPIFFS, passPath, "");
      writeFile(SPIFFS, tokentelegramPath, "");
      request->send(SPIFFS, "/index.html", "text/html", false);
    });

    
      

            server.on("/restart", HTTP_GET, [](AsyncWebServerRequest * request) {
ESP.restart();
//    request->send_P(200, "text/plain", "Taking Photo");
   // request->send(SPIFFS, "/", "text/html", false, processor);
  });

  server.on("/indexx", HTTP_GET, [](AsyncWebServerRequest * request) {
 request->send(SPIFFS, "/index.html", "text/html", false);
  }); 
 server.on("/index", HTTP_GET, [](AsyncWebServerRequest * request) {
 request->send(SPIFFS, "/index.html", "text/html", false);
  });
      
      
      server.on("/capture", HTTP_GET, [](AsyncWebServerRequest * request) {
    Serial.println("TASK capture pagina stream");
    takeNewPhoto = true;
    request->send(SPIFFS, "/index.html", "text/html", false);
  });

  server.on("/saved-photo", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, FILE_PHOTO, "image/jpg", false);
  });


    server.begin();

delay(300);
  
  }
  else {
    // Connect to Wi-Fi network with SSID and password
    Serial.println("Setting AP (Access Point)");
    // NULL sets an open Access Point
    WiFi.softAP("GrANA3D-config", NULL);

    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP); 

    // Web Server Root URL
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
      request->send(SPIFFS, "/wifimanager.html", "text/html");
    });
    
    server.serveStatic("/", SPIFFS, "/");

    server.on("/capture", HTTP_GET, [](AsyncWebServerRequest * request) {
    takeNewPhoto = true;
    request->send(SPIFFS, "/wifimanager.html", "text/html", false);
    });

    server.on("/saved-photo", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, FILE_PHOTO, "image/jpg", false);
    });

    server.on("/", HTTP_POST, [](AsyncWebServerRequest *request) {
      int params = request->params();
      for(int i=0;i<params;i++){
        AsyncWebParameter* p = request->getParam(i);
        if(p->isPost()){
          // HTTP POST ssid value
          if (p->name() == PARAM_INPUT_1) {
            ssid = p->value().c_str();
            Serial.print("SSID set to: ");
            Serial.println(ssid);
            // Write file to save value
            writeFile(SPIFFS, ssidPath, ssid.c_str());
          }
          // HTTP POST pass value
          if (p->name() == PARAM_INPUT_2) {
            pass = p->value().c_str();
            Serial.print("Password set to: ");
            Serial.println(pass);
            // Write file to save value
            writeFile(SPIFFS, passPath, pass.c_str());
          }

          // HTTP POST tokentelegram value
          if (p->name() == PARAM_INPUT_5) {
            tokentelegram = p->value().c_str();
            Serial.print("Token Telgeram set to: ");
            Serial.println(tokentelegram);
            // Write file to save value
            writeFile(SPIFFS, tokentelegramPath, tokentelegram.c_str());
            }
            
                  // HTTP POST tokentelegram value
          if (p->name() == PARAM_INPUT_14) {
            nomedobot = p->value().c_str();
            Serial.print("Nome do bot set to: ");
            Serial.println(nomedobot);
            // Write file to save value
            writeFile(SPIFFS, nomedobotPath, nomedobot.c_str());
          }


          //Serial.printf("POST[%s]: %s\n", p->name().c_str(), p->value().c_str());
        }
      }


String configg = "Prontinho. GrANA3DCAM reiniciando, conecte no seu WIFI e clique no endereco abaixo";
       configg += "<br><br>";
       configg += "CLIQUE -> VVVVVVVV";
       configg += "<br><br>";
       configg += "<a href='";
       configg += "http://";       
       configg += nomedobot.c_str();
       configg += ".local";
       configg += "'>";
       configg += nomedobot.c_str();
       configg += ".local";             
       configg += "</a>";

      
      request->send(200, "text/html", configg);
      delay(3000);
      ESP.restart();
    });
    server.begin();
  }




//############################################ Configura LOOP TELEGRAM
 
 //xStack = (uint8_t*)heap_caps_calloc(1, 5000, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT | MALLOC_CAP_32BIT);
//pxStackBuffer = (uint8_t*)heap_caps_calloc(1, 5000, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT | MALLOC_CAP_32BIT);
  delay(200);


     // Create the task pinned to core 0 without using any dynamic memory allocation.
     xHandle = xTaskCreateStatic(
   //  xHandle = xTaskCreateStaticPinnedToCore(
                   TELE,       // Function that implements the task.
                   "TELE",          // Text name for the task.
                   STACK_SIZE,      // Stack size in bytes, not words.
                   ( void * ) 1,    // Parameter passed into the task.   ( void * ) 1
                   1,// Priority at which the task is created.
                   xStack,          // Array to use as the task's stack.
                   &xTaskBuffer);//,    // Variable to hold the task's data structure.
         //          0 );     

//######################################### Sensor Acelerometro


}





void loop() {



ifs();
}
