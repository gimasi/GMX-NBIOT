
#include "gmx_bc95.h"

char tx_buffer[32];
char rx_buffer[256];

  
#define DEBUG 1

void _debug_init()
{
  Serial.begin(9600);
}

void _debug(char *string )
{
  Serial.println(string);
}

unsigned char IMEI[32];
unsigned char IMSI[32];
bool network_attached;

long int timer_period_to_tx = 20000;
long int timer_millis_tx = 0;

unsigned char devAddress[8] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
unsigned char SendBuffer[256];

char debug_string[128];
char debug_payload[256];
char hex_string[3];
uint16_t i;

#define OPERATOR_APN    "___YOUR_OPERATOR_APN___"
#define OPERATOR_CODE   "___YOUR_OPERATOR_CODE___"
#define UDP_IP_ADDRESS  "1.1.1.1"
#define UDP_PORT        1234
  

void setup() {
  uint8_t socket_num;
  
    // put your setup code here, to run once:

    _debug_init();
    _debug("Starting");

    if ( gmxBC95_init(OPERATOR_APN,OPERATOR_CODE) ==  0 )
     _debug("BC95 Init OK!");
    
   
    delay(2000);

    if ( gmxBC95_getIMEI( IMEI, 32 ) == 0 )
    {
      unsigned char imei_buffer[32];
      
      _debug("IMEI_OK");
      _debug(IMEI);

       sprintf(imei_buffer,"0%s",IMEI);
 
      for (int i=0;i<8;i++)
      {
        uint8_t high,low;

        high = (imei_buffer[i*2]-0x30) * 16;
        low =  (imei_buffer[(i*2)+1]-0x30);

       devAddress[i] = high + low;
      }

    } 
    
    if ( gmxBC95_getIMSI( IMSI, 32 ) == 0 )
    {
      _debug("IMSI_OK");
      _debug(IMSI);
    } 
    else
    {
      _debug("IMSI_KO");
    }

    if (  gmxBC95_start() == 0 ) 
    {
      _debug("BC95 start OK");
    } 

    gmxBC95_attach();

    network_attached = false;

}

void loop() {
  // put your main code here, to run repeatedly:
  int network_status;
  int rssi,ber;
  long int delta_tx;

  uint8_t socket_num;
  int received_len;
  

  delta_tx = millis() - timer_millis_tx;

  if (( delta_tx > timer_period_to_tx) && network_attached )
  {
   
    
      Serial.println("TX DATA");
      memcpy(tx_buffer,devAddress,8);
      tx_buffer[8]=0x02;    // => num of bytes
      tx_buffer[9]=0x01;
      tx_buffer[10]=0x02;
    
    if (gmxBC95_SocketOpen(9200,&socket_num)==0)
      Serial.println("Socket Open Ok!");

 
    if ( gmxBC95_TXRXData(socket_num,UDP_IP_ADDRESS,UDP_PORT,tx_buffer,11,rx_buffer,256,&received_len) == 0 )
      Serial.println("Send Ok!");

    if (gmxBC95_SocketClose(socket_num)==0)
      Serial.println("Socket Close Ok!");

     timer_millis_tx = millis();

     if ( gmxBC95_signalQuality(&rssi,&ber) == 0 )
     {
        Serial.print("RSSI:");
        Serial.println(rssi);

        Serial.print("BER:");
        Serial.println(ber);
     }


     // We check if we are still attached
     
    if (  gmxBC95_isNetworkAttached(&network_status) == 0 )
    {
      if (( network_status == 1 ) || (network_status == 5 ))
        network_attached = true;
      else
        network_attached = false;
    }
  }


  if ( network_attached == false )
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

    delay(5000);
  }
}
