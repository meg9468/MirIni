# MirIni
INI File Class for Arduino / ESP32 / ESP8266


This class implements a really simple way to handle INI files for Arduino / ESP32 / ESP8266....
I have tried to make it indepedent of FileSystem. Tested only with SPIFFS


How to use :

0) INI file Example :
```
[General]
version=1

[Wifi]
ssid=mySSid
password=myPass
```


1) Open and read INI file

```
    #include "MirQueue.h"
    #include "MirIni.h"

    [...]

    MirIni *configIni=NULL;
    if(!SPIFFS.begin(true)){
        Serial.println(F("An Error has occurred while mounting SPIFFS"));
    } else {
      if(SPIFFS.exists("/config.ini")) {
        File dataFile = SPIFFS.open("/config.ini", "r"); //Ouverture fichier pour le lire
        configIni=new MirIni(&dataFile);
        dataFile.close();
      }
    }
    // At this point :
    // configIni contains all INI variables
    // file "/config.ini" is closed
```

2) Get/Set variables

```
    #define SSID_SIZE 20
    #define PASSWORD_SIZE 20
    char ssid[SSID_SIZE];
    char password[PASSWORD_SIZE];
    [...]

    configIni->getVariableString("Wifi","ssid","",ssid,SSID_SIZE);
    configIni->getVariableString("Wifi","password","",password,SSID_SIZE);


    int tempResetCount=configIni->getVariableInt("General","resetCount",0);
    configIni->setVariableInt("General","resetCount",tempResetCount+1);

    // At this point ssid and password have been set from INI variable from section Wifi
    // And new variable 'resetCount' has been set into section General
    // But File has not been updated

```

3) Update INI file with updated vars
```
    if (configIni!=NULL){
      File dataFile = SPIFFS.open(configIni->getFilename(), "w");
      configIni->commit(&dataFile);
      dataFile.close();
    }

      SPIFFS.end(); // if nedded
```


Methodes :

```
  // Construcor/Destructor
  MirIni(File* dataFile);
  ~MirIni();

  void getVariableString(char* section, char* variable, char* defaultValue, char* bufferOut,int bufferSize);
  int getVariableInt(char* section, char* variable, int defaultValue);
  void setVariableString(char* section, char* variable, char* value);
  void setVariableInt(char* section, char* variable, int value);

  void removeSection(char* section);
  void removeVariable(char* section, char* variableName);

  void commit(File* dataFile);

  // debugTools
  void showVariables(); // Serial.print all vars loaded

  int getState(){return state;}
  char* getFilename(){return iniFileName;};

```
