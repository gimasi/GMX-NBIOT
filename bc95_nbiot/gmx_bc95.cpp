
#include "at_client.h"
#include "gmx_bc95.h"

#define DEBUG 1

char operator_apn[256];
char operator_code[32];

char tmp_string[128];
char data_payload[256];


void _log(String data )
{
  #ifdef DEBUG
    Serial.println(data);
  #endif
}

void _strip_all_newlines(char *data )
{
  
  String _temp= String(data);

  _temp.replace("\n","");
  _temp.replace("\r","");
  
  _temp.toCharArray(data,_temp.length()+1);

}


void _resetGMX()
{

  pinMode(GMX_RESET,OUTPUT);
  
  // Reset 
  digitalWrite(GMX_RESET,HIGH);
  delay(200);
  digitalWrite(GMX_RESET,LOW);
  delay(500);
  digitalWrite(GMX_RESET,HIGH);
}

void _GPIOInit()
{
    pinMode(GMX_GPIO1,OUTPUT);
    pinMode(GMX_GPIO2,OUTPUT);
    pinMode(GMX_GPIO3,OUTPUT);
    pinMode(GMX_GPIO4,OUTPUT);
    pinMode(GMX_GPIO5,OUTPUT);

    // RESET PIN SETUP
    pinMode(GMX_RESET,OUTPUT);
    digitalWrite(GMX_RESET,LOW);
    
    pinMode(GMX_GPIO4,LOW);
    
}


void _LedBootFlash()
{
    digitalWrite(GMX_GPIO1,LOW);
    digitalWrite(GMX_GPIO2,LOW);
    digitalWrite(GMX_GPIO3,LOW);
  
    digitalWrite(GMX_GPIO1,HIGH);
    delay(500);
    digitalWrite(GMX_GPIO1,LOW);
        
    digitalWrite(GMX_GPIO2,HIGH);
    delay(500);
    digitalWrite(GMX_GPIO2,LOW);
    
    digitalWrite(GMX_GPIO3,HIGH);
    delay(500);
    digitalWrite(GMX_GPIO3,LOW);
    
    digitalWrite(GMX_GPIO1,HIGH);
    delay(500);
    digitalWrite(GMX_GPIO1,LOW);
    
    digitalWrite(GMX_GPIO2,HIGH);
    delay(500);
    digitalWrite(GMX_GPIO2,LOW);
    
    digitalWrite(GMX_GPIO3,HIGH);
    delay(500); 
    digitalWrite(GMX_GPIO3,LOW);
  
    digitalWrite(GMX_GPIO1,LOW);
    digitalWrite(GMX_GPIO2,LOW);
    digitalWrite(GMX_GPIO3,LOW); 
}



uint8_t gmxBC95_init( char *_apn,char *_code) 
{
   
    _log("GMX BC95 Init");

    _GPIOInit();
    _LedBootFlash();
    
    delay(500);
   
    // Init AT Interface
    at_serial_init(&Serial1, 9600);
    _resetGMX();

    if ( at_read_for_response_single("Neul",GMX_BC95_BOOT_TIMEOUT) != 0 )
    {
      return GMX_BC95_KO;
    }

    _log("Boot OK !!!");

    at_send_command("AT");

    if ( at_read_for_response_single("OK",GMX_BC95_DEFAULT_TIMEOUT) != 0 )
    {
      return GMX_BC95_KO;
    }

    _log("AT OK !!!");

     
     strcpy(operator_apn,_apn);
     strcpy(operator_code,_code);

     // Delay for SIM reading
     delay(1000);
     
     return GMX_BC95_OK;
}



uint8_t gmxBC95_getIMEI(char *data, uint8_t max_size)
{
  
  at_send_command("AT+CGSN=1");
  if ( at_read_for_response_single( ":",  GMX_BC95_DEFAULT_TIMEOUT) != AT_COMMAND_SUCCESS )
    return GMX_BC95_KO;

  if ( at_read_dual_and_copy_to_buffer(data,"OK", "ERROR", max_size,GMX_BC95_DEFAULT_TIMEOUT) != AT_COMMAND_SUCCESS) 
    return GMX_BC95_KO;

    
   _strip_all_newlines( data );

  return GMX_BC95_OK;
}

uint8_t gmxBC95_getIMSI(char *data, uint8_t max_size)
{
  delay(1000);  
  at_send_command("AT+CIMI");

  if ( at_read_dual_and_copy_to_buffer(data,"OK", "ERROR", max_size,GMX_BC95_DEFAULT_TIMEOUT) != AT_COMMAND_SUCCESS) 
    return GMX_BC95_KO;

  _strip_all_newlines( data );
  
  return GMX_BC95_OK;
}

uint8_t gmxBC95_start()
{
 
  sprintf(tmp_string,"AT+CFUN=1");
_log(tmp_string);  
  at_send_command(tmp_string);
  if ( read_for_responses_dual("OK","ERROR",GMX_BC95_DEFAULT_TIMEOUT) != 0 )
    return  GMX_BC95_KO;
  
  sprintf(tmp_string,"AT+CGDCONT=1,\"IP\",\"%s\"",operator_apn);
_log(tmp_string);
  at_send_command(tmp_string);
  if ( read_for_responses_dual("OK","ERROR",GMX_BC95_DEFAULT_TIMEOUT) != 0 )
    return  GMX_BC95_KO;

  return  GMX_BC95_OK;    
}

uint8_t gmxBC95_setBand(uint8_t band)
{
  sprintf(tmp_string,"AT+NBAND=%d",band);
  at_send_command(tmp_string);

  if ( read_for_responses_dual("OK","ERROR",GMX_BC95_DEFAULT_TIMEOUT) != 0 )
    return  GMX_BC95_KO;    
 
  return GMX_BC95_OK;
}

uint8_t gmxBC95_attach()
{

  at_send_command("AT+CGATT=1");

  if ( read_for_responses_dual("OK","ERROR",GMX_BC95_LONG_TIMEOUT) != 0 )
    return  GMX_BC95_KO;    
 
  return GMX_BC95_OK;
}

uint8_t gmxBC95_setOperator()
{

  sprintf(tmp_string,"AT+COPS=1,2,\"%s\"",operator_code);

  at_send_command(tmp_string);
  if ( read_for_responses_dual("OK","ERROR",GMX_BC95_DEFAULT_TIMEOUT) != 0 )
    return  GMX_BC95_KO;
 
  return GMX_BC95_OK;
}


uint8_t gmxBC95_isNetworkAttached(int *attach_status)
{
  char data[16];
  int network_status;
  
  at_send_command("AT+CEREG?");

  if ( at_read_for_response_single( ":", GMX_BC95_DEFAULT_TIMEOUT) != AT_COMMAND_SUCCESS )
    return GMX_BC95_KO;

  memset(data,0,sizeof(data));
  
  if ( at_copy_serial_to_buffer(data, ',', 16,  GMX_BC95_DEFAULT_TIMEOUT) != AT_COMMAND_SUCCESS )
    return GMX_BC95_KO;

  if ( at_copy_serial_to_buffer(data, '\r', 16,  GMX_BC95_DEFAULT_TIMEOUT) != AT_COMMAND_SUCCESS )
    return GMX_BC95_KO;

  *attach_status = atoi(data);

  if ( read_for_responses_dual("OK","ERROR",GMX_BC95_DEFAULT_TIMEOUT) != 0 )
    return  GMX_BC95_KO;   

  return GMX_BC95_OK;
}


uint8_t gmxBC95_signalQuality(int *rssi, int *ber )
{
  char data[16];
  char *pch;
  
  at_send_command("AT+CSQ");

  memset(data,0,sizeof(data));
  
  if ( at_read_for_response_single( ":", GMX_BC95_DEFAULT_TIMEOUT) != AT_COMMAND_SUCCESS )
    return GMX_BC95_KO;

  memset(data,0,sizeof(data));
  if ( at_copy_serial_to_buffer(data, '\r', 16,  GMX_BC95_DEFAULT_TIMEOUT) != AT_COMMAND_SUCCESS )
    return GMX_BC95_KO;
    
  _strip_all_newlines( data );

  // we have 2 items
  pch = strtok (data,",");
  
  if (pch!=NULL)
  {
    *rssi = atoi( pch );
    pch = strtok (NULL,",");

    if ( pch != NULL )
    {
      *ber = atoi( pch );

      return GMX_BC95_OK;
    }
  }

   return  GMX_BC95_KO; 
}

uint8_t gmxBC95_SocketOpen(uint32_t port_number, uint8_t *socket_num)
{
  char data[16];

  sprintf(tmp_string,"AT+NSOCR=DGRAM,17,%d,1",port_number);

  at_send_command(tmp_string);

  if ( at_copy_serial_to_buffer(data, 'O', 16,  GMX_BC95_DEFAULT_TIMEOUT) != AT_COMMAND_SUCCESS )
        return  GMX_BC95_KO;   

  _strip_all_newlines( data );
  
  *socket_num = atoi( data );
  
  return GMX_BC95_OK;
}


uint8_t gmxBC95_SocketClose(uint8_t socket_num)
{
  
  sprintf(tmp_string,"AT+NSOCL=%d",socket_num);

  at_send_command(tmp_string);

  if ( read_for_responses_dual("OK","ERROR",GMX_BC95_DEFAULT_TIMEOUT) != 0 )
    return  GMX_BC95_KO;    
 
  return GMX_BC95_OK;
}


uint8_t gmxBC95_TXData(uint8_t socket_num, char *remote_addr, uint32_t port, char *data, int data_len )
{

  char tmp1[64];
  char hex_string[3];
  uint16_t i;

  sprintf(data_payload,"");
  
  for(i=0;i<data_len;i++)
  {
    sprintf(hex_string,"%02X",data[i]);
    strcat(data_payload,hex_string);
  } 

  sprintf(tmp1,"AT+NSOST=%d,%s,%d",socket_num,remote_addr,port);
  sprintf(tmp_string,"%s,%d,%s",tmp1,data_len,data_payload);

  at_send_command(tmp_string);
 
  if ( read_for_responses_dual("OK","ERROR",GMX_BC95_DEFAULT_TIMEOUT) != 0 )
    return  GMX_BC95_KO; 

     
  return GMX_BC95_OK;
}




uint8_t gmxBC95_TXRXData(uint8_t socket_num, char *remote_addr, uint32_t port, char *data, int data_len, char *rx_buffer, int rx_buffer_len, int *rx_received_len )
{

  uint16_t rx_socket,rx_len;
  char *pch;
  char rx_data[16];

  if ( gmxBC95_TXData( socket_num,  remote_addr,  port, data, data_len ) != GMX_BC95_OK )
     return GMX_BC95_KO;

  /* Wait for RX */
  sprintf(rx_buffer,"");
  *rx_received_len = 0;
  
 if ( at_read_for_response_single("+NSONMI:",2*GMX_BC95_READ_TIMEOUT) != 0 )
    return  GMX_BC95_OK; 

  memset(rx_data,0,sizeof(rx_data));

  if ( at_copy_serial_to_buffer(rx_data, '\r', 16,  GMX_BC95_DEFAULT_TIMEOUT) != AT_COMMAND_SUCCESS )
    return GMX_BC95_OK;

  Serial.println( rx_data );
  Serial.println("**RX**");
    
  _strip_all_newlines( rx_data );

  // we have 2 items
  pch = strtok (rx_data,",");
  
  if (pch!=NULL)
  {
    rx_socket = atoi( pch );
    pch = strtok (NULL,",");

    if ( pch != NULL )
    {
      rx_len = atoi( pch );
      *rx_received_len = rx_len;
    }
  }

Serial.println(rx_socket);
Serial.println(rx_len);

if ( rx_len > 0 )
{
    sprintf(tmp_string,"AT+NSORF=%d,%d",rx_socket,rx_len);  
    Serial.println( tmp_string );
    at_send_command(tmp_string);

    at_read_for_response_single(",",GMX_BC95_DEFAULT_TIMEOUT);
    at_read_for_response_single(",",GMX_BC95_DEFAULT_TIMEOUT);
    at_read_for_response_single(",",GMX_BC95_DEFAULT_TIMEOUT);
    at_read_for_response_single(",",GMX_BC95_DEFAULT_TIMEOUT);

    if ( at_copy_serial_to_buffer(rx_buffer, ',', rx_buffer_len,  GMX_BC95_DEFAULT_TIMEOUT) != AT_COMMAND_SUCCESS )
      return  GMX_BC95_OK;   

    _strip_all_newlines( rx_buffer);
  
    Serial.println( rx_buffer );
    
}
  
  return GMX_BC95_OK;
}
