
#include "UDP.h"
#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#define TRANSACT_ID     0x00003D1D

#define SERVER_IP       (0xC0A800C8)   /* 192.168.0.200     */

extern "C"
{
   void DebugHandler(char * file, int line);
   uint16_t GetDeviceNumb(void);
   void showPack(uint8_t *data, uint32_t len);
}

//extern void assert_failed(uint8_t *file, uint32_t line);
//extern void SetFlagServerUBD(bool val);
extern void showMAC(char *mac);
extern sServerUDB servUDB;
extern void SetFlagServerUBD(bool val);
//extern void StopTimeOutSARP(void);
extern void showIP(uint32_t ip);
extern "C" void SetEvent(uint8_t evpos, uint16_t dataSz, uint8_t *dataPtr);
extern void incUdpPackCntr(void);
//extern void showPack(uint8_t *data, uint32_t len);
extern uint32_t reverseDWORD(uint32_t d);
extern uint16_t reverseWORD(uint16_t d);
extern void EthernetSend(uint8_t *data, uint16_t len);
extern uint32_t GetServerIPA(void);
extern uint32_t GetServerIPB(void);
extern void SetServerMAC(char lan, uint8_t *mac);
extern void setMyOwnPort(uint16_t port);
extern uint16_t getMyOwnPort(void);
extern void setTerminalPort(uint16_t port);
extern uint16_t getTerminalPort(void);
extern void SetArpFlag(char lan);
extern "C" void GetServerMAC(char lan, uint8_t *mac);
//extern void HandleUdpRxPack(uint8_t *udpD, uint16_t dlen);
//extern void DebugHandler(char * file, int line);
extern uint32_t GetDeviceIP(void);
//extern uint32_t GetMyOwnIPB(void);
extern void restartLanA(void);
extern void appendUdpTX(uint8_t *pack, uint16_t len);
//extern CEthernet *pEthernet;
extern uint16_t onCRC16(const uint8_t *nData, uint16_t wLength);
//extern "C" void sendUDPA(uint8_t *mac, uint32_t IP, uint16_t trgport, uint16_t srcport, uint8_t *data, uint16_t len);


#define TX_BUFFER_SIZE_LAN       1000
uint8_t TxBufferLAN[TX_BUFFER_SIZE_LAN];

uint8_t* GetTxBuffLAN(void)
{
   return (uint8_t*)TxBufferLAN;
}

uint16_t GetTxBuffSzLAN(void)
{
   return (uint16_t)TX_BUFFER_SIZE_LAN;
}

uint16_t GetIpCRC(uint8_t *b, uint16_t len)
{
//   uint32_t sum = 0;
//   uint16_t word16 = 0x00FF;
//   while (len > 1)
//   {
//      sum += * b++;
//      len -= 2;
//   };
//   //if any bytes left, pad the bytes and add
//   if(len > 0)
//   {
//      sum += ((*b)&word16);
//      //sum += ((*b)&htons(0xFF00));
//   };
//   //Fold sum to 16 bits: add carrier to result
//   while (sum>>16)
//   {
//      sum = (sum & 0xffff) + (sum >> 16);
//   };
//   //one's complement
//   sum = ~sum;
//   return ((uint16_t)sum);


   uint32_t sum = 0;
   int32_t i;
   uint16_t word16;
   for(i=0; i<len-1; i+=2)
   {
      word16 = *(uint16_t*)(&b[i]);
      sum += word16;
   };

   if(len & 1)
   {
      word16 = (uint16_t)(b[i]);
      sum += word16;
   };
   while(sum >> 16) sum = (sum & 0xFFFF) + (sum >> 16);
   return (uint16_t)(~sum);



//   uint32_t sum = 0;
//   for(int32_t i=0; i<len-1; i+=2)
//   {
//      sum += *(uint16_t*)(&b[i]);
//      while(sum >> 16) sum = (sum & 0xFFFF) + (sum >> 16);
//   };
//   if(len & 1)
//   {
//      sum += ((uint16_t)(b[len-1]) & 0x0000FFFF);
//      while(sum >> 16) sum = (sum & 0xFFFF) + (sum >> 16);
//   };
//   return (uint16_t)(~sum);

}
/*****************************************************************************/

/*****************************************************************************/
CBuffLAN::CBuffLAN(uint16_t packLen, uint16_t packCnt) :
   m_Buff(0),
   m_Len(0),
   m_lenPack(packLen),
   m_cntPack(packCnt),
   m_cntr(0),
   m_wrtr(0),
   m_read(0)
{
   m_Buff = new uint8_t* [m_cntPack];
   for(uint16_t i=0; i<m_cntPack; i++)
   {
      m_Buff[i] = new uint8_t[m_lenPack];
   };
   m_Len = new uint16_t [m_cntPack];
}

CBuffLAN::~CBuffLAN()
{
   for(uint8_t i=0; i<m_cntPack; i++)
   {
      delete m_Buff[i];
   };
   delete [] m_Buff;
   delete m_Len;
}

bool CBuffLAN::onCheck(void)
{
   if(m_cntr > 0) return true;
   return false;
}

void CBuffLAN::onWrite(uint8_t *data, uint16_t len)
{
   if(m_cntr < m_cntPack)
   {
      if(len > m_lenPack) len = m_lenPack;
      for(uint16_t i=0; i<len; i++)
      {
         m_Buff[m_wrtr][i] = data[i];
      };
      m_Len[m_wrtr] = len;
      if(++m_wrtr >= m_cntPack) m_wrtr = 0;
      ++m_cntr;
   };
}

bool CBuffLAN::onGetWriteBuff(dPTR &ptr)
{
   if(m_cntr < m_cntPack)
   {
      ptr.data = &(m_Buff[m_wrtr][0]);
      ptr.byteCount = &(m_Len[m_wrtr]);
      if(++m_wrtr >= m_cntPack) m_wrtr = 0;
      ++m_cntr;
      return true;
   };
   return 0;
}

bool CBuffLAN::onGetReadBuff(dPTR &ptr)
{
   if(m_cntr > 0)
   {
      ptr.data = &(m_Buff[m_read][0]);
      ptr.byteCount = &(m_Len[m_read]);
      if(++m_read >= m_cntPack) m_read = 0;
      --m_cntr;
      return true;
   };
   return false;
}

bool CBuffLAN::onRead(uint8_t *data, uint16_t &len)
{
   bool res = false;
   if(m_cntr > 0)
   {
      len = m_Len[m_read];
      for(uint16_t i=0; i<len; i++)
      {
          data[i] = m_Buff[m_read][i];
      };
      if(++m_read >= m_cntPack) m_read = 0;
      --m_cntr;
      if(len > 0) res = true;
   }
   else
   {
      len = 0;
   };
   return res;
}

/****************************** CArp **********************************/
CArp::CArp() :
   m_pArpREQ(0),
   m_pArpRSP(0)
{
   m_pArpREQ = new ArpPack;
   m_pArpRSP = new ArpPack;
}

CArp::~CArp()
{
}

void CArp::onInit(uint8_t *devMac)
{
   for(uint8_t i=0; i<6; i++)
   {
      m_pArpREQ->dMAC.srcMAC[i] = devMac[i];
      m_pArpREQ->dMAC.targMAC[i] = 0xFF;
      m_pArpREQ->sendMAC[i] = devMac[i];
      m_pArpREQ->targMAC[i] = 0;
   };
   m_pArpREQ->dMAC.type[0] = 0x08;
   m_pArpREQ->dMAC.type[1] = 0x06;
   m_pArpREQ->htype[0] = 0x00;
   m_pArpREQ->htype[1] = 0x01;
   m_pArpREQ->protocol[0] = 0x08;
   m_pArpREQ->protocol[1] = 0x00;
   m_pArpREQ->hsize = 0x06;
   m_pArpREQ->psize = 0x04;
   m_pArpREQ->opcode[0] = 0x00;
   m_pArpREQ->opcode[1] = 0x01;

   for(uint8_t i=0; i<6; i++)
   {
      m_pArpRSP->dMAC.srcMAC[i] = devMac[i];
      m_pArpRSP->dMAC.targMAC[i] = 0xFF;
      m_pArpRSP->sendMAC[i] = devMac[i];
      m_pArpRSP->targMAC[i] = 0xFF;
   };
   m_pArpRSP->dMAC.type[0] = 0x08;
   m_pArpRSP->dMAC.type[1] = 0x06;
   m_pArpRSP->htype[0] = 0x00;
   m_pArpRSP->htype[1] = 0x01;
   m_pArpRSP->protocol[0] = 0x08;
   m_pArpRSP->protocol[1] = 0x00;
   m_pArpRSP->hsize = 0x06;
   m_pArpRSP->psize = 0x04;
   m_pArpRSP->opcode[0] = 0x00;
   m_pArpRSP->opcode[1] = 0x02;

   for(uint8_t i=0; i<18; i++)
   {
      m_pArpRSP->padding[i] = 0;
      m_pArpREQ->padding[i] = 0;
   };
}

//DebugHandler((char *)(__FILE__), (int)__LINE__);

char CArp::onCheckArpA(uint8_t *data, uint16_t len)                     // MAIN LAN A
{
   char res = 0;
   ArpPack *pPack = (ArpPack *)&(data[0]);

   if((pPack->dMAC.type[0] == 0x08) && (pPack->dMAC.type[1] == 0x06))   /** if ARP PACK */
   {
      if(pPack->opcode[1] == 0x01)           /// if opcode request
      {
         if(reverseDWORD(pPack->targIP) == GetDeviceIP())
         {
            for(uint16_t i=0; i<6; i++)
            {
               m_pArpRSP->dMAC.targMAC[i] = pPack->sendMAC[i];
               m_pArpRSP->targMAC[i] = pPack->sendMAC[i];
            };
            m_pArpRSP->opcode[1] = 0x02;     /// Set response 2
            m_pArpRSP->targIP = pPack->sendIP;
            m_pArpRSP->sendIP = reverseDWORD(GetDeviceIP());
            EthernetSend((uint8_t *)m_pArpRSP, (uint16_t)sizeof(*m_pArpRSP));
         };
         res = 1;
      }
      else if(pPack->opcode[1] == 0x02)      /// if opcode is response 2
      {
         res = 2;
         uint8_t *pDMAC = GetDeviceMAC();
         for(uint16_t i=0; i<6; i++)
         {
            if(pPack->dMAC.targMAC[i] != pDMAC[i]) res = 0;
         };

         if(res == 2)
         {
            if(pPack->sendIP == servUDB.serverIP)
            {
               servUDB.serverMAC[0] = pPack->sendMAC[0];
               servUDB.serverMAC[1] = pPack->sendMAC[1];
               servUDB.serverMAC[2] = pPack->sendMAC[2];
               servUDB.serverMAC[3] = pPack->sendMAC[3];
               servUDB.serverMAC[4] = pPack->sendMAC[4];
               servUDB.serverMAC[5] = pPack->sendMAC[5];
               SetFlgARP(true);
            };
         };
      }
      else res = 3;     // ERROR
   };
   return res;
}


//char tsrr[100];
uint8_t *CArp::onMakeArpReq(uint32_t trgIP, uint16_t &len)
{
   uint8_t *pPtr = (uint8_t *)m_pArpREQ;
   len = (uint16_t)(sizeof(*m_pArpREQ));
   for(uint16_t i=0; i<6; i++)
   {
      m_pArpREQ->dMAC.targMAC[i] = 0xFF;
      m_pArpREQ->targMAC[i] = 0x00;
   };
   m_pArpREQ->targIP = reverseDWORD(trgIP);
   m_pArpREQ->sendIP = reverseDWORD(GetDeviceIP());
   return pPtr;
}

/*********************************************************************/
CUdp::CUdp() :
   m_BuffA(0),
   m_dataID(0),
   m_tmpLen(0),
   m_tmpCrc(0),
   m_tmpRCrc(0)
//   m_sCmdLen((uint16_t)sizeof(struct sCommandT)),
//   m_oldNmbSPX_A(0),
//   m_oldNmbSPX(0)
{
   m_BuffA = GetTxBuffLAN();
   m_UdpA = (UdpPack *)(m_BuffA);
   m_UdpA->dMAC.type[0] = 0x08;
   m_UdpA->dMAC.type[1] = 0x00;
   m_UdpA->hIP.headLen = 0x45;
   m_UdpA->hIP.diffSrv = 0x00;
   m_UdpA->hIP.totLen = 0;
   m_UdpA->hIP.dataID = 0;
   m_UdpA->hIP.flags = 0x0000;   // don't fragment
   m_UdpA->hIP.ttl = 0x80;
   m_UdpA->hIP.protoc = 0x11;    // UDP (17)
   m_UdpA->hIP.hdCrc = 0;
//   m_UdpA->srcPort = getMyOwnPort();
//   m_UdpA->destPort = getTerminalPort();
}

CUdp::~CUdp()
{
}

void CUdp::onInit(uint8_t *devMAC, uint32_t devIp, uint16_t srcPort)
{
   UdpPack *pPack = m_UdpA;
   for(uint8_t i=0; i<6; i++)
   {
      pPack->dMAC.srcMAC[i] = devMAC[i];
   };
   pPack->hIP.srcIP = reverseDWORD(devIp);
   pPack->srcPort = reverseWORD(srcPort);
   //pPack->destPort = getTerminalPort();
}

void CUdp::setAddr(uint8_t *trgMAC, uint32_t trgIP, uint16_t trgPort)
{
   UdpPack *pPack = m_UdpA;
   for(uint8_t i=0; i<6; i++)
   {
      pPack->dMAC.targMAC[i] = trgMAC[i];
   };
   pPack->hIP.trgIP = trgIP;
   pPack->destPort = reverseWORD(trgPort);
}

uint8_t *CUdp::makeUdp(uint8_t *data, uint16_t &len)
{
   uint8_t *resPtr = m_BuffA;
   uint16_t dlen = len;
   UdpPack *pUdp = m_UdpA;

   m_UdpA->dMAC.type[0] = 0x08;
   m_UdpA->dMAC.type[1] = 0x00;
   m_UdpA->hIP.headLen = 0x45;
   m_UdpA->hIP.diffSrv = 0x00;
   m_UdpA->hIP.totLen = 0;
   m_UdpA->hIP.dataID = 0;
   m_UdpA->hIP.flags = 0x0000;   // don't fragment
   m_UdpA->hIP.ttl = 0x80;
   m_UdpA->hIP.protoc = 0x11;    // UDP (17)
   m_UdpA->hIP.hdCrc = 0;

   uint8_t *pData = (uint8_t *)&(pUdp->datagram);
   for(uint16_t i=0; i<dlen; i++)
   {
      pData[i] = data[i];
   };

   uint32_t tlen = (uint32_t)sizeof(*pUdp) + (uint32_t)dlen - 1;     // 42 bytes
   pUdp->hIP.totLen = reverseWORD((uint16_t)(tlen - 14));
   pUdp->length = reverseWORD((uint16_t)(tlen - 34));
   pUdp->hIP.dataID = m_dataID;
   ++m_dataID;

   PseudoHeader *pPsd = (PseudoHeader *)&(resPtr[tlen]);
   pPsd->clientIP = pUdp->hIP.srcIP;
   pPsd->targIP = pUdp->hIP.trgIP;
   pPsd->reserved = 0x00;
   pPsd->protocol = pUdp->hIP.protoc;
   pPsd->length = pUdp->length;

   uint8_t *pD = (uint8_t *)(&resPtr[14]);
   pUdp->hIP.hdCrc = 0;
   pUdp->dataCRC = 0;
   uint16_t hCrc = GetIpCRC(pD, 20);
   pD = (uint8_t *)(&resPtr[34]);
   uint16_t dCrc = GetIpCRC(pD, tlen - 34 + 12);
   pUdp->hIP.hdCrc = hCrc;
   pUdp->dataCRC = dCrc;
   len = tlen;
   return resPtr;
}

uint8_t udpBuffRXA[UDP_BUFF_SZ];
uint8_t tMAC[6];
//uint8_t pPackCmd1[100];

void CUdp::onParseUDPA(uint8_t *data, uint16_t len)   /** call from main() **/
{
   printf("parse RX LAN A\n");
   if(len > UDP_BUFF_SZ) len = UDP_BUFF_SZ;
   for(uint16_t i=0; i<len; i++)
   {
      udpBuffRXA[i] = data[i];
   };
   //showPack(udpBuffRXA, len);
//   UdpPack *pUdpPtr = (UdpPack *)&(udpBuffRXA[0]);
//   if(pUdpPtr->hIP.protoc != (uint8_t)0x11) return;               // if not UDP
//   if(pUdpPtr->hIP.trgIP != GetDeviceIP()) return;                // if for not MY IP
//   if(pUdpPtr->destPort != reverseWORD(UDP_PORT)) return;         // if for not MY port

//   sHeaderT *pHdrT = (sHeaderT *)&(pUdpPtr->datagram);
//   uint16_t rxLen = reverseWORD(pUdpPtr->length);
//   if(rxLen >= 8) rxLen -= 8;
//   uint16_t tlen = reverseWORD(pUdpPtr->length) + 34;
//   if(len > tlen) len = tlen;
//   if(pHdrT->devNumb == GetDeviceNumb())         /** on handle for me **/
//   {
//      //DebugHandler((char *)(__FILE__), __LINE__);
//      if(!(pHdrT->reqDir & BIT7)) return;                            // if not station <- terminal
//      //DebugHandler((char *)(__FILE__), __LINE__);
//      PseudoHeader *pPsd = (PseudoHeader *)&(udpBuffRXA[len]);
//      pPsd->clientIP = pUdpPtr->hIP.srcIP;
//      pPsd->targIP = pUdpPtr->hIP.trgIP;
//      pPsd->reserved = 0x00;
//      pPsd->protocol = pUdpPtr->hIP.protoc;
//      pPsd->length = pUdpPtr->length;
//      uint8_t *pDt = (uint8_t *)&(udpBuffRXA[14]);
//      m_tmpRCrc = pUdpPtr->hIP.hdCrc;
//      pUdpPtr->hIP.hdCrc = 0;
//      m_tmpCrc = GetIpCRC(pDt, 20);
//      if(m_tmpCrc != m_tmpRCrc) return;                         /** if header crc not correct */
//      //DebugHandler((char *)(__FILE__), __LINE__);
//      pDt = (uint8_t *)&(udpBuffRXA[34]);
//      m_tmpRCrc = pUdpPtr->dataCRC;
//      pUdpPtr->dataCRC = 0;
//      m_tmpLen = reverseWORD(pPsd->length) + 12;
//      m_tmpCrc = GetIpCRC(pDt, m_tmpLen);
//      //DebugHandler((char *)(__FILE__), __LINE__);
//      if(m_tmpCrc != m_tmpRCrc) return;                         /** if data crc not correct */
//      if(pHdrT->code == SPX_CODE)                  /** SPX + CMD **/
//      {
//      }
//      else if(pHdrT->code == SPC_CODE)                /** ... **/
//      {
//      }
//      else if(pHdrT->code == STC_CODE)                /** ... **/
//      {
//      };
//   };                                              /***************************/
}


void SendDiscoverDHCP(uint32_t ID)
{
   printf("DHCP -> DSC\n");
   const uint16_t packLen = (uint16_t)sizeof(struct sDhcpDISCOVER);
   sDhcpDISCOVER* pDHCP = (sDhcpDISCOVER*)GetTxBuffLAN();
   uint8_t *pBFF = (uint8_t *)GetTxBuffLAN();
   uint8_t *pMAC = GetDeviceMAC();
   for(uint8_t i=0; i<6; i++)
   {
      pDHCP->dMAC.targMAC[i] = 0xFF;
      pDHCP->dMAC.srcMAC[i] = pMAC[i];
      pDHCP->DHCP.clntMAC[i] = pMAC[i];
      pDHCP->clntMAC1[i] = pMAC[i];
   };
   for(uint8_t i=6; i<16; i++)
   {
      pDHCP->DHCP.clntMAC[i] = 0;
   };
   for(uint8_t i=0; i<64; i++)
   {
      pDHCP->DHCP.sname[i] = 0;
   };
   for(uint8_t i=0; i<128; i++)
   {
      pDHCP->DHCP.bname[i] = 0;
   };
   pDHCP->dMAC.type[0] = 0x08;
   pDHCP->dMAC.type[1] = 0x00;
   pDHCP->hIP.headLen = 0x45;
   pDHCP->hIP.diffSrv = 0x00;
   pDHCP->hIP.dataID = reverseWORD(0x0001);
   pDHCP->hIP.flags = 0x0000;
   pDHCP->hIP.ttl = 0x80;
   pDHCP->hIP.protoc = 0x11;
   pDHCP->hIP.hdCrc = 0x0000;
   pDHCP->hIP.srcIP = 0x00000000;
   pDHCP->hIP.trgIP = 0xFFFFFFFF;
   pDHCP->subUDP.srcPort = 0x4400;        // port 68 DHCP client
   pDHCP->subUDP.destPort = 0x4300;       // port 67 DHCP server
   pDHCP->subUDP.length = reverseWORD(packLen - 34);
   pDHCP->subUDP.dataCRC = 0x0000;
   pDHCP->DHCP.msgType = 0x01;
   pDHCP->DHCP.hrdType = 0x01;
   pDHCP->DHCP.addrLen = 0x06;
   pDHCP->DHCP.hops = 0x00;
   pDHCP->DHCP.transID = reverseDWORD(ID);
   pDHCP->DHCP.secElps = 0x00;
   pDHCP->DHCP.broadCast = 0x0000;
   pDHCP->DHCP.senderIP = 0x00000000;
   pDHCP->DHCP.yourIP = 0x00000000;
   pDHCP->DHCP.nserverIP = 0x00000000;
   pDHCP->DHCP.relayIP = 0x00000000;
   pDHCP->DHCP.magCookie[0] = 0x63;
   pDHCP->DHCP.magCookie[1] = 0x82;
   pDHCP->DHCP.magCookie[2] = 0x53;
   pDHCP->DHCP.magCookie[3] = 0x63;
   pDHCP->DHCP.msgType1 = 53;    /// opt 53
   pDHCP->DHCP.length0 = 1;
   pDHCP->DHCP.DHCP = 0x01;
   pDHCP->autoConf = 116;        /// opt 116
   pDHCP->length1 = 1;
   pDHCP->autoConf1 = 0x01;
   pDHCP->clientID = 61;         /// opt 61
   pDHCP->length2 = 7;
   pDHCP->hrdType1 = 0x01;
   ///pDHCP->clntMAC1[6];

   pDHCP->ReqIpAddr = 50;        /// opt 50
   pDHCP->length3 = 4;
   pDHCP->reqIP = 0;

   pDHCP->hostName = 12;         /// opt 12
   pDHCP->length4 = 15;
   pDHCP->name[0] = 'A';
   pDHCP->name[1] = 'p';
   pDHCP->name[2] = 'o';
   pDHCP->name[3] = 'g';
   pDHCP->name[4] = 'e';
   pDHCP->name[5] = 'y';
   pDHCP->name[6] = 'S';
   pDHCP->name[7] = 'M';
   pDHCP->name[8] = 'C';
   pDHCP->name[9] = '-';
   pDHCP->name[10] = 'v';
   pDHCP->name[11] = '1';
   pDHCP->name[12] = '.';
   pDHCP->name[13] = '0';
   pDHCP->name[14] = '0';

   pDHCP->listRQ = 55;           /// opt 55
   pDHCP->length5 = 4;
   pDHCP->subMsk = 1;
   pDHCP->router = 3;
   pDHCP->dns = 6;
   pDHCP->NTimeP = 42;
   //pDHCP->lisTime = 51;
   pDHCP->optEnd = 0xFF;
   pDHCP->padding = 0;
   pDHCP->hIP.totLen = reverseWORD(packLen - 14);
   //pDHCP->hIP.dataID = reverseWORD(CntrID);

   PseudoHeader *pPsd = (PseudoHeader *)(&pBFF[packLen]);
   pPsd->clientIP = pDHCP->hIP.srcIP;
   pPsd->targIP = pDHCP->hIP.trgIP;
   pPsd->reserved = 0x00;
   pPsd->protocol = pDHCP->hIP.protoc;
   pPsd->length = pDHCP->subUDP.length;

   pDHCP->hIP.hdCrc = 0;
   pDHCP->subUDP.dataCRC = 0;
   uint8_t *pD = (uint8_t *)(&pBFF[14]);
   uint16_t hcrc = GetIpCRC(pD, 20);
   pD = (uint8_t *)(&pBFF[34]);
   uint16_t dcrc = GetIpCRC(pD, packLen - 34 + 12);
   pDHCP->hIP.hdCrc = hcrc;
   pDHCP->subUDP.dataCRC = dcrc;
   EthernetSend(pBFF, packLen);
   //if(++CntrID >= 10000) CntrID = 1;

//   //showPack(pBFF, packLen);
}

void SendRequestDHCP(uint8_t *addrStruct, uint32_t ID)
{
   printf("DHCP -> REQ\n");
   sAddrLAN *aLan = (sAddrLAN *)addrStruct;
   //static uint16_t CntrID = 100;
   const uint16_t packLen = (uint16_t)sizeof(struct sDhcpREQ);
   sDhcpREQ *pDHCP = (sDhcpREQ*)GetTxBuffLAN();
   uint8_t *pBFF = GetTxBuffLAN();;
   uint8_t *pMAC = GetDeviceMAC();
   for(uint8_t i=0; i<6; i++)
   {
      pDHCP->dMAC.targMAC[i] = 0xFF;
      pDHCP->dMAC.srcMAC[i] = pMAC[i];
      pDHCP->DHCP.clntMAC[i] = pMAC[i];
      pDHCP->clntMAC1[i] = pMAC[i];
   };
   for(uint8_t i=6; i<16; i++)
   {
      pDHCP->DHCP.clntMAC[i] = 0;
   };
   for(uint8_t i=0; i<64; i++)
   {
      pDHCP->DHCP.sname[i] = 0;
   };
   for(uint8_t i=0; i<128; i++)
   {
      pDHCP->DHCP.bname[i] = 0;
   };
   pDHCP->dMAC.type[0] = 0x08;
   pDHCP->dMAC.type[1] = 0x00;
   pDHCP->hIP.headLen = 0x45;
   pDHCP->hIP.diffSrv = 0x00;
   //pDHCP->hIP.dataID = 0xA836;
   pDHCP->hIP.dataID = reverseWORD(0x0001);
   //pDHCP->hIP.dataID = reverseWORD(0xA836);
   pDHCP->hIP.flags = 0x0000;
   //pDHCP->hIP.ttl = 0x80;
   pDHCP->hIP.ttl = 0xFA;
   pDHCP->hIP.protoc = 0x11;
   pDHCP->hIP.hdCrc = 0x0000;
   pDHCP->hIP.srcIP = 0x00000000;
   pDHCP->hIP.trgIP = 0xFFFFFFFF;
   pDHCP->subUDP.srcPort = 0x4400;
   pDHCP->subUDP.destPort = 0x4300;
   pDHCP->subUDP.length = reverseWORD(packLen - 34);
   pDHCP->subUDP.dataCRC = 0x0000;
   pDHCP->DHCP.msgType = 0x01;
   pDHCP->DHCP.hrdType = 0x01;
   pDHCP->DHCP.addrLen = 0x06;
   pDHCP->DHCP.hops = 0x00;
   pDHCP->DHCP.transID = reverseDWORD(ID);
   pDHCP->DHCP.secElps = 0x00;
   pDHCP->DHCP.broadCast = 0x0000;
   pDHCP->DHCP.senderIP = 0x00000000;
   pDHCP->DHCP.yourIP = 0x00000000;
   pDHCP->DHCP.nserverIP = 0x00000000;
   pDHCP->DHCP.relayIP = 0x00000000;
   pDHCP->DHCP.magCookie[0] = 0x63;
   pDHCP->DHCP.magCookie[1] = 0x82;
   pDHCP->DHCP.magCookie[2] = 0x53;
   pDHCP->DHCP.magCookie[3] = 0x63;
   pDHCP->DHCP.msgType1 = 53;    /// opt 53
   pDHCP->DHCP.length0 = 1;
   pDHCP->DHCP.DHCP = 0x03;      /// 3 - request
//   pDHCP->autoConf = 116;        /// opt 116
//   pDHCP->length1 = 1;
//   pDHCP->autoConf1 = 0x01;
   pDHCP->clientID = 61;         /// opt 61
   pDHCP->length2 = 7;
   pDHCP->hrdType1 = 0x01;
   ///pDHCP->clntMAC1[6];

   pDHCP->rqAddrIP = 50;         /// opt 50
   pDHCP->length3 = 4;
   pDHCP->clientIP1 = reverseDWORD(aLan->deviceIP);

   pDHCP->dhcpServerID = 54;     /// opt 54
   pDHCP->length4 = 4;
   pDHCP->dhcpIP = reverseDWORD(aLan->dhcpServIP);

   pDHCP->hostName = 12;         /// opt 12
   pDHCP->length5 = 15;
   pDHCP->name[0] = 'A';
   pDHCP->name[1] = 'p';
   pDHCP->name[2] = 'o';
   pDHCP->name[3] = 'g';
   pDHCP->name[4] = 'e';
   pDHCP->name[5] = 'y';
   pDHCP->name[6] = 'S';
   pDHCP->name[7] = 'M';
   pDHCP->name[8] = 'C';
   pDHCP->name[9] = '-';
   pDHCP->name[10] = 'v';
   pDHCP->name[11] = '1';
   pDHCP->name[12] = '.';
   pDHCP->name[13] = '0';
   pDHCP->name[14] = '0';

   pDHCP->listRQ = 55;           /// opt 55
   pDHCP->length7 = 4;
   pDHCP->subMsk = 1;
   pDHCP->router = 3;
   pDHCP->dns = 6;
   pDHCP->NTimeP = 42;
   pDHCP->optEnd = 0xFF;
   //pDHCP->padding = 0;

   pDHCP->hIP.totLen = reverseWORD(packLen - 14);

   PseudoHeader *pPsd = (PseudoHeader *)(&pBFF[packLen]);
   pPsd->clientIP = pDHCP->hIP.srcIP;
   pPsd->targIP = pDHCP->hIP.trgIP;
   pPsd->reserved = 0x00;
   pPsd->protocol = pDHCP->hIP.protoc;
   pPsd->length = pDHCP->subUDP.length;
   pDHCP->hIP.hdCrc = 0;
   pDHCP->subUDP.dataCRC = 0;
   uint8_t *pD = (uint8_t *)(&pBFF[14]);
   uint16_t hcrc = GetIpCRC(pD, 20);
   pD = (uint8_t *)(&pBFF[34]);
   uint16_t dcrc = GetIpCRC(pD, packLen - 34 + 12);
   pDHCP->hIP.hdCrc = hcrc;
   pDHCP->subUDP.dataCRC = dcrc;
   EthernetSend(pBFF, packLen);
}

void assert_failed(uint8_t *file, uint32_t line)
{
   printf("%s, line %d\r\n", file, line);
}
