#include <MIDI.h>
#include <MCP3008.h>
 
//define pin connections
#define CS_PIN_ONE 9
#define CS_PIN_TWO 10
#define CLOCK_PIN 13
#define MOSI_PIN 11
#define MISO_PIN 12
 
MCP3008 adcOne(CLOCK_PIN, MOSI_PIN, MISO_PIN, CS_PIN_ONE);
MCP3008 adcTwo(CLOCK_PIN, MOSI_PIN, MISO_PIN, CS_PIN_TWO);

MIDI_CREATE_DEFAULT_INSTANCE();

int counter=0;
int LEDpins[6]{3,4,5,6,7,8};

int midiChanButtonCurrent = 0;
int midiChanButtonPrevious = 0;

int waveformButtonCurrent = 0;
int waveformButtonPrevious = 0;

int paramStates[20]{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,1};

void setup() {
  for (int i=0;i<6;i++){
    pinMode(LEDpins[i],OUTPUT);
  }
  displayMidiChan(paramStates[19]);
  displayWaveform(paramStates[18]);
  MIDI.setHandleNoteOn(handleNoteOn);
  MIDI.setHandleNoteOff(handleNoteOff);
  MIDI.begin(MIDI_CHANNEL_OMNI);
  MIDI.turnThruOff();
}

void handleNoteOn(byte channel, byte pitch, byte velocity){
    MIDI.sendNoteOn(pitch, 127, 1);
}
void handleNoteOff(byte channel, byte pitch, byte velocity){ 
    MIDI.sendNoteOff(pitch, 0, 1);
}  

//there's a better way to do this but i'm too lazy rn
void displayMidiChan(int chan){
  if (chan==1){digitalWrite(3,LOW);digitalWrite(4,LOW);digitalWrite(5,LOW);digitalWrite(6,HIGH);}
  if (chan==2){digitalWrite(3,LOW);digitalWrite(4,LOW);digitalWrite(5,HIGH);digitalWrite(6,LOW);}
  if (chan==3){digitalWrite(3,LOW);digitalWrite(4,LOW);digitalWrite(5,HIGH);digitalWrite(6,HIGH);}
  if (chan==4){digitalWrite(3,LOW);digitalWrite(4,HIGH);digitalWrite(5,LOW);digitalWrite(6,LOW);}
  if (chan==5){digitalWrite(3,LOW);digitalWrite(4,HIGH);digitalWrite(5,LOW);digitalWrite(6,HIGH);}
  if (chan==6){digitalWrite(3,LOW);digitalWrite(4,HIGH);digitalWrite(5,HIGH);digitalWrite(6,LOW);}
  if (chan==7){digitalWrite(3,LOW);digitalWrite(4,HIGH);digitalWrite(5,HIGH);digitalWrite(6,HIGH);}
  if (chan==8){digitalWrite(3,HIGH);digitalWrite(4,LOW);digitalWrite(5,LOW);digitalWrite(6,LOW);}
  if (chan==9){digitalWrite(3,HIGH);digitalWrite(4,LOW);digitalWrite(5,LOW);digitalWrite(6,HIGH);}
  if (chan==10){digitalWrite(3,HIGH);digitalWrite(4,LOW);digitalWrite(5,HIGH);digitalWrite(6,LOW);}
  if (chan==11){digitalWrite(3,HIGH);digitalWrite(4,LOW);digitalWrite(5,HIGH);digitalWrite(6,HIGH);}
  if (chan==12){digitalWrite(3,HIGH);digitalWrite(4,HIGH);digitalWrite(5,LOW);digitalWrite(6,LOW);}
  if (chan==13){digitalWrite(3,HIGH);digitalWrite(4,HIGH);digitalWrite(5,LOW);digitalWrite(6,HIGH);}
  if (chan==14){digitalWrite(3,HIGH);digitalWrite(4,LOW);digitalWrite(5,HIGH);digitalWrite(6,LOW);}
  if (chan==15){digitalWrite(3,HIGH);digitalWrite(4,HIGH);digitalWrite(5,HIGH);digitalWrite(6,HIGH);}
  if (chan==16){digitalWrite(3,LOW);digitalWrite(4,LOW);digitalWrite(5,LOW);digitalWrite(6,LOW);}
}

void displayWaveform(int WFnum){
  if (WFnum==1){digitalWrite(7,LOW);digitalWrite(8,HIGH);}
  if (WFnum==2){digitalWrite(7,HIGH);digitalWrite(8,LOW);}
  if (WFnum==3){digitalWrite(7,HIGH);digitalWrite(8,HIGH);}
}

void readInputStatus(){

  //check for WF button press
  waveformButtonCurrent=digitalRead(A4);
  if (waveformButtonCurrent&&!waveformButtonPrevious){
    paramStates[18]+=1;
    if (paramStates[18]==4){
      paramStates[18]=1;
    }
    displayWaveform(paramStates[18]);
  }

  //check for midiRX chan button press
  midiChanButtonCurrent=digitalRead(A2);
  if (midiChanButtonCurrent!=midiChanButtonPrevious){
    paramStates[19]+=1;
    if (paramStates[19]==17){
      paramStates[19]=1;
    }
    displayMidiChan(paramStates[19]);
  }

  //reads the 16 pots on the mcp3008's (all synth param pots)
  for(int i=0;i<8;i++){
    paramStates[i]=map(adcOne.readADC(i), 1023, 0, 0, 127);
    paramStates[i+8]=map(adcTwo.readADC(i), 1023, 0, 0, 127); 
  }
  
  paramStates[16]=map(analogRead(A0),1023,0,0,127);//volume knob on AO
  paramStates[17]=digitalRead(17);//LFO WF select switch on A1
  
  waveformButtonPrevious=waveformButtonCurrent;
  midiChanButtonPrevious=midiChanButtonCurrent;
}

//sends the pot values as MIDI CC messages to the chip
void sendMIDI(){
  MIDI.sendControlChange( 7, paramStates[16], 1);  //volume
  MIDI.sendControlChange( 1, paramStates[3], 1);   //LFO mod
  MIDI.sendControlChange(16, paramStates[15], 1);   //LFO rate
  
  if (paramStates[17]){
    MIDI.sendControlChange(20,   0, 1);           //LFO waveform 0-63 tri, 64-127 S+H
  }else{
    MIDI.sendControlChange(20,   127, 1);   
  }
  
  MIDI.sendControlChange(74, paramStates[13], 1);   //cutoff 
  MIDI.sendControlChange(71, paramStates[4], 1);   //resonance
  MIDI.sendControlChange(82, paramStates[11], 1);   //Filter EG Attack
  MIDI.sendControlChange(83, paramStates[8], 1);   //Filter EG Decay
  MIDI.sendControlChange(28, paramStates[12], 1);   //Filter EG Sustain
  MIDI.sendControlChange(29, paramStates[7], 1);   //Filter EG Release
  MIDI.sendControlChange(81, paramStates[0], 1);   //Filter EG depth
                                                                                                                  
  if (paramStates[18]==1){//triangle               //DCO waveform select                     
    MIDI.sendControlChange(76, 0, 1);   
  }else if (paramStates[18]==2){//pulse
    MIDI.sendControlChange(76, 50, 1);   
  }else{                    //saw
    MIDI.sendControlChange(76, 100, 1);   
  }
  
  MIDI.sendControlChange( 4, paramStates[1], 1);    //DCO wrap
  MIDI.sendControlChange(21, paramStates[6], 1);   //DCO range
  MIDI.sendControlChange(93, paramStates[2], 1);   //DCO detune
  MIDI.sendControlChange(73, paramStates[10], 1);   //Amp EG Attack
  MIDI.sendControlChange(75, paramStates[9], 1);   //Amp EG Decay
  MIDI.sendControlChange(31, paramStates[14], 1);   //Amp EG Sustain
  MIDI.sendControlChange(72, paramStates[5], 1);   //Amp EG Release 
}

void loop() {
  
  MIDI.read(paramStates[19]);
  
  if (counter==10){
    readInputStatus();
    sendMIDI();
    counter=0;
  }
  
  counter++;
}
