#include "spi_cf.h"
#include "socket.h"
#include "wizchip_conf.h"
#include "w5500.h"
#include "dhcp.h"

#include "stdio.h"
#include "stdlib.h"
#include "string.h"

#define Data_BuffSize   2048
#define MAX_DHCP_RET    3

void Delay_ms(uint16_t time);
void Delay_ns(uint16_t time);
void W5500_Network_Init(void);

uint8_t socket_sn = 0;
uint8_t client_sn = 0;
uint8_t gDataBuff[Data_BuffSize];

wiz_NetInfo gWIZNETINFO = { .mac = {0x00, 0x08, 0xdc, 0x11, 0x11},
                            .ip = {192, 168, 32, 124},
                            .sn = {255, 255, 255, 0},
                            .gw = {192, 168, 32, 1},
                            .dns = {8, 8, 8, 8},
                            .dhcp = NETINFO_DHCP };
void W5500_Network_Init(void)
{
        uint8_t tmpstr[6] = {0,};
	ctlnetwork(CN_SET_NETINFO, (void*)&gWIZNETINFO);
	
	ctlnetwork(CN_GET_NETINFO, (void*)&gWIZNETINFO);
	
	//Display Network Information use USART
	ctlwizchip(CW_GET_ID, (void*)tmpstr);
	printf("\r\n=== %s NET CONF ===\r\n", (char*)tmpstr);
	printf("MAC:%02X:%02X:%02X:%02X:%02X:%02X\r\n", gWIZNETINFO.mac[0], gWIZNETINFO.mac[1], 
	gWIZNETINFO.mac[2],  gWIZNETINFO.mac[3], gWIZNETINFO.mac[4], gWIZNETINFO.mac[5]);
	printf("SIP: %d.%d.%d.%d\r\n", gWIZNETINFO.ip[0], gWIZNETINFO.ip[1], gWIZNETINFO.ip[2], gWIZNETINFO.ip[3]);
	printf("GAR: %d.%d.%d.%d\r\n", gWIZNETINFO.gw[0], gWIZNETINFO.gw[1], gWIZNETINFO.gw[2], gWIZNETINFO.gw[3]);
	printf("SUB: %d.%d.%d.%d\r\n", gWIZNETINFO.sn[0], gWIZNETINFO.sn[1], gWIZNETINFO.sn[2], gWIZNETINFO.sn[3]);
	printf("SUB: %d.%d.%d.%d\r\n", gWIZNETINFO.dns[0], gWIZNETINFO.dns[1], gWIZNETINFO.dns[2], gWIZNETINFO.dns[3]);
	printf("==============================\r\n");
	
	wizchip_init(0, 0);
	
	wiz_NetTimeout w_NetTimeout;
	w_NetTimeout.retry_cnt = 50;
	w_NetTimeout.time_100us = 1000;
	wizchip_settimeout(&w_NetTimeout);
}

void my_ip_assign(void)
{
  getIPfromDHCP(gWIZNETINFO.ip);
  getGWfromDHCP(gWIZNETINFO.gw);
  getSNfromDHCP(gWIZNETINFO.sn);
  getDNSfromDHCP(gWIZNETINFO.dns);
  gWIZNETINFO.dhcp = NETINFO_DHCP;
  
  ctlnetwork(CN_SET_NETINFO, (void*)&gWIZNETINFO);
}

void my_ip_conflict(void) 
{
  printf("CONFLICT IP from DHCP \r\n");
  while(1);
}


int main(void)
{
  uint8_t dhcp_ret = 0;
  
  W5500_Init();
  
  setSHAR(gWIZNETINFO.mac);
  
  DHCP_init(socket_sn, gDataBuff);
  
  reg_dhcp_cbfunc(my_ip_assign, my_ip_assign, my_ip_conflict);
  while(1)
  {
    switch (DHCP_run())
    {
      case DHCP_IP_ASSIGN:
      case DHCP_IP_CHANGED:
        break;
      case DHCP_IP_LEASED:
        //Code cho chuong trinh thi viet vao day
        socket_sn = socket_sn;
        break;
      case DHCP_FAILED:
        dhcp_ret++;
        if (dhcp_ret > MAX_DHCP_RET)
        {
          dhcp_ret = 0;
          DHCP_stop(); //if restart, recall DHCP_init()
          W5500_Network_Init();
        }
        break;
      default:
        break;
    }
  }
}

/*
*
*
*
*
*/
void Delay_ms(uint16_t time)
{
uint32_t time_n=time*12000;
	while(time_n!=0){time_n--;}

}
void Delay_ns(uint16_t time)
{
uint32_t time_n=time*12;
	while(time_n!=0){time_n--;}

}
