#include <MIDI.h>
#include <SPI.h>
 
#define PIN_CS 10
#define GAIN_1 0x1
#define GAIN_2 0x0
#define DAC_MAX 4095

#define TRIG_PIN 3
#define SEMITONE 83.333 //millivolts per semitone
#define PITCH_BEND 833.33
#define ZERONOTE 33 //zero volts midi note
#define MAXNOTE 82

byte lastNoteOn = 0;
int lastPitchBend = 0;

void setup()
{
  pinMode(PIN_CS, OUTPUT);
  SPI.begin();  
  SPI.setClockDivider(SPI_CLOCK_DIV2);
  
  MIDI.begin(MIDI_CHANNEL_OMNI);
  MIDI.setHandleNoteOn(HandleNoteOn);
  MIDI.setHandlePitchBend(HandlePitchBend);
}
 
//assuming single channel, gain=2
void setOutput(unsigned int val)
{
  byte lowByte = val & 0xff;
  byte highByte = ((val >> 8) & 0xff) | 0x10;
   
  PORTB &= 0xfb;
  SPI.transfer(highByte);
  SPI.transfer(lowByte);
  PORTB |= 0x4;
}

void loop() {
  MIDI.read();
}

void HandleNoteOn(byte channel, byte pitch, byte velocity) {
  if(pitch >= ZERONOTE && pitch <= MAXNOTE) {
    if(velocity > 0) {
      lastNoteOn = pitch;
      int freq = round(SEMITONE*(lastNoteOn-ZERONOTE) + PITCH_BEND*(lastPitchBend/8192.0));
      freq = max(min(freq, DAC_MAX), 0);
      setOutput(freq);
      digitalWrite(TRIG_PIN, HIGH);
    }
    else if(pitch == lastNoteOn) {
      digitalWrite(TRIG_PIN, LOW);
    }
  }
}

void HandlePitchBend(byte channel, int bend) {
  lastPitchBend = bend;
  int freq = round(SEMITONE*(lastNoteOn-ZERONOTE) + PITCH_BEND*(lastPitchBend/8192.0));
  freq = max(min(freq, DAC_MAX), 0);
  setOutput(freq);
}
