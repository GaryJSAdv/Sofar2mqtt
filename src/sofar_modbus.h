#ifndef sofarmodbush
#define sofarmodbush

#include <Arduino.h>

// This is the return object for the sendModbus() function. Since we are a modbus master, we
// are primarily interested in the responces to our commands.
typedef struct 
{
	uint8_t errorLevel;
	uint8_t data[64];
	uint8_t dataSize;
	char* errorMessage;
} TModbusResponce;

// Sofar Modbus commands.
// The two CRC bytes at the end are padded with zeros to make the frames the correct size. They get replaced using calcCRC at send time.
const uint8_t slaveId = 0x01;
const uint8_t readSingleRegister = 0x03;
const uint8_t passiveMode = 0x42;
extern uint8_t getRunningState[8];
extern uint8_t getGridVoltage[8];
extern uint8_t getGridCurrent[8] ;
extern uint8_t getGridFrequency[8]; 
extern uint8_t getBatteryPower[8] ;
extern uint8_t getBatteryVoltage[8]; 
extern uint8_t getBatteryCurrent[8];
extern uint8_t getBatterySOC[8];
extern uint8_t getBatteryTemperature[8] ;
extern uint8_t getBatteryCycles[8] ;
extern uint8_t getGridPower[8] ;
extern uint8_t getLoadPower[8];
extern uint8_t getSolarPV[8] ;
extern uint8_t getSolarPVToday[8] ;
extern uint8_t getGridExportToday[8] ;
extern uint8_t getGridImportToday[8] ;
extern uint8_t getLoadPowerToday[8] ;
extern uint8_t getInternalTemp[8] ;
extern uint8_t getHeatSinkTemp[8] ;
extern uint8_t getSolarPVCurrent[8]; 
extern uint8_t setStandby[8] ;
extern uint8_t setAuto[8];
extern uint8_t sendHeartbeat[8];
extern uint8_t setDischarge[8];
extern uint8_t setCharge[8];

#endif

extern void InitSofarModbus (void);

extern TModbusResponce sendModbus(uint8_t frame[], byte frameSize);