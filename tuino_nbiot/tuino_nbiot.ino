/* _____  _____  __  __             _____  _____ 
  / ____||_   _||  \/  |    /\     / ____||_   _|
 | |  __   | |  | \  / |   /  \   | (___    | |  
 | | |_ |  | |  | |\/| |  / /\ \   \___ \   | |  
 | |__| | _| |_ | |  | | / ____ \  ____) | _| |_ 
  \_____||_____||_|  |_|/_/    \_\|_____/ |_____|
  (c) 2017 GIMASI SA                                               

 * tuino_lora.ino
 *
 *  Created on: July 27, 2017
 *      Author: Massimo Santoli
 *      Brief: Example Sketch to use NBIoT GMX-NB Module
 *      Version: 1.0
 *
 *      License: it's free - do whatever you want! ( provided you leave the credits)
 *
 */
 
#include "gmx_nbiot.h"


long int timer_period_to_tx = 20000;
long int timer_millis_tx = 0;

void setup() {

  String version;
  byte join_status;
  int join_wait=0;

  
  Serial.begin(9600);
  Serial.println("Starting");

  // Init NB IoT board
  gmxNB_init("1.1.1.1","9200",NULL);

  gmxNB_getVersion(version);
  Serial.println("GMXNB Version:"+version);

  gmxNB_getIMEI(version);
  Serial.println("GMXNB IMEI:"+version);

  gmxNB_startDT(); 

  while((join_status = gmxNB_isNetworkJoined()) != NB_NETWORK_JOINED) {
    gmxNB_Led2(GMXNB_LED_ON);
    delay(500);
    gmxNB_Led2(GMXNB_LED_OFF);
    Serial.print("Waiting to connect:");
    Serial.println(join_wait);
    join_wait++;
    
    delay(2500);  
  }
  
  Serial.println("Connected!!!");
  gmxNB_Led2(GMXNB_LED_ON);
}

void loop() {

  long int delta_tx;


  delta_tx = millis() - timer_millis_tx;

  if ( delta_tx > timer_period_to_tx) {
    Serial.println("TX DATA");
    gmxNB_TXData("313233");
   
    timer_millis_tx = millis();

    // flash LED
    gmxNB_Led3(GMXNB_LED_ON);
    delay(200);
    gmxNB_Led3(GMXNB_LED_OFF);
    
   }

}
