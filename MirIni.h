
#ifndef MIRINI_H_INCLUDED
#define MIRINI_H_INCLUDED

#include "MirQueue.h"

#include <FS.h>
#include <FSImpl.h>



//Buffer for one line of INI file
#ifndef INI_BUFFER_MEMORY
#define INI_BUFFER_MEMORY 128
#endif

// Max Size of filename
#ifndef FILENAME_SIZE
#define FILENAME_SIZE 32
#endif

// Size of section, variable name and variable content
#define SECTION_SIZE 16
#define KEY_SIZE 16
#define VARIABLE_SIZE 64


// Etats de l'objet
#define NO_STATE 0
#define INIT_OK 1
#define SPIFFS_ERROR -1
#define INI_FILE_NOT_FOUND -2


class MirIni{

private:

typedef struct _MirIniSectionType{
    char section[SECTION_SIZE];
    MirQueue *sectionQueue;
} MirIniSectionType; 

typedef struct _MirIniElementType{
    char key[KEY_SIZE];
    char value[VARIABLE_SIZE];
} MirIniElementType; 


private:
	char mirIniBuffer[INI_BUFFER_MEMORY];
	MirQueue mirQueueObject;
  char iniFileName[FILENAME_SIZE];

  int state=NO_STATE;
	
 
	
public:
	// Construcor
	
  MirIni(File* dataFile);
  ~MirIni();

  

  int getState(){return state;}
  char* getFilename(){return iniFileName;};

  void getVariableString(char* section, char* variable, char* defaultValue, char* bufferOut,int bufferSize);
  int getVariableInt(char* section, char* variable, int defaultValue);
  void setVariableString(char* section, char* variable, char* value);
  void setVariableInt(char* section, char* variable, int value);

  void removeSection(char* section);
  void removeVariable(char* section, char* variableName);
  
 
  //void commit();
  void commit(File* dataFile);

  // debugTool
  void showVariables();

  

private:    
  // internals tools.
  MirIniElementType * getVariableInSection(MirIniSectionType* section,char * variableName);
  MirIniElementType * setVariableInSection(MirIniSectionType* section,char * variableName,char* value);
	MirIniSectionType * getSection(char* sectionToFind);
  MirIniSectionType * addSection(char* sectionToAdd);

	
  // tools:
  static int strPos(char* str,char caract);
  

};


#endif

 
