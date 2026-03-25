/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#define MBR_TIME_OUT       4

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "stdbool.h"
#include "stdint.h"

#define BIT0   0x01
#define BIT1   0x02
#define BIT2   0x04
#define BIT3   0x08
#define BIT4   0x10
#define BIT5   0x20
#define BIT6   0x40
#define BIT7   0x80

/**
LOCAL DEVICE PORT   40100
LOCAL SERVER PORT   40001
MAIN  SERVER PORT   40000

LOCAL ROUTER IP 192.168.0.1
LOCAL SERVER IP 192.168.0.200
**/

//void assert_failed(uint8_t *file, uint32_t line);
void _Error_Handler(char * file, int line);
//#define Error_Handler() _Error_Handler(__FILE__, __LINE__)

/* Exported functions prototypes ---------------------------------------------*/
//void Error_Handler(void);
void SystemClock_Config(void);
void handleRxIrqLanA(void);
void onStopTXMB(void);
void showPack(uint8_t *data, uint32_t len);
void handleRxUSB(uint8_t *data, uint16_t len);
uint8_t CDC_Transmit_FS(uint8_t* Buf, uint16_t Len);
void showIP(uint32_t ip);
void setEnableRS485TX(bool flg);
void setEnableTimerTX(uint32_t val);
uint32_t computeTxTime(uint32_t bps, uint32_t packLen);
void tickTIMER(void);
void resetWDT(void);
void HandleRxUDP(uint8_t* data, uint16_t dlen);
void onCopyBuff(uint8_t* sBuff, uint8_t* tBuff, uint16_t Len);
void getDiagnosticReg8(bool stop, uint8_t adr, uint32_t len);
void getDiagnosticReg16(bool stop, uint8_t adr, uint32_t len);
void Diagnostic(void);
void SystemReset(void);
void makeRqDB(uint8_t *UID, uint8_t cardType, uint8_t readerNo);
void onStartTimer4(uint32_t time);
void onHandleARP(void);
void onHandleDHCP(void);
void onHandlePING(void);
void onHandleUDP(void);
void onMakeBlinkRedLED(void);
void StartBlinkRedLed(void);
void onMakeBlinkGreenLED(void);
void StartBlinkGreenLed(void);
void Usart2Reset(void);
bool onGetIRQ_DMA_SPI3(void);
void SystemResetD(char * file, int line);

#ifdef __cplusplus
}
#endif


#pragma pack(push,1)
struct sARP                // 42 BYTE
{
   uint8_t destMAC[6];     // Destination MAC
   uint8_t srcMAC[6];      // Source MAC
   uint8_t type[2];        // Packet type (ARP, IP...)
   uint8_t htype[2];       // Hardware type (Ethernet=0x01)
   uint8_t protocol[2];    // Protocol type (IP=0x8000)
   uint8_t hsize;          // Hardware (MAC) size (6)
   uint8_t psize;          // Protocol size (4)
   uint8_t opcode[2];      // Opcode request (0x0001)
   uint8_t sendMAC[6];     // Sender MAC
   uint32_t sendIP;        // Sender IP
   uint8_t targMAC[6];     // Target MAC
   uint32_t targIP;        // Target IP
   uint8_t reserved[22];
};
#pragma pack(pop)

#pragma pack(push,1)
struct sMBR                // 42 BYTE
{
   uint8_t devAddr;        /** device address      */
   uint8_t settUART;       /** UART settings       */
   uint8_t nonuse1;        /** free                */
   uint8_t makerCode;      /** card maker code     */
   uint32_t cardUID;       /** card UID            */
   uint8_t redLedStart;    /** red led state       */
   uint8_t redLedStTime;   /** red led state       */
   uint8_t redLedNext;     /** red led state       */
   uint8_t redLedNxTime;   /** red led state       */
   uint8_t redLedCicle;    /** red led state       */
   uint8_t greenLedStart;  /** green led state     */
   uint8_t greenLedStTime; /** green led state     */
   uint8_t greenLedNext;   /** green led state     */
   uint8_t greenLedNxTime; /** green led state     */
   uint8_t greenLedCicle;  /** green led state     */
   uint8_t zoomStart;      /** zoomer state        */
   uint8_t zoomStTime;     /** zoomer state        */
   uint8_t zoomNext;       /** zoomer state        */
   uint8_t zoomNxTime;     /** zoomer state        */
   uint8_t zoomCicle;      /** zoomer state        */
   uint8_t nonuse2;        /** free                */
   uint8_t timeRESP;       /** response time delay */
   uint8_t nonuse3;        /** free                */
};
#pragma pack(pop)


void EthernetSend(uint8_t *data, uint16_t len);
//void SetMbrType(uint8_t type);
//uint8_t GetMbrType(void);
//uint16_t GetMbrAddr(void);
bool CheckIpLeaseTime(void);
void SetIpLeaseTime(bool val);
uint32_t reverseDWORD(uint32_t d);
uint16_t reverseWORD(uint16_t d);
void SetFlagServerUBD(bool val);
bool GetFlagServerUBD(void);
bool checkInitFlag(void);
//void HandleLanSockets(uint8_t *data, uint16_t len);
bool CheckRxDeviceMAC(uint8_t *mac);
bool CheckRxBroadcastMAC(uint8_t *mac);
void HandleDhcpOffer(uint8_t *data, uint16_t len);
void ParseDhcpOption(uint8_t *addrLAN, uint8_t *data);
void HandleDhcpAck(uint8_t *data, uint16_t len);
void ArpReqServerUBD(void);
uint32_t GetDeviceIP(void);
//void StopTimeOutSARP(void);
void showMAC(char *mac);
void PingToRouter(void);
void HandleICMP(uint8_t *data, uint16_t len);
uint32_t GetServerIPA(void);
void sendUDPA(uint8_t *mac, uint32_t IP, uint16_t trgport, uint8_t *data, uint16_t len);
void ParseRxUSB(void);
void WriteEEPROM(uint8_t *data, uint16_t len);
//void SystemReset(void);
//void assert_failed(uint8_t *file, uint32_t line);
//void resetWDT(void);
//void tickTIMER(void);
//void StartTimeOutMBTX();
//extern uint8_t CDC_Transmit_FS(uint8_t* Buf, uint16_t Len);
void assert_failed(uint8_t *file, uint32_t line);
void setOnIRQ(bool flg);


#endif /* __MAIN_H */
