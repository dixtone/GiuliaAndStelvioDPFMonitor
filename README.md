### [UPDATE 05/02/2025]
Some Bug fixing related to broadcast packets handling.

### [UPDATE 29/01/2025]

Introduced new unified library to connect **OBD2 CanBus** through Bluetooth Dongle or 2 wire using a transceiver like the SN65HVD230. 

You can find info about my OBD2 Library here: **https://github.com/dixtone/OBD2**

With this important update, now you can:
1) connect directly to canbus or using bluetooth, simply changing platformio.ini build_flags
2) when using direct connection, you can disable your start&stop by simulating user press button on panel, after device starts
3) reading Engine Oil pressure (finally!!!)

#### final result: ####

![alt text](images/mounted_monitor.jpeg)


#### Here is and example about direct connection to canbus: ####

![alt text](images/dixtone_giulia_stelvio_connection.png)

Removing button panel, you can take a wire from pin 5 and check with a multimeter: when you press start and stop button disabler, you have to read about 1.5kΩ to GND. 
This means using upper schematics, that pulling down GPio pin with 1.5kΩ resistor, you can emulate user press button

![alt text](images/start-stop-connector.jpeg)

![alt text](images/3dbox.jpeg)


Removed unused libraries, code clean and optimizations.

********
<img border="0" src="https://github.com/user-attachments/assets/622e0cb5-ba29-4840-badb-5596f89e2a06" width="400" >

<img border="0" src="https://github.com/user-attachments/assets/ceffba97-147f-44e0-8d5f-9202af58b010" width="400" >


I share this project to take control of regeneration status in your Alfa Romeo Giulia or Stelvio Diesel.

Stopping regenerations on this type of engine can cause many problems, such as oil quality degradation, engine oil level increase, oil pressure issues and oil in the intake filter.
As Giulia owner, I think this car deserves to be treated as it is: a masterpiece and I want to minimize all possible component breakages.

The purpose of this project is very simple: it uses an OBD2 Bluetooth 4 adapter to read the parameters from the diagnostic socket, and show on the screen when the car performs DPF regeneration.

All information you can get are:

DPF:
1) regeneration process status
2) total number of regenerations
3) clogging
4) exaust temp
5) engine colant temp (water)
6) battery IBS status and voltage
7) oil quality/degradation
8) oil level

ENGINE:
1) differential pressure of DPF
2) turbo boost pressure
3) engine butterfly valve %
4) **Start & Stop disabling (NEW HOT!)** (when direct canbus connection!)
   
KEYS and Remote (useful to get how many copies of key/remote exists)
1) Key Number and Last used
2) Remote Number and Last used

SERVICE:
1) oil degradation
2) oil level
3) last service date
4) total number of services
5) remain km to service
6) remain days to service
7) **OIL pressure (NEW HOT!)** (when direct canbus connection!)

BATTERY:
1) Battery voltage
2) IBS charge level %
3) Battery temperature

DIESEL:
1) temperature
2) pressure
3) water presence

TIRES:
1) temperature for all 4 TPMS
2) pressure in bar for all 4 TPMS

... and soon (having time to add :) )

Project is not full completed: i want to add the READ/CLEAR DTC functionality and some more features.

I used some existent libraries to complete the project:
1) ~~A customized and light version of ELMduino  https://github.com/PowerBroker2/ELMduino~~
2) A customized version of DWIN DGUS HMI Arduino Library https://github.com/dwinhmi/DWIN_DGUS_HMI/tree/master
3) ESP32 Buzzer Library https://github.com/hammikb/ESP32-Buzzer
4) ~~Esp32-SoftwareSerial  https://github.com/junhuanchen/Esp32-SoftwareSerial/tree/master~~
5) WS2812Led https://github.com/RadioShuttle/WS2812Led


The project can be used as is: I do not provide any guarantee or responsibility in case of damage caused to your vehicle or person.


****************** 

Project is based on ESP32 and Dwin Display DMG40960F040_01WTC (COF display 960x400 res) or DMG48480C028_03WTC (COB display 480x480 res).
You can use display as you want but you need to design your own graphics.

This is the schematics: 

![schematic](https://github.com/user-attachments/assets/2369686e-838d-48aa-961f-b31e9447bd95)

and my own PCB Design to support COB and COF display or only COF:

![IMG_4433](https://github.com/user-attachments/assets/0626f264-54c9-4286-a8c6-bbb71eeded09)


You can find display used here: 

COF: https://it.aliexpress.com/item/1005005579119952.html

COF ADAPTER to burn project (HDL662S) : https://it.aliexpress.com/item/1005002346359419.html

COB: https://it.aliexpress.com/item/1005005479523806.html

COB FLAT CABLE (10Pin FCC1015A): https://it.aliexpress.com/item/1005002346359419.html

You can use my coupon code: <i><strong>DWINSPINA</strong></i>  to obtain some discount


****************** 

If you appreciate my work, pay me a coffee: 

<a target="_blank" href="https://www.paypal.com/donate/?business=UPRH64ZWV5M4E&no_recurring=0&currency_code=EUR"><img border="0" src="https://github.com/user-attachments/assets/17768a9f-a07d-4521-9839-ec5e2b77f3cd"></a>

****************** 


Here is some pics of working project

![IMG_3756](https://github.com/user-attachments/assets/f0979082-420f-46ee-9823-8e28603577ed)



![IMG_4422](https://github.com/user-attachments/assets/b3226f9f-2f54-4ed2-bad4-6995cc9fcdf2)
![IMG_4419](https://github.com/user-attachments/assets/cc0b36db-9a3c-401c-87c9-45f2591e705a)
![IMG_4416](https://github.com/user-attachments/assets/774ef953-7c30-4068-b9cb-28dbc79b6c21)
![IMG_4421](https://github.com/user-attachments/assets/d72baa0e-949c-4142-91e4-04f92746f187)
![IMG_4420](https://github.com/user-attachments/assets/42c97e46-0938-4218-a03c-e7efa59b8a88)
![IMG_4418](https://github.com/user-attachments/assets/5b1a1ae7-2786-4004-8513-fe798668c6cd)

![IMG_4424](https://github.com/user-attachments/assets/1627e4bf-573e-46af-bb34-d9d29346455c)
![IMG_4427](https://github.com/user-attachments/assets/b7dbda4a-e100-487d-90f8-834fccf39813)
![IMG_4426](https://github.com/user-attachments/assets/3c602638-e4d3-462c-b906-c0f0c6dd17a8)
![IMG_4425](https://github.com/user-attachments/assets/a7f7943e-15b0-40cf-8237-bfb5237d3d9a)
