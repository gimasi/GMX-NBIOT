#ifndef _GMX_BC95_H_
#define _GMX_BC95_H_

#include <Arduino.h>

#define GMX_BC95_OK                         0 
#define GMX_BC95_KO                        -1


#define GMX_BC95_DEFAULT_TIMEOUT         1000
#define GMX_BC95_READ_TIMEOUT            3000
#define GMX_BC95_BOOT_TIMEOUT            6000
#define GMX_BC95_LONG_TIMEOUT           30000



uint8_t gmxBC95_init(char *apn, char *operator_code);
uint8_t gmxBC95_getIMEI(char *data, uint8_t max_size);
uint8_t gmxBC95_getIMSI(char *data, uint8_t max_size);
uint8_t gmxBC95_setBand(uint8_t band);
uint8_t gmxBC95_start();
uint8_t gmxBC95_attach(void);
uint8_t gmxBC95_setOperator(void);
uint8_t gmxBC95_isNetworkAttached(int *attach_status);
uint8_t gmxBC95_signalQuality(int *rssi, int *ber );


uint8_t gmxBC95_SocketOpen(uint32_t port_number, uint8_t *socket_num);
uint8_t gmxBC95_SocketOpen(uint32_t port_number);
uint8_t gmxBC95_SocketClose(uint8_t socket_num);

uint8_t gmxBC95_TXData(uint8_t socket_num, char *remote_addr, uint32_t port, char *data, int data_len );
uint8_t gmxBC95_TXRXData(uint8_t socket_num, char *remote_addr, uint32_t port, char *data, int data_len, char *rx_buffer, int rx_len, int *rx_buffer_len );


#endif 
