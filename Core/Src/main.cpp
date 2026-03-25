/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "i2c.h"
#include "iwdg.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "usb_device.h"
#include "gpio.h"
#include "modbus.h"
#include "UDP.h"
#include "stdio.h"
//#include "string.h"

//#define DEBUG_PRINT


#define MODBUS_REQ
#define ARP_SECTOR
#define DHCP_SECTOR
#define PING_SECTOR
#define UDP_SECTOR

//#define DEBUG_TRACE
//#define DEBUG_FLASH
#define WDOG_ON
//#define EEPROM_WRITE

#define DEVICE_NUMBER               1
#define SERVER_IP                   (0xC0A800C8)   /* 192.168.0.200     */
#define DNS_GOOGLE_IP               (0x08080808)   /* 8.8.8.8           */
#define ARP_TIME_OUT                500            /* x10 msec = 5 sec  */
#define ARP_TIME_PERIOD             6000           /* x10 msec = 1 min  */
#define PING_TIME_PERIOD            6000           /* x10 msec = 1 min  */
#define PING_TIME_OUT               100            /* x10 msec = 1 sec  */
//#define TRG_PORT_A                 40001
//#define SRC_PORT_A                  40100

static volatile uint8_t step_ARP = 0;
static volatile uint8_t step_DHCP = 0;
static volatile uint8_t step_PING = 0;
static volatile uint8_t step_UDP = 0;
static volatile uint8_t step_5 = 0;
static volatile uint8_t step_6 = 0;
static volatile uint32_t timeout_1 = 0;
static volatile uint32_t timeout_2 = 0;
static volatile uint32_t timeout_3 = 0;
static volatile uint32_t timeout_4 = 0;
static volatile uint32_t timeout_5 = 0;
static volatile uint32_t count_5 = 0;
static volatile uint32_t timeout_6 = 0;
static volatile uint32_t count_6 = 0;

static volatile uint32_t count_IRQ_LAN = 0;
//char tstr[100];
//#endif

//NetStat mNST;
//sRqDB mRqDB;
uint8_t buffDataReqUID[100];
///** UDP data pack for card UID - send to Data Base request */
//sDataUDP *pDataUDP = (sDataUDP *)buffData;

CByteBuff mBuffMB(255);
//CByteBuff mBuffPING(1600);
dPTR m_dPtr;
dPTR m_dPtrRx;
dPTR m_dPtrTx;

CByteBuff *pBuffMB = &mBuffMB;      // Pack buffer for send MBus req-t
//CByteBuff *pBuffPING = &mBuffPING;
CBuffLAN mBuffUSB(100, 3);
CBuffLAN *pBuffUSB = &mBuffUSB;     // Pack buffer
//CBuffLAN mBuffLAN(ENC28J60_MAXFRAME, 3);
CBuffLAN mBuffRxLAN(gm_max_sz_Eth, 3); // max size: 600 Byte
CBuffLAN *pBuffRxLAN = &mBuffRxLAN;

CBuffLAN mBuffTxLAN(gm_max_sz_Eth, 3); // max size: 600 Byte
CBuffLAN *pBuffTxLAN = &mBuffTxLAN;

CFlash mFlashM;
CFlash *pFlashM = &mFlashM;

//CTimeOut *pTimeOutMBTX = 0;

CTimeOut mTimeOutRELAY;
//CTimeOut mTimeOutLEDR;
CTimeOut mTimeOutLEDG;
CTimeOut mTimeOutMBR;
CTimeOut mTimeOutUID;
//CTimeOut mTimeOutPING;
//CTimeOut mTimeOutSARP;
//CTimeOut mTimeActiveRX;

CTimeOut *pTimeOutRELAY = &mTimeOutRELAY;
//CTimeOut *pTimeOutLEDR = &mTimeOutLEDR;
CTimeOut *pTimeOutLEDG = &mTimeOutLEDG;
CTimeOut *pTimeOutMBR = &mTimeOutMBR;
CTimeOut *pTimeOutUID = &mTimeOutUID;
//CTimeOut *pTimeOutPING = &mTimeOutPING;
//CTimeOut *pTimeOutSARP = &mTimeOutSARP;
//CTimeOut *pTimeActiveRX = &mTimeActiveRX;

//CBuffLAN *pBuffLanA = 0;
CEthernet mEthernet;
CEthernet *pEthernet = &mEthernet;

CPortM  mPortMB;
CPortM *pPortMB = &mPortMB;
CModbus mModbus;
CModbus *pModbus = &mModbus;

CArp mArp;
CArp *pArp = &mArp;
CUdp mUdp;
CUdp *pUdp = &mUdp;
sARP arp;
sAddrLAN DhcpOfferAddr;
sAddrLAN addrLAN;
sServerUDB servUDB;
//sDhcpDISCOVER mDhcpDsc;
//sDhcpREQ mDhcpReq;

static volatile bool flagBlinkRedLED = false;
static volatile bool flagBlinkGreenLED = false;

static volatile bool txFlgLAN1 = false;
static volatile bool txFlgLAN2 = false;
static volatile bool txFlgLAN3 = false;

static volatile bool debugFLAG1 = false;
static volatile bool debugFLAG2 = false;
static volatile bool debugFLAG3 = false;

static volatile bool debugFLAG11 = false;
static volatile bool debugFLAG12 = false;
static volatile bool debugFLAG13 = false;

static volatile bool RxFlgUSART = false;
static volatile bool RxFlgLAN = false;
static volatile bool makeFlgReqUID = false;
static volatile bool respFlgReqUID = false;

static volatile bool RxFlgLAN_DMA = false;

static volatile uint8_t gFlagARP_A = 0;
uint8_t EthBuffRXA[RX_BUFF_SZ];
uint16_t EthBffLenA = 0;

uint8_t deviceMAC[6];
uint32_t servIPA = 0;
uint16_t DeviceNumber = 0;
static uint32_t DhcpCntrID = 0;
volatile bool IpLeaseTimeON = false;
static volatile bool flagTmW1 = false;
static volatile bool initFlag = false;

static volatile bool UdpRequestFlg = false;
static volatile bool DhcpOfferFlg = false;
static volatile bool DhcpAckFlg = false;
static volatile bool PingRespFlg = false;

static volatile uint32_t timeResetCounter = 0;
static volatile uint32_t CountLanIRQ = 0;
//const uint32_t Time1 = 10;
static volatile uint32_t countTm1 = 0;
static volatile uint32_t countTm0 = 0;
static volatile uint32_t cntTime = 0;
static volatile uint32_t cntErrUSART = 0;
static volatile bool flagTm1 = false;
static volatile bool flagTime = false;
static volatile uint32_t cntTestTime = 0;
static volatile bool FlgCLOCK = false;
bool sysFlg = false;
bool sysFlgW = true;
void tickTIMER(void)
{
   if(initFlag == true)
   {
      //if(++timeResetCounter > 3600000) SystemReset(); // time 60 min for next reset
      FlgCLOCK = true;
      pTimeOutMBR->onTick();
      pTimeOutLEDG->onTick();
      pTimeOutRELAY->onTick();
   };
}
/*****************************************************************************/


void setOnIRQ(bool flg)
{
   if(flg) __enable_irq();
   else __disable_irq();
}

void setFlgDMA_LAN_RX(bool flg)
{
   RxFlgLAN_DMA = flg;
   printf("flg DMA LAN\n\r");
}
/*****************************************************************************/
void setEnableTimerTX(uint32_t val)
{
   onStartTimer3(val);
}
/*****************************************************************************/

/*****************************************************************************/
uint32_t computeTxTime(uint32_t bps, uint32_t packLen)
{
   if(bps == 0) bps = 1;
   uint32_t res = (12000000*packLen)/bps;
   return res;
}
/*****************************************************************************/

/*****************************************************************************/
uint32_t onGetSpeedRS485(void)
{
   return pFlashM->onGetSpeedRS485();
}
/*****************************************************************************/

/*****************************************************************************/
void onStopTXMB(void)
{
   //printf("---\n");
   setEnableRS485TX(false);
}
/*****************************************************************************/

/*****************************************************************************/
uint32_t reverseDWORD(uint32_t d)
{
   return ((d>>24 & 0x000000ff) | (d>>8 & 0x0000ff00) | (d<<8 & 0x00ff0000) | (d<<24 & 0xff000000));
}
/*****************************************************************************/

/*****************************************************************************/
uint16_t reverseWORD(uint16_t d)
{
   return ((d>>8 & 0x00ff) | (d<<8 & 0xff00));
}
/*****************************************************************************/

/*****************************************************************************/
uint32_t GetDhcpCntrID(void)
{
   return DhcpCntrID;
}
/*****************************************************************************/

/*****************************************************************************/
/**
UID:        0x0001;
PORT:       0x0000;
TIME RSP:   0x000A;
GLED:       0x0006;
RLED:       0x0004;
ZOOMER:     0x0009;
*/
#define   DEBUG_PRINT_MBR
sMBR m_MBR;
void ParseModbusRX(uint8_t *data, uint16_t len)    // USART - RS485 RX: Modbus
{
   uint8_t* pTxUID = 0;
   uint8_t cardType = 0;
   uint16_t tcrc16 = 0xFFFF;
   uint16_t rcrc16 = 0xFFFF;
   rcrc16 = 0x00FF & (uint16_t)data[len-1];
   rcrc16 |= 0xFF00 & ((uint16_t)data[len-2])<<8;
   tcrc16 = onCRC16(data, len-2);
   //showPack(data, len);
   //assert_failed((uint8_t *)__FILE__, __LINE__);
   if(tcrc16 != rcrc16) return;
   //assert_failed((uint8_t *)__FILE__, __LINE__);
   if(data[0] != 0)
   {
     //assert_failed((uint8_t *)__FILE__, __LINE__);
      if(data[1] == 0x03)
      {
        //assert_failed((uint8_t *)__FILE__, __LINE__);
         //byteCount = data[2];
         pTxUID = (uint8_t*)&(data[5]);
         cardType = data[4];
         /** send UDP request in local net
         for check card UID in Data Base */
         if(cardType == 0x04)
         {
            makeRqDB(pTxUID, cardType, data[0]);
#ifdef   DEBUG_PRINT_MBR
            printf("UID: %02X%02X%02X%02X\n\r", (int)data[5], (int)data[6], (int)data[7], (int)data[8]);
#endif
         }
         else if(cardType == 0x07)
         {
            makeRqDB(pTxUID, cardType, data[0]);
#ifdef   DEBUG_PRINT_MBR
            printf("UID: %02X%02X%02X%02X%02X%02X%02X\n\r", (int)data[5], (int)data[6],
                  (int)data[7], (int)data[8], (int)data[9], (int)data[10], (int)data[11]);
#endif
         };
      }
      else if(data[1] == 0x06)
      {
      };
   };
}
/*****************************************************************************/

/*****************************************************************************/
uint8_t *GetDeviceMAC(void)
{
   return deviceMAC;
}
/*****************************************************************************/

/*****************************************************************************/
void makeRqDB(uint8_t *UID, uint8_t cardType, uint8_t readerNo)
{  /** UDP data pack for card UID - send to Data Base request */
   uint8_t cnt = 0;
   sDataUDP *pDataUDP = (sDataUDP *)buffDataReqUID;
   pDataUDP->readerNumb = readerNo;
   if(cardType == 4)
   {
      pDataUDP->cardID[cnt++] = cardType;
      pDataUDP->cardID[cnt++] = UID[0];
      pDataUDP->cardID[cnt++] = UID[1];
      pDataUDP->cardID[cnt++] = UID[2];
      pDataUDP->cardID[cnt++] = UID[3];
      pDataUDP->cardID[cnt++] = 0;
      pDataUDP->cardID[cnt++] = 0;
      pDataUDP->cardID[cnt++] = 0;
   }
   else if(cardType == 7)
   {
      pDataUDP->cardID[cnt++] = cardType;
      pDataUDP->cardID[cnt++] = UID[0];
      pDataUDP->cardID[cnt++] = UID[1];
      pDataUDP->cardID[cnt++] = UID[2];
      pDataUDP->cardID[cnt++] = UID[3];
      pDataUDP->cardID[cnt++] = UID[4];
      pDataUDP->cardID[cnt++] = UID[5];
      pDataUDP->cardID[cnt++] = UID[6];
   };
   pDataUDP->cardSTAT = 0;          // 0x01 - enabled, 0x00 - disabled
   pDataUDP->mcookie[0] = 0xCC;     // 0xCC, 0xAA, 0x53, 0x11
   pDataUDP->mcookie[1] = 0xAA;
   pDataUDP->mcookie[2] = 0x53;
   pDataUDP->mcookie[3] = 0x11;
   pDataUDP->msgType = 0x07;        // 0x07 - REQ, 0x08 -RESP
   pDataUDP->reqID += 1;
   pDataUDP->textlen = 16;
   pDataUDP->text[0] = 'T';
   pDataUDP->text[1] = 'e';
   pDataUDP->text[2] = 's';
   pDataUDP->text[3] = 't';
   pDataUDP->text[4] = '-';
   pDataUDP->text[5] = 'T';
   pDataUDP->text[6] = 'e';
   pDataUDP->text[7] = 'x';
   pDataUDP->text[8] = 't';
   pDataUDP->text[9] = '.';
   pDataUDP->text[10] = 0;
   pDataUDP->text[11] = 0;
   makeFlgReqUID = true;
}
/*****************************************************************************/

/*****************************************************************************/
void EthernetSend(uint8_t *data, uint16_t len)
{
   pEthernet->onSend(data, len);
}
/*****************************************************************************/
/**   IRQ LAN   IRQ LAN   IRQ LAN   IRQ LAN   IRQ LAN   IRQ LAN   IRQ LAN   **/
/*****************************************************************************/
static bool nextFlgRX = false;
volatile static bool IrqFlgRX = false;
void handleRxIrqLanA(void)
{
   IrqFlgRX = true;
   //++count_IRQ_LAN;
   //setStep(1, 1);
//   /**
//   nextFlgRX = true;
//   for(uint8_t i=0; i<10; i++)
//   {
//      if(nextFlgRX)
//      {
//         if(pBuffLAN->onGetWriteBuff(m_dPtr))
//         {
//            pEthernet->m_pLanA->enc28j60_recv_packet(m_dPtr.data, m_dPtr.byteCount, nextFlgRX);
//         };
//      }
//      else break;
//   };
//   **/
}
/*****************************************************************************/

/*****************************************************************************/
void handleRxUSB(uint8_t *data, uint16_t len)
{
   pBuffUSB->onWrite(data, len);
}
/*****************************************************************************/

/*****************************************************************************/
uint8_t RxUSB[100];
uint8_t ReadFlash[100];
void ParseRxUSB(void)
{
   sAddrFLASH *pAFL = (sAddrFLASH *)RxUSB;
   uint16_t len = 0;
   pBuffUSB->onRead(RxUSB, len);
   uint16_t crc16 = pAFL->crc;
   pAFL->crc = 0;
   bool res = true;
   if(pAFL->codeWord != FLASH_CODE_WORD) res = false;
   if(crc16 != onCRC16(RxUSB, len)) res = false;
   if(res == false) printf("RxUSB crc16 - error\n");
   else
   {
      printf("RxUSB - OK\n");
      //showPack(RxUSB, len);
      if(pAFL->flagReq == 0x01)
      {
         if(pAFL->flagFunc == 0x01)
         {  /** write Address pack to the FLASH */
            uint8_t flen = (uint8_t)sizeof(struct sAddrFLASH);
            //pAFL->serverIP = reverseDWORD(pAFL->serverIP);
            //pAFL->routerIP = reverseDWORD(pAFL->routerIP);
            pAFL->crc = 0;
            crc16 = onCRC16(RxUSB, flen);
            pAFL->crc = crc16;
            ///////////////////////////////////////////////////////
            if(pFlashM != 0)
            {
               pFlashM->onWriteProtect(0);
               for(uint16_t i=0; i<len; i++)
               {
                  pFlashM->onWriteByte(i, RxUSB[i]);
               };
               HAL_Delay(100);
               pFlashM->onWriteProtect(1);
               pFlashM->onInit();
               HAL_Delay(100);
            };
            ///////////////////////////////////////////////////////
            //WriteEEPROM(RxUSB, len);
            //SystemReset();
            SystemResetD((char *)__FILE__, __LINE__);
         }
         else if(pAFL->flagFunc == 0x02)
         {  /** read Address pack from the FLASH */
            printf("read address from EEPROM\n");
            uint8_t flen = (uint8_t)sizeof(struct sAddrFLASH);
            sAddrFLASH *pAFLR = (sAddrFLASH *)ReadFlash;
            for(uint8_t i=0; i<flen; i++)
            {
               ReadFlash[i] = pFlashM->onReceiveByte(i);
            };
            //pAFLR->serverIP = reverseDWORD(pAFLR->serverIP);
            //pAFLR->routerIP = reverseDWORD(pAFLR->routerIP);
            pAFLR->flagReq = 0x02;
            pAFLR->flagFunc = 0x02;
            pAFLR->crc = 0;
            crc16 = onCRC16(ReadFlash, flen);
            pAFLR->crc = crc16;
            CDC_Transmit_FS(ReadFlash, flen);
         };
      };
   };
   //showPack(RxUSB, len);
}
/*****************************************************************************/

/*****************************************************************************/
void SystemResetD(char * file, int line)
{
   //initFlag = false;
   //pEthernet->m_pLanA->enc28j60_soft_reset();
   //pPortMB->onClearFlgRX();
   //pPortMB->onDeInit();
//   printf("RESET SYSTEM from: \n%s,\nline %d\r\n", file, line);
//   delay_us(10000);
//   NVIC_SystemReset();
}
/*****************************************************************************/

/*****************************************************************************/
void SystemReset(void)
{
   //initFlag = false;
   //pEthernet->m_pLanA->enc28j60_soft_reset();
   //pPortMB->onClearFlgRX();
   //pPortMB->onDeInit();
   NVIC_SystemReset();
}
/*****************************************************************************/

/*****************************************************************************/
void Usart2Reset(void)
{
   pPortMB->onClearFlgRX();
   pPortMB->onSetRX(15);
   //pPortMB->onDeInit();
   //HAL_Delay(1);
   //pPortMB->onInit();
   //HAL_Delay(1);
}
/*****************************************************************************/

/*****************************************************************************/
bool CheckRxDeviceMAC(uint8_t *mac)
{
   uint8_t *dMAC = GetDeviceMAC();
   if(mac[0] != dMAC[0]) return false;
   if(mac[1] != dMAC[1]) return false;
   if(mac[2] != dMAC[2]) return false;
   if(mac[3] != dMAC[3]) return false;
   if(mac[4] != dMAC[4]) return false;
   if(mac[5] != dMAC[5]) return false;
   return true;
}
/*****************************************************************************/

/*****************************************************************************/
bool CheckRxBroadcastMAC(uint8_t *mac)
{
   if(mac[0] != 0xFF) return false;
   if(mac[1] != 0xFF) return false;
   if(mac[2] != 0xFF) return false;
   if(mac[3] != 0xFF) return false;
   if(mac[4] != 0xFF) return false;
   if(mac[5] != 0xFF) return false;
   return true;
}
/*****************************************************************************/

/*****************************************************************************/
uint8_t *pPtrArpA = 0;
uint16_t lenArpA = 0;
void ArpReqServerUBD(void)
{
   printf("ARP->REQ\n");
   pPtrArpA = pArp->onMakeArpReq(reverseDWORD(servUDB.serverIP), lenArpA);
   EthernetSend(pPtrArpA, lenArpA);

#ifdef DEBUG_TRACE
assert_failed((uint8_t *)__FILE__, __LINE__);
printf("ARP->\n");
#endif
}
/*****************************************************************************/

/*****************************************************************************/
uint32_t GetDeviceIP(void)
{
   return addrLAN.deviceIP;
}
/*****************************************************************************/
/**     HANDLE LAN   HANDLE LAN   HANDLE LAN   HANDLE LAN   HANDLE LAN      **/
/*****************************************************************************/
void HandleLanSockets(uint8_t *data, uint16_t len)
{
/**  Handle RX from LAN: select ARP or DHCP or ICMP or UDP-data **/
   bool check = true;
   uint8_t *pMAC = (uint8_t *)data;
   showPack(data, len);
   /** Check and Handle ARP, DHCP, ICMP, UDP */
   if(0 == pArp->onCheckArpA(data, len))
   {                                                       /** RX pack - NOT ARP */
      if(CheckRxDeviceMAC(pMAC))
      {
         headIP *pHeadIP = (headIP *)&(data[14]);
         if(pHeadIP->protoc == 0x11)              /** message type - UDP or DHCP */
         {
            //assert_failed((uint8_t *)__FILE__, __LINE__);

            sDhcpDISCOVER *pDHCP = (sDhcpDISCOVER *)data;
            if(pDHCP->subUDP.destPort != 0x4400) check = false;
            if(pDHCP->subUDP.srcPort != 0x4300) check = false;
            if(pDHCP->DHCP.magCookie[0] != 0x63) check = false;
            if(pDHCP->DHCP.magCookie[1] != 0x82) check = false;
            if(pDHCP->DHCP.magCookie[2] != 0x53) check = false;
            if(pDHCP->DHCP.magCookie[3] != 0x63) check = false;
            if(check)
            {
               //assert_failed((uint8_t *)__FILE__, __LINE__);                                            /** message type - DHCP */
               if(pDHCP->DHCP.DHCP == 2)
               {
                  //assert_failed((uint8_t *)__FILE__, __LINE__);                                                /** DHCP - OFFER */
                  HandleDhcpOffer(data, len);
               }
               else if(pDHCP->DHCP.DHCP == 5)
               {
                  //assert_failed((uint8_t *)__FILE__, __LINE__);                                                  /** DHCP - ACK */
                  HandleDhcpAck(data, len);
               };
            }
            else                                          /** message type - UDP */
            {
               HandleRxUDP(data, len);
            };
         }
         else if(pHeadIP->protoc == 0x01)                /** message type - ICMP */
         {
            HandleICMP(data, len);
         }
         else if(pHeadIP->protoc == 0x06)                 /** message type - TCP */
         {
         };
      }
      else if(CheckRxBroadcastMAC(pMAC))
      {
      };
   };
}
/*****************************************************************************/

/*****************************************************************************/
void HandleRxUDP(uint8_t* data, uint16_t dlen)
{
   //printf("UDPD <-\n");
   bool res = true;
   uint32_t t = 0;
   UdpPack* pUDP = (UdpPack*)data;
   sDataUDP* pData = (sDataUDP*)&(pUDP->datagram);
   //if(pData->reqID != pDataUDP->reqID) res = false;
   //if(pData->reqID != 0x08) res = false;
   if(pData->mcookie[0] != 0xCC) res = false;
   if(pData->mcookie[1] != 0xAA) res = false;
   if(pData->mcookie[2] != 0x53) res = false;
   if(pData->mcookie[3] != 0x11) res = false;
   if(res)
   {
      respFlgReqUID = true;
      if(pData->cardSTAT & 0x01)
      {
//         printf("/************************************/\n");
//         printf("/** UID Mifare card is:  ENABLED!!! */\n");
//         printf("/************************************/\n");
         t = 0x0000007F & ((pData->cardSTAT)>>1);
         t *= 1000;
//         printf("Time Relay: %u\r\n", (unsigned int)t);
         pTimeOutRELAY->onStart(t, 7);
         setRelayON(true);
         pModbus->onWriteREG(pData->readerNumb, 0x0006, ((uint16_t)(pData->cardSTAT) & 0x00FF));
         StartBlinkGreenLed();

      }
      else
      {
//         printf("/************************************/\n");
//         printf("/** UID Mifare card is: DISABLED!!! */\n");
//         printf("/************************************/\n");
      };
   };
}
/*****************************************************************************/

/*****************************************************************************/
void HandleICMP(uint8_t *data, uint16_t len)
{
   uint16_t headCRC = 0;
   uint16_t dataCRC = 0;
   uint16_t headCRCN = 0;
   uint16_t dataCRCN = 0;
   packICMP *pICMP = (packICMP *)data;
   uint8_t *pD = (uint8_t *)(&data[14]);
   headCRC = pICMP->hIP.hdCrc;
   dataCRC = pICMP->crcDat;
   pICMP->hIP.hdCrc = 0;
   pICMP->crcDat = 0;
   headCRCN = GetIpCRC(pD, 20);
   pD = (uint8_t *)(&data[34]);
   dataCRCN = GetIpCRC(pD, len - 34);
   if((headCRC == headCRCN) && (dataCRC == dataCRCN))
   {
      if(pICMP->typeRq == 0x08)
      {
         uint8_t *TxICMP = GetTxBuffLAN();
         packICMP *pTxICMP = (packICMP *)TxICMP;
         uint8_t *pDMAC = GetDeviceMAC();
         debugFLAG1 = true;                  /// DEBUG FLAG - DEBUG FLAG - DEBUG FLAG
         for(int i=0; i<6; i++)
         {
            pTxICMP->dMAC.srcMAC[i] = pDMAC[i];
            pTxICMP->dMAC.targMAC[i] = pICMP->dMAC.srcMAC[i];
         };
         pTxICMP->dMAC.type[0] = 0x08;
         pTxICMP->dMAC.type[1] = 0x00;
         pTxICMP->hIP.headLen = 0x45;
         pTxICMP->hIP.diffSrv = 0x00;
         pTxICMP->hIP.totLen = pICMP->hIP.totLen;
         pTxICMP->hIP.dataID = pICMP->hIP.dataID;
         pTxICMP->hIP.flags = pICMP->hIP.flags;
         pTxICMP->hIP.ttl = pICMP->hIP.ttl;
         pTxICMP->hIP.protoc = 0x01;
         pTxICMP->hIP.hdCrc = 0x0000;
         pTxICMP->hIP.srcIP = reverseDWORD(GetDeviceIP());
         pTxICMP->hIP.trgIP = pICMP->hIP.srcIP;
         pTxICMP->typeRq = 0;
         pTxICMP->Code = 0;
         pTxICMP->crcDat = 0x0000;
         pTxICMP->IdBE = pICMP->IdBE;
         pTxICMP->IdLE = pICMP->IdLE;
         pTxICMP->SeqBE = pICMP->SeqBE;
         pTxICMP->SeqLE = pICMP->SeqLE;
         uint16_t Cnt = len - 42;
         for(uint16_t C=0; C<Cnt; C++)
         {
            pTxICMP->data[C] = pICMP->data[C];
         };
         pD = (uint8_t *)(&TxICMP[14]);
         headCRCN = GetIpCRC(pD, 20);
         pD = (uint8_t *)(&TxICMP[34]);
         dataCRCN = GetIpCRC(pD, len - 34);
         pTxICMP->hIP.hdCrc = headCRCN;
         pTxICMP->crcDat = dataCRCN;
         EthernetSend(TxICMP, len);
      }
      else if(pICMP->typeRq == 0x00)
      {
         PingRespFlg = true;
         debugFLAG2 = true;                  /// DEBUG FLAG - DEBUG FLAG - DEBUG FLAG
      };
   };

//   SWO_PrintString("Check CRC ICMP:\n");
//   sprintf(dsstr, "old headCRC: 0x%04X, new headCRC: 0x%04X\n", (int)headCRC, (int)headCRCN);
//   SWO_PrintString(dsstr);
//   sprintf(dsstr, "old dataCRC: 0x%04X, new dataCRC: 0x%04X\n", (int)dataCRC, (int)dataCRCN);
//   SWO_PrintString(dsstr);

}
/*****************************************************************************/

/*****************************************************************************/
void HandleDhcpOffer(uint8_t *data, uint16_t len)
{
   debugFLAG11 = true;
   bool res = true;
   uint16_t headCRC = 0;
   uint16_t dataCRC = 0;
   uint16_t headCRCN = 0;
   uint16_t dataCRCN = 0;
   sDhcpOFFER *pDHCP = (sDhcpOFFER *)data;
   uint32_t rxID = reverseDWORD(pDHCP->DHCP.transID);
   uint32_t valID = GetDhcpCntrID();
   if(rxID != valID) res = false;
   if(res == true)
   {
      uint8_t *pD = (uint8_t *)(&data[14]);
      headCRC = pDHCP->hIP.hdCrc;
      dataCRC = pDHCP->subUDP.dataCRC;
      pDHCP->hIP.hdCrc = 0;
      pDHCP->subUDP.dataCRC = 0;
      headCRCN = GetIpCRC(pD, 20);

      PseudoHeader *pPsd = (PseudoHeader *)(&data[len]);
      pPsd->clientIP = pDHCP->hIP.srcIP;
      pPsd->targIP = pDHCP->hIP.trgIP;
      pPsd->reserved = 0x00;
      pPsd->protocol = pDHCP->hIP.protoc;
      pPsd->length = pDHCP->subUDP.length;
      pDHCP->hIP.hdCrc = 0;
      pDHCP->subUDP.dataCRC = 0;
      pD = (uint8_t *)(&data[34]);
      dataCRCN = GetIpCRC(pD, len - 34 + 12);
      if(headCRC != headCRCN) res = false;
      if(dataCRC != dataCRCN) res = false;
   };
   if(res == true)
   {
      ParseDhcpOption((uint8_t *)&DhcpOfferAddr, data);
      if(DhcpOfferAddr.deviceIP != 0)
      {
         addrLAN.deviceIP = DhcpOfferAddr.deviceIP;
         DhcpOfferFlg = true;
      };
   };
}
/*****************************************************************************/

/*****************************************************************************/
void HandleDhcpAck(uint8_t *data, uint16_t len)
{
   debugFLAG12 = true;
   bool res = true;
   uint16_t headCRC = 0;
   uint16_t dataCRC = 0;
   uint16_t headCRCN = 0;
   uint16_t dataCRCN = 0;
   sDhcpACK *pDHCP = (sDhcpACK *)data;
   uint32_t rxID = reverseDWORD(pDHCP->DHCP.transID);
   uint32_t valID = GetDhcpCntrID();
   if(rxID != valID) res = false;
   if(res == true)
   {
      uint8_t *pD = (uint8_t *)(&data[14]);
      headCRC = pDHCP->hIP.hdCrc;
      dataCRC = pDHCP->subUDP.dataCRC;
      pDHCP->hIP.hdCrc = 0;
      pDHCP->subUDP.dataCRC = 0;
      headCRCN = GetIpCRC(pD, 20);
      PseudoHeader *pPsd = (PseudoHeader *)(&data[len]);
      pPsd->clientIP = pDHCP->hIP.srcIP;
      pPsd->targIP = pDHCP->hIP.trgIP;
      pPsd->reserved = 0x00;
      pPsd->protocol = pDHCP->hIP.protoc;
      pPsd->length = pDHCP->subUDP.length;
      pDHCP->hIP.hdCrc = 0;
      pDHCP->subUDP.dataCRC = 0;
      pD = (uint8_t *)(&data[34]);
      dataCRCN = GetIpCRC(pD, len - 34 + 12);
      if(headCRC != headCRCN) res = false;
      if(dataCRC != dataCRCN) res = false;
   };
   if(res == true)
   {
      DhcpAckFlg = true;
      addrLAN.deviceIP = DhcpOfferAddr.deviceIP;
      addrLAN.dhcpServIP = DhcpOfferAddr.dhcpServIP;
      addrLAN.dnsIP = DhcpOfferAddr.dnsIP;
      addrLAN.leaseTime = DhcpOfferAddr.leaseTime;
      addrLAN.routerIP = DhcpOfferAddr.routerIP;
      addrLAN.servMAC[0] = DhcpOfferAddr.servMAC[0];
      addrLAN.servMAC[1] = DhcpOfferAddr.servMAC[1];
      addrLAN.servMAC[2] = DhcpOfferAddr.servMAC[2];
      addrLAN.servMAC[3] = DhcpOfferAddr.servMAC[3];
      addrLAN.servMAC[4] = DhcpOfferAddr.servMAC[4];
      addrLAN.servMAC[5] = DhcpOfferAddr.servMAC[5];
      addrLAN.subNetMsk = DhcpOfferAddr.subNetMsk;
      SetIpLeaseTime(true);
      debugFLAG13 = true;
   };
}
/*****************************************************************************/

/*****************************************************************************/
void ParseDhcpOption(uint8_t *addrLAN, uint8_t *data)
{
   sDhcpOFFER *pDHCP = (sDhcpOFFER *)data;
   sAddrLAN *aLan = (sAddrLAN *)addrLAN;
   uint8_t *optPtr2 = (uint8_t *)&data[285];
   uint8_t *optPtrNext = optPtr2;
   int nptr = 0;
   uint8_t olen = 0;
   uint32_t *val = 0;
   for(int i=0; i<10; i++)
   {
      if(optPtrNext[nptr] == 54)          /// Server Identifier
      {
         aLan->deviceIP = reverseDWORD(pDHCP->DHCP.yourIP);
         aLan->servMAC[0] = pDHCP->dMAC.srcMAC[0];
         aLan->servMAC[1] = pDHCP->dMAC.srcMAC[1];
         aLan->servMAC[2] = pDHCP->dMAC.srcMAC[2];
         aLan->servMAC[3] = pDHCP->dMAC.srcMAC[3];
         aLan->servMAC[4] = pDHCP->dMAC.srcMAC[4];
         aLan->servMAC[5] = pDHCP->dMAC.srcMAC[5];
         olen = optPtrNext[nptr+1];
         if(olen == 4)
         {
            val = (uint32_t *)&(optPtrNext[nptr+2]);
            aLan->dhcpServIP = reverseDWORD(val[0]);
         };
         nptr += olen + 2;
      }
      else if(optPtrNext[nptr] == 50)     /// Requested IP Address
      {
         olen = optPtrNext[nptr+1];
         if(olen == 4)
         {
            val = (uint32_t *)&(optPtrNext[nptr+2]);
            aLan->deviceIP = reverseDWORD(val[0]);
         };
         nptr += olen + 2;
      }
      else if(optPtrNext[nptr] == 51)     /// IP Address Lease Time
      {
         olen = optPtrNext[nptr+1];
         if(olen == 4)
         {
            val = (uint32_t *)&(optPtrNext[nptr+2]);
            aLan->leaseTime = reverseDWORD(val[0]);
         };
         nptr += olen + 2;
      }
      else if(optPtrNext[nptr] == 1)      /// Subnet Mask
      {
         olen = optPtrNext[nptr+1];
         if(olen == 4)
         {
            val = (uint32_t *)&(optPtrNext[nptr+2]);
            aLan->subNetMsk = reverseDWORD(val[0]);
         };
         nptr += olen + 2;
      }
      else if(optPtrNext[nptr] == 3)      /// Router
      {
         olen = optPtrNext[nptr+1];
         if(olen == 4)
         {
            val = (uint32_t *)&(optPtrNext[nptr+2]);
            aLan->routerIP = reverseDWORD(val[0]);
         };
         nptr += olen + 2;
      }
      else if(optPtrNext[nptr] == 5)      /// Name Server
      {
         olen = optPtrNext[nptr+1];
         if(olen == 4)
         {
            val = (uint32_t *)&(optPtrNext[nptr+2]);
            aLan->nameServ = reverseDWORD(val[0]);
         };
         nptr += olen + 2;
      }
      else if(optPtrNext[nptr] == 6)      /// Domain name server
      {
         olen = optPtrNext[nptr+1];
         if(olen == 4)
         {
            val = (uint32_t *)&(optPtrNext[nptr+2]);
            aLan->dnsIP = reverseDWORD(val[0]);
         };
         nptr += olen + 2;
      }
      else if(optPtrNext[nptr] == 15)     /// Domain name
      {
         olen = optPtrNext[nptr+1];
         nptr += olen + 2;
      }
      else if(optPtrNext[nptr] == 0xFF)   /// End options
      {
         break;
      }
      else                                /// other
      {
         olen = optPtrNext[nptr+1];
         nptr += olen + 2;
      };
   };
}
/*****************************************************************************/

/*****************************************************************************/
static volatile uint16_t pingCntID = 1;
void PingToRouter(void)
{
   //printf("P>\r\n");
   const uint16_t totalLen = 60;
   uint8_t *TxPING = GetTxBuffLAN();
   uint16_t headCRC = 0;
   uint16_t dataCRC = 0;
   packICMP *pICMP = (packICMP *)TxPING;
   uint8_t *pD = (uint8_t *)(&TxPING[14]);
   uint8_t *pDMAC = GetDeviceMAC();

   for(int i=0; i<6; i++)
   {
      pICMP->dMAC.targMAC[i] = addrLAN.servMAC[i];
      //pICMP->dMAC.targMAC[i] = servUDB.serverMAC[i];
      pICMP->dMAC.srcMAC[i] = pDMAC[i];
   };
   pICMP->dMAC.type[0] = 0x08;
   pICMP->dMAC.type[1] = 0x00;
   pICMP->hIP.headLen = 0x45;
   pICMP->hIP.diffSrv = 0x00;
   pICMP->hIP.totLen = reverseWORD(totalLen);
   pICMP->hIP.dataID = reverseWORD(pingCntID);
   pICMP->hIP.flags = 0x0000;
   pICMP->hIP.ttl = 0x80;
   pICMP->hIP.protoc = 0x01;
   pICMP->hIP.hdCrc = 0x0000;

   pICMP->hIP.srcIP = reverseDWORD(GetDeviceIP());
   pICMP->hIP.trgIP = reverseDWORD(addrLAN.routerIP);
   //pICMP->hIP.trgIP = reverseDWORD(servIPA);
   pICMP->typeRq = 8;
   pICMP->Code = 0;
   pICMP->crcDat = 0x0000;
   pICMP->IdBE = 0x00;
   pICMP->IdLE = 0x01;
   pICMP->SeqBE = 0x00;
   pICMP->SeqLE = 0x10;

   uint16_t dataCnt = totalLen + 14 - 42;
   for(uint16_t C=0; C<dataCnt; C++)
   {
      pICMP->data[C] = (uint8_t)C;
   };
   pD = (uint8_t *)(&TxPING[14]);
   headCRC = GetIpCRC(pD, 20);
   pD = (uint8_t *)(&TxPING[34]);
   dataCRC = GetIpCRC(pD, totalLen - 20);
   pICMP->hIP.hdCrc = headCRC;
   pICMP->crcDat = dataCRC;
   EthernetSend(TxPING, totalLen+14);
   //printf("P>\n");

   if(++pingCntID >=60000) pingCntID = 1;
}
/*****************************************************************************/

/*****************************************************************************/
void sendUDPA(uint8_t *mac, uint32_t IP, uint16_t trgport, uint8_t *data, uint16_t len)
{
   /*
   TARG_PORT_A     40001 (LOCAL SERVER USER DATA BASE)
   SRC_PORT_A      40100 (CONTROLLER for NFC READERS)
   */
   uint16_t tlen = len;
   pUdp->onInit(GetDeviceMAC(), GetDeviceIP(), LOCAL_DEVICE_PORT);
   pUdp->setAddr(mac, IP, trgport);
   uint8_t *pPack = pUdp->makeUdp(data, tlen);
   pEthernet->m_pLanA->onSend(pPack, tlen);
   //pEthernet->onSend(pPack, tlen);
}
/*****************************************************************************/

/*****************************************************************************/
uint32_t GetServerIPA(void)
{
   return servIPA;
}
/*****************************************************************************/

/*****************************************************************************/
bool CheckIpLeaseTime(void)
{
   bool res = true;
   if(IpLeaseTimeON == false) res = false;
   return res;
}
/*****************************************************************************/

/*****************************************************************************/
void SetIpLeaseTime(bool val)
{
   if((val == true) && (IpLeaseTimeON == false))
   {
      IpLeaseTimeON = true;
      //setGreenLed(true);
   }
   else if((val == false) && (IpLeaseTimeON == true))
   {
      IpLeaseTimeON = false;
      //setGreenLed(false);
   };
}
/*****************************************************************************/

/*****************************************************************************/
void SetFlagServerUBD(bool val)
{
   if((val == true) && (servUDB.flag == 0))
   {
      servUDB.flag = 1;
      setRedLed(true);
   }
   else if((val == false) && (servUDB.flag == 1))
   {
      servUDB.flag = 0;
      setRedLed(false);
   };
}
/*****************************************************************************/

/*****************************************************************************/
bool GetFlagServerUBD(void)
{
   if(servUDB.flag == 0) return false;
   return true;
}
/*****************************************************************************/

/*****************************************************************************/
#ifdef EEPROM_WRITE
const char* firmName = "Apogey smart-card reader v1.0\n";
void WriteEEPROM(uint8_t *data, uint16_t len)
{
   if(pFlashM != 0)
   {
      //printf("Start write EEPROM\n");
      pFlashM->onWriteProtect(0);
      //sAddrFLASH *pFlsh = (sAddrFLASH *)data;
      //pFlsh->codeWord = FLASH_CODE_WORD;
      //pFlsh->devNumb = DEVICE_NUMBER;

      //pFlsh->serverIP = reverseDWORD(SERVER_IP);      // 192.168.1.200
      //printf(firmName);
//      for(uint16_t n=0; n<108; n++)
//      {
//         pFlsh->text[n] = firmName[n];
//         if(firmName[n] == 0) break;
//      };
      for(uint16_t i=0; i<len; i++)
      {
         pFlashM->onWriteByte(i, data[i]);
      };
      HAL_Delay(100);
      pFlashM->onWriteProtect(1);
      pFlashM->onInit();
      //printf("End of write EEPROM\n");
      HAL_Delay(100);
   };
}
#endif /* EEPROM_WRITE */
/*****************************************************************************/

/*****************************************************************************/
void initAddrLAN(void)     // Read from EEPROM settings for init device
{


   if(pFlashM->onCheckWritten())
   {
      //printf("init address from EEPROM\n");
      uint8_t *pMACA = pFlashM->onGetOwnMACA();
      for(uint8_t i=0; i<6; i++)
      {
         deviceMAC[i] = pMACA[i];
      };
      //showMAC((char *)deviceMAC);
      /** get server IP of the user DataBase */
      servIPA = pFlashM->onGetSrvIPA();
      servUDB.serverIP = reverseDWORD(servIPA);
      //showIP(servUDB.serverIP);
      servUDB.serverPort = LOCAL_SERVER_PORT;
      DeviceNumber = pFlashM->onGetDevNumb();
//      sprintf(tstr, "T=%d, sec\n", (int)pFlashM->onGetSpeedRS485());
//      printf(tstr);
   }
   else
   {
      //printf("init DEFAULT DEVICE!\n");
      for(uint8_t i=0; i<6; i++)
      {
         deviceMAC[i] = 0xFF;
      };
      servIPA = 0x01010101;
      servUDB.serverPort = LOCAL_SERVER_PORT;
      DeviceNumber = 0;
   };
}
/*****************************************************************************/

/*****************************************************************************/
bool checkInitFlag(void)
{
   return initFlag;
}
/*****************************************************************************/

/*****************************************************************************/
void onCopyBuff(uint8_t* sBuff, uint8_t* tBuff, uint16_t Len)
{
   for(uint16_t i=0; i<Len; i++)
   {
      tBuff[i] = sBuff[i];
   };
};
/*****************************************************************************/

/*****************************************************************************/
static volatile uint32_t cccc = 0;
//uint8_t tmpDatPING[1000];
//static uint16_t tmpDatLen = 0;
static volatile uint32_t arpTimeCnt = 0;
static volatile uint32_t pingTimeCnt = 0;
static volatile uint32_t sendUdpCntr = 0;
static volatile uint32_t countID = 0;
static volatile uint32_t countPing = 0;
static volatile uint32_t testCnt = 0;
static volatile uint32_t testMbrCnt = 0;
static volatile uint32_t countTm = 0;
static volatile uint32_t countTimeControl = 0;
static volatile bool swchFlg = false;
static volatile uint32_t countCicle = 0;
static volatile uint32_t cntReqUDP = 0;
static volatile uint32_t cntPingError = 0;
uint8_t rxPackBuff[256];
static uint16_t rxPackLen = 0;
static volatile uint32_t controlTic = 0;
static volatile bool controlTicFlg = false;
static volatile bool flagStateARP = false;
static volatile uint32_t timeCntWD = 0;
static volatile uint32_t arpCnt = 0;
static volatile uint32_t dhcpCnt = 0;

extern uint8_t *pBuffRX_MA;

void StartBlinkRedLed(void)
{
   step_5 = 1;
}

void StartBlinkGreenLed(void)
{
   step_6 = 1;
}

void onSetSelENC28J60(bool flg)
{
   if(flg) pEthernet->m_pLanA->onSetSelect();
   else pEthernet->m_pLanA->onClrSelect();
}

void SetFlgARP(bool flg)
{
   flagStateARP = flg;
}
static volatile bool qflg = false;
void onSetIRQ_DMA_SPI3(void)
{
   qflg = true;
}

bool onGetIRQ_DMA_SPI3(void)
{
   return qflg;
}
/*****************************************************************************/
void onRestartInitEthernet(void)
{
   pEthernet->onStartInit(GetDeviceMAC());
}
/*****************************************************************************/
void onSetRxFlgUSART(bool flg)   /** ------  call from USART RX IRQ  ------ **/
{
   RxFlgUSART = flg;
}
/*****************************************************************************/
//#define   DEBUG_PRINT_1
//#define   DEBUG_PRINT_0
//#define   DEBUG_PRINT
/*****************************************************************************/
/***** MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN ******/
/***** MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN ******/
void main(void)
{
	HAL_Init();
	SystemClock_Config();
   uint32_t tmpi = 0;
	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	//MX_DMA_Init();
	MX_I2C1_Init();
	//MX_SPI3_Init();
	//MX_TIM2_Init();
	MX_USB_DEVICE_Init();
   MX_TIM3_Init();
   MX_TIM4_Init();
   pFlashM->onInit();
   initAddrLAN();
   pPortMB->onInit();
   pEthernet->onStartInit(GetDeviceMAC());
   pArp->onInit(GetDeviceMAC());
   SetFlagServerUBD(false);
   step_ARP = 1; // start ARP loop
   countTimeControl = 0;
   pPortMB->onClearFlgRX();
   initFlag = true;

#ifdef WDOG_ON
   MX_IWDG_Init();
#endif

#ifdef   DEBUG_PRINT_0
   printf("Start main loop\r\n");
#endif

	while (1)
	{
      if(FlgCLOCK)
      {
         FlgCLOCK = false;
         sysFlg = true;
      };

      /** STATUS CONTROL                                     */
//      if(GPIO_PIN_RESET == HAL_GPIO_ReadPin(GPIOD,GPIO_PIN_2))
//      {
//         setStep(1, 1);
//      };
//      if(count_IRQ_LAN > 0)
//      {
//         --count_IRQ_LAN;
//         //setStep(1, 1);
//         /**
//         nextFlgRX = true;
//         for(uint8_t i=0; i<10; i++)
//         {
//            if(nextFlgRX)
//            {
//               if(pBuffLAN->onGetWriteBuff(m_dPtr))
//               {
//                  pEthernet->m_pLanA->enc28j60_recv_packet(m_dPtr.data, m_dPtr.byteCount, nextFlgRX);
//               };
//            }
//            else break;
//         };
//         **/
//      };

//      if(IrqFlgRX)
//      {
//         setOnIRQ(false);
//         IrqFlgRX = false;
//         setOnIRQ(true);
//         pEthernet->onRunTime();
//      };

//      if(RxFlgLAN_DMA)
//      {
//         RxFlgLAN_DMA = false;
//         printf("LAN DMA RX <-\n");
//         if(pBuffRxLAN->onCheck())
//         {
//            if(pBuffRxLAN->onGetReadBuff(m_dPtr)) HandleLanSockets(m_dPtr.data, *(m_dPtr.byteCount));
//         };
//      };

      if(RxFlgUSART)
      {
         printf("USART RX ->\n");
         /** RS485 - Modbus RX  */
         RxFlgUSART = false;
         rxPackLen = pPortMB->onGetRxLen();
         for(uint8_t n=0; n<rxPackLen; n++)
         {
            rxPackBuff[n] = pBuffRX_MA[n];
         };

         /** flag ARP               flag DHCP              flag PING **/
         if((txFlgLAN1 == true) && (txFlgLAN2 == true) && (txFlgLAN3 == true)) SetFlagServerUBD(true);
         else SetFlagServerUBD(false);
         ParseModbusRX(rxPackBuff, rxPackLen);
         cntErrUSART = 0;
      };

      mainRunTime();

//      if(onGetIRQ_DMA_SPI3())
//      {
//         qflg = false;
//         if(pBuffLAN->onGetReadBuff(m_dPtr)) HandleLanSockets(m_dPtr.data, *(m_dPtr.byteCount));
//      };

//      if(pBuffRxLAN->onCheck())
//      {
//         if(pBuffRxLAN->onGetReadBuff(m_dPtrRx)) HandleLanSockets(m_dPtrRx.data, *(m_dPtrRx.byteCount));
//      };

      /** Start UDP request to UBD server over Ethernet LAN **/
      if(GetFlagServerUBD())
      {
         if(makeFlgReqUID)
         {
            step_UDP = 1;
            timeout_4 = 0;
            makeFlgReqUID = false;
         };
      };

#ifdef   DEBUG_PRINT
      if(debugFLAG1)
      {
         debugFLAG1 = false;
         ///printf("ICMP OK! ->\n");
      };

      if(debugFLAG2)
      {
         debugFLAG2 = false;
         ///printf("ICMP OK! <-\n");
      };

      if(debugFLAG11)
      {
         debugFLAG11 = false;
         ///printf("DHCP <- OFR\n");
      };

      if(debugFLAG12)
      {
         debugFLAG12 = false;
         ///printf("DHCP <- ACK\n");
      };

      if(debugFLAG13)
      {
         printf("DHCP OK!\n");
         debugFLAG13 = false;
         ///printf("leaseTime: %d sec\r\n", addrLAN.leaseTime);
//         printf("DHCP Server MAC: ");
//         showMAC((char *)(addrLAN.servMAC));
//         printf("Your IP Address: ");
//         showIP(addrLAN.deviceIP);
      };
#endif
      /***********************************************************************/
      /******************* TIC 1 msec : TIC 1 msec : TIC 1 msec **************/
      if(sysFlg)
      {
         sysFlg = false;
         pEthernet->onTickRunTime();
         mainTickRunTime();

#ifdef   DEBUG_PRINT_1
         if(++controlTic > 1000)
         {
            controlTic = 0;
            printf("T=%d, sec\n", (int)countTimeControl);
            if(++countTimeControl > 3600) countTimeControl = 0;
         };
#endif

         /********************************************/
         /**    for drive pin enable GREEN LED OFF   */
         if(pTimeOutLEDG->onIsTimeOut())
         {
            pTimeOutLEDG->onStop();
            setGreenLed(false);
         };
         /********************************************/
         /**     for drive pin enable RELAY OFF      */
         if(pTimeOutRELAY->onIsTimeOut())
         {
            pTimeOutRELAY->onStop();
            setRelayON(false);
         };

         /********************************************************************/
         /*************** TIC 30 msec : TIC 30 msec : TIC 30 msec ************/
         //if(++countTm >= 50) // 50 msec
         if(++countTm >= 30) // 50 msec
         {
            countTm = 0;
            /** loop request slave RS485-MBR address: from 1 ... to 24, time = 24*30msec = 720msec */
            if(++tmpi <= 24)
            {
               /***** MB RQ: Read UID card status, reg. address: 0x0001, count: 5 ***/
#ifdef MODBUS_REQ
               pModbus->onReadREG(tmpi, 0x0001, 5);
               //pModbus->onReadREG(1, 0x0001, 5);
               if(tmpi == 1)
               {
                  //printf("Q\r\n");
                  ++cntErrUSART;
                  //if(cntErrUSART > 1) printf("Lost USART TX-RX: %d\r\n", cntErrUSART);
                  if(cntErrUSART > 2) Usart2Reset();
                  //if(cntErrUSART > 5) SystemReset();
                  if(cntErrUSART > 5) SystemResetD((char *)__FILE__, __LINE__);
               };
#endif

#ifdef WDOG_ON
               resetWDT();
#endif

#ifdef MODBUS_REQ_PRINTF
               if(tmpi == 1) printf("MBR RQ: ");
               printf("%d, ", (int)tmpi);
               if(tmpi == 24) printf("\r\n");
#endif
            };
            if(tmpi >= 24) tmpi = 0;
         };
         /*************** TIC 25 msec : TIC 25 msec : TIC 25 msec ************/
         /********************************************************************/

#ifdef ARP_SECTOR
         /***************** ARP SECTOR - ARP SECTOR - ARP SECTOR *************/
         onHandleARP();          /**  GET MAC addr of LOCAL SERVER USER DB  **/
         /********************************************************************/
#endif

#ifdef DHCP_SECTOR
         /*************** DHCP SECTOR - DHCP SECTOR - DHCP SECTOR ************/
         onHandleDHCP();                  /** GET LOCAL IP addr from ROUTER **/
         /********************************************************************/
#endif

#ifdef PING_SECTOR
         /*************** PING SECTOR - PING SECTOR - PING SECTOR ************/
         onHandlePING();        /** PING CONTROL from ROUTER - if is ONLINE **/
         /********************************************************************/
#endif

#ifdef UDP_SECTOR
         /**************** UDP SECTOR - UDP SECTOR - UDP SECTOR **************/
         onHandleUDP();
         /********************************************************************/
#endif

         onMakeBlinkRedLED();
         onMakeBlinkGreenLED();
      };
      /******************* TIC 1 msec : TIC 1 msec : TIC 1 msec **************/
      /***********************************************************************/

      if(true == pBuffUSB->onCheck()) ParseRxUSB();
	};
}
/***** MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN MAIN ******/
/*****************************************************************************/
/*****************************************************************************/

/*****************************************************************************/
void onHandleARP(void)
{
   //     cycle ARP request for get MAC of the User DataBase Server    //
   if(step_ARP == 1)
   {
      if(++timeout_1 > 1000)
      {
         ArpReqServerUBD();
         timeout_1 = 0;
         ++step_ARP;
         arpCnt = 0;
      };
   }
   else if(step_ARP == 2)
   {
      if(++timeout_1 > 1000)
      {
         flagStateARP = false;
         ArpReqServerUBD();
         timeout_1 = 0;
         ++step_ARP;
         //if(++arpCnt > 3) SystemReset();
         if(++arpCnt > 3) SystemResetD((char *)__FILE__, __LINE__);
      };
   }
   else if(step_ARP == 3)
   {
      if(flagStateARP)
      {
         arpCnt = 0;
         txFlgLAN1 = true;
         ++step_ARP;
         timeout_1 = 0;
         step_DHCP = 1;
      }
      else
      {
         if(++timeout_1 > 1000)
         {
            timeout_1 = 0;
            step_ARP = 2;
         };
      };
   }
   else if(step_ARP == 4)
   {
      if(++timeout_1 > 1800000)
      {
         timeout_1 = 0;
         step_ARP = 2;
         step_DHCP = 0;
         step_PING = 0;    /// PING
         step_UDP = 0;    /// UDP
      };
   };
}
/*****************************************************************************/

/*****************************************************************************/
void onHandleDHCP(void)
{
   if(step_DHCP == 1)      /** send Request DHCP to ROUTER for
                        automatic get local IP address */
   {
      if(++timeout_2 > 1000)
      {
         ++step_DHCP;
         timeout_2 = 0;
         dhcpCnt = 0;
         if(++DhcpCntrID >= 1000000) DhcpCntrID = 1;
      };
   }
   else if(step_DHCP == 2)
   {
      DhcpOfferFlg = false;
      SendDiscoverDHCP(GetDhcpCntrID());
      //if(++dhcpCnt > 5) SystemReset();
      if(++dhcpCnt > 5) SystemResetD((char *)__FILE__, __LINE__);
      ++step_DHCP;
   }
   else if(step_DHCP == 3)
   {
      if(DhcpOfferFlg)
      {
         ++step_DHCP;
         timeout_2 = 0;
         DhcpOfferFlg = false;
         dhcpCnt = 0;
      }
      else
      {
         if(++timeout_2 > 1000)
         {
            step_DHCP = 2;
            timeout_2 = 0;
         };
      };
   }
   else if(step_DHCP == 4)
   {
      DhcpAckFlg = false;
      SendRequestDHCP((uint8_t *)&DhcpOfferAddr, GetDhcpCntrID());
      //if(++dhcpCnt > 5) SystemReset();
      if(++dhcpCnt > 5) SystemResetD((char *)__FILE__, __LINE__);
      timeout_2 = 0;
      ++step_DHCP;
   }
   else if(step_DHCP == 5)
   {
      if(++timeout_2 > 1000)
      {
         step_DHCP = 4;
         timeout_2 = 0;
      };
      if(DhcpOfferFlg)
      {
         step_DHCP = 4;
         timeout_2 = 0;
         dhcpCnt = 0;
         DhcpOfferFlg = false;
      };
      if(DhcpAckFlg)
      {
         step_DHCP = 6;
         timeout_2 = 0;
         dhcpCnt = 0;
         DhcpAckFlg = false;
         txFlgLAN2 = true;
         pPortMB->onClearFlgRX();
         step_PING = 1;                /** init start PING loop **/
      };
   }
   else if(step_DHCP == 6)
   {
      //if(++timeout_2 > 1800000) step_DHCP = 2;      // 10 min
   };
}
/*****************************************************************************/
static volatile uint32_t xCnt = 0;
static volatile bool flgPNG = true;
/*****************************************************************************/
void onHandlePING(void)
{
   if(flgPNG == true)
   {
      switch(step_PING)
      {  /** ================================== **/
         case 1:  //-----------------------//
            if(++timeout_3 > 1000)  /// PING time out = 1 sec
            {
               ++step_PING;
               timeout_3 = 0;
            };
            break;
         case 2:  //-----------------------//
            PingToRouter();
            txFlgLAN3 = false;
            setGreenLed(false);
            timeout_3 = 0;
            ++step_PING;
            break;
         case 3:  //-----------------------//
            if(PingRespFlg)
            {
               PingRespFlg = false;
               timeout_3 = 0;
               ++step_PING;
               txFlgLAN3 = true;
               setGreenLed(true);
               cntPingError = 0;
            }
            else
            {
               if(++timeout_3 > 500)
               {
                  step_PING = 5;
                  timeout_3 = 0;
                  //if(++cntPingError > 3) SystemReset();
                  if(++cntPingError > 3) SystemResetD((char *)__FILE__, __LINE__);

               };
            };
            break;
         case 4:  //-----------------------//
            if(++timeout_3 > 1000)
            {
               step_PING = 2;
               timeout_3 = 0;
            };
            break;
         case 5:  //-----------------------//
            if(++timeout_3 > 100)
            {
               printf("lost %d PING!\r\n", cntPingError);
               step_PING = 2;
               timeout_3 = 0;
            };
            break;
         default:  //-----------------------//
            break;
      };
   };
}
/*****************************************************************************/

/*****************************************************************************/
void onHandleUDP(void)
{
static volatile uint8_t oldstep = 0;
static volatile uint32_t err_count = 0;

   oldstep = step_UDP;
   switch(step_UDP)    /**  **/
   {
      case 1:
         if(++timeout_4 > 0)
         {
            ++step_UDP;
            timeout_4 = 0;
            cntReqUDP = 0;
            step_PING = 0;
            timeout_3 = 0;
            flgPNG = false;
            err_count = 0;
         };
         break;
      case 2:
         if(GetFlagServerUBD())     /// if server UDB is online
         {
            sendUDPA(servUDB.serverMAC, servUDB.serverIP, servUDB.serverPort , buffDataReqUID, (uint16_t)sizeof(struct sDataUDP));
            StartBlinkRedLed();
            ++step_UDP;
            if(++cntReqUDP >= 3) step_UDP = 0;
         }
         else step_UDP = 0;
         break;
      case 3:
         if(respFlgReqUID)
         {
            ++step_UDP;
            timeout_4 = 0;
            respFlgReqUID = false;
            err_count = 0;
         }
         else
         {
            if(++timeout_4 > 1000)
            {
               step_UDP = 2;
               timeout_4 = 0;
               printf("!!! === timeout response UDP === !!!\r\n");
               if(++err_count > 3)  SystemResetD((char *)__FILE__, __LINE__);
            };
         };
         break;
      case 4:
         timeout_4 = 0;
         step_UDP = 0;
         break;
      default:
         break;
   };

   if((oldstep != 0) && (step_UDP == 0))
   {
      if(flgPNG == false)
      {
         flgPNG = true;
         step_PING = 1;
         timeout_3 = 0;
      };
   };
}
/*****************************************************************************/

/*****************************************************************************/
void onMakeBlinkRedLED(void)
{
   if(step_5 == 1)
   {
      if(++timeout_5 > 50)
      {
         ++step_5;
         timeout_5 = 0;
         if(flagBlinkRedLED) flagBlinkRedLED = false;
         else flagBlinkRedLED = true;
         setBlinkRedLed(flagBlinkRedLED);
      };
   }
   else if(step_5 == 2)
   {
      if(++timeout_5 > 50)
      {
         timeout_5 = 0;
         if(flagBlinkRedLED) flagBlinkRedLED = false;
         else flagBlinkRedLED = true;
         setBlinkRedLed(flagBlinkRedLED);
         if(++count_5 > 3) step_5 = 3;
         else step_5 = 1;
      };
   }
   else if(step_5 == 3)
   {
      if(checkRedL()) setBlinkRedLed(true);
      else setBlinkRedLed(false);
      count_5 = 0;
      timeout_5 = 0;
      step_5 = 0;
   };
}
/*****************************************************************************/

/*****************************************************************************/
void onMakeBlinkGreenLED(void)
{
   if(step_6 == 1)
   {
      if(++timeout_6 > 100)
      {
         ++step_6;
         timeout_6 = 0;
         if(flagBlinkGreenLED) flagBlinkGreenLED = false;
         else flagBlinkGreenLED = true;
         setBlinkGreenLed(flagBlinkGreenLED);
      };
   }
   else if(step_6 == 2)
   {
      if(++timeout_6 > 100)
      {
         timeout_6 = 0;
         if(flagBlinkGreenLED) flagBlinkGreenLED = false;
         else flagBlinkGreenLED = true;
         setBlinkGreenLed(flagBlinkGreenLED);
         if(++count_6 > 3) step_6 = 3;
         else step_6 = 1;
      };
   }
   else if(step_6 == 3)
   {
      if(checkGreenL()) setBlinkGreenLed(true);
      else setBlinkGreenLed(false);
      count_6 = 0;
      timeout_6 = 0;
      step_6 = 0;
   };
}
/*****************************************************************************/

/*****************************************************************************/
/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  //RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  //RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    _Error_Handler((char *)__FILE__, __LINE__);
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    _Error_Handler((char *)__FILE__, __LINE__);
  }

    /** Configure the Systick interrupt time */
  //HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq()/1000);

    /** Configure the Systick */
  //HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);

  /* SysTick_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(SysTick_IRQn, 1, 3);
}
/*****************************************************************************/

/*****************************************************************************/
/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */


void _Error_Handler(char * file, int line)
{
   printf((const char *)"ERROR - ");
   printf(file);
   printf("; line - %u\n", line);
   while(1)
   {
   };
}

void Diagnostic(void)
{
   printf("ESTAT: ");
   getDiagnosticReg8(false, ESTAT, 1);
   printf("EIE: ");
   getDiagnosticReg8(false, EIE, 1);
   printf("EIR: ");
   getDiagnosticReg8(false, EIR, 1);
   printf("ECON1: ");
   getDiagnosticReg8(false, ECON1, 1);
   printf("ECON2: ");
   getDiagnosticReg8(false, ECON2, 1);
}

void getDiagnosticReg8(bool stop, uint8_t adr, uint32_t len) // ESTAT_INT
{
   for(uint32_t i=0; i<len; i++)
   {
      pEthernet->m_pLanA->enc28j60_prnt8(adr+i);
   };

   if(stop)
   {
      while(1)
      {
      };
   };
}

void getDiagnosticReg16(bool stop, uint8_t adr, uint32_t len)
{
   for(uint32_t i=0; i<len; i++)
   {
      pEthernet->m_pLanA->enc28j60_prnt16(adr+i);
   };

   if(stop)
   {
      while(1)
      {
      };
   };
}


void showPack(uint8_t *data, uint32_t len)
{
   printf("{\n");
   for(uint32_t i=0; i<len; i++)
   {
      printf("%d: 0x%02X\n", (int)i, (int)data[i]);
      //printf("0x%02X, ", (int)data[i]);
      //printf(tstr);
   };
   printf("}\n\r");
}

void showMAC(char *mac)
{
   printf("MAC: %02X:%02X:%02X:%02X:%02X:%02X\n", (int)mac[0], (int)mac[1], (int)mac[2], (int)mac[3], (int)mac[4], (int)mac[5]);
}

void showIP(uint32_t ip)
{
   uint8_t d1, d2, d3, d4;
   d1 = (uint8_t)(ip>>24);
   d2 = (uint8_t)(ip>>16);
   d3 = (uint8_t)(ip>>8);
   d4 = (uint8_t)(ip>>0);
   printf("IP: %d.%d.%d.%d\n", (int)d1, (int)d2, (int)d3, (int)d4);
}

void onShowREG32(uint32_t reg)
{
   //printf("a:0x%02X v:0x%04X, bin: ", adr, res);
   uint32_t d = 0;
   for(uint32_t i=0; i<32; i++)
   {
      if((i == 4) || (i == 8) || (i == 12) || (i == 16) || (i == 20) || (i == 24) || (i == 28)) printf(" ");
      d = 0x00000001 & (reg>>(31-i));
      printf("%d", d);
   };
   printf("\r\n");
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
   printf("%s, line %d\r\n", file, line);
}
#endif /* USE_FULL_ASSERT */


#ifdef  USE_COPY_CODE
// EOF EOF EOF EOF EOF EOF EOF EOF EOF EOF EOF EOF EOF EOF EOF EOF EOF EOF EOF EOF EOF EOF

      /*********************************************************************/
      /** RX PIN USART2 STATUS CONTROL                                     */
//      if(GPIO_PIN_RESET == HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_6))
//      {
//         if(GPIO_PIN_RESET == HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_3)) // if USART PIN:PA3 RX DATA
//         {
//            //pTimeActiveRX->onStart(10, 9);
//         };
//      };

//      if(pTimeActiveRX->onIsTimeOut())
//      {
//         printf("USART RX ERROR!!!\r\n");
//         pTimeActiveRX->onStop();
//         //Usart2Reset();
//
////         pPortMB->onClearFlgRX();
////         pPortMB->onDeInit();
////         pPortMB->onInit();
//         //SystemReset();
//      };

      /** RX PIN USART2 STATUS CONTROL                                     */
      /*********************************************************************/

//#ifdef DEBUG_TRACE
//assert_failed((uint8_t *)__FILE__, __LINE__);
//#endif

//      printf("DHCP Server MAC: ");
//      showMAC((char *)(addrLAN.servMAC));
//      printf("Your IP Address: ");
//      showIP(addrLAN.deviceIP);
//      SWO_PrintString("Server Identifier: ");
//      showIP(addrLAN.dhcpServIP);
//      sprintf(dsstr, "Lease Time: %d\n", (int)(addrLAN.leaseTime));
//      SWO_PrintString(dsstr);
//      SWO_PrintString("Subnet Mask: ");
//      showIP(addrLAN.subNetMsk);
//      SWO_PrintString("Router: ");
//      showIP(addrLAN.routerIP);
//      SWO_PrintString("Name Server: ");
//      showIP(addrLAN.nameServ);
//      SWO_PrintString("Domain name server: ");
//      showIP(addrLAN.dnsIP);

//   __disable_irq();
//   __enable_irq();
//   enableExtiIRQ_A(true);

//   delete pTimeOutMBR;
   //delete pTimeOutMBTX;
//   delete pTimeOutUID;
//   delete pTimeOutPING;
//   delete pTimeOutSARP;
//   delete pBuffUSB;
//   delete pBuffMB;
//   delete pBuffPING;

//   pBuffMB = new CByteBuff(255);
//   pBuffPING = new CByteBuff(1600);
//   pTimeOutMBR = new CTimeOut();
//   pTimeOutRELAY = new CTimeOut();
   //pTimeOutMBTX = new CTimeOut();
//   pTimeOutLEDR = new CTimeOut();
//   pTimeOutLEDG = new CTimeOut();
//   pTimeOutUID = new CTimeOut();
//   pTimeOutPING = new CTimeOut();
//   pTimeOutSARP = new CTimeOut();
//   pBuffLanA = new CBuffLAN(RX_BUFF_SZ, 2);
//   pBuffUSB = new CBuffLAN(100, 3);


//		/** if response MBR pack from slave             */
//      if(true == pTimeOutMBR->onIsActive(countID))
//      {
//         if(pBuffMB->onCheck())
//         {
//            pBuffMB->onCopyRX(rxPackBuff, rxPackLen);
//            //printf("RESPONSE-MBR:\n");
//            rcrc16 = 0x00FF & (uint16_t)rxPackBuff[rxPackLen-1];
//            rcrc16 |= 0xFF00 & ((uint16_t)rxPackBuff[rxPackLen-2])<<8;
//            tcrc16 = onCRC16(rxPackBuff, rxPackLen-2);
//#ifdef   DEBUG_PRINT
//            showPack(rxPackBuff, rxPackLen);
//            sprintf(dstr, "crc16-A: 0x%04X; crc16-B: 0x%04X\n", (int)rcrc16, (int)tcrc16);
//            printf((char *)dstr);
//#endif
//            if(tcrc16 == rcrc16) ParseModbusRX(rxPackBuff, rxPackLen); // -> line 189
//            pTimeOutMBR->onStop();
//         };
//      };


               /**
//               if(pTimeOutMBR->onIsTimerOFF())
//               {
//                  ++countID;
//                  SetMbrType(MBR_TYPE_UID);
//                  pModbus->onReadREG(tmpi, GetMbrAddr(), 5);
//                  pTimeOutMBR->onStart(MBR_TIME_OUT, countID); // t = 4 msec
//                  //printf("REQ-MBR\n");
//               };
               */

//      /** period T=10 msec                            */
//      if(flagTmW1 != flagTm1)
//      {
//         if(flagTmW1) flagTmW1 = false;
//         else flagTmW1 = true;

//         /** send Request DHCP to ROUTER for
//             automatic get local IP address */
//         if(false == CheckIpLeaseTime())
//         {  /** don't have local IP address */
//            if(testCnt == 0)
//            {  /** cycle DHCP request
//               period time = 10 sec */
//               if(++DhcpCntrID >= 1000000) DhcpCntrID = 0;
//               SendDiscoverDHCP(GetDhcpCntrID());
//            };
//            if(++testCnt >= 1000) testCnt = 0;
//         };

//         if(true == CheckIpLeaseTime())
//         {
//            /** cycle ARP request period = 3 min (180 sec)
//            for get MAC of the User DataBase Server */
//            if(arpTimeCnt == 2)
//            {
//               ArpReqServerUBD();
//               pTimeOutSARP->onStart(ARP_TIME_OUT, 200);
//            };
//            if(++arpTimeCnt >= ARP_TIME_PERIOD) arpTimeCnt = 0;

//            /** cycle PING request period = 3 min (180 sec)
//            for check connection to the Router */
//            if(pingTimeCnt == 1)
//            {
//               ++countPing;
//               PingToRouter();
//               /** PING time out = 1 sec */
//               pTimeOutPING->onStart(PING_TIME_OUT, countPing);
//            };
//            if(++pingTimeCnt >= PING_TIME_PERIOD) pingTimeCnt = 0;
//         };

//         if(true == pTimeOutSARP->onIsTimeOut())
//         {  /** time out of ARP request
//            to the Server UBD */
//            printf("Timeout request ARP to the Server UBD\n");
//            pTimeOutSARP->onStop();
//            SetFlagServerUBD(false);
//         };

//         /** test: send UDP pack */
//         if(sendUdpCntr == 2)
//         {
////            if(servUDB.flag == 1) sendUDPA(servUDB.serverMAC, GetServerIPA(), TARG_PORT_A, tstData, (uint16_t)sizeof(struct sDataUDP));
//         };
//         if(++sendUdpCntr >= 100)
//         {
//            sendUdpCntr = 0;
//         };

//         uint16_t *ptr = 0;

//      };

#endif

