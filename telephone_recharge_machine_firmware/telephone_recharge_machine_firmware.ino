
#include <ArduinoJson.h>
#include<BluetoothSerial.h>
#include<SPI.h>
#include<MFRC522.h>


#define MACHINE_ID "VS24RM01"
#define SS_PIN 5
#define RST_PIN 0

//creating instance of the class MFRC522
MFRC522 rfid(SS_PIN,RST_PIN);

//creating the struct instance of the MIFARE_Key
MFRC522::MIFARE_Key key;

//creating instance of the class BluetoothSerial
BluetoothSerial bt;

//creating instance of the JsonDocument
JsonDocument doc;


void controller(String inputJsonMessage) {
  DeserializationError error=deserializeJson(doc,inputJsonMessage);

  if(error) {

    doc.clear();

    doc["est"]=1;
    doc["ety"]=1;

    String jsonMessage;

    serializeJson(doc,jsonMessage);
    sendResponse(jsonMessage.c_str());

    return;
  }

  byte status = doc["st"];

  switch(status) {
    case 1:
      sendMachineId();
      break;
    case 2:
      sendCardBalance();
      break;
    case 3:
      writeAmountToCard();
      break;
    default:
      sendInvalidOptionResponse();
      return;
  }

} 

void sendResponse(const char * jsonMessage) {
  for(byte i=0; jsonMessage[i]; i++) {
    bt.write(jsonMessage[i]);
  }
}


uint32_t convertBytesToUnsignedInt32(byte *buffer){
  uint32_t res= 0;
  
  res |= (uint32_t)buffer[0] << 24;
  res |= (uint32_t)buffer[1] << 16;
  res |= (uint32_t)buffer[2] << 8;
  res |= (uint32_t)buffer[3];

  return res;
}

void covertUnsignedInt32ToBytes(uint32_t amount, byte *buffer) {
  buffer[0] = amount >> 24 & 0xFF;
  buffer[1] = amount >> 16 & 0xFF;
  buffer[2] = amount >> 8 & 0xFF;
  buffer[3] = amount & 0xFF;
}

void sendMachineId() {
  doc.clear();

  doc["est"]=0;
  doc["mty"]=1;

  String machineId = MACHINE_ID;

  machineId.toLowerCase();

  doc["mid"]=machineId;

  String jsonMessage;
  serializeJson(doc,jsonMessage);

  sendResponse(jsonMessage.c_str());
}


void sendCardBalance() {

  if(!rfid.PICC_IsNewCardPresent()) {
    //sending error json response if card is not present
    doc.clear();
    doc["est"]=1;
    doc["ety"]=2;

    String jsonMessage;
    serializeJson(doc,jsonMessage);
    sendResponse(jsonMessage.c_str());

    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();

    return;
  }

  if(!rfid.PICC_ReadCardSerial()) {
    //sending json respnose if rfid card uid reading failed
    doc.clear();
    doc["est"]=1;
    doc["ety"]=3;

    String jsonMessage;
    serializeJson(doc,jsonMessage);
    sendResponse(jsonMessage.c_str());

    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();


    return;
  }

  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);

  if(piccType != MFRC522::PICC_TYPE_MIFARE_1K ) {
    doc.clear();

    doc["est"]=1;
    doc["ety"]=4;

    String jsonMessage;
    serializeJson(doc,jsonMessage);
    sendResponse(jsonMessage.c_str());

    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();

    return;
  }

  byte sector=1;
  byte blockAddress=4;
  byte trailerBlock=7;

  MFRC522::StatusCode status = rfid.PCD_Authenticate(MFRC522::PICC_CMD_MF_AUTH_KEY_A,trailerBlock,&key,&(rfid.uid));

  if(status != MFRC522::STATUS_OK) {

    doc.clear();

    doc["est"]=1;
    doc["ety"]=5;

    String jsonMessage;
    serializeJson(doc,jsonMessage);
    sendResponse(jsonMessage.c_str());

    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();

    return;
  }

  byte buffer[18];

  byte bufferSize = sizeof(buffer);

  status = rfid.MIFARE_Read(blockAddress, buffer, &bufferSize);

  if(status != MFRC522::STATUS_OK) {
    
    doc.clear();

    doc["est"]=1;
    doc["ety"]=6;

    String jsonMessage;

    serializeJson(doc,jsonMessage);
    sendResponse(jsonMessage.c_str());

    rfid.PICC_HaltA();
    rfid.PCD_StopCrypto1();
    
  
    return;
  }

  uint32_t balance =  convertBytesToUnsignedInt32(buffer);

  doc.clear();

  doc["est"]=0;
  doc["mty"]=2;

  doc["bl"]=balance;


  String jsonMessage;
  serializeJson(doc,jsonMessage);
  sendResponse(jsonMessage.c_str());

}

void writeAmountToCard() {

  byte blockAddress=4;
  byte trailerBlock=7;
  byte buffer[16]={0};
  byte bufferSize=sizeof(buffer);

  uint32_t amount = doc["amt"];

  covertUnsignedInt32ToBytes(amount,buffer);

  MFRC522::StatusCode status = rfid.MIFARE_Write(blockAddress, buffer, bufferSize);

  if(status != MFRC522::STATUS_OK){
    doc.clear();

    doc["est"]=1;
    doc["ety"]=7;

    String jsonMessage;
    serializeJson(doc,jsonMessage);
    sendResponse(jsonMessage.c_str());

    return;
  }

  doc.clear();

  doc["est"]=0;
  doc["mty"]=3;

  String jsonMessage;
  serializeJson(doc,jsonMessage);
  sendResponse(jsonMessage.c_str());

  rfid.PICC_HaltA();
  rfid.PCD_StopCrypto1();

}

void sendInvalidOptionResponse(){
  doc.clear();

  doc["est"]=1;
  doc["ety"]=8;

  String jsonMessage;
  serializeJson(doc,jsonMessage);
  sendResponse(jsonMessage.c_str());
}

void setup() {
  Serial.begin(9600);
  bt.begin(MACHINE_ID);
  //init SPI bus
  SPI.begin();
  //init MFRC522 
  rfid.PCD_Init();

  //setting the RFID MIFARE Key
    for(byte i=0; i<6; i++) {
        key.keyByte[i]=0xFF;
    }
}

void loop() {
  if(bt.available()) {
    controller(bt.readString());
  }
}
