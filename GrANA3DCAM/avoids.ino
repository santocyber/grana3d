  void flash(){  
     // ledflash[0] = CRGB::Red;
   
       fill_solid( ledflash, 1, CRGB::White);
         FastLED.show();
  }
  
  void flashoff(){   
 fill_solid( ledflash, 1, CRGB::Black);
         FastLED.show(); 

 }
