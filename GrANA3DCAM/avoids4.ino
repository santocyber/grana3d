//feito pela AI



void apagarsd(AsyncWebServerRequest *request){
     File root = SD_MMC.open("/");

    while(File file = root.openNextFile()) {
      if (!file.isDirectory()) {
      //  file.remove();
String fileName = "/";
       fileName += file.name();

      //fs::FS &fs = SD_MMC;
      deleteFile(SD_MMC, fileName.c_str());
        //fs.remove("/" + fileName);


Serial.println("Deleting file: ");
Serial.println(fileName);


        
      file = root.openNextFile();
    }}
  }



void listFilesOnWebPage(AsyncWebServerRequest *request) {
  File root = SD_MMC.open("/");

 String output = "<html><body>";
    output += "<h1>Arquivos no cartao SD</h1><br>";
    output += "<a href='./'>VOLTAR</a>";
    output += "<br><br>";

    root.rewindDirectory();
    File file = root.openNextFile();
    while(file){
      if(!file.isDirectory()){
        String fileName = file.name();
        output += "<a href='/lersdx/";
        output += fileName;
        output += "'>";
        output += fileName;
        output += "</a><br>";
      }
      file = root.openNextFile();
    }
    
    output += "<br><br>";
    output += "<a href='apagarsd'>CUIDADO APAGA TODO CARTAO</a>";    
    output += "</body></html>";
    request->send(200, "text/html", output);
    
    }



void handleFile(AsyncWebServerRequest *request) {
File root = SD_MMC.open("/sdcard");

  String fileName = request->url();
    fileName.remove(0, 7);
    Serial.print(fileName);
    if(SD_MMC.exists(fileName)){
      File file = SD_MMC.open(fileName, FILE_READ);
      request->send(file, fileName, "text/plain");
    } else {
      request->send(404, "text/plain", "Arquivo nao encontrado");
    }
}




//##Feito por mim msm agora

void flashcam(AsyncWebServerRequest *request) {
 if(ledStateCAM == "flash"){
         Serial.println("FLASHOFF");
         ledStateCAM = "flashoff"; 
        }
        else{
           Serial.println("FLASH");
         
           ledStateCAM = "flash";
           }
      request->send(SPIFFS, "/index.html", "text/html", false);
    } 
   
    
    
    
    void pir(AsyncWebServerRequest *request) {
      if (pirState == "pirON"){
writeFile(SPIFFS, PIRSAVE, "pirOFF");
pirState = readFile(SPIFFS, PIRSAVE);
Serial.println(pirState);
        pir_enabled = false;
        }
        else{
          writeFile(SPIFFS, PIRSAVE, "pirON");
          Serial.println("pirON");
          pirState = "pirON";
          pir_enabled = true;

          }
      request->send(SPIFFS, "/index.html", "text/html", false);
    }

        void timelapse(AsyncWebServerRequest *request) {
    timeLapse = readFile(SPIFFS, TIMELAPSE);


          if (timeLapse == "timeLapseON"){
      //   timeLapse = "timeLapseOFF";
writeFile(SPIFFS, TIMELAPSE, "timeLapseOFF");
timeLapse = readFile(SPIFFS, TIMELAPSE);

         Serial.println(timeLapse);     
        }
        else{
       writeFile(SPIFFS, TIMELAPSE, "timeLapseON");
timeLapse = readFile(SPIFFS, TIMELAPSE);

         Serial.println(timeLapse);
        } 
      request->send(SPIFFS, "/index.html", "text/html", false);
    }



    
