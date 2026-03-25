#ifndef UDP_H_INCLUDED
#define UDP_H_INCLUDED

#include "stdint.h"

//#define DEBUG_FLASH        /** DEBUG MODE - DEBUG MODE -  DEBUG MODE **/
//#define UDP_PORT        40000
#define LOCAL_SERVER_PORT     40001
#define LOCAL_DEVICE_PORT     40100

const uint16_t gm_max_sz_Eth = 600;

#define  IDLE_MODE            1
#define  BTTN_PRSD            2
#define  SENT_STC             3
#define  GOT_STC              4

#define SENDER_STAT_IDLE   0
#define SENDER_STAT_WORK   100
#define SENDER_STAT_STOP   200

#define UDP_BUFF_SZ     1600

#pragma pack(push,1)
struct NetStat             // sTcpPseudo
{
   bool devIP;             // device IP from DHCP request
   bool servMAC;           // local server MAC from ARP request
   bool servIsON;          // local server is online from PING
};
#pragma pack(pop)

#pragma pack(push,1)
struct PseudoHeader          // sTcpPseudo
{
   uint32_t clientIP;      // source IP
   uint32_t targIP;        // target IP
   uint8_t reserved;
   uint8_t protocol;
   uint16_t length;         // UDP length (TCP Header+TCP Data)
};
#pragma pack(pop)

#pragma pack(push,1)
struct devMAC
{
   uint8_t targMAC[6];     // Destination MAC (0xFF...0xFF)
   uint8_t srcMAC[6];      // Source MAC (0x00...0x00)
   uint8_t type[2];        // Packet type (ARP, IP...) 0x0800
};    // 14 Bytes
#pragma pack(pop)

#pragma pack(push,1)
struct headIP
{
   uint8_t headLen;     // 0x45 (01000101)=(0100-IP)(0101-len-5*4=20)
   uint8_t diffSrv;     // 0x00
   uint16_t totLen;     // Total Length: (sizeof(pack) - 14) (reverse), from pos 14 to end
   uint16_t dataID;     // Identification: number of datagram fragment (reverse)
   uint16_t flags;      // 0x4000 - (reverse)
   uint8_t ttl;         // 0x80
   uint8_t protoc;      // TCP - 0x06, UDP - 0x11, ICMP - 0x01
   uint16_t hdCrc;      // Header checksum (from pos 14 to pos 33) (reverse)
   uint32_t srcIP;      // Source IP
   uint32_t trgIP;      // Target IP
};    // 20 Bytes
#pragma pack(pop)

#pragma pack(push,1)
struct ArpPack          // sArpTX
{
   devMAC dMAC;            // 14 bytes
   uint8_t htype[2];       // Hardware type (Ethernet=0x01)
   uint8_t protocol[2];    // Protocol type (IP=0x8000)
   uint8_t hsize;          // Hardware (MAC) size (6)
   uint8_t psize;          // Protocol size (4)
   uint8_t opcode[2];      // Opcode request (0x0001)
   uint8_t sendMAC[6];     // Sender MAC
   uint32_t sendIP;        // Sender IP
   uint8_t targMAC[6];     // Target MAC
   uint32_t targIP;        // Target IP
   uint8_t padding[18];
};    // 42 Byte
#pragma pack(pop)
/******************************/
#pragma pack(push,1)
struct UdpPack             // Dhcp_DISCOV (length 316 Byte)
{
   devMAC dMAC;            // 14 bytes
   headIP hIP;             // 20 bytes
   uint16_t srcPort;       // Source Port (40001)
   uint16_t destPort;      // Destination Port (40000)
   uint16_t length;        // Datagram Length: from 34 pos
   uint16_t dataCRC;       // Data checksum (from pos 34 to pos end + pseudo header)
   uint8_t datagram;       // Datagram 0 ...
   /** datagram ... **/
};
#pragma pack(pop)

#pragma pack(push,1)
struct UdpSubh             // Dhcp_DISCOV (length 316 Byte)
{
   uint16_t srcPort;       // Source Port (40001)
   uint16_t destPort;      // Destination Port (40000)
   uint16_t length;        // Datagram Length: from 34 pos
   uint16_t dataCRC;       // Data checksum (from pos 34 to pos end + pseudo header)
};
#pragma pack(pop)

#pragma pack(push,1)
struct DhcpPack             // Dhcp (length 243 Byte)
{
   uint8_t msgType;        // Message type: Boot request (1) 		"op code"
   uint8_t hrdType;        // Hardware type: Ethernet (1)			"htype"
   uint8_t addrLen;        // Hardware address length: (6)			"hlen"
   uint8_t hops;           // Hops: 0								      "hops"
   uint32_t transID;       // Transaction ID:						   "xid"
   uint16_t secElps;       // Seconds elapsed: 0					   "secs"
   uint16_t broadCast;     // Broadcast flags: broadcast(0x8000)	"flags"
   uint32_t senderIP;      // Sender IP address (0)					"ciaddr"
   uint32_t yourIP;        // Your (Client) IP address (0)			"yiaddr"
   uint32_t nserverIP;     // Next server IP address (0)			   "siaddr"
   uint32_t relayIP;       // Gateway IP Address (0)				   "giaddr"
   uint8_t clntMAC[16];    // Client Hardware MAC address (0...)  "chaddr"
   uint8_t sname[64];      // server name 						      "sname"
   uint8_t bname[128];     // boot file name				            "bname"
   uint8_t magCookie[4];   // Magic cookie: (from pos: 278 - {0x63,0x82,0x53,0x63})
   uint8_t msgType1;       // Options: 53 - DHCP Message type (0x35)
   uint8_t length0;        // Options: Length - 0x01
   uint8_t DHCP;      	   // Options: DHCP - Disc 0x01, Offer 0x02, Req 0x03, Ack 0x05
};
#pragma pack(pop)

#pragma pack(push,1)
struct packICMP
{
	devMAC dMAC;            // 14 bytes
	headIP hIP;             // 20 bytes
	uint8_t typeRq;			// Type (Echo) ping-request
	uint8_t Code;				// Code
	uint16_t crcDat;			// Checksum data
	uint8_t IdBE;				// Identifier (BE)
	uint8_t IdLE;				// Identifier (LE)
	uint8_t SeqBE;		      // Sequence (BE)
	uint8_t SeqLE;		      // Sequence (LE)
	uint8_t data[100];	   // Data
};    // 74 bytes
#pragma pack(pop)

#pragma pack(push,1)
struct NetAddr             // 12 bytes
{
   uint8_t MAC[6];         // 6 bytes
   uint32_t IP;            // 4 bytes
   uint16_t port;          // 2
};
#pragma pack(pop)

/** <<< DHCP >>> */
#pragma pack(push,1)
//typedef struct          	// sDhcpTX_DISC (length 314 Byte)
struct sDhcpDISCOVER       // sDhcpTX_DISC (length 314 Byte)
{
   devMAC dMAC;            // UDP:  14 bytes
   headIP hIP;             // UDP:  20 bytes
   UdpSubh subUDP;         // UDP:  8 bytes
   DhcpPack DHCP;          // DHCP: 243 byte
   uint8_t autoConf;       // DHCP Options: 116 - DHCP Auto-Configuration (0x74)
   uint8_t length1;        // DHCP: Length - 0x01
   uint8_t autoConf1;      // DHCP Options: AutoConfigure 0x01
   uint8_t clientID;       // DHCP: Options: Client identifier (0x3D)
   uint8_t length2;        // DHCP: Length: 0x07
   uint8_t hrdType1;       // Hardware type: Ethernet (1)
   uint8_t clntMAC1[6];    // Client MAC address (0...0)
   uint8_t ReqIpAddr;      // DHCP Options: Requested IP address (50)
   uint8_t length3;        // Length: 4
   uint32_t reqIP;          //
   uint8_t hostName;       // DHCP Options: Host Name
   uint8_t length4;        // Length: 15
   char name[15];           // "ApogeySMC-v1.00"
   uint8_t listRQ;         // DHCP Options: 55 - Parameter request list (0x37)
   uint8_t length5;        // Length: 0x03
   uint8_t subMsk;         // Parameter request list Item: (1) Subnet Mask
   uint8_t router;         // Parameter request list Item: (3) Router
   uint8_t dns;            // Parameter request list Item: (6) Domain Name Server
   uint8_t NTimeP;         // Parameter request list Item: (42) Network Time protocol
   uint8_t optEnd;         // Option End: 255 (0xFF)
   uint8_t padding;
};
#pragma pack(pop)

#pragma pack(push,1)
struct sDhcpOFFER          // sDhcpTX_OFFER (length 320+270 Byte)
{
   devMAC dMAC;            // UDP:  14 bytes
   headIP hIP;             // UDP:  20 bytes
   UdpSubh subUDP;         // UDP:  8 bytes
   DhcpPack DHCP;          // DHCP: 243 byte + 42 = 285 byte
   uint8_t DHCPsrvIP;      // Options: DHCP server Identifier (0x36)
   uint8_t length1;      	// Length - 0x04
   uint32_t serverIP;      // DHCP server IP address (c0:a8:00:01)
   uint8_t leaseTimeIP;    // Options: IP addres lease time (0x33)
   uint8_t length2;        // Length: 0x04
   uint32_t leaseTime;     // DHCP server lease Time (0x0000012C) (300sec, 5min)
   uint8_t subMaskOp;      // Subnet mask (1)
   uint8_t length3;        // Length: 0x04
   uint32_t subMask;       // Subnet mask (255.255.255.0)
   uint8_t routerOp;       // Router (0x03)
   uint8_t length4;        // Length: 0x04
   uint32_t routerIP;      // Router IP address (c0:a8:00:01)
   uint8_t dnsOp;          // Parameter request list Item: (0x06) Domain Name Server
   uint8_t length5;        // Length: 0x08
   uint32_t dnsIP;         // DNS server IP address (c0:a8:00:01)
   uint32_t adnsIP;        // DHCP server IP address (08:08:08:08)
   uint8_t optEnd;         // Option End: 255 (0xFF)
};
#pragma pack(pop)

#pragma pack(push,1)
struct sDataUDP         	// 36 Byte
{
   uint8_t mcookie[4];     // 0xCC, 0xAA, 0x53, 0x11        4
   uint8_t readerNumb;     //                               5
   uint8_t cardID[8];      //                               13
   uint8_t cardSTAT;       // UID - card status             14
   uint8_t msgType;        // 0x07 - REQ, 0x08 -RESP        15
   uint32_t reqID;         //                               19
   uint8_t textlen;        //                               20
   uint8_t text[16];       //                               36 byte
};
#pragma pack(pop)

#pragma pack(push,1)
struct sPackUDP         	//
{
   devMAC dMAC;            // UDP:  14 bytes
   headIP hIP;             // UDP:  20 bytes
   UdpSubh subUDP;         // UDP:  8 bytes
   uint8_t dataUDP[30];
};
#pragma pack(pop)

#pragma pack(push,1)
struct sDhcpREQ          	// sDhcpTX_RQ (length 338 Byte)
{
   devMAC dMAC;            // UDP:  14 bytes
   headIP hIP;             // UDP:  20 bytes
   UdpSubh subUDP;         // UDP:  8 bytes
   DhcpPack DHCP;          // DHCP: 243 byte
   uint8_t clientID;       // Client identifier (0x3D)
   uint8_t length2;        // Length: 0x07
   uint8_t hrdType1;       // Hardware type: Ethernet (0x01)
   uint8_t clntMAC1[6];    // Client MAC address (0...0)
   uint8_t rqAddrIP;       // Requested IP address: 0x32
   uint8_t length3;        // Length: 0x04
   uint32_t clientIP1;     // Client IP: 0x00000000
   uint8_t dhcpServerID;   // DHCP server Identifier: 0x36
   uint8_t length4;        // Length: 0x04
   uint32_t dhcpIP;        // DHCP IP: 0x00000000
   uint8_t hostName;       // Host Name: 0x0C
   uint8_t length5;        // Length: 0x09
   char name[15];          // "ApoClient"
   uint8_t listRQ;         // Options: 55 - Parameter request list (0x37)
   uint8_t length7;        // Length: 4
   uint8_t subMsk;         // Parameter request list Item: (0x01) Subnet Mask
   uint8_t router;         // Parameter request list Item: (0x03) Router
   uint8_t dns;            // Parameter request list Item: (0x06) Domain Name Server
   uint8_t NTimeP;         // Parameter request list Item: (42) Network Time protocol
   uint8_t optEnd;         // Option End: 255 (0xFF)
};
#pragma pack(pop)

#pragma pack(push,1)
struct sDhcpACK          	// sDhcpTX_ACK (length 320+270 Byte)
{
   devMAC dMAC;            // UDP:  14 bytes
   headIP hIP;             // UDP:  20 bytes
   UdpSubh subUDP;         // UDP:  8 bytes
   DhcpPack DHCP;          // DHCP: 243 byte
   uint8_t DHCPsrvIP;      // Options: DHCP server Identifier (0x36)
   uint8_t length1;      	// Options: Length - 0x04
   uint32_t serverIP;      // DHCP server IP address (c0:a8:00:01)
   uint8_t leaseTimeIP;    // Options: IP addres lease time (0x33)
   uint8_t length2;        // Length: 0x04
   uint32_t leaseTime;     // DHCP server lease Time (0x0000012C) (300sec, 5min)
   uint8_t subMaskOp;      // Subnet mask (1)
   uint8_t length3;        // Length: 0x04
   uint32_t subMask;       // Subnet mask (255.255.255.0)
   uint8_t routerOp;       // Router (0x03)
   uint8_t length4;        // Length: 0x04
   uint32_t routerIP;      // Router IP address (c0:a8:00:01)
   uint8_t dnsOp;          // Parameter request list Item: (0x06) Domain Name Server
   uint8_t length5;        // Length: 0x08
   uint32_t dnsIP;         // DNS server IP address (c0:a8:00:01)
   uint32_t adnsIP;        // DHCP server IP address (08:08:08:08)
   uint8_t optEnd;         // Option End: 255 (0xFF)
};
#pragma pack(pop)
/** >>> DHCP <<< */

#pragma pack(push,1)
struct ReSendBuff             // 12 bytes
{
   NetAddr addr;
   uint16_t dlen;
   uint8_t bff[UDP_BUFF_SZ];
};
#pragma pack(pop)

#pragma pack(push,1)
struct TimeSett            // 4 bytes
{
   uint16_t period;        // 2 bytes
   uint16_t attempt;       // 2 bytes
};
#pragma pack(pop)

#pragma pack(push,1)
struct DatePack            // 6 bytes
{
   uint8_t *pDat;          // 4 bytes
   uint16_t len;           // 2 bytes
};
#pragma pack(pop)

#pragma pack(push,1)
struct sAddrLAN                // 42 BYTE
{
   uint8_t servMAC[6];        /** server MAC          */
   uint32_t deviceIP;         /** device IP address   */
   uint32_t dhcpServIP;       /** DHCP server IP      */
   uint32_t routerIP;         /** router IP address   */
   uint32_t nameServ;         /** Name Server         */
   uint32_t dnsIP;            /** DNS IP address      */
   uint32_t subNetMsk;        /** subnet mask         */
   uint32_t leaseTime;        /** Lease Time          */
};
#pragma pack(pop)

#pragma pack(push,1)
struct sServerUDB                // 42 BYTE
{
   uint8_t serverMAC[6];      /** server MAC          */
   uint8_t flag;              /** status flag  0, 1   */
   uint8_t reserve;           /** 0x00                */
   uint32_t serverIP;         /** server IP address   */
   uint16_t serverPort;       /** server Port         */
};
#pragma pack(pop)

#pragma pack(push,1)
struct dPTR
{
   uint8_t *data;
   uint16_t *byteCount;
   uint8_t status;
};
#pragma pack(pop)

#pragma pack(push,1)
struct sRqDB
{
   uint8_t typeUID;
   uint8_t UID[7];
   bool status;
};
#pragma pack(pop)

class CArp
{
public:
   CArp();
   ~CArp();
   void onInit(uint8_t *devMac);
   char onCheckArpA(uint8_t *data, uint16_t len);
   uint8_t *onMakeArpReq(uint32_t trgIP, uint16_t &len);

protected:
private:
   ArpPack *m_pArpREQ;
   ArpPack *m_pArpRSP;

};

class CUdpSpeexBuffer;

class CUdp
{
public:
   CUdp();
   ~CUdp();
   void onInit(uint8_t *devMAC, uint32_t devIp, uint16_t srcPort);
   void setAddr(uint8_t *trgMAC, uint32_t trgIP, uint16_t trgPort);
   uint8_t *makeUdp(uint8_t *data, uint16_t &len);
   void onResetID(void);
   void onParseUDPA(uint8_t *data, uint16_t len);
protected:
private:
   uint8_t *m_BuffA;
   UdpPack *m_UdpA;
   uint16_t m_dataID;
   uint16_t m_tmpLen;
   uint16_t m_tmpCrc;
   uint16_t m_tmpRCrc;
   //const uint16_t m_sCmdLen;
//   uint32_t m_oldNmbSPX_A;
//   uint32_t m_oldNmbSPX;
//   uint32_t m_newNmbSPX;
//   uint32_t m_countSPX;
   uint32_t m_fixCnt;
   CUdpSpeexBuffer *p_USBF;

};

#define FIFO_UDP_CNT    50
#define UDPTX_MAXSZ     300

class CBuffLAN
{
public:
   explicit CBuffLAN(uint16_t packLen, uint16_t packCnt);
   ~CBuffLAN();
   void onWrite(uint8_t *data, uint16_t len);
   bool onGetWriteBuff(dPTR &ptr);
   bool onGetReadBuff(dPTR &ptr);
   bool onRead(uint8_t *data, uint16_t &len);
   bool onCheck(void);

protected:
private:
   uint8_t **m_Buff;
   uint16_t *m_Len;
   const uint16_t m_lenPack;
   const uint16_t m_cntPack;
   uint8_t m_cntr;
   uint8_t m_wrtr;
   uint8_t m_read;
};

uint16_t GetIpCRC(uint8_t *b, uint16_t len);
void SetAbonentPos(uint8_t pos);
void SystemReset(void);
void SendDiscoverDHCP(uint32_t ID);
void SendRequestDHCP(uint8_t *addrStruct, uint32_t ID);
uint8_t *GetDeviceMAC(void);
uint32_t GetDhcpCntrID(void);
uint32_t GetDeviceIP(void);
uint8_t *GetDeviceMAC(void);
uint8_t* GetTxBuffLAN(void);
uint16_t GetTxBuffSzLAN(void);
void SetFlgARP(bool flg);
bool onCheckRxCount(void);

void assert_failed(uint8_t *file, uint32_t line);

#endif /* UDP_H_INCLUDED */
