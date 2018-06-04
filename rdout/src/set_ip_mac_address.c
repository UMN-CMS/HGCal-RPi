// gcc -O set_ip_mac_address.c ctl_orm.c spi_common.c -l bcm2835 -o set_ip_mac_address.exe

#include "spi_common.h"
#include "ctl_orm.h"

#include <stdio.h>
#include <stdlib.h>


// base IP and MAC addresses
#define IP1 ((192 << 8) | 168)
#define IP0 ((222 << 8) | 200)

#define MAC2 0x0800
#define MAC1 0x3200
#define MAC0 0xff00

int main(int argc, char *argv[]) {

    // argument processing
    if(argc < 2) {
        printf("usage: sudo ./set_ip [BOARD NUMBER]\n");
        return 1;
    }
    int boardnum = atoi(argv[1]);
    printf("Board number: %d (0x%02x)\n", boardnum, boardnum);
    if((boardnum < 0) || boardnum > (0xff-(IP0&0xff))) {
        printf("board number must be between 0 and %d\n", (0xff-(IP0&0xff)));
        return 1;
    }


    // startup SPI
    init_spi();

  
    // get old IP and MAC
    int ip0, ip1;
    ip0 = CTL_get_ip_address0();
    ip1 = CTL_get_ip_address1();
    printf("Old IP Address: %d.%d.%d.%d\n",
            (ip1>>8), (ip1&0xff),
            (ip0>>8), (ip0&0xff));

    int mac0, mac1, mac2;
    mac0 = CTL_get_mac_address0();
    mac1 = CTL_get_mac_address1();
    mac2 = CTL_get_mac_address2();
    printf("Old MAC Address: %02x:%02x:%02x:%02x:%02x:%02x\n",
            (mac2>>8), (mac2&0xff),
            (mac1>>8), (mac1&0xff),
            (mac0>>8), (mac0&0xff));


    // Set new IP and MAC
    printf("Setting IP to %d.%d.%d.%d\n",
            (IP1>>8), (IP1&0xff),
            (IP0>>8), ((IP0+boardnum)&0xff));
    CTL_put_ip_address0(IP0+boardnum);
    CTL_put_ip_address1(IP1);

    printf("Setting MAC to %02x:%02x:%02x:%02x:%02x:%02x\n",
            (MAC2>>8), (MAC2&0xff),
            (MAC1>>8), (MAC1&0xff),
            (MAC0>>8), ((MAC0+boardnum)&0xff));
    CTL_put_mac_address0(MAC0+boardnum);
    CTL_put_mac_address1(MAC1);
    CTL_put_mac_address2(MAC2);


    // resets
    printf("resetting the si5324\n");
    spi_put_16bits(4,0x1,0xF); // Reset the si5324
    printf("resetting ipbus\n");
    spi_put_16bits(4,0x1,0x0); // Reset ipbus.


    // Get new IP
    ip0 = CTL_get_ip_address0();
    ip1 = CTL_get_ip_address1();
    printf("New IP Address: %d.%d.%d.%d\n",
            (ip1>>8), (ip1&0xff),
            (ip0>>8), (ip0&0xff));
    if((ip0 != (IP0+boardnum)) || (ip1 != IP1)) {
        printf("IP was not correctly set\n");
        end_spi();
        return 1;
    }

    mac0 = CTL_get_mac_address0();
    mac1 = CTL_get_mac_address1();
    mac2 = CTL_get_mac_address2();
    printf("New MAC Address: %02x:%02x:%02x:%02x:%02x:%02x\n",
            (mac2>>8), (mac2&0xff),
            (mac1>>8), (mac1&0xff),
            (mac0>>8), (mac0&0xff));
    if((mac0 != (MAC0+boardnum)) || (mac1 !=  MAC1) || (mac2 != MAC2)) {
        printf("MAC was not correctly set\n");
        end_spi();
        return 1;
    }


    // closing actions
    end_spi();
    return 0;
}
