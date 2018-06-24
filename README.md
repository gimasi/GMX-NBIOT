# GMX-NBIOT
Find in the <b>bc95_nbiot</b> directory the new library for handling the BC96 NBIOT module on the Tuino ( it should work with any Arduino compatible board that has a BC95 connected to a serial port ).<br/>
The library consists of the gmx_bc95 and at_client files, we have added a simple sketch bc95_nbiot.ino to show how to use them.<br/>
<br>

Here a quick 'logical sequence' of the calls 

```c
/*
 * Init
 */
   if ( gmxBC95_init("**** OPERATOR APN ****"," **** OPERATORE CODE *** ") ==  0 )
     _debug("BC95 Init OK!");



  /*
   * Starts the Module
   */
  if (  gmxBC95_start() == 0 ) 
      _debug("BC95 start OK");
  
  
  /*
   * Tries to attach to network
   */
  gmxBC95_attach();



  /* 
   * Waits for Attach
   */

 while ( network_attached == false )
  {
    if (  gmxBC95_isNetworkAttached(&network_status) == 0 )
    {
      if (( network_status == 1 ) || (network_status == 5 ))
      {
        network_attached = true;
        Serial.println("Attached!");
      }
      else
      {
        Serial.println("Not Attached!");
      }
    }

    delay(2000);
  }



  /*
   * Transmit some data
   */

  tx_buffer[0]=0x02;    
  tx_buffer[1]=0x01;
  tx_buffer[2]=0x02;
    
  if (gmxBC95_SocketOpen(__UDP_PORT__,&socket_num)==0)
     Serial.println("Socket Open Ok!");

 
  if ( gmxBC95_TXData(socket_num,"__your_udp_address",_your_udp_port,tx_buffer,3) == 0 )
    Serial.println("Send Ok!");

  if (gmxBC95_SocketClose(socket_num)==0)
    Serial.println("Socket Close Ok!");



```


All function returns 0 or -1 in case of error.
```c
#define GMX_BC95_OK                         0 
#define GMX_BC95_KO                        -1

```



## Initialization Functions
```c
uint8_t gmxBC95_init(char *apn, char *operator_code);
```
Initialization of the library, you need to pass the apn and operator code.

```c
uint8_t gmxBC95_getIMEI(char *data, uint8_t max_size);
```
Returns the IMEI of the module in the *data char buffer, you need to specify the size of the char buffer you are passing.

```c
uint8_t gmxBC95_getIMSI(char *data, uint8_t max_size);
```
Returns the IMSI of the SIM in the *data char buffer, you need to specify the size of the char buffer you are passing. If no SIM is inserted this funxtion will return error.

```c
uint8_t gmxBC95_setBand(uint8_t band);
```
Sets the band of the module. The BC95 is available in fixed and multiband version. Currently we have only the fixed B20 or B8 GMX modules available.<br/>
On a fixed band module this function is useless, if you specify the correct band if will return OK, but doesn't really do anything, it could be used to check on whic band the module is from software.

```c
uint8_t gmxBC95_start();
```
Configures APN and starts modules<br/>

## Attaching to network

```c
uint8_t gmxBC95_attach(void);
uint8_t gmxBC95_setOperator(void);
```

You can use one of these two functions to attach.<br/>
<b>gmxBC5_attach</b> sends an AT+CGATT=1 to the module, while <b>gmxBC95_setOperator</b> forces the operator code for the attach AT+COPS=1,2,"code". It's up to you to decide which one to use. <br/>
<br/>

```c
uint8_t gmxBC95_isNetworkAttached(int *attach_status);
```
This function simply check's if we are attached returning in the attach_status variable this possible values:

* 0     Not registered, MT is not currently searching an operator to register to
* 1     Registered, home network
* 2     Not registered, but MT is currently trying to attach or searching an operator to register to
* 3     Registration denied
* 4     Unknown (e.g. out of E-UTRAN coverage)
* 5     Registered, roaming
* 6     Registered for “SMS only”, home network (not applicable)
* 7     Registered for “SMS only”, roaming (not applicable)
* 8     Attached for emergency bearer services only
* 9     Registered for “CSFB not preferred”, home network (not applicable)
* 10    Registered for “CSFB not preferred”, roaming (not applicable)


## Sending ( and receiving ) Data

```c
uint8_t gmxBC95_SocketOpen(uint32_t port_number, uint8_t *socket_num);
```
Opens UDP socket that can be then used for trasmission. In socket_num you will have the socket number for susccesive calls, usually it is 0.

```c
uint8_t gmxBC95_SocketClose(uint8_t socket_num);
```
Closes the specified socket.

```c
uint8_t gmxBC95_TXData(uint8_t socket_num, char *remote_addr, uint32_t port, char *data, int data_len );
```
Transmits data on the specified socket, to the remote_addr IP addres, on the specified port.<br/>
the data buffer must contain a binary sequence of bytes, and data_len is the lenght of the bytes to tranmsit.

```c
uint8_t gmxBC95_TXRXData(uint8_t socket_num, char *remote_addr, uint32_t port, char *data, int data_len, char *rx_buffer, int rx_len, int *rx_buffer_len );
```
Same as TXData that this function waits for a RX packet coming from the server. It will wait up to 5 seconds ( you can tweak this in the code).<br/>
The TX parameters are identical to the gmxBC95_TXData function. If something is received the rx_len value will be different than 0.<br/>
rx_buffer is the data buffer that will contain hexadecimal strings of the the received paylaod, rx_buffer_len is the max len of this buffer.<br/>

## Utility Functions

```c
uint8_t gmxBC95_signalQuality(int *rssi, int *ber );
```
Returns ths radio signal level, here the extract from BC95 manual<br/>
<b>rssi</b>  Integer type<br/>
* 0 -113dBm or less
* 1 -111dBm
* 2...30 -109dBm... -53dBm
* 31 -51dBm or greater
* 99 Not known or not detectable
<br/>

<b>ber</b> Integer type; channel bit error rate (in percent)<br/>
* 0...7 As RXQUAL values (please refer to 3GPP specifications)
* 99 Not known or not detectable


# Payload Encoding when sending data to GIoTTY
To send data to our IoT cloud ( GIoTTY) we have implemented a very simple protocol that consists in sending the IMEI of the module, followed by the actual payload.<br>
We format the IMEI by adding a leading zero so it becomes a 16 digit number, and we treat it as an hexadecimal string, so transforming it in a 8 byte value<br/>

```c

	sprintf(imei_buffer,"0%s",IMEI);
 
    for (int i=0;i<8;i++)
    {
        uint8_t high,low;

        high = (imei_buffer[i*2]-0x30) * 16;
        low =  (imei_buffer[(i*2)+1]-0x30);

       devAddress[i] = high + low;
   }
```

we then prepare the payload by adding the lenght of the payload and the actual paylaod.

```c
	 memcpy(tx_buffer,devAddress,8);
     tx_buffer[8]=0x02;    // lenght if payload
     tx_buffer[9]=0x01;
     tx_buffer[10]=0x02;
    
```

# Receiving data

(coming soon)



<br/>
<br/>
<img src="/docs/gmx-nbiot.jpg"/>

<br/>
<br/>

We have left the very old ( but working ) version tuino_nbiot for anybody which is  still using it.<br/>

