#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include "FS.h"
#include "SPIFFS.h"


extern void WritePCF(int pin,int val);
extern int ReadPCF(int pin);

extern WebServer server;

#define HTML_FILE       0
#define TEXT_FILE       1
#define CSS_FILE        2
#define JS_FILE         3
#define PNG_FILE        4

#define MAX_CSTR      256
#define FILESYSTEM SPIFFS

int FileSize=0;
char *FileData=0;

void freeFile(void)
{
  if(FileData) free(FileData);
  FileData=0;
  FileSize=0;
}



int readFile(fs::FS &fs, const char * path){
  int fsize;
    Serial.printf("Reading file: %s\r\n", path);

    File file = fs.open(path);
    if(!file || file.isDirectory()){
        Serial.println("- failed to open file for reading");
        return 0;
    }
    fsize=file.size();
    Serial.printf("File size: %d\r\n",(int)fsize);
    
  freeFile();

    FileSize=fsize;
    FileData=(char *)malloc(FileSize+1);

    if(FileData){
      FileData[FileSize]=0; // null terminate
      file.read((uint8_t*)FileData,FileSize);
    }
    else{
      Serial.println("File malloc failed");
      return 0;
    }
    file.close();
    return 1;
}


int serveFile(char *filename,int plaintext)
{
  Serial.printf("SERVE FILE: %s\r\n",filename);
  if(readFile(FILESYSTEM,filename))
  {
      if(plaintext==TEXT_FILE)
          server.send_P(200,"text/plain",FileData,FileSize);
      else if(plaintext==HTML_FILE)
          server.send(200,"text/html",FileData);
        else if(plaintext==CSS_FILE)
          server.send(200,"text/css",FileData);
        else if(plaintext==JS_FILE)
          server.send_P(200,"text/javascript",FileData,FileSize);
        else if(plaintext==PNG_FILE)
          server.send_P(200,"image/png",FileData,FileSize);

    freeFile();
    return 0;
  }
  else
  {
  Serial.printf("unable to read file\r\n");

  }
  return 1;
}

void handleNotFound() {  // 404
 String filename=server.uri();
  if(serveFile((char *)filename.c_str(),0)) {
    String message = "<!DOCTYPE html><html><head><title>Boulter Tester</title></head><body>";
    message +="<h1>404</h1><pre>";
    message +="File Not Found\n\n";
    message += "URI: ";
    message += server.uri();
    message += "\nMethod: ";
    message += (server.method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += server.args();
    message += "\n";
    for (uint8_t i = 0; i < server.args(); i++) {
      message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    }
    message+="</pre></body></html>";
    server.send(404, "text/html", message);
  }
}

void handleFilename(char *filename,int plaintext)
{
 if(serveFile(filename,plaintext)){
  handleNotFound();
 }
}


void GetPin()
{
    String sbuffer,numstr,valstr;
    int pinnum,pinval;

   if(server.args()) {
    if(server.argName(0)=="pin") {
            numstr=server.arg(0);
            pinnum=numstr.toInt();
            pinval=ReadPCF(pinnum);
        Serial.printf("Reading PIN %d as %d\r\n",pinnum,pinval);

            sbuffer=pinval;
            server.send(200,"text/plain",sbuffer);  

    }
    else Serial.printf("First arg not pin (%s)\r\n",server.argName(0));
   }
   else Serial.println("No pin specified");
}

void SetPin()
{
  String sbuffer,numstr,valstr;
  int pinnum,pinval;

    Serial.println("ChangePin called");

  if(server.args()) {
    if(server.argName(0)=="pin")
    {
      numstr=server.arg(0);

      if(server.argName(1)=="val"){
        valstr=server.arg(1);
        pinnum=numstr.toInt();
        pinval=valstr.toInt();
        Serial.printf("Setting PIN %d to %d\r\n",pinnum,pinval);

        valstr=server.arg(1);
        WritePCF(pinnum,pinval);
      }
      else Serial.printf("Second arg not val (%s)\r\n",server.argName(1));
    }
    else Serial.printf("First arg not pin (%s)\r\n",server.argName(0));
  }
  else Serial.println("No arguments");

  sbuffer="OK";
  server.send(200,"text/plain",sbuffer);  

}


#include "json.hpp"
using json = nlohmann::json;


extern int KnownGood[];
extern int TestResult[];

extern const char *PinDescriptions[];

void GetMap(void) {
  json refresh;
   
   String index_string;

  for(int index=0;index<64;index++) {
    index_string=index;
    std::string nought=std::to_string(index);
    refresh[nought]={KnownGood[index*4+1],KnownGood[index*4+2],KnownGood[index*4+3],PinDescriptions[index*3],PinDescriptions[index*3+1]};
  }

  std::string refresh_std =refresh.dump(1);
  String refresh_string=String(refresh_std.c_str());

  server.send(200,"application/json",refresh_string);
}


void SetMap(void){

}


extern String ComparisonScan(void);

void ScanMap(void){
  json refresh;
   
   String index_string;

  ComparisonScan();

  for(int index=0;index<64;index++) {
    index_string=index;
    std::string nought=std::to_string(index);
    refresh[nought]={TestResult[index*4+1],TestResult[index*4+2],TestResult[index*4+3],PinDescriptions[index*3],PinDescriptions[index*3+1]};
  }

  std::string refresh_std =refresh.dump(1);
  String refresh_string=String(refresh_std.c_str());

  server.send(200,"application/json",refresh_string);
}

extern String ComparisonScan(void);
extern String scan_result;

void ScanReport(void){

 String sbuffer=ComparisonScan();
  scan_result=sbuffer;
  server.send(200,"text/plain",sbuffer);  
}

String retainedResult="";

void RefreshMap() {
  server.send(200,"text/plain",scan_result);  

}

extern String PairScan(void);

void PairScanner() {
  String pairscan=PairScan();
  //String pairscan="Blah blah buh-lah";
  server.send(200,"text/plain",pairscan);
}

void InitServer(void){
  
  server.enableCORS();

  server.on("/",                [](){ handleFilename((char *)"/index.html",     HTML_FILE);   }); 
  server.on("/index.html",      [](){ handleFilename((char *)"/index.html",     HTML_FILE);    });
  server.on("/index.css",       [](){ handleFilename((char *)"/index.css",      CSS_FILE);    });
  server.on("/index.js",        [](){ handleFilename((char *)"/index.js",       JS_FILE);    });
  server.on("/logo.png",        [](){ handleFilename((char *)"/logo.png",       PNG_FILE);    });
  server.on("/logoblack.png",   [](){ handleFilename((char *)"/logoblack.png",  PNG_FILE);    });
  server.on("/favicon.ico",     [](){ handleFilename((char *)"/favicon.ico",    PNG_FILE);    });

  server.on("/setpin",            SetPin       );
  server.on("/getpin",            GetPin       );
  server.on("/getmap",            GetMap       );
  server.on("/setmap",            SetMap       );
  server.on("/scanmap",           ScanMap      );
  server.on("/scanreport",        ScanReport   );
  server.on("/refresh",           RefreshMap   );
  server.on("/pairscan",          PairScanner  );

//server.on("/writeone",          WriteOne      );
 
// 404 code- searches for file and serves as HTML or give error page
  server.onNotFound(              handleNotFound);
  
  server.begin();
  Serial.println("HTTP server started");
  
}
