# GrANA3D IMPRESSORA
Impressora reprap do tipo wilson2 com algumas modificações de tamanho , detecção de filamento e envio do g-code via wifi

#### Tamanho de impressao
- X 430mm
- Y 400mm
- Z 450mm

# BOM Wilson2 #GrANAMOD
### Eletronicos 
- 1 Placa controladora esp32 WiFi mks tinybee com 5 drivers TMC2209
- 5 motores nema 17 17hs4401s ou 17hs8401
- 3 KW11 Interruptor de Limite 10x20mm

### Frame
- 4 barra extrusion aluminio 2020 650mm
- 3 barra extrusion aluminio 2020 530mm
- 2 barra extrusion aluminio 2020 600mm
### Barras e rolamentos 
 - 6 haste linear 600mm comprimento 10mm diametro
 - 2 Parafuso de chumbo t8  Pitch 2mm Lead 8mm comprimento 550mm
 - 12 rolamento de trilho linear bucha lm10uu  
 - 3 rolamento de esferas em aço cromado 624rs 
 - 2 rolamentos de esferas 608zz 
### Polias e correias
 - 2 2gt 20 dentes synchronus polia de sincronização gt2 16/20teeh
 - 3 metros de correia dentada de borracha de fibra de vidro gt2 6mm
 ### Extrusora
 - 1 hotend e3d v6  
 - 1 engrenagem MK7 polia de filamento para extrusora
 - 2  parafuso de nivelamento m3
 - 1 motor nema 17 17hs08-1004s
 - 1  ventilador de refrigeração 4010 12v 15.000 RPM  40mm x 40mm x 10mm fan
 - 1 3d sensor de toque cama automática sensor nivelamento bl toque bltouch 
 ### Mesa de impressao
 - 4 m4 parafuso da mola porca de nivelamento 
 - cama aquecida
 - sensor de temperatura da cama
 
 ### Fonte
 - PSU 12v 30A





## Placa controladora ligação dos cabos

 ![esp32MBwilsonGRANA](https://user-images.githubusercontent.com/3370347/226597925-2871dc9e-c282-459e-9dd8-d61ff7ce8155.jpg)
 
 
 ## Expectativa
 
 ![wilson-ii-preview](https://user-images.githubusercontent.com/3370347/226598745-309b49a7-3d9f-4617-b82f-093da86d27c4.jpg)
 
 ## Realidade
 ...
 imprimindo as pecas e esperando chegar os pacotes da china rs



## Instalar as bibliotecas no arduino

### TMCStepper
https://github.com/teemuatlut/TMCStepper

### ESP3D
https://github.com/luc-github/ESP3DLib

