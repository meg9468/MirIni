#include "MirQueue.h" 
#include "MirIni.h" 

#include <FS.h>
#include <FSImpl.h>





#ifndef PRINTLN
#define PRINTLN Serial.println
//#define PRINT //
#endif
#ifndef PRINT
#define PRINT Serial.print
//#define PRINT //
#endif
#ifndef TRACE
//#define PRINTLN Serial.println
#define TRACE //
#endif





////////////////////////////    Construcor / Destructor ////////////////////////////   


MirIni::MirIni(File* dataFile){
    char c;  
    char currentSection[32];
    currentSection[0]=0;
    MirIniElementType * iniElement=NULL;
    MirIniSectionType * iniSection=NULL;

    strcpy(iniFileName,dataFile->name()); 
 
    int sizeFile=dataFile->size(); 
    int readCounter=0;
    int bufferIndex=0;
    int bufferStart=0;
    //int bufferStop=0;
    int equalIndex=-1;
    int endSectionIndex;
    
    // Reading one line
    c=(char)dataFile->read();readCounter++;
    while (readCounter<sizeFile){
      bufferIndex=0;
      while ((readCounter<sizeFile) && (c!=10)){
        if (c>13) mirIniBuffer[bufferIndex++]=c;
        c=(char)dataFile->read();readCounter++;
      }
      mirIniBuffer[bufferIndex]=0;
      if (c==10){
        c=32;
    
        // End of line....
        // Remove spaces
        bufferStart=0;        
        while ((mirIniBuffer[bufferStart]==' ')  && (bufferStart<bufferIndex)) bufferStart++;


        // Empty line???
        if (bufferStart>=bufferIndex-2) continue;
        
        // Case 1 :  [ ==> Section 
        if (mirIniBuffer[bufferStart]=='['){
          endSectionIndex=strPos(&mirIniBuffer[bufferStart],']')+bufferStart;
          if (endSectionIndex<0) {
   
            continue; // ] missing => Unknown 
          }
          mirIniBuffer[endSectionIndex]=0;
          strcpy(currentSection,&mirIniBuffer[bufferStart+1]);

        
        iniSection=(MirIniSectionType*)malloc(sizeof(MirIniSectionType));
        strcpy(iniSection->section,currentSection);
        iniSection->sectionQueue=new MirQueue();
        iniElement=NULL;
                

        mirQueueObject.push(iniSection); 
     

        } else { // if (mirIniBuffer[bufferStart]=='['){
        
        // Case 2 : text... we should have a '='
          equalIndex=strPos(&mirIniBuffer[bufferStart],'=')+bufferStart;
          if (equalIndex<0) {

            continue; // ligne inconnue, ni section, ni xx=yyy
          }
          // Left : key / Right variable
          mirIniBuffer[equalIndex]=0;

        iniElement=(MirIniElementType*)malloc(sizeof(MirIniElementType));
        strcpy(iniElement->key,&mirIniBuffer[bufferStart]);
        strcpy(iniElement->value,&mirIniBuffer[equalIndex+1]);
        
        iniSection->sectionQueue->push(iniElement); 

        } 
        
 
        
      } // if (c==10){
    } //EOF

    state=INIT_OK;
    
  

 } // Construcor


 
// Destructor
MirIni::~MirIni(){ 
  // each session...
  MirIniSectionType *testSection;
   if (mirQueueObject.initIterator()!=NULL)   
      do{
        testSection=(MirIniSectionType*)mirQueueObject.getCurrentPayload();  

        // In this section, we remove the variables...
        PRINT("section : ");
        PRINTLN(testSection->section);
        delete (testSection->sectionQueue);
        delete (testSection->section); //  necessary ??  ( static array [] )


        PRINT("section deleted : ");
        PRINTLN(testSection->section);
        
      }while (mirQueueObject.getNext()!=NULL);

    
}


////////////////////////////  getSection  ////////////////////////////   

MirIni::MirIniSectionType * MirIni::getSection(char* sectionToFind){
    if (state<=0) return NULL;
    
    MirIniSectionType *testSection;

    if (mirQueueObject.initIterator()!=NULL)   
      do{
        testSection=(MirIniSectionType*)mirQueueObject.getCurrentPayload();  
        if (!strcmp(testSection->section,sectionToFind)) return testSection;
      }while (mirQueueObject.getNext()!=NULL);

      return NULL; // Not found !
}


////////////////////////////  addSection  ////////////////////////////   

MirIni::MirIniSectionType * MirIni::addSection(char* sectionToAdd){
  if (state<=0) return NULL;
      
  // Scan for section, if exists, will not create, just return the section
  MirIniSectionType *iniSection=getSection(sectionToAdd);

  if (iniSection!=NULL) return iniSection;

  // Add section...
  iniSection=(MirIniSectionType*)malloc(sizeof(MirIniSectionType));
  strcpy(iniSection->section,sectionToAdd);
  iniSection->sectionQueue=new MirQueue();
                
TRACE("MirINI::addSection : push new section ");
TRACE(iniSection->section);
TRACE("\n");
   mirQueueObject.push(iniSection); 

   return iniSection;
  
}



////////////////////////////  getVariableInSection  ////////////////////////////   


MirIni::MirIniElementType * MirIni::getVariableInSection(MirIniSectionType* section,char * variableName){
  if (state<=0) return NULL;
      
  if (section==NULL) return NULL;
  MirIniElementType  *testPayload;
  
  if (section->sectionQueue->initIterator()!=NULL){
    do{
      testPayload=(MirIniElementType*)section->sectionQueue->getCurrentPayload();   
      if (!strcmp(testPayload->key,variableName)) return testPayload;
    } while (section->sectionQueue->getNext()!=NULL);
  }//f (testSection->sectionQueue.initIterator()!=NULL){ 
  return NULL;
}

////////////////////////////  setVariableInSection  ////////////////////////////   

MirIni::MirIniElementType * MirIni::setVariableInSection(MirIniSectionType* section,char * variableName,char* value){
  if (section==NULL) return NULL;

  // Scan for variable, if exists, will not create, just set this one, else will add
  MirIniElementType  *iniElement=getVariableInSection(section,variableName);

  if (iniElement!=NULL){
    strcpy(iniElement->value,value);
    return iniElement;
  }

  // Create new var
  iniElement=(MirIniElementType*)malloc(sizeof(MirIniElementType));
  strcpy(iniElement->key,variableName);
  strcpy(iniElement->value,value);

  section->sectionQueue->push(iniElement); 

  return iniElement;
}



////////////////////////////  getVariableString()  ////////////////////////////   

void MirIni::getVariableString(char* section, char* variable, char* defaultValue, char* bufferOut,int bufferSize){
    if (state<=0) return;
  
    MirIniElementType  *tempPayload;
    MirIniSectionType *tempSection;

    tempSection=getSection(section);
    if (tempSection==NULL){
      strncpy(bufferOut,defaultValue,bufferSize); 
      return;
    }
    tempPayload=getVariableInSection(tempSection,variable);
    if (tempPayload==NULL){
      strncpy(bufferOut,defaultValue,bufferSize); 
      return;
    }

    strncpy(bufferOut,tempPayload->value,bufferSize); 
    
}


int MirIni::getVariableInt(char* section, char* variable, int defaultValue){
  if (state<=0) return NULL;
  
  char tempBufer[20];
  char tempDefault[20];
  
  sprintf(tempDefault,"%d",defaultValue);
 
  getVariableString( section, variable, tempDefault, tempBufer,20);

  
  return atoi(tempBufer);

  
}

////////////////////////////  setVariableString()  ////////////////////////////   

void MirIni::setVariableString(char* section, char* variable, char* value){
    if (state<=0) return;
  
    MirIniElementType  *tempPayload;
    MirIniSectionType *tempSection;

    tempSection=addSection(section);
    if (tempSection==NULL) return; // ERREUR !!

    setVariableInSection(tempSection,variable,value);
    
    
}

void MirIni::setVariableInt(char* section, char* variable, int value){
    if (state<=0) return;
  
    MirIniElementType  *tempPayload;
    MirIniSectionType *tempSection;

    char tempBuffer[20];
    sprintf(tempBuffer,"%d",value);

    tempSection=addSection(section);
    if (tempSection==NULL) return; // ERROR !!

    setVariableInSection(tempSection,variable,tempBuffer);
    
    
}

 
////////////////////////////   removeSection() ////////////////////////////   
void MirIni::removeSection(char* sectionToDelete){
    if (state<=0) return;
    
    MirIniSectionType *testSection;

    if (mirQueueObject.initIterator()!=NULL)   
      do{
        testSection=(MirIniSectionType*)mirQueueObject.getCurrentPayload();  
        if (!strcmp(testSection->section,sectionToDelete)) break;        
      }while (mirQueueObject.getNext()!=NULL);

      if (!strcmp(testSection->section,sectionToDelete)){
        testSection=(MirIniSectionType*)mirQueueObject.peek();

        delete (testSection->sectionQueue);
        delete (testSection);
      }

}

////////////////////////////   removeVariable() ////////////////////////////   
void MirIni::removeVariable(char* section, char* variableName){
    if (state<=0) return;
  
    MirIniElementType  *tempPayload;
    MirIniSectionType *tempSection;

    tempSection=getSection(section);
    if (tempSection==NULL) return; // ERREUR !!


    if (tempSection->sectionQueue->initIterator()!=NULL){
      do{
        tempPayload=(MirIniElementType*)tempSection->sectionQueue->getCurrentPayload();   
        if (!strcmp(tempPayload->key,variableName)) break;
      } while (tempSection->sectionQueue->getNext()!=NULL);
    }//f (testSection->sectionQueue.initIterator()!=NULL){ 

    if (!strcmp(tempPayload->key,variableName)) {
        tempPayload=(MirIniElementType*)tempSection->sectionQueue->peek();
        //FIXME delete ??? sans le Delete on a un memory leak de 100....
        delete tempPayload;
    }
 
}
  



////////////////////////////   Tool : strPos() ////////////////////////////   
  
int MirIni::strPos(char* str,char caract){
  for (int i=0;i<strlen(str);i++)
    if (str[i]==caract) return i;
    
  return -1;
}





 void MirIni::commit(File *dataFile ){

    char buffer[256];
    MirIniElementType  *testPayload;
    MirIniSectionType *testSection;

 
    if (mirQueueObject.initIterator()!=NULL){   
      do{
        testSection=(MirIniSectionType*)mirQueueObject.getCurrentPayload();  
        
        if (testSection->sectionQueue->initIterator()!=NULL){
          sprintf(buffer,"[%s]\n",testSection->section);
          dataFile->println(buffer);
TRACE(buffer);         
          do{           
              testPayload=(MirIniElementType*)testSection->sectionQueue->getCurrentPayload();   
              sprintf(buffer,"%s=%s\n",testPayload->key,testPayload->value);
              dataFile->println(buffer);
TRACE(buffer);

          } while (testSection->sectionQueue->getNext()!=NULL);
        }//f (testSection->sectionQueue.initIterator()!=NULL){
      }while (mirQueueObject.getNext()!=NULL);
    } // if (mirQueueObject.initIterator()!=NULL){
    
    
}

////////////////////////////   showVariables()  ////////////////////////////   

 void MirIni::showVariables(){

    char buffer[256];
    MirIniElementType  *testPayload;
    MirIniSectionType *testSection;

    PRINT("\n********************\nMirINI::showVariables\n");
    // Iterator
    if (mirQueueObject.initIterator()!=NULL){
    
      do{

        testSection=(MirIniSectionType*)mirQueueObject.getCurrentPayload();  
        
        if (testSection->sectionQueue->initIterator()!=NULL){
         
          sprintf(buffer,"MirINI::showVariables Section  [%s]\n",testSection->section);
          PRINT(buffer);
          
          do{
            
              testPayload=(MirIniElementType*)testSection->sectionQueue->getCurrentPayload();   
   
              sprintf(buffer,"MirINI::showVariables: [%s] (%s,%s)\n",testSection->section,testPayload->key,testPayload->value);
              PRINT(buffer);

          } while (testSection->sectionQueue->getNext()!=NULL);


        }//f (testSection->sectionQueue.initIterator()!=NULL){


      }while (mirQueueObject.getNext()!=NULL);

      
    } // if (mirQueueObject.initIterator()!=NULL){
    

 PRINTLN("MirINI::showVariables\n********************\n");
 
 }




  
