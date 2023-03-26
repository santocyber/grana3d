


void connect()//Funçao para Conectar ao wifi e verificar à conexao.
{
   if (WiFi.status() != WL_CONNECTED)//Caso nao esteja conectado ao WiFi, Ira conectarse
   {
   Serial.print("Task CONNECT running on core ");
   Serial.println(xPortGetCoreID());
   
    WiFi.begin(ssid.c_str(), pass.c_str());
    //WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
     delay(400);
    client.setInsecure();
     delay(400);
    client.setHandshakeTimeout(1200000);
   // client.setTimeout(3000);

    delay(10);
   }
}

//#################################################################PING WDT

void pingando(){

   Serial.print("Task PING running on core ");
   Serial.println(xPortGetCoreID());

    if(Ping.ping("google.com", 1)) {
      ConnectedCounter++;
 Serial.println("Ping pong OK contando....");
    Serial.print(Ping.averageTime());
    Serial.println(" ms");
    Serial.printf("\n%lu: Remaining free mem: %u\n", millis(), ESP.getFreeHeap());

      }

else{ 
      notConnectedCounter++;
          Serial.println("Falha do ping da internet, contando...");
          Serial.println(notConnectedCounter);

    if(notConnectedCounter > 5) { // Reset a placa apos 50 erros    
    Serial.println("Reiniciando esp por falha na internet");
    
          ESP.restart();
     
   }
   }

}

//##############VOID TELEGRAM

void TELE(void*p){
//portVALID_STACK_MEM(pxStackBuffer); 
    
    configASSERT( ( uint32_t ) p == 1UL );
    
  // Block for 500ms.
const TickType_t xDelay = 30000 / portTICK_PERIOD_MS;

  for( ;; )
  {
      // Simply toggle the LED every 500ms, blocking between each toggle.
  readTel();
      vTaskDelay( xDelay );
  }

  }


  void readTel(){

  



   Serial.print("Task TELE running on core ");
   Serial.println(xPortGetCoreID());

  
   int newmsg = bot.getUpdates(bot.last_message_received + 1);
//   int msgtxt = bot.getUpdates(bot.last_message_received);
 //  String msgtele = bot.messages[1].text;

   for (int i = 0; i < newmsg; i++)//Caso haja X mensagens novas, fara este loop X Vezes.
   {
      id = bot.messages[i].chat_id;//Armazenara o ID do Usuario à Váriavel.
      text = bot.messages[i].text;//Armazenara o TEXTO do Usuario à Váriavel.
      //text.toUpperCase();//Converte a STRING_TEXT inteiramente em Maiuscúla.
      from_name = bot.messages[i].from_name;
      //bot.messages[i].type == "channel_post";

    Serial.printf("\nPeguei essa mensagem %s\n", text);

   
    if (from_name == "") from_name = "Cade o nick?";

    String hi = nomedobot.c_str(); 
    hi += ":";
    hi += "Vc disse isso mesmo?";
    hi += "\n";
    hi += text;
    hi += "\n";
    hi += "Vou executar!";
      
    bot.sendMessage(id, hi, "Markdown");


  
    if (text.indexOf("flash") > -1)//Caso o texto recebido contenha "ON"
      {        
  if(ledStateCAM == "flash"){
         Serial.println("FLASHOFF");
         ledStateCAM = "flashoff";
         bot.sendMessage(id, ledStateCAM, "");//Envia uma Mensagem para a pessoa que enviou o Comando.
     
        }
        else{
           Serial.println("FLASH");
           ledStateCAM = "flash";} 
           bot.sendMessage(id, ledStateCAM, "");//Envia uma Mensagem para a pessoa que enviou o Comando.
              

         }

if (text.indexOf("avi") > -1)//Caso o texto recebido contenha "ON"
      {        
  if(avi_enabled){
         Serial.println("AVI OFF");
         avi_enabled = false;
         bot.sendMessage(id, "AVI OFF", "");//Envia uma Mensagem para a pessoa que enviou o Comando.
     
        }
        else{
           Serial.println("AVI ON");
           avi_enabled = true;
           } 
           bot.sendMessage(id, "AVI ON", "");//Envia uma Mensagem para a pessoa que enviou o Comando.
              

         }

         

    if (text.indexOf("timelapse") > -1)//Caso o texto recebido contenha "ON"
      {        
  timeLapse = readFile(SPIFFS, TIMELAPSE);


          if (timeLapse == "timeLapseON"){
      //   timeLapse = "timeLapseOFF";
writeFile(SPIFFS, TIMELAPSE, "timeLapseOFF");
timeLapse = readFile(SPIFFS, TIMELAPSE);

         Serial.println(timeLapse);


         bot.sendMessage(id, "timeLapseOFF", "");//Envia uma Mensagem para a pessoa que enviou o Comando.
     
        }
        else{
       writeFile(SPIFFS, TIMELAPSE, "timeLapseON");
timeLapse = readFile(SPIFFS, TIMELAPSE);

         Serial.println(timeLapse);
           bot.sendMessage(id, "timeLapseON", "");//Envia uma Mensagem para a pessoa que enviou o Comando.
        } 

         }
         

   

     

      if (text.indexOf("/foto") > -1){

      fb = NULL;


    //   digitalWrite(FLASH_LED_PIN, HIGH);

      fb = esp_camera_fb_get();
      esp_camera_fb_return(fb);
      delay(100);

      // Take Picture with Camera
      fb = esp_camera_fb_get();
      if (!fb) {
        Serial.println("Camera capture failed");
        bot.sendMessage(id, "Camera capture failed", "");
        return;
      }
      
ledStateCAM = "flash";
       fill_solid( ledflash, 1, CRGB::White);
         FastLED.show();

      currentByte = 0;
      fb_length = fb->len;
      fb_buffer = fb->buf;

      Serial.println("\n>>>>> Sending, bytes=  " + String(fb_length));

        bot.sendPhotoByBinary(id, "image/jpeg", fb_length,
                              isMoreDataAvailable, getNextByte,
                              nullptr, nullptr);

        dataAvailable = true;

        Serial.println("done!");
      ledStateCAM = "flashoff";
      esp_camera_fb_return(fb);
     }




      else if (text.indexOf("caption") > -1){

          fb = NULL;


    //   digitalWrite(FLASH_LED_PIN, HIGH);

      fb = esp_camera_fb_get();
      esp_camera_fb_return(fb);
      delay(100);

      // Take Picture with Camera
      fb = esp_camera_fb_get();
      if (!fb) {
        Serial.println("Camera capture failed");
        bot.sendMessage(id, "Camera capture failed", "");
        return;
      }
      
            currentByte = 0;
      fb_length = fb->len;
      fb_buffer = fb->buf;
      
        Serial.println("\n>>>>> Sending with a caption, bytes=  " + String(fb_length));

    String sent = bot.sendMultipartFormDataToTelegramWithCaption("sendPhoto", "photo", "img.jpg",
                  "image/jpeg", "Take a pictureee: " + nomedobot, id, fb_length,
                  isMoreDataAvailable, getNextByte, nullptr, nullptr);

        Serial.println("done!");

      }


 else if (text.indexOf("vga") > -1)//Caso o texto recebido contenha "OFF"
      {
 fb = NULL;

      sensor_t * s = esp_camera_sensor_get();
      s->set_framesize(s, FRAMESIZE_HD);

      Serial.println("\n\n\nSending VGA");

      // Take Picture with Camera
      fb = esp_camera_fb_get();
      if (!fb) {
        Serial.println("Camera capture failed");
        bot.sendMessage(id, "Camera capture failed", "");
        return;
      }

      currentByte = 0;
      fb_length = fb->len;
      fb_buffer = fb->buf;

      Serial.println("\n>>>>> Sending as 512 byte blocks, with jzdelay of 0, bytes=  " + String(fb_length));

      bot.sendPhotoByBinary(id, "image/jpeg", fb_length,
                            isMoreDataAvailable, getNextByte,
                            nullptr, nullptr);

      esp_camera_fb_return(fb);

      }




       else if (text.indexOf("clip") > -1)//Caso o texto recebido contenha "OFF"
      {


  avi_enabled = true;

      // record the video
      bot.longPoll =  0;
     // xTaskCreate(the_camera_loop,"FOTO", 20000, NULL, 10, &the_camera_loop_task);     

      xTaskCreatePinnedToCore( the_camera_loop, "the_camera_loop", 20000, NULL, 1, &the_camera_loop_task, 0);
    //  xTaskCreatePinnedToCore( the_camera_loop, "the_camera_loop", 10000, NULL, 1, &the_camera_loop_task, 0);  //v8.5

      if ( the_camera_loop == NULL ) {
        //vTaskDelete( xHandle );
        Serial.printf("do_the_steaming_task failed to start! %d\n", the_camera_loop);
      }
        avi_enabled = false;

    }

    else if (text.indexOf("/video") > -1){


  avi_enabled = true;

      // record the video
      bot.longPoll =  0;
      xTaskCreate(the_camera_loop,"FOTO", 20000, NULL, 1, NULL);     

     // xTaskCreatePinnedToCore( the_camera_loop, "the_camera_loop", 10000, NULL, 1, &the_camera_loop_task, 1);
    //  xTaskCreatePinnedToCore( the_camera_loop, "the_camera_loop", 10000, NULL, 1, &the_camera_loop_task, 0);  //v8.5

      if ( the_camera_loop == NULL ) {
        //vTaskDelete( xHandle );
        Serial.printf("do_the_steaming_task failed to start! %d\n", the_camera_loop);
      }
        avi_enabled = false;
        
      }
      
           else if (text.indexOf("restart") > -1)//Caso o texto recebido contenha "OFF"
      {
        bot.sendMessage(id, "reiniciando esp", "");//Envia uma Mensagem para a pessoa que enviou o Comando.
        delay(500);
        ESP.restart();
      }

      

       else if (text.indexOf("status") > -1)//Caso o texto recebido contenha "OFF"
      {
    welcome = "Nome do bot: \n";
    welcome +=  nomedobot.c_str();
    welcome += "\n";
    welcome += "Endereço MAC: \n";
    welcome += WiFi.macAddress();
    welcome += "\n";
    welcome += "SSID: ";    
    welcome += ssid.c_str();
    welcome += "\n";    
    welcome += "Tempo ping: ";    
Ping.ping("google.com", 1);
    welcome +=  Ping.averageTime();
    welcome += "ms";
    welcome += "\n";
    welcome += "MMC Size:";
    welcome += SD_MMC.totalBytes()/ (1024 * 1024);
    welcome += "MB\n"; 
    welcome += "MMC Bytes USADO: ";
    welcome += SD_MMC.usedBytes()/ (1024 * 1024);
    welcome += "MB\n";
    welcome += "HEAP Size: ";
    welcome += ESP.getHeapSize();  
    welcome += "\nRemaining free mem: ";
    welcome += ESP.getFreeHeap();
    welcome += "\n";
    welcome += "PSRAM Size: ";
    welcome += ESP.getPsramSize();  
    welcome += "\nFree PSRAM:";
    welcome += ESP.getFreePsram();
    welcome += "\n";
    welcome += "Millis: ";
    welcome += millis();
    welcome += "\n";
    welcome += "Nivel de sinal WIFI: ";
    welcome += WiFi.RSSI();
    welcome += "\n";
    welcome += "IP Local: ";
    welcome += WiFi.localIP().toString();
    welcome += "\n";
    welcome += "Numero de Falhas na internet: ";
    welcome += notConnectedCounter;
    welcome += "\n";
    welcome += "Numero de PING na internet: ";
    welcome += ConnectedCounter;
    welcome += "\n";
    welcome += "Estado FLASH: ";
    welcome +=  ledStateCAM;
    welcome += "\n";
    welcome += "TimeLapse: ";
    welcome += timeLapse;
    welcome += "\n";
    welcome += "PIR: ";
    welcome += pirState;


      bot.sendMessage(id, welcome, "Markdown");      
      }
          
      else if(text.indexOf("start") > -1)//Caso o texto recebido contenha "START"
      {
      if (from_name == "")
      from_name = "Guest";

      welcome = "Bem vindo, " + from_name + ".\n";
      welcome += "Essa eh a GrANA3D\n\n";
      welcome += "Agora se chama\n";
      welcome +=  nomedobot.c_str();
      welcome += "\n";
      welcome += "/foto : Tira uma foto\n";
      welcome += "/caption : Tira uma foto com legenda\n";
      welcome += "/video : Grava um mini video clip\n";
      welcome += "/flash : Liga e desliga o flash\n";
      welcome += "/pir : Ativa o sensor de presença\n";
      welcome += "/timelapse : Liga e desliga o timelapse\n";
      welcome += "/status : STATUS\n";
      welcome += "/start : Abre esse menu\n";
      welcome += "/restart : Reinicia o Controlador\n";
      welcome += "Acesse o ip http://";
      welcome +=  WiFi.localIP().toString(); 
      welcome += "\n";
      welcome += "Para mais controles e vizualizar o historico dos sensores\n";
      welcome += "\n";
      welcome += "Codigo fonte em https://github.com/santocyber/grana3d\n";
     

      bot.sendMessage(id, welcome, "Markdown");      
      }

   }
//delay(200);
//client.flush();
//client.stop();
esp_get_free_heap_size();

//  vTaskDelete(NULL);
       //   vTaskDelete(teletask);
   // vTaskSuspend(NULL);

}
