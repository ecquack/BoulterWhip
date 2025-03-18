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


char HOSTNAME[]="wtester"; // we are HOSTNAME.local via mDNS
char ssid[]="TELUS0609";
char password[]="t6z7gmkhd5";

// SDA is white wire
// SCL is brown wire

// digitalRead and digitalWrite pin definitions

#define RED_LED     32
#define WHITE_LED   14
#define GREEN_LED   15

#define SCAN_BUTTON 4


const char *PinDescriptions[]={ // pin description/name followed by connector description/name
  // bank 0
  "0 n.c.","","",//1
  "1 n.c.","","",//2
  "2 n.c.","","",//3
  "3 n.c.","","",//4
  "4 n.c.","","",//5
  "5 n.c.","","",//6
  "6 n.c.","","",//7
  "7 n.c.","","",//8
  "8 n.c.","","",//9
  "9 n.c.","","",//10
  "10 n.c.","","",//11
  "11 n.c.","","",//12
  "12 n.c.","","",//13
  "13 n.c.","","",//14
  "14 n.c.","","",//15
  "15 n.c.","","",//16
  // bank 1
  "16 n.c.","","",//17
  "17 n.c.","","",//18
  "18 n.c.","","",//19
  "19 n.c.","","",//20
  "20 n.c.","","",//21
  "21 n.c.","","",//22
  "22 n.c.","","",//23
  "23 n.c.","","",//24
  "24 n.c.","","",//25
  "25 n.c.","","",//26
  "26 n.c.","","",//27
  "27 n.c.","","",//28
  "28 n.c.","","",//29
  "29 n.c.","","",//30
  "30 n.c.","","",//31
  "31 n.c.","","",//32
  // bank 3
  "32 n.c.","","",//33
  "33 n.c.","","",//34
  "34 n.c.","","",//35
  "35 n.c.","","",//36
  "36 n.c.","","",//37
  "37 n.c.","","",//38
  "38 n.c.","","",//39
  "39 n.c.","","",//40
  "40 n.c.","","",//41
  "41 n.c.","","",//42
  "42 n.c.","","",//43
  "43 n.c.","","",//44
  "44 n.c.","","",//45
  "45 n.c.","","",//46
  "46 n.c.","","",//47
  "47 n.c.","","",//48
  "Array Overflow","error"
};

#define PIN_COUNT 48  
#define GOOD_WIDTH 4

int KnownGood[]={
00,-1,-1,-1,
01,31,-1,-1,
02,13,-1,-1,
03,12,-1,-1,
04,-1,-1,-1,
05,15,-1,-1,
06,11,-1,-1,
07,-1,-1,-1,
 8,17,-1,-1,
 9,-1,-1,-1,
10,20,-1,-1,
11,06,-1,-1,
12,03,-1,-1,
13,02,-1,-1,
14,21,-1,-1,
15,05,-1,-1,
16,28,-1,-1,
17, 8,-1,-1,
18,29,-1,-1,
19,30,-1,-1,
20,10,-1,-1,
21,14,-1,-1,
22,-1,-1,-1,
23,-1,-1,-1,
24,26,46,-1,
25,27,47,-1,
26,24,46,-1,
27,25,47,-1,
28,16,-1,-1,
29,18,-1,-1,
30,19,-1,-1,
31,01,-1,-1,
32,-1,-1,-1,
33,-1,-1,-1,
34,-1,-1,-1,
35,-1,-1,-1,
36,-1,-1,-1,
37,-1,-1,-1,
38,-1,-1,-1,
39,-1,-1,-1,
40,-1,-1,-1,
41,-1,-1,-1,
42,-1,-1,-1,
43,-1,-1,-1,
44,-1,-1,-1,
45,-1,-1,-1,
46,24,26,-1,
47,25,27,-1
};

int TestResult[GOOD_WIDTH*PIN_COUNT];

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
  for(int index=0;index<PIN_COUNT;index++) {
    WritePCF(index,1);
  }

  for(int outpin=0;outpin<PIN_COUNT;outpin++) {
    WritePCF(outpin,0);
    Serial.printf("%02d",outpin);
    sprintf(sbuffer,"%02d",outpin);
    pairscan=pairscan+sbuffer;
    for(int inpin=0;inpin<PIN_COUNT;inpin++)
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

int ComparisonResult[PIN_COUNT*GOOD_WIDTH];

#define MAX_STR 256

String ComparisonScan() {
  int mill=millis();
  char sbuffer[MAX_STR];
  int comparison[GOOD_WIDTH],cdex,errorcount=0;
  int connections=0,tests=0;
  String results;

  results="GPIO MAP  Description                      Connector  Wire \r\n\r\n";



  // set all pins high
  for(int index=0;index<PIN_COUNT;index++) {
    WritePCF(index,1);

    TestResult[index*GOOD_WIDTH]=-1;
    TestResult[index*GOOD_WIDTH+1]=-1;
    TestResult[index*GOOD_WIDTH+2]=-1;
    TestResult[index*GOOD_WIDTH+3]=-1;

  }

  for(int outpin=0;outpin<PIN_COUNT;outpin++) {

    if(outpin%4>1) {// 10 hertz? this is empirical and is determined by the GPIO speed
      digitalWrite(RED_LED,0);
      digitalWrite(GREEN_LED,1);
    }
    else
    {
      digitalWrite(RED_LED,1);
      digitalWrite(GREEN_LED,0);

    }

    WritePCF(outpin,0);
    for(cdex=0;cdex<GOOD_WIDTH;cdex++) comparison[cdex]=-1;
    cdex=0;
    comparison[cdex++]=outpin;
    for(int inpin=0;inpin<PIN_COUNT;inpin++)
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
      else TestResult[outpin*GOOD_WIDTH]=outpin;
    }
    int bad=0;
    for(cdex=0;cdex<4;cdex++) 
      if(KnownGood[outpin*GOOD_WIDTH+cdex]!=comparison[cdex]) bad++;
      
    if(bad) { 
   
      sprintf(sbuffer,
      "%02d->%02d,%02d %-32s %-10s %s\r\n",
      KnownGood[outpin*GOOD_WIDTH+0],
      KnownGood[outpin*GOOD_WIDTH+1],
      KnownGood[outpin*GOOD_WIDTH+2],
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
