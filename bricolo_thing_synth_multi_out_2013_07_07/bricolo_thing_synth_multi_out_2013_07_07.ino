//variables setup
//boolean debug= true;  //set to true to change baud rate to 9600 & activate debugger 
//boolean debug= true;
boolean debug= false;

#define pot_1 0
#define numStrikeVals 128
#define numOuts 4

int chan = 2;


int firstPin=11; //lowest pin out the outputs  probably 2
byte incomingByte;
byte note;
byte pNote;
byte velocity;
byte currentNote;
byte notesPlaying;

float scaleCalc; 
float potValue;

int analogIn;
int pAnalogIn = analogIn;

int statusLed = 13;   // select the pin for the LED
int action=2; //0 =note off ; 1=note on ; 2= nada


#define numNotes 127 //how many outputs
//int firstPin = 22; //what's the first pin
int lowNote = 0;

int solenoid[numOuts];
long strikeTime[numOuts];
long strikeMark[numOuts];
long restMark[numOuts];
long restTime[numOuts];
boolean strikeState[numOuts];
boolean pulseState[numOuts];
int lastNote[numOuts];
long lastNoteOff[numOuts];


//float strikeTime =2500.0;
float strikeTime_upper =2000.0;

//float pStrikeTime=strikeTime;
float strikeTimeLow = 125;
float strikeTimeHigh = 3000;

//float strikeVals[10] = {800.0,1400.0,2000.0,2200.0,2500.0,2800.0,3000.0,3500.0,4000.0,4500.0};
float strikeVals[numNotes];
float baseTimes[numNotes]={61162.07951,57736.72055,54495.91281,51413.88175,48543.68932,45808.52038,43252.59516,40816.32653,38520.80123,36363.63636,34317.08991,32393.90994,30581.03976,28860.02886,27240.53391,25713.55104,24271.84466,22909.50745,21621.62162,20408.16327,19264.11096,18181.81818,17161.48962,16196.95497,15288.18224,14430.01443,13620.26696,12856.77552,12134.4497,11453.44176,10810.81081,10204.08163,9631.127805,9090.909091,8580.744809,8099.133393,7644.675484,7215.527816,6810.597289,6428.38776,6067.592986,5727.048852,5405.405405,5102.040816,4815.79581,4545.454545,4290.372404,4049.566696,3822.191645,3607.763908,3405.298645,3214.090573,3033.704457,2863.44243,2702.775751,2551.020408,2407.897905,2272.727273,2145.186202,2024.783348,1911.132346,1803.849415,1702.620333,1607.071113,1516.852228,1431.721215,1351.369613,1275.526474,1203.934458,1136.363636,1072.581597,1012.381425,955.566173,901.9328421,851.3101663,803.5290998,758.4318663,715.8657322,675.6848066,637.7632368,601.9672289,568.1818182,536.2907983,506.1932747,477.7830865,450.966421,425.6550832,401.7645499,379.2159331,357.931585,337.8424033,318.8816184,300.9836145,284.0909091,268.1461182,253.0959968,238.8909725,225.4832105,212.8275416,200.8826785,189.6082162,178.9665287,168.9221018,159.4414145,150.492827,142.0464756,134.0741723,126.5493115,119.4467805,112.7428764,106.4152262,100.442713,94.80540473,89.48448822,84.46220608,79.72179757,75.24744265,71.02420917,67.03800302,63.27552114,59.72420709,56.37220917,53.20834083,50.22204337,47.40335069,44.74285605,42.23168064};

//float baseTime = 60000;
float baseTime = 61162;
//float baseTime = 20000;

float restVals[numNotes];

void setup() {
if(debug== true){
  Serial.begin(9600);  
}
else(Serial.begin(31250));


for(int i=0; i<numOuts; i++){
  solenoid[i] = i+firstPin;
  
  pinMode(solenoid[i],OUTPUT);

restMark[i] = 0;
restTime[i] =0;
strikeState[i] = false;
pulseState[i]=false;
 
}

//pinMode(statusLed,OUTPUT);   // declare the LED's pin as output
//pinMode(solenoid_1,OUTPUT);
//scaleCalc=196.0/185.0;
//float e=1/12;
//scaleCalc = pow(2,e);
scaleCalc =1.0594630944;
//Serial.println(scaleCalc, 5);

//populate rest time values
for(int i=0; i<numNotes; i++){
//float x = pow(scaleCalc,i);
//Serial.print(i);
//String space=" ";
//Serial.print(space);
 //Serial.println(baseTimes[i]*pow(1.001,(i/12)-2));

if((i)%12==0 && debug){
  Serial.println(baseTimes[i]);
}
 if(i<72){
    baseTimes[i]-=215*(pow(.81,((i/12)-1))); 
  strikeVals[i]=(baseTimes[i]/(pow(i,.25)))*100;
 }
  if(i>=72){
   baseTimes[i]-=205*(pow(.81,((i/12)-1)));
   strikeVals[i]=(baseTimes[i]/6)*100;
  }
  
  restVals[i] =(baseTimes[i]*100-strikeVals[i]);
  //strikeVals[i]=baseTime/x/2;
 // restVals[i] =baseTime/x-strikeVals[i];
if(debug== true){
  float _baseTime=strikeVals[i]+restVals[i];
  float hz= 1000000/_baseTime*100;

String stringone=": hz= ";
String stringtwo=" ,time= ";
if((i)%12==0){
Serial.println();
Serial.print(i);
Serial.print(stringone);
Serial.print(hz);
Serial.print(stringtwo);
Serial.print(_baseTime/100);
Serial.print(stringtwo);
Serial.println(baseTimes[i]);
}
}
}
}

//loop: wait for serial data, and interpret the message
void loop () {

 //checkPot();
if(debug){
debugger();
}

  if (Serial.available() > 0) {
    // read the incoming byte:
    incomingByte = Serial.read();

    // wait for as status-byte, channel 1, note on or off
    if (incomingByte== 143+chan){ // note on message starting starting
      action=1;
    }else if (incomingByte== 127+chan){ // note off message starting
      action=0;
    }else if (incomingByte== 207+chan){ // aftertouch message starting
       //not implemented yet
    }else if (incomingByte== 159+chan){ // polypressure message starting
       //not implemented yet
    }else if ( (action==0)&&(note==0) ){ // if we received a "note off", we wait for which note (databyte)
      note=incomingByte;
      playNote(note, 0);
      note=0;
      velocity=0;
      
     
    
      action=2;
    }else if ( (action==1)&&(note==0) ){ // if we received a "note on", we wait for the note (databyte)
      pNote=note;
      note=incomingByte;
     // currentNote=note;
      
    }else if ( (action==1)&&(note!=0) ){ // ...and then the velocity
      velocity=incomingByte;
      playNote(note, velocity);
      note=0;
      velocity=0;
      action=0;
      
      for(int i; i<numOuts; i++){
      if(note=lastNote[i]){
      lastNoteOff[i] = millis();
      }
       }
    }else{
      //nada
    }
  }

for(int i=0; i<numOuts;i++){
pulse(i);
}
  
for(int i=0; i<numOuts; i++){  
timeoutCheck(i);

}
}


void playNote(byte note, byte velocity){
  int value=LOW;
  if (velocity >10){
      value=HIGH;
  }else{
   value=LOW;
  } 


 //since we don't want to "play" all notes we wait for a note in range
 if(note>=lowNote && note<lowNote+numNotes){
   if(value==HIGH && notesPlaying<numOuts){
  
   for(int i = 0; i<numOuts-1; i++){ 
  if(pulseState[i]==false){
   pulseState[i]=true;
   restTime[i]=restVals[note-lowNote];
   strikeTime[i]=strikeVals[note-lowNote];
   lastNote[i]=note;
   notesPlaying++;
   break;  
}
   }
   
    }
   if(value==LOW){
      for(int i = 0; i<numOuts-1; i++){ 
     if(lastNote[i] == note && pulseState[i]){
     pulseState[i]=false;
     notesPlaying--;
     }
      }
    
   } 
 }

}

void pulse(int p){
//strikeTime=strike;
//restTime=rest;
if(pulseState[p]==true){
if(micros()*100-restMark[p]>=restTime[p] && strikeState[p]==false){
strikeState[p]=true;
digitalWrite(solenoid[p], LOW);
digitalWrite(solenoid[p], HIGH);
strikeMark[p]=micros()*100;
}
///*
if(micros()*100-strikeMark[p]>=strikeTime[p] && strikeState[p]==true){
strikeState[p]=false;
digitalWrite(solenoid[p], LOW);
restMark[p]=micros()*100;


}
}

else digitalWrite(solenoid[p], LOW);
}


void timeoutCheck(int p){
//  if(millis()-lastNoteOff>1000){
if(millis()-lastNoteOff[p]>2000){
  incomingByte=127+chan;
  playNote(lastNote[p], 0);
  pulseState[p] = false;
 // strikeState = false;
 digitalWrite(solenoid[p], LOW);
 }
}

void debugger(){
//Serial.println(analogRead(pot_1));
//Serial.println(strikeTime);
//delay(1000);
}



void checkPot(){
analogIn = analogRead(pot_1);

if( analogIn != pAnalogIn){ 
//strikeTime= strikeVals[map(analogIn, 0, 1023, 0, numStrikeVals-1)];


  //strikeTime= strikeVals[map(analogIn, 0, 1023, 0, 9)];
//strikeTime= map(analogIn, 0, 1023, strikeTimeLow, strikeTimeHigh);

pAnalogIn=analogIn;
}


//if(strikeTime !=pStrikeTime){
//for(int i=0; i<numNotes; i++){
//float x = pow(scaleCalc,i);
  //restVals[i] =baseTime/x-strikeTime;
  // Serial.println(i);
  //Serial.println(restVals[i], 5);
//}

//pStrikeTime=strikeTime;

/*
if(strikeTime>3500){
baseTime =30000;
lowNote = 12;
}
if(strikeTime<=3500){
baseTime =15000;
lowNote = 24;
}

if(strikeTime<=1000){
baseTime =7500;
lowNote = 36;
}
*/
//}
}


