// gcc -O set_ip_mac_address.c ctl_orm.c spi_common.c -l bcm2835 -o set_ip_mac_address.exe

#include "spi_common.h"
#include "ctl_orm.h"

#include <stdio.h>

#define USE_BD1 0
#define USE_BD2 0
#define USE_BD3 1

int main() 
{
  init_spi();
  
  int ip0, ip1;
  ip0 = CTL_get_ip_address0();
  ip1 = CTL_get_ip_address1();
  printf("old IP0 Address: %x\n", ip0);
  printf("old IP1 Address: %x\n", ip1);

  int mac0, mac1, mac2;
  mac0 = CTL_get_mac_address0();
  mac1 = CTL_get_mac_address1();
  mac2 = CTL_get_mac_address2();
  printf("old MAC0 Address: %x\n", mac0);
  printf("old MAC1 Address: %x\n", mac1);
  printf("old MAC2 Address: %x\n", mac2);
  
  if (USE_BD1) {
    CTL_put_ip_address0(0xdec8);
    CTL_put_ip_address1(0xc0a8);
    
    CTL_put_mac_address0(0xffaf);
    CTL_put_mac_address1(0x3200);
    CTL_put_mac_address2(0x0800);
  }
  
  else if (USE_BD2) {
    CTL_put_ip_address0(0xdec9);
    CTL_put_ip_address1(0xc0a8);
    
    CTL_put_mac_address0(0xffbf);
    CTL_put_mac_address1(0x3200);
    CTL_put_mac_address2(0x0800);
  }
  
  else if (USE_BD3) {
    CTL_put_ip_address0(0xdeca);
    CTL_put_ip_address1(0xc0a8);
    
    CTL_put_mac_address0(0xffcf);
    CTL_put_mac_address1(0x3200);
    CTL_put_mac_address2(0x0800);
  }
  
  else return(-1);
  
  ip0 = CTL_get_ip_address0();
  ip1 = CTL_get_ip_address1();
  printf("new IP0 Address: %x\n", ip0);
  printf("new IP1 Address: %x\n", ip1);
  
  mac0 = CTL_get_mac_address0();
  mac1 = CTL_get_mac_address1();
  mac2 = CTL_get_mac_address2();

  printf("resetting the si5324\n");
  spi_put_16bits(4,0x1,0xF); // Reset the si5324 and then ipbus.
  spi_put_16bits(4,0x1,0x0); // Reset the si5324 and then ipbus.

  printf("new MAC0 Address: %x\n", mac0);
  printf("new MAC1 Address: %x\n", mac1);
  printf("new MAC2 Address: %x\n", mac2);
  
  end_spi();
  return 0;
}
