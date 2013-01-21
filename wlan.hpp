// 802.11 Programming example
// Author: Michel Barbeau, January 2004
//
// File: wlan.hpp
//

typedef int Outcome;
#define OK 0
#define NOK -1

#define MAX_NAME_LEN 128

// Declaration of struct WLANAddr.
#define WLAN_ADDR_LEN 6
struct WLANAddr
{
   // address
   unsigned char data[WLAN_ADDR_LEN];
   // return the address in a human readable form
   char * wlan2asc();
   // defined the address from a human readable form
   Outcome str2wlan(char s[]);
};

// format of an WLAN header
struct WLANHeader
{
   WLANAddr destAddr;
   WLANAddr srcAddr;
   unsigned short type;
   // make IP different from the real IP number (x0800)
   // types x3*** are not reserved
   #define IP_TYPE 0x3901
};
#define WLAN_HEADER_LEN 14 // bytes

// wireless interface configuration
struct Ifconfig
{
   int sockid; // socket descriptor
   int ifindex; // index of the interface
   WLANAddr hwaddr; // MAC address
   int mtu; // maximum transmission unit
};
