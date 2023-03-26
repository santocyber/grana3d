


void ifs(){

  if (ledStateCAM == "flash") {

    flash();
    }
    if (ledStateCAM == "flashoff") {

    flashoff();
    }


  
//#######################PIR
#if (USEPIR == 1)
if (pirState == "pirON"){

  if(motionSensorFlag){    
    if(digitalRead(PIR) == LOW){
      Serial.println("Motion Detected");
      Serial.println("Preparing photo");
      xTaskCreatePinnedToCore(foto, "fototask", 20000, NULL, 1, NULL, 1);
      delay(1000);
    }
  }
}
#endif

//#######################TIME LAPSE
if (timeLapse == "timeLapseON"){


  if (millis() - tempotimelapse > 60000)//Faz a verificaçao das funçoes a cada 30min
   {
          
          
    Serial.println("TimeLapse millis");


          Serial.println("TimeLapse millis ON");

         xTaskCreate(savesd, "sdtask", 5000, NULL, 0, &savesdtask); 
         delay(400);
          
   }

tempotimelapse = millis();
}


 //##Loop paralelo



     if (millis() - tempoping > 240000)//Faz a verificaçao das funçoes a cada 30min
   {
    Serial.println("pingando");
    connect();
    pingando();

    //esp_wifi_stop(); 
    delay(500);
    WiFi.disconnect();
    delay(500);
    connect();
    esp_get_free_heap_size();
    
      tempoping = millis();
     
   }





#if (CAMERA == 1)

//#################################################TIRA FOTO
    if (takeNewPhoto) {

//capturePhotoSaveSpiffs();
  //    delay(200);
 //   xTaskCreate(foto,"TAKEAPICTURE", 20000, NULL, 1, &savespifftask);     
    //  delay(200);
    //xTaskCreate(savesd,"TAKEAPICTURE", 20000, NULL, 0, &savesdtask);  
    //  delay(200);
  //capturePhotoSaveSpiffs();

      xTaskCreatePinnedToCore(foto, "fototask", 20000, NULL, 1, NULL, 0);
      delay(2000);
  //   xTaskCreatePinnedToCore(savesd, "sdtask", 20000, NULL, 0, &savesdtask, 1);


    takeNewPhoto = false;
     delay(600);
  }

  
  delay(50);


  if (picture_ready) {
    picture_ready = false;
    send_the_picture();
  }

  if (video_ready) {
    
    video_ready = false;
    send_the_video();
  }

#endif





}
