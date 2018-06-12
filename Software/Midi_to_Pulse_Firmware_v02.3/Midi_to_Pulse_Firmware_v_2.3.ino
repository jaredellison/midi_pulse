/* Midi to Pulse Firmware v 2.3

   This code is written for a eurorack synthesizer module
   which takes a midi input and outputs 8 channels of velocity
   sensitive pulses to trigger analog drum synthesizer modules.

   The code is written for the Arduino Uno compatible Adafruit
   Metromini with an Atmel 328 microcontroller.

   A Texas Instruments 8 Bit DAC5578 octal DAC is used to output
   pulses of varying voltage levels. Any cryptic numbers in this
   code are likely derived from its datasheet (Revised August 2010)
   available here:
   www.ti.com/lit/pdf/sbas496

   Jared Ellison August 2017
*/

//        ###########################
//        #                         #
//        #         Libaries        #
//        #                         #
//        ###########################

#include <Wire.h> //For I2C communication with the DAC chip.

// Software serial allows for a serial port using pins other than 0 and 1,
// freeing those up for regular serial debugging.

#include <SoftwareSerial.h>

//        ###########################
//        #                         #
//        # Variables And Constants #
//        #                         #
//        ###########################

int pulseLed[8] = {4, 5, 6, 7, 8, 9, 10, 11};
int pulseState[8] = {0, 0, 0, 0, 0, 0, 0, 0};
unsigned long chanTime[8] = {0, 0, 0, 0, 0, 0, 0, 0};
int pulseDuration = 20; // In Milliseconds

byte midiReceiveChannel = 9; // Midi Channel 1

// constants
#define PIN_LED 13

// MIDI commands
byte midiNoteOn = 144 + midiReceiveChannel;
byte midiNoteOff = 128 + midiReceiveChannel;

// states
#define STATE_NONE 0
#define STATE_NOTE_ON 1
#define STATE_NOTE 2
int state;

// received MIDI data
byte midiByte;
byte midiChannel;
byte midiCommand;
byte midiNote;
byte midiVelocity;

byte currentPulseOut = -1;

SoftwareSerial mySerial(2, 3); // Intialize a serial port with pin #s  (RX, TX)

// Uncomment the DEBUG definition for debug messages over serial. This will mess
// with overall timing.
#define DEBUG 1

//        ###########################
//        #                         #
//        #          Setup          #
//        #                         #
//        ###########################

void setup() {
  Wire.begin(); // join i2c bus (address optional for master)
  mySerial.begin(31250); // Initiate MIDI serial connection

#ifdef DEBUG
  Serial.begin(9600); // Initiate serial connection to terminal for debugging
#endif

  delay(100);

  // Set Led indicator pins as output and intialize them low
  for (byte i = 0; i <= 7; i++) {
    pinMode(pulseLed[i], OUTPUT);
    digitalWrite(pulseLed[i], LOW);
  }

  state = STATE_NONE;

}

//        ###########################
//        #                         #
//        #         Main Loop       #
//        #                         #
//        ###########################


void loop() {

  // Check for any new MIDI data to be read.

  if (mySerial.available() > 0) {

    midiByte = mySerial.read(); // read MIDI byte

#ifdef DEBUG
    if (midiByte != 248) {
      Serial.println(midiByte);
    }
#endif DEBUG

    switch (state) {
      case STATE_NONE:
        midiChannel = midiByte & B00001111;// remove channel info
        midiCommand = midiByte & B11110000;

        if (midiChannel == midiReceiveChannel){
          if (midiCommand == midiNoteOn){
            state = STATE_NOTE_ON;
          }
        }
        break;

      case STATE_NOTE_ON:
        midiNote = midiByte;
        state = STATE_NOTE;
        break;

      case STATE_NOTE:
        midiVelocity = midiByte;
        state = STATE_NONE;
        currentPulseOut = noteToPulseOut(midiNote);

        if (currentPulseOut >= 0 && currentPulseOut <= 7 && midiVelocity > 0) {
          chanTime[currentPulseOut] = millis();
          pulseState[currentPulseOut] = 1;
          updateDac(currentPulseOut, midiVelocity * 2);
          digitalWrite(pulseLed[currentPulseOut], HIGH);

#ifdef DEBUG
          Serial.print("Midi Note to Pulse#: ");
          Serial.println(midiVelocity);

          Serial.print("Pulse #:");
          Serial.print(noteToPulseOut(midiNote));
          Serial.println(" on!");
#endif

        }

        break;

    } // switch



  } // mySerial.available()

  // channel off after predetermined time

  for (int i = 0; i <= 7; i++) {
    if (pulseState[i]) {
      if ((millis() - chanTime[i]) > pulseDuration) {
        pulseState[i] = 0;
        updateDac(i, 0);
        digitalWrite(pulseLed[i], LOW);
#ifdef DEBUG
        Serial.print("Channel #:");
        Serial.print(i);
        Serial.println(" off!");
#endif

      }
    }
  }// for
}// loop

//        ###########################
//        #                         #
//        #         Functions       #
//        #                         #
//        ###########################


/*****
    updateDac
      Sets the output voltage to a certain level
      at a given output channel of the DAC5578 chip.

      The chip requires a message that is formatted as a sequence
      of 4 bytes:
        -An address byte to begin the communcation and identify
         the specific chip on the i2c bus. The TSSOP16 package of
         this chip features a three state input pin called ADDR0
         that allows 3 chips to be used on the same bus. With the
         ADDR0 pin grounded the chip address is B10010000.

        -A commmand address byte which specifies which function
         and channel of the chip accesss. This function uses the
         commands which identify and update individual channels.
         Channels 0-8 use the CA bytes B00110000 to B00110111 aka
         dec 48 to 55.

        -The Most Significant Data Byte to specify a level to set
        the given channel.

        -The Least Significant Data Byte which is not used on the
        8 bit DAC5578 and remains set at B00000000.


    Arguments:
      channel - Which channnel to write to from 0-7.
      value - What level to set the output to. from 0 to 255 for
        the 8 bit DAC.

    Returns:
      0 if successful
      1 if there is an error

 *****/
int updateDac(byte channel, byte value) {
  // The write and update command for channels 0->7 runs from 48->55.
  // This points the command address byte to the right channel using 48 as an offset.
  byte CA = channel + 48;

  byte MSDB = value;
  byte LSDB = 0; //For an 8 bit DAC the last 8 bits are set to zero and ignored.

  Wire.beginTransmission(0x48); // With the ADDR0 Pin 2 of the DAC5578 grounded, the chip address is B1001000 aka Dec 144
  Wire.write(CA);
  Wire.write(MSDB);
  Wire.write(LSDB);
  Wire.endTransmission();
  return 0;
}

/*****
     noteToPulseOut
     This function takes a midi note number and pulses the corresponding
     DAC channel.

     Arguments:
      noteIn - A MIDI note number stripped from note on message
     Returns:
      1-8 to specify a channel
      0 if the input note is not mapped


      Note# ---->  Pulse Channel#
      36            0
      37            1
      38            2
      39            3
      40            4
      41            5
      42            6
      43            7
******/
// Midi Note to DAC Channel Mapping
#define CH_0_NOTE 36
#define CH_1_NOTE 37
#define CH_2_NOTE 38
#define CH_3_NOTE 39
#define CH_4_NOTE 40
#define CH_5_NOTE 41
#define CH_6_NOTE 42
#define CH_7_NOTE 43

byte noteToPulseOut(int noteIn) {
  switch (noteIn) {
    case CH_0_NOTE:
      return 0;
    case CH_1_NOTE:
      return 1;
    case CH_2_NOTE:
      return 2;
    case CH_3_NOTE:
      return 3;
    case CH_4_NOTE:
      return 4;
    case CH_5_NOTE:
      return 5;
    case CH_6_NOTE:
      return 6;
    case CH_7_NOTE:
      return 7;
    default:
      return -0;
  }
}

