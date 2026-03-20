#ifndef MODBUS_H_INCLUDED
#define MODBUS_H_INCLUDED

#include "stdint.h"

uint16_t CRC16(const uint8_t *nData, uint16_t wLength);

class CModbus
{
public:
   CModbus();
   ~CModbus();

   void onFunc0(void);
   void onReadREG(uint8_t devAddr, uint16_t regPos, uint16_t regCount);
   void onWriteREG(uint8_t devAddr, uint16_t regPos, uint16_t regValue);
   void onWriteREGW(uint8_t devAddr, uint16_t regPos, uint16_t regCount, uint16_t *regValue);


protected:
private:
   uint8_t *m_buffTX;
   const uint16_t m_txBuffSz;
};

#endif /* MODBUS_H_INCLUDED */
