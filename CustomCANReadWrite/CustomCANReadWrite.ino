/*
   -------------------------------------------------------------------

   https://www.xsimulator.net/community/threads/driving-real-gauges.3278/

   Speedo / tacho:
   CAN.setMessageID (message_id_201);
   my_data[0] = (RPM * 4) / 256;    // rpm
   my_data[1] = (RPM * 4) % 256;    // rpm
   my_data[2] = 0xFF;       // Unknown, 0xFF from 'live'.
   my_data[3] = 0xFF;       // Unknown, 0xFF from 'live'.
   my_data[4] = (kph * 100 + 10000) / 256;  // speed
   my_data[5] = (kph * 100 + 10000) % 256;  // speed
   my_data[6] = 0x00;       // Unknown possible accelerator pedel if Madox is correc
   my_data[7] = 0x00;       // Unknown

   Warning Lights:
   CAN.setMessageID (message_id_212);
   my_data[0] = 0xFE;       // Unknown
   my_data[1] = 0xFE;       // Unknown
   my_data[2] = 0xFE;       // Unknown
   my_data[3] = 0x34;       // DSC OFF in combo with byte 5 Live data only seen 0x34
   my_data[4] = 0x00;       // B01000000: Brake warning, B00001000: ABS warning
   my_data[5] = 0x40;       // TCS in combo with byte 3
   my_data[6] = 0x00;       // ETC
   my_data[7] = 0x00;       // Unused

   Other gauges / warning lights:
   CAN.setMessageID (message_id_420);
   my_data[0] = 0x98;       // temp gauge //~168 is red, ~163 last bar, 151/152 centre, 90 first bar, 92 second bar
   my_data[1] = 0x00;       // Distance (0.2m resolution – resets every 51m); something to do with trip meter 0x10, 0x11, 0x17 increments by 0.1 miles
   my_data[2] = 0x00;       // unknown
   my_data[3] = 0x00;       // unknown
   my_data[4] = 0x01;       // Oil Pressure (not really a gauge)
   my_data[5] = 0x00;       // check engine light
   my_data[6] = 0x00;       // Coolant, oil and battery
   my_data[7] = 0x00;       // unused

   Cruise Control Light:
   CAN.setMessageID (message_id_650);
   my_data[0] = 0xFF;       // cruise control light 0x80 is yellow, 0xFF is green

   -------------------------------------------------------------------

   https://docs.google.com/spreadsheets/d/1SKfXAyo6fbAfMUENw1KR3w4Fvx_Ihj6sTPSVXBdOXKk/edit#gid=0

   Mazda3 2nd gen - HS CAN Bus

   -------------------------------------------------------------------

   http://www.diyelectriccar.com/forums/showpost.php?s=46f8f5a424ac112421c829665f41028b&p=577066&postcount=178

   RPM Meter: CAN message ID 513 (hex 0x201), length 8 bit. Controlled by B0 and B1.

   0%: B0 = 0; B1 = 0

   first pointer movement: B0 = 2; B1 = 200
   3000 RPM: B0 = 45; B1 = 120
   6000 RPM: B0 = 89; B1 = 0

   The two bits resolution is unnecessarily big I think, because the analog gauge can not show so detailed values.
   I think controlling RPM gauge for Kostov K11 Alpha is very much possible only by linear signal where B0 = <0 - 89>,
   but Power Steering Controller will definitely need the full range for correct and error free operation.

   If I will translate high resolution pulse output of Kostov K11 Alpha sensor into CAN messages for
   Instrument cluster AND PSC, I think it should be relatively easy and very elegant to integrate signal
   divider and make my own Rebbl signal converter board.

   -------------------------------------------------------------------

   Speed: CAN message ID513 (hex 0x201), length 8 bit, Controlled by B4 and B5

   0 km/h: B4 = 39; B5 = 16
   1 km/h: B4 = 39; B5 = 61
   2 km/h: B4 = 39; B5 = 157
   3 km/h: B4 = 39; B5 = 255
   4 km/h: B4 = 40; B5 = 100
   298 km/h: B4 = 154; B5 = 200
   299 km/h: B4 = 155+ B5 = 0

   Again the resolution is very big, but digital gauge can use more of it. I think speed CAN messages are sent by ABS computer,
   because there is no connection for ABS wires in Power Steering Controller. Since power steering assist amount is speed dependent,
   it means that PSC will probably need to be fed by speed signal in full resolution of two bits to prevent errors.

   At least it seems to be linear. So I will have to catch the signal from vehicle CAN line and send it into PSC over separated
   CAN line to protect PSC from nonsenses sent by ECU.

   -------------------------------------------------------------------

   Oil Pressure: CAN message ID 1056 (hex 0x420), length 7 bits, controlled by B4

   0% oil pressure: B4 = 0
   75% oil pressure: B4 = 1

   This is all, the gauge is kind of fake, it might be simple MIL and the user would still be same informed.
   You can not make it show for example 25%.
   I think this might be suitable for DC/DC active / inactive status.
   The oil pressure signal (both B4 = 0 and 1) shuts off the red MIL which looks like merry-go-around,
   I suppose it is equivalent of "Check engine soon" MIL.

   -------------------------------------------------------------------

   Engine temperature: CAN message ID 1056 (hex 0x420), length 7 bits, controlled by B0

   no temp shown: B0 % 69

   Beginning of white field (0% temp): B0 = 90
   45% temp: B0 = 110
   50% temp: B0 = 151
   60% temp: B0 = 155
   end of white field (100% temp): B0 = 165
   redline (overheating): B0 = 170

   Temp. readings does not appear to be linear. The pointer is significantly "slower" between B0 = 110 and B0 = 150.
   I would like to use temp. meter to show me temperature of the kostov motor.
   For this I will need my hardware to translate resistance values from Kostov motor thermistor from let's say 20 C degrees
   to 120 C degrees into can message 0x420 where B0 = from 90 to 170 with respect to non-linearity of the CAN message.

   -------------------------------------------------------------------

   http://www.diyelectriccar.com/forums/showpost.php?p=597082&postcount=190

   PSC lamp: ID 768 (hex 0x300), length 1, bit 0 = 128 LAMP ON, bit0 = 1 LAMP OFF

   DSC lamp, slide lamp: ID 530 (hex 0x212), lenght 7, bit 5:
   DSC OFF, slide OFF: 64
   DSC OFF, slide ON: 80
   DSC ON, slide OFF: 0 / 8
   DSC ON, slide ON: 16
   DSC ON, slide blinking: 34 and around
   DSC OFF, slide blinking: 98 and around

   ABS lamp, brake pad lamp: ID530, length 7, bit4
   ABS ON, brake pad OFF: 8
   ABS OFF, brake pad OFF: 0
   ABS OFF, brake pad ON: 64
   ABS ON, brake pad on: 88

   Check engine soon lamp: message 1056, lenght 7, bit 5:
   0 - OFF
   64 - ON
   128 - blinking

   Merry go round (radiator level), charging, oil pressure: message 1056, length 7, bit 6:
   radiator OFF, charging OFF, oil press OFF: 0
   radiator ON, charging OFF, oil press OFF: 2
   radiator OFF, charging ON, oil press OFF: 64
   radiator ON, charging ON, oil press OFF: 66
   radiator OFF, charging OFF, oil press ON: 128
   radiator ON, charging OFF, oil press ON: 130
   radiator OFF, charging ON, oil press ON: 192
   radiator ON, charging ON, oil press ON: 194
*/

/* -------------------------------------------------------------------

  http://www.diyelectriccar.com/forums/showpost.php?p=709234&postcount=261

  The ECU must be removed and following CAN messages must be simulated to have no errors on display,
  control cluster, etc. The stability control, ABS works with this setup! Tested on Mazda RX 2004

  201 38 A8 FF FF 3A 2B C8 81    // 0,1 bytes RPM; 4 - Speed (26=0;3F=65; 4F=106; 5F=147; 6F=189)
  420 6B 23 C7 00 00 00 61 81    // 0 byte Temp (non linear) (5B-0%; 60-10%; 68-25%; 98=50%; 9E-75%; A4=100%)
             Fault codes: 01-ok; 00 error: 4 byte: Oil pressure, 5 Check engine, 6 battery charge
  215 02 2D 02 2D 02 2A 06 81    // Some ECU status
  231 0F 00 FF FF 02 2D 06 81    // Some ECU status
  240 04 00 28 00 02 37 06 81    // Some ECU status
  250 00 00 CF 87 7F 83 00 00    // Some ECU status
  200 00 00 FF FF 00 32 06 81    // EPS doesn't work without this
  202 89 89 89 19 34 1F C8 FF    // EPS doesn't work without this

  -------------------------------------------------------------------

  http://blog.hiroaki.jp/2010/04/000470.html#extended

  RPM: ID 0x201, (data[0] * 256 + data[1]) / 4
  Vehicle Speed(km/h): ID 0x201, (data[4] * 256 + data[5] - 10000) / 100
  Accel Throttle Position(0 - 255): ID 0x201, data[6]
  Hand Brake(on/off): ID 0x212, (data[4] & 0x40)
  Foot Brake(on/off): ID 0x212, (data[5] & 0x08)
  Declutching(on/off): ID 0x231, (data[0] & 0xf0)
  Engine Coolant Temp(degree celsius): ID 0x240, data[3] - 40
  Intake Air Temp(degree celsius): ID 0x250, data[3] - 40
*/

#include <mcp_can.h>
#include <SPI.h>

//Send
unsigned long prevTx = 0;
unsigned int invlTx = 1000;
//byte txData[] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

//Receive
long unsigned int rxId;
unsigned char len = 0;
unsigned char rxBuf[8];
char msgString[128];                        // Array to store serial string

#define CAN0_INT 2                              // Set INT to pin 2
MCP_CAN CAN0(10);                               // Set CS to pin 10


void setup()
{
  Serial.begin(115200);

  if (CAN0.begin(MCP_ANY, CAN_500KBPS, MCP_8MHZ) == CAN_OK)
    Serial.println("MCP2515 Initialized Successfully!");
  else
    Serial.println("Error Initializing MCP2515...");

  CAN0.setMode(MCP_NORMAL);                     // Set operation mode to normal so the MCP2515 sends acks to received data.

  pinMode(CAN0_INT, INPUT);                            // Configuring pin for /INT input

  Serial.println("Ready...");
}

//0x201
float engineRPM;
float carSpeed;
float pedalPosition;

//0x231
float susBin;

//0x240
float coolantTemp;

//0x420
float engineTemp;
float oilPressure;

//0x4b0 - unsure the difference between 0x4b0 and 0x4b1
float rearRightW;
float rearLeftW;
float aveWheelSpeed;
float gearRatio; //inc final drive, idk how to deduce between them without looking
float wheelCirc; //i'm guessing for now
float wheelRPM;

void printEverything(long unsigned int rxId, unsigned char len, unsigned char rxBuf[8]) {
  Serial.print(rxBuf[0]);
  Serial.print("\t");
  Serial.print(rxBuf[1]);
  Serial.print("\t");
  Serial.print(rxBuf[2]);
  Serial.print("\t");
  Serial.print(rxBuf[3]);
  Serial.print("\t");
  Serial.print(rxBuf[4]);
  Serial.print("\t");
  Serial.print(rxBuf[5]);
  Serial.print("\t");
  Serial.print(rxBuf[6]);
  Serial.print("\t");
  Serial.print(rxBuf[7]);
  Serial.println();
}

void messageDecode(long unsigned int rxId, unsigned char len, unsigned char rxBuf[8]) {
  switch (rxId) {
    case 0x201:
      engineRPM = ((rxBuf[0] * 256.0) + (rxBuf[1] / 4.0)) / 3.85;
      carSpeed = (rxBuf[4] * 256 + rxBuf[5] - 10000) / 100;
      pedalPosition = (rxBuf[6]) / 2;
      //      Serial.print("EngineRPM:\t");
      //            Serial.println(engineRPM);
      //            Serial.print("\t");
      ////            Serial.print(carSpeed);
      //            Serial.print("Pedal Position:\t");
      //            Serial.print(pedalPosition);
      //            Serial.print("\n");
      break;

    case 0x203: //Traction Control light or torque or something
      //NEED TO TESTS THIS
      //      Serial.print("ingector:\t");
      //      Serial.println(rxBuf[5]);
      //      Serial.print("First Traction Control Row:\t");
      //      printEverything(rxId, len, rxBuf);
      break;

    case 0x231: //Also to do with traction control apparently
      //Also need to test his
      //        Serial.print("SusBIN:\t"); //rxbuf[0] 255 in gear, 15 out of gear
      //        Serial.print(rxBuf[0]);
      //        Serial.println();
      //      Serial.print("Second Traction Control Row:\t");
      //      printEverything(rxId, len, rxBuf);
      break;


    case 0x240:
      coolantTemp = rxBuf[3] - 40;
      //            Serial.print("Coolant Temp:\t");
      //      Serial.println(coolantTemp);
      break;

    case 0x420:
      //20 - 120c -> 90 - 170
      engineTemp = map(rxBuf[0], 90, 170, 20, 120);
      oilPressure = rxBuf[4]; //useless LOL
      //            Serial.print("Oil Pressure:\t");
      //      Serial.println(oilPressure);
      break;


    case 0x430: //Need to test this -> Apparently fuel level??????
      //      Serial.print("Fuel Level:\t");

      printEverything(rxId, len, rxBuf);
      break;


    case 0x4b1:
      rearLeftW = ((rxBuf[4] + rxBuf[5]) - 10000) / 100; //this needs to be tested lol in kph
      rearRightW = ((rxBuf[6] + rxBuf[7]) - 10000) / 100; //same lol

      aveWheelSpeed = (rearRightW + rearLeftW) / 2; //Average wheel speed for rear wheels -> approx.

      wheelCirc = 0.66294; //m, i'm guessing lol 26.1 inch

      wheelRPM = aveWheelSpeed / wheelCirc;

      if (engineRPM > 0) {
        gearRatio = engineRPM / wheelRPM;
      }
      break;
  }
}

/*Ideas for emulating devices:
 *  To emulate something where the message only needs sending once i need to first:
 *    - receive the correct device's CAN ID, length and byte
 *    - send back the same message - basically copy and paste
 *    
 *  To emulate a device that is plugged into the car and constantly sending data i need to first:
 *    - receive the correct device's CAN ID, length and byte
 *    - find the frequency at which the message is being sent
 *    
 *    
 *  When sending a message I need to be careful with overwritting messages inside the same CAN ID,
 *  so a bitwise operation needs to be carried out to only send the specific bytes
 * 
*/

void sendData() {
   if((millis() - prevTx) >= invlTx){ //invlTX is the interval time between sending messages
    prevTx = millis();
    if(CAN0.sendMsgBuf(FUNCTIONAL_ID, 8, txData) == CAN_OK){
      Serial.println("Message Sent Successfully!");
    } else {
      Serial.println("Error Sending Message...");
    }
}

void loop()
{
  //Receive
  if (!digitalRead(CAN0_INT))
  {
    CAN0.readMsgBuf(&rxId, &len, rxBuf);

    if ((rxId & 0x80000000) == 0x80000000)    // Determine if ID is standard (11 bits) or extended (29 bits)
      sprintf(msgString, "Extended ID: 0x%.8lX  DLC: %1d  Data:", (rxId & 0x1FFFFFFF), len);
    else
      sprintf(msgString, "0x%.3lX,%1d", rxId, len); //Standard.

    //    Serial.print(msgString);
    messageDecode(rxId, len, rxBuf);
  }

//  sendData();

}
