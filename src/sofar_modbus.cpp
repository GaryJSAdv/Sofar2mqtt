
#include <Arduino.h>
#include "sofar_modbus.h"
#include "sofar_mqtt.h"
#include "HardwareSerial.h"
#include "globals.h"

#include "config.h"

// The built-in serial port remains available for flashing and debugging.
#define RS485_TX HIGH
#define RS485_RX LOW

HardwareSerial RS485Serial(RS485_SERIAL_CHANNEL);

unsigned long time_3 = 0;
#define SEND_INTERVAL 10000


uint8_t getRunningState[] = {slaveId, readSingleRegister, 0x02, 0x00, 0x00, 0x04, 0x00, 0x00};
uint8_t getGridVoltage[] = {slaveId, readSingleRegister, 0x02, 0x06, 0x00, 0x01, 0x00, 0x00};
uint8_t getGridCurrent[] = {slaveId, readSingleRegister, 0x02, 0x07, 0x00, 0x02, 0x00, 0x00};
uint8_t getGridFrequency[] = {slaveId, readSingleRegister, 0x02, 0x0C, 0x00, 0x01, 0x00, 0x00};
uint8_t getBatteryPower[] = {slaveId, readSingleRegister, 0x02, 0x0D, 0x00, 0x01, 0x00, 0x00};
uint8_t getBatteryVoltage[] = {slaveId, readSingleRegister, 0x02, 0x0E, 0x00, 0x02, 0x00, 0x00};
uint8_t getBatteryCurrent[] = {slaveId, readSingleRegister, 0x02, 0x0F, 0x00, 0x10, 0x00, 0x00};
uint8_t getBatterySOC[] = {slaveId, readSingleRegister, 0x02, 0x10, 0x00, 0x02, 0x00, 0x00};
uint8_t getBatteryTemperature[] = {slaveId, readSingleRegister, 0x02, 0x11, 0x00, 0x02, 0x00, 0x00};
uint8_t getBatteryCycles[] = {slaveId, readSingleRegister, 0x02, 0x2C, 0x00, 0x01, 0x00, 0x00};
uint8_t getGridPower[] = {slaveId, readSingleRegister, 0x02, 0x12, 0x00, 0x02, 0x00, 0x00};
uint8_t getLoadPower[] = {slaveId, readSingleRegister, 0x02, 0x13, 0x00, 0x02, 0x00, 0x00};
uint8_t getSolarPV[] = {slaveId, readSingleRegister, 0x02, 0x15, 0x00, 0x02, 0x00, 0x00};
uint8_t getSolarPVToday[] = {slaveId, readSingleRegister, 0x02, 0x18, 0x00, 0x02, 0x00, 0x00};
uint8_t getGridExportToday[] = {slaveId, readSingleRegister, 0x02, 0x19, 0x00, 0x02, 0x00, 0x00};
uint8_t getGridImportToday[] = {slaveId, readSingleRegister, 0x02, 0x1A, 0x00, 0x02, 0x00, 0x00};
uint8_t getLoadPowerToday[] = {slaveId, readSingleRegister, 0x02, 0x1B, 0x00, 0x02, 0x00, 0x00};
uint8_t getInternalTemp[] = {slaveId, readSingleRegister, 0x02, 0x38, 0x00, 0x02, 0x00, 0x00};
uint8_t getHeatSinkTemp[] = {slaveId, readSingleRegister, 0x02, 0x39, 0x00, 0x02, 0x00, 0x00};
uint8_t getSolarPVCurrent[] = {slaveId, readSingleRegister, 0x02, 0x36, 0x00, 0x02, 0x00, 0x00};
uint8_t setStandby[] = {slaveId, passiveMode, 0x01, 0x00, 0x55, 0x55, 0x00, 0x00};
uint8_t setAuto[] = {slaveId, passiveMode, 0x01, 0x03, 0x00, 0x00, 0x00, 0x00};
uint8_t sendHeartbeat[] = {slaveId, 0x49, 0x22, 0x01, 0x22, 0x02, 0x00, 0x00};
uint8_t setDischarge[] = {slaveId, passiveMode, 0x01, 0x01, 0x00, 0x00, 0x00, 0x00};
uint8_t setCharge[] = {slaveId, passiveMode, 0x01, 0x02, 0x00, 0x00, 0x00, 0x00};


void InitSofarModbus (void)
{
    pinMode(SERIAL_COMMUNICATION_CONTROL_PIN, OUTPUT);
	digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN, RS485_RX);
	RS485Serial.begin(9600);
}

unsigned int calcCRC(uint8_t frame[], byte frameSize) 
{
  unsigned int temp, temp2, flag;
  temp = 0xFFFF;
  for (unsigned char i = 0; i < frameSize; i++)
  {
    temp = temp ^ frame[i];
    for (unsigned char j = 1; j <= 8; j++)
    {
      flag = temp & 0x0001;
      temp >>= 1;
      if (flag)
        temp ^= 0xA001;
    }
  }
  // Reverse byte order. 
  temp2 = temp >> 8;
  temp = (temp << 8) | temp2;
  temp &= 0xFFFF;
  return temp; // the returned value is already swopped - crcLo byte is first & crcHi byte is last
}


//calcCRC and checkCRC are based on...
//https://github.com/angeloc/simplemodbusng/blob/master/SimpleModbusMaster/SimpleModbusMaster.cpp
bool checkCRC(uint8_t frame[], byte frameSize) 
{
	unsigned int calculated_crc, recieved_crc;
	recieved_crc = ((frame[frameSize-2] << 8) | frame[frameSize-1]);
	calculated_crc = calcCRC(frame, frameSize-2);
	if (recieved_crc == calculated_crc)
	{
		return true;
	}
	else
	{
		return false;
	}
}

//Listen for a responce 
TModbusResponce listen()
{
	uint8_t inFrame[64];
	uint8_t inByteNum;
	uint8_t inFrameSize;
	uint8_t inFunctionCode;
	uint8_t inDataBytes;
	TModbusResponce ret;
	ret.dataSize = 0;

	for (unsigned char i = 0; i < 64; i++)
	{
		inFrame[i] = 0;
	}
	inFrameSize = 0;
	inFunctionCode = 0;
	inByteNum = 0;
	inDataBytes =0;
	delay(200);
	if (RS485Serial.available())
	{
		while (RS485Serial.available()) 
		{
			byte inChar = ((byte)RS485Serial.read());
			//Process the byte
			if (inByteNum == 0 && inChar == slaveId)  //If we're looking for the first byte but it dosn't match the slave ID, we're just going to drop it.
			{
				//This might be the start of a frame. Let's presume it is for now.
				inFrame[inByteNum] = inChar;
				inByteNum++;
			}
			else if (inByteNum == 1)
			{
				//This is the second byte in a frame, where the function code lives.
				inFunctionCode = inChar;
				inFrame[inByteNum] = inChar;
				inByteNum++;
			}
			else if (inByteNum == 2)
			{
				//This is the third byte in a frame, which tells us the number of data bytes to follow.
				inDataBytes = inChar;
				inFrame[inByteNum] = inChar;
				inByteNum ++;
			}
			else if (inByteNum > 2 && inByteNum < inDataBytes + 3) //There are three bytes before the data and two after (the CRC).
			{
				//This is presumed to be a data byte.
				inFrame[inByteNum] = inChar;
				ret.data[inByteNum-3] = inChar;
				ret.dataSize++;
				inByteNum++;
			}
			else if (inByteNum == inDataBytes + 3)
			{
				//This is the first CRC byte (maybe).
				inFrame[inByteNum] = inChar;
				inByteNum++;
			}
			else if (inByteNum == inDataBytes + 4)
			{
				//This is the second CRC byte (maybe).
				inFrame[inByteNum] = inChar;
				inByteNum++;
				break;
			}
		}
		inFrameSize = inByteNum;
	}
	RS485Serial.flush();
	// Now check to see if the last two bytes are a valid CRC.
	if (checkCRC(inFrame, inFrameSize))
	{
		ret.errorLevel = 0;
		ret.errorMessage = "Valid data frame";
	}
	else
	{
		ret.errorLevel = 1;
		ret.errorMessage = "Error: invalid data frame";
	}
	return ret;
}


TModbusResponce sendModbus(uint8_t frame[], byte frameSize)
{
TModbusResponce ret;
	//Calculate the CRC and overwrite the last two bytes.
	unsigned int crc = calcCRC(frame, frameSize-2);
	frame[frameSize-2] = crc >> 8;
	frame[frameSize-1] = crc & 0xFF;
	//Send
	digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN, RS485_TX);
	RS485Serial.write(frame, frameSize);
	// It's important to reset the SERIAL_COMMUNICATION_CONTROL_PIN as soon as 
	// we finish sending so that the serial port can start to buffer the responce.
	digitalWrite(SERIAL_COMMUNICATION_CONTROL_PIN, RS485_RX);
	ret = listen();
	return ret;
}


TSysStatus sys_status;


void sendData()
{
	// Update all parameters and send to MQTT.
	if(millis() >= time_3 + SEND_INTERVAL)
	{
		time_3 +=SEND_INTERVAL;
		String state = "{";
		TModbusResponce rs = sendModbus(getRunningState, sizeof(getRunningState));
		if (rs.errorLevel == 0)
		{
			unsigned int a = ((rs.data[0] << 8) | rs.data[1]);
			if (!( state == "{")) { state += ","; }
			state += "\"running_state\":"+String(a);
		}

		TModbusResponce gv = sendModbus(getGridVoltage, sizeof(getGridVoltage));
		if (gv.errorLevel == 0)
		{
			unsigned int b = ((gv.data[0] << 8) | gv.data[1]);
			if (!( state == "{")) { state += ","; }
			state += "\"grid_voltage\":"+String(b);
		}

		TModbusResponce gc = sendModbus(getGridCurrent, sizeof(getGridCurrent));
		if (gc.errorLevel == 0)
		{
			unsigned int c = ((gc.data[0] << 8) | gc.data[1]);
			if (!( state == "{")) { state += ","; }
			state += "\"grid_current\":"+String(c);
		}

		TModbusResponce gf = sendModbus(getGridFrequency, sizeof(getGridFrequency));
		if (gf.errorLevel == 0)
		{
			unsigned int d = ((gf.data[0] << 8) | gf.data[1]);
			if (!( state == "{")) { state += ","; }
			state += "\"grid_freq\":"+String(d);
		}
		
		TModbusResponce bp = sendModbus(getBatteryPower, sizeof(getBatteryPower));
		if (bp.errorLevel == 0)
		{
			unsigned int e = ((bp.data[0] << 8) | bp.data[1]);
			if (!( state == "{")) { state += ","; }
			state += "\"battery_power\":"+String(e);
			sys_status.battery_power = e;
		}
		
		TModbusResponce bv = sendModbus(getBatteryVoltage, sizeof(getBatteryVoltage));
		if (bv.errorLevel == 0)
		{
			unsigned int f = ((bv.data[0] << 8) | bv.data[1]);
			if (!( state == "{")) { state += ","; }
			state += "\"battery_voltage\":"+String(f);
		}
		
		TModbusResponce bc = sendModbus(getBatteryCurrent, sizeof(getBatteryCurrent));
		if (bc.errorLevel == 0)
		{
			unsigned int g = ((bc.data[0] << 8) | bc.data[1]);
			if (!( state == "{")) { state += ","; }
			state += "\"battery_current\":"+String(g);
		}
		
		TModbusResponce bs = sendModbus(getBatterySOC, sizeof(getBatterySOC));
		if (bs.errorLevel == 0)
		{
			unsigned int h = ((bs.data[0] << 8) | bs.data[1]);
			if (!( state == "{")) { state += ","; }
			state += "\"batterySOC\":"+String(h);
			sys_status.battery_charge = h;
		}

		TModbusResponce bt = sendModbus(getBatteryTemperature, sizeof(getBatteryTemperature));
		if (bt.errorLevel == 0)
		{
			unsigned int i = ((bt.data[0] << 8) | bt.data[1]);
			if (!( state == "{")) { state += ","; }
			state += "\"battery_temp\":"+String(i);
		}

		TModbusResponce cy = sendModbus(getBatteryCycles, sizeof(getBatteryCycles));
		if (cy.errorLevel == 0)
		{
			unsigned int j = ((cy.data[0] << 8) | cy.data[1]);
			if (!( state == "{")) { state += ","; }
			state += "\"battery_cycles\":"+String(j);
		}

		TModbusResponce gp = sendModbus(getGridPower, sizeof(getGridPower));
		if (gp.errorLevel == 0)
		{
			unsigned int k = ((gp.data[0] << 8) | gp.data[1]);
			if (!( state == "{")) { state += ","; }
			state += "\"grid_power\":"+String(k);
			sys_status.grid_power = k;
		}

		TModbusResponce lp = sendModbus(getLoadPower, sizeof(getLoadPower));
		if (lp.errorLevel == 0)
		{
			unsigned int l = ((lp.data[0] << 8) | lp.data[1]);
			if (!( state == "{")) { state += ","; }
			state += "\"consumption\":"+String(l);
		}

		TModbusResponce sp = sendModbus(getSolarPV, sizeof(getSolarPV));
		if (sp.errorLevel == 0)
		{
			unsigned int m = ((sp.data[0] << 8) | sp.data[1]);
			if (!( state == "{")) { state += ","; }
			state += "\"solarPV\":"+String(m);
			sys_status.solar_power = m;
		}

		TModbusResponce st = sendModbus(getSolarPVToday, sizeof(getSolarPVToday));
		if (st.errorLevel == 0)
		{
			unsigned int n = ((st.data[0] << 8) | st.data[1]);
			if (!( state == "{")) { state += ","; }
			state += "\"today_generation\":"+String(n);
		}

		TModbusResponce et = sendModbus(getGridExportToday, sizeof(getGridExportToday));
		if (et.errorLevel == 0)
		{
			unsigned int o = ((et.data[0] << 8) | et.data[1]);
			if (!( state == "{")) { state += ","; }
			state += "\"today_exported\":"+String(o);
		}

		TModbusResponce it = sendModbus(getGridImportToday, sizeof(getGridImportToday));
		if (it.errorLevel == 0)
		{
			unsigned int p = ((it.data[0] << 8) | it.data[1]);
			if (!( state == "{")) { state += ","; }
			state += "\"today_purchase\":"+String(p);
		}

		TModbusResponce pt = sendModbus(getLoadPowerToday, sizeof(getLoadPowerToday));
		if (pt.errorLevel == 0)
		{
			unsigned int q = ((pt.data[0] << 8) | pt.data[1]);
			if (!( state == "{")) { state += ","; }
			state += "\"today_consumption\":"+String(q);
		}

		TModbusResponce vt = sendModbus(getInternalTemp, sizeof(getInternalTemp));
		if (vt.errorLevel == 0)
		{
			unsigned int r = ((vt.data[0] << 8) | vt.data[1]);
			if (!( state == "{")) { state += ","; }
			state += "\"inverter_temp\":"+String(r);
		}

		TModbusResponce ht = sendModbus(getHeatSinkTemp, sizeof(getHeatSinkTemp));
		if (ht.errorLevel == 0)
		{
			unsigned int s = ((ht.data[0] << 8) | ht.data[1]);
			if (!( state == "{")) { state += ","; }
			state += "\"inverterHS_temp\":"+String(s);
		}

		TModbusResponce sc = sendModbus(getSolarPVCurrent, sizeof(getSolarPVCurrent));
		if (sc.errorLevel == 0)
		{
			unsigned int t = ((sc.data[0] << 8) | sc.data[1]);
			if (!( state == "{")) { state += ","; }
			state += "\"solarPVAmps\":"+String(t);
		}
		state = state+"}";
		
		//Prefixt the mqtt topic name with deviceName.
		String topic (deviceName);
		topic += "/state";
		sendMqtt (const_cast<char*>(topic.c_str()), state);
	}
}