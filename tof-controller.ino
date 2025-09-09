/* This example shows how to use continuous mode to take
range measurements with the VL53L0X. It is based on
vl53l0x_ContinuousRanging_Example.c from the VL53L0X API.

The range readings are in units of mm. */

#include <Wire.h>
#include <VL53L0X.h>
#include <MIDI.h>

VL53L0X sensor;

#define HIGH_SPEED
#define CCNUMBER 1
#define CCCHANEL 1
MIDI_CREATE_DEFAULT_INSTANCE();

int cc = 0;
unsigned long prevTime = 0;
int noteFlag = 0;

void setup() {
  delay(500);
  // Serial.begin(9600);
  Wire.setClock(10000);
  Wire.begin();
  Wire.setWireTimeout(3000 /* us */, true /* reset_on_timeout */);
  MIDI.begin(4);  // Launch MIDI and listen to channel 4

  sensor.setTimeout(500);
  if (!sensor.init()) {
    //Serial.println("Failed to detect and initialize sensor!");
    while (1) {}
  }

  sensor.setMeasurementTimingBudget(18000);

  // Setting a lower limit increases 
  // the potential range of the sensor 
  // but also increases the likelihood of getting 
  // an inaccurate reading because of reflections 
  // from objects other than the intended target. 
  // This limit is initialized to 0.25 MCPS by default. 
  //  try adjusting this to fine-tune the range/responsiveness
  sensor.setSignalRateLimit(0.5);
  // Serial.print(sensor.getSignalRateLimit());

  // Start continuous back-to-back mode (take readings as
  // fast as possible).  To use continuous timed mode
  // instead, provide a desired inter-measurement period in
  // ms (e.g. sensor.startContinuous(100)).
  sensor.startContinuous();
}

void loop() {
  uint16_t range = 0;
  if ((sensor.readReg(sensor.RESULT_INTERRUPT_STATUS) & 0x07) != 0) {
    range = sensor.readReg16Bit(sensor.RESULT_RANGE_STATUS + 10);
    sensor.writeReg(sensor.SYSTEM_INTERRUPT_CLEAR, 0x01);

    // Serial.println(millis() - prevTime); // elapsed time for measurement
    // Serial.println(range);

    if ((millis() - prevTime) < 16 && noteFlag == 1) {
      MIDI.sendNoteOff(1, 0, 1);
      noteFlag = 0;
    };

    if (range > 2000) {
      MIDI.sendControlChange(CCNUMBER, 0, CCCHANEL);
    } else {
      cc = constrain(map(range, 0, 700, 0, 127), 0, 127);
      MIDI.sendControlChange(CCNUMBER, cc, CCCHANEL);
    };
    prevTime = millis();
  };

  // send separate cc & note on/off
  // note will be ~13ms faster, because measurement takes ~31ms
  // but note triggers after 18ms... yey 13ms !
  if ((millis() - prevTime) > 20 && noteFlag == 0) {
    // MIDI.sendNoteOn(1, 127, 1);
    noteFlag = 1;
  };

///////////////////////////////////////////////
  /////////// stock library version ///////////
  // int distanceReading = sensor.readRangeContinuousMillimeters();
  // cc = constrain(map(distanceReading, 0, 700, 0, 127), 0, 127);
  // if (distanceReading > 6000){
  //   MIDI.sendControlChange(CCNUMBER, 0, CCCHANEL);
  // }
  // else
  // {
  //   MIDI.sendControlChange(CCNUMBER, cc, CCCHANEL);
  // }
}
