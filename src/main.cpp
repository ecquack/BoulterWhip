#include <Arduino.h>

#include "PCF8575.h"
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include "FS.h"
#include "SPIFFS.h"
#include <vector>

//
// network ID and credentials
//


char HOSTNAME[]="htester"; // we are HOSTNAME.local via mDNS
char ssid[]="TELUS0609";
char password[]="t6z7gmkhd5";

// SDA is white wire
// SCL is brown wire

// digitalRead and digitalWrite pin definitions

#define RED_LED     32
#define WHITE_LED   14
#define GREEN_LED   32

#define SCAN_BUTTON 4


const char *PinDescriptions[]={ // pin description/name followed by connector description/name
  "1 pendant stop","circular","black/red",// 0
  "2 pendant start","circular","black/white",//1
  "3 pendant jog","circular","black",//2
  "4 pendant red","circular","white/red",//3
  "5 pendant green","circular","white",//4
  "6 pendant alarm","circular","white/black",//5
  "7 chassis ground","circular","green",//6
  "8 interlock stop +24","circular","orange/black",//7
  "9 interlock stop gnd","circular","blue/black",//8
  "n.c.","circular","",//9
  "11 clutch switched gnd","circular","black",//10
  "12 pendant +24v","circular","orange",//11
  "13 pendant gnd","circular","blue",//12
  "14 clutch power","circular","brown",//13
  "n.c.","circular","",//14
  "16 missing flat/egg +24","circular","orange/red",//15
  "17 missing flat/egg gnd","circular","blue/red",//16
  "18 cycle stop","circular","green",//17
  "19 missing flat","circular","red",//18
  "20 missing egg","circular","red/black",//19
  "21 flow control","circular","red/white",//20
  "n.c.","circular","",//21
  "n.c.","circular","",//22
  "n.c.","circular","",//23
  "34 interlock","circular","green/black",//24
  "35 interlock n.c.","circular","green/white",//25
  "n.c.","","",//26
  "n.c.","","",//27
  "n.c.","","",//28
  "n.c.","","",//29
  "n.c.","","",//30
  "n.c.","","",//31
  // outputs 
  "pendant red",  "purple","brown (alt. white)",//32
  "pendant gnd",  "purple","violet (alt. blue)",//33
  "pendant start","purple","blue (alt. green)",//34
  "pendant jog",  "purple","black (alt. yellow)",//35
  "pendant green","purple","pink",//36
  "pendant alarm","purple","violet (alt. blue)",//37
  "pendant +24V", "purple","white (alt. brown)",//38
  "pendant stop", "purple","orange (alt. red)",//39
  "n.c.","","",//40
  "n.c.","","",//41
  "n.c.","","",//42
  "n.c.","","",//43
  "clutch power","red","red",//44
  "n.c.","","",//45
  "n.c.","","",//46
  "clutch switched GND","red","black",//47
  "interlock stop +24V","orange","",//48
  "n.c","","",//49
  "missing flat +24V","orange","",//50
  "cycle stop","orange","",//51
  "interlock cycle stop GND","blue","",//52
  "flow control","blue","",//53
  "missing egg GND","blue","",//54
  "missing egg","blue","",//55
  "stacker ","black","",//56
  "missing egg +24V","black","",//57
  "stacker int","black","",//58
  "interlock stop +24V","black","",//59
  "missing flat","yellow","",//60
  "missing flat egg GND","yellow","",//61
  "n.c.","","",//62
  "interlock cycle stop GND","yellow","",//63
  "Array Overflow","error"
};

//
// wiring harness map for Boutler Egg Packer 
// -1 means N.C.
//



int KnownGood[]={
  0,39,-1,-1,
  1,34,-1,-1,
  2,35,-1,-1,
  3,32,-1,-1,
  4,36,-1,-1,
  5,37,-1,-1,
  6,40,-1,-1,
  7,48,59,-1,
  8,52,63,-1,
  9,-1,-1,-1,
  10,47,-1,-1,
  11,38,-1,-1,
  12,33,-1,-1,
  13,44,-1,-1,
  14,-1,-1,-1,
  15,50,57,-1,
  16,54,61,-1,
  17,51,-1,-1,
  18,60,-1,-1,
  19,55,-1,-1,
  20,53,-1,-1,
  21,-1,-1,-1,
  22,-1,-1,-1,
  23,-1,-1,-1,
  24,58,-1,-1,
  25,56,-1,-1,
  26,-1,-1,-1,
  27,-1,-1,-1,
  28,-1,-1,-1,
  29,-1,-1,-1,
  30,-1,-1,-1,
  31,-1,-1,-1,
  32, 3,-1,-1,
  33,12,-1,-1,
  34, 1,-1,-1,
  35, 2,-1,-1,
  36, 4,-1,-1,
  37, 5,-1,-1,
  38,11,-1,-1,
  39, 0,-1,-1,
  40, 6,-1,-1,
  41,-1,-1,-1,
  42,-1,-1,-1,
  43,-1,-1,-1,
  44,13,-1,-1,
  45,-1,-1,-1,
  46,-1,-1,-1,
  47,10,-1,-1,
  48, 7,59,-1,
  49,-1,-1,-1,
  50,15,57,-1,
  51,17,-1,-1,
  52, 8,63,-1,
  53,20,-1,-1,
  54,16,61,-1,
  55,19,-1,-1,
  56,25,-1,-1,
  57,15,50,-1,
  58,24,-1,-1,
  59, 7,48,-1,
  60,18,-1,-1,
  61,16,54,-1,
  62,-1,-1,-1,
  63, 8,52,-1
};
int TestResult[256];


const char *FailNames[]={
  "purple","pendant",
  "red","clutch",
  "yellow","",
  "black","",
  "orange","",
  "blue","",
  "error","overflow"
};

int FailList[]={

 0, 1, 2, 3, 4, 5,11,12, // purple
10,13,-1,-1,-1,-1,-1,-1, // red
 8,16,18,-1,-1,-1,-1,-1, // yellow
 7,15,24,25,-1,-1,-1,-1, // black
 7,15,17,-1,-1,-1,-1,-1, // orange
 8,16,19,20,-1,-1,-1,-1  // blue
};

int FailArray[6];


int FailScan(){
  for(int connector=0;connector<6;connector++)
  {
    FailArray[connector]=0;
    for(int pin=0;pin<8;pin++) {
      int xpin=FailList[connector*8+pin];
      if(xpin>=0)
      if(xpin<32)
      if(TestResult[xpin*4+1]==-1) {
       // Serial.printf("%s %s %d %d\r\n",FailNames[connector*2],FailNames[connector*2+1],connector,xpin);
        FailArray[connector]++;
      } 
      //else if(TestResult[xpin*4+2]==-1) 
      {
        //Serial.printf("Blah %d\r\n",TestResult[xpin*4+2]);
        //FailArray[connector]++;
      }
    }
  }

//  for(int showfails=0;showfails<6;showfails++) {
//    if(FailArray[showfails]) Serial.printf("%s is disconnected (%d)\r\n",FailNames[showfails*2],FailArray[showfails]);
// }
  return 0;
}



//  adjust addresses if needed
#include "PCF8575.h"

// these should be an array or vector. maybe next version

PCF8575 PCF;
PCF8575 PCF0(0x20);
PCF8575 PCF1(0x21);
PCF8575 PCF2(0x22);
PCF8575 PCF3(0x23);
PCF8575 PCF4(0x24);
PCF8575 PCF5(0x25);
PCF8575 PCF6(0x26);
PCF8575 PCF7(0x27);

int PCFS[8];

WebServer server(80);
extern void InitServer(void);


int InitWiFi(void){
  int timeout=0;
  WiFi.mode(WIFI_STA);
  WiFi.setHostname(HOSTNAME);
  WiFi.begin(ssid,password);



  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if(timeout++>20) {
      Serial.println("\r\nTimeout: Failed to connect to WiFi");
      return 0;
    }
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  return 1;
}


void InitFileSystem(void){
   #define FORMAT_FS_IF_FAILED true

    if(!SPIFFS.begin(FORMAT_FS_IF_FAILED)){
        Serial.println("FS Mount Failed");
    }
    else
    {
       Serial.println("FS Mounted");
    }
}

void InitMDNS(void) {
  if (MDNS.begin(HOSTNAME)) {
    MDNS.addService("telnet", "tcp", 23);
    Serial.printf("MDNS %s.local\r\n",HOSTNAME);
  }
  else
    Serial.println("MDNS failed to start");
}

void InitPCF(PCF8575 PCF,int msg) {
  PCFS[msg]=0;
  if (!PCF.begin())
  {
    Serial.print(msg);
    Serial.println(" => not initialized");
  }
  else
  if (!PCF.isConnected())
  {
    Serial.print(msg);
    Serial.println(" => not connected");
  }
  else
  {
    Serial.print(msg);
    Serial.println(" => connected!!");
    PCFS[msg]=1;
  }

}

void InitPCFS(void) {
  InitPCF(PCF0,0);
  InitPCF(PCF1,1);
  InitPCF(PCF2,2);
  InitPCF(PCF3,3);
  InitPCF(PCF4,4);
  InitPCF(PCF5,5);
  InitPCF(PCF6,6);
  InitPCF(PCF7,7);

}

void WritePCF(int pin,int val) {

  int device,ppin;

  device=pin/16;
  ppin=pin%16;

  if(PCFS[device]) {

  if(device==0) PCF0.write(ppin,val);
  if(device==1) PCF1.write(ppin,val);
  if(device==2) PCF2.write(ppin,val);
  if(device==3) PCF3.write(ppin,val);
  if(device==4) PCF4.write(ppin,val);
  if(device==5) PCF5.write(ppin,val);
  if(device==6) PCF6.write(ppin,val);
  if(device==7) PCF7.write(ppin,val);
  }
//  else Serial.printf("PCF%d offline\r\n",device);
}

int ReadPCF(int pin) {
  int device,ppin,val;

  device=pin/16;
  ppin=pin%16;

  if(PCFS[device]) {

    if(device==0) val=PCF0.read(ppin);
    if(device==1) val=PCF1.read(ppin);
    if(device==2) val=PCF2.read(ppin);
    if(device==3) val=PCF3.read(ppin);
    if(device==4) val=PCF4.read(ppin);
    if(device==5) val=PCF5.read(ppin);
    if(device==6) val=PCF6.read(ppin);
   if(device==7) val=PCF7.read(ppin);
  }
 // else Serial.printf("PCF%d offline\r\n",device);
  //Serial.printf("Read device %d pin %d as %d\r\n",device,ppin,val);

  return val;
}

String PairScan(void) {
  String pairscan;
  char sbuffer[10];
  // set all pins high
  for(int index=0;index<80;index++) {
    WritePCF(index,1);
  }

  for(int outpin=0;outpin<80;outpin++) {
    WritePCF(outpin,0);
    Serial.printf("%02d",outpin);
    sprintf(sbuffer,"%02d",outpin);
    pairscan=pairscan+sbuffer;
    for(int inpin=0;inpin<80;inpin++)
    {
      if(inpin!=outpin)
        if(ReadPCF(inpin)==0)
        {
          Serial.printf(",%02d",inpin);
          sprintf(sbuffer,",%02d",outpin);
          pairscan=pairscan+sbuffer;
        }
    }
    Serial.println();
    pairscan=pairscan+"\r\n";
    WritePCF(outpin,1);
  }
  return pairscan;
}

int ComparisonResult[64*4];

#define MAX_STR 256

String ComparisonScan() {
  int mill=millis();
  char sbuffer[MAX_STR];
  int comparison[4],cdex,errorcount=0;
  int connections=0,tests=0;
  String results;

  results="GPIO MAP  Description                      Connector  Wire \r\n\r\n";



  // set all pins high
  for(int index=0;index<64;index++) {
    WritePCF(index,1);

    TestResult[index*4]=-1;
    TestResult[index*4+1]=-1;
    TestResult[index*4+2]=-1;
    TestResult[index*4+3]=-1;

  }

  for(int outpin=0;outpin<64;outpin++) {

    if(outpin%4>1) {// 10 hertz?
      digitalWrite(RED_LED,0);
      digitalWrite(GREEN_LED,1);
    }
    else
    {
      digitalWrite(RED_LED,1);
      digitalWrite(GREEN_LED,0);

    }

    WritePCF(outpin,0);
    for(cdex=0;cdex<4;cdex++) comparison[cdex]=-1;
    cdex=0;
    comparison[cdex++]=outpin;
    for(int inpin=0;inpin<64;inpin++)
    {
      if(inpin!=outpin)
      {
        if(ReadPCF(inpin)==0) {
          TestResult[outpin*4+cdex]=inpin;        
          comparison[cdex++]=inpin;
          connections++;
        }
        tests++;
      }
      else TestResult[outpin*4]=outpin;
    }
    int bad=0;
    for(cdex=0;cdex<4;cdex++) 
      if(KnownGood[outpin*4+cdex]!=comparison[cdex]) bad++;
      
    if(bad) { 
   
      sprintf(sbuffer,
      "%02d->%02d,%02d %-32s %-10s %s\r\n",
      KnownGood[outpin*4+0],KnownGood[outpin*4+1],KnownGood[outpin*4+2],
 //       comparison[0],comparison[1],comparison[2],comparison[3],
        PinDescriptions[comparison[0]*3],PinDescriptions[comparison[0]*3+1],PinDescriptions[comparison[0]*3+2]);

      results=results+sbuffer;

      errorcount++;
    }
    
    WritePCF(outpin,1);
  }
  if(errorcount==0){
     results="";
      digitalWrite(RED_LED,0);
      digitalWrite(GREEN_LED,1);
  }
  else {
    digitalWrite(RED_LED,1);
    digitalWrite(GREEN_LED,0);
    FailScan();
    results=results+"\r\n";
    for(int showfails=0;showfails<6;showfails++) {
      if(FailArray[showfails]) {
        sprintf(sbuffer,"<h1 style=\"line-height:0px;color:red;\">%s is disconnected (%d)</h1>\r\n",FailNames[showfails*2],FailArray[showfails]);
        results=results+sbuffer;
      }
  }
  }
  sprintf(sbuffer,"\r\n<b>Scan complete, %d errors<b>\r\n\r\n%d milliseconds elapsed\r\n%d connections tested \r\n%4d connections detected\r\n",errorcount,millis()-mill,tests,connections);

  results=results+sbuffer;

  return results;
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("It's.... alive! ");

Serial.print("PCF8575_LIB_VERSION:\t");
  Serial.println(PCF8575_LIB_VERSION);


  Wire.begin();

  InitPCFS();

  InitWiFi();
  InitMDNS();
  InitFileSystem();
  //httpUpdater.setup(&server);

  InitServer();
  
  MDNS.addService("http", "tcp", 80);
  Serial.println("HTTP service added");

  pinMode(RED_LED,OUTPUT);
  pinMode(GREEN_LED,OUTPUT);
  pinMode(WHITE_LED,OUTPUT);
  pinMode(SCAN_BUTTON,INPUT_PULLUP);

  WritePCF(64+5,1);
  WritePCF(64+6,0);
  WritePCF(64+7,0);

 // int mill=millis();
 // Serial.println("Comparison Scan");
 // Serial.println(ComparisonScan());
 int tBytes = SPIFFS.totalBytes(); int uBytes = SPIFFS.usedBytes();
 Serial.printf("SPIFFS Total: %d Used: %d Free: %d\r\n",tBytes,uBytes,tBytes-uBytes);
 PairScan();
}

int last=0;

String scan_result="";

void loop() {

  server.handleClient();        // handle any pending HTTP requests     

  if(digitalRead(SCAN_BUTTON)==0) {
    if(last==0)
    {
      Serial.println("Comparison Scan");
      Serial.println(scan_result=ComparisonScan());
    }
    last=1;
  }
  else last=0;

  if(WiFi.status() != WL_CONNECTED)
  {

    if(millis()%1000>800) // blink at 1 hertz 20% duty cycle
      digitalWrite(WHITE_LED,1);
      else
      digitalWrite(WHITE_LED,0);
      return;
  }
  else digitalWrite(WHITE_LED,1);
}
