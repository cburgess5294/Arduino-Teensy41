/*
 * Teensy 4.0 Triple CAN Demo
 * 
 * For use with:
 * http://skpang.co.uk/catalog/teensy-40-triple-can-board-include-teensy-40-p-1575.html
 * 
 * can1 and can2 are CAN2.0B
 * can3 is CAN FD
 * 
 * Ensure FlexCAN_T4 is installed first
 * https://github.com/tonton81/FlexCAN_T4
 * 
 * www.skpang.co.uk October 2019 
 * 
 */
#include <FlexCAN_T4.h>

FlexCAN_T4FD<CAN3, RX_SIZE_256, TX_SIZE_16> FD;  // can3 port
FlexCAN_T4<CAN1, RX_SIZE_256, TX_SIZE_16> can1;  // can1 port 

int led = 13;
IntervalTimer timer;
uint8_t d=0;


void setup(void) {
  pinMode(led, OUTPUT);
  pinMode(34, OUTPUT);
  pinMode(35, OUTPUT);
  digitalWrite(led,HIGH);
  Serial.begin(115200); delay(1000);
  Serial.println("Teensy 4.0 Triple CAN test");
  digitalWrite(led,LOW);
  digitalWrite(34,LOW);
  digitalWrite(35,LOW);
 

  FD.begin();
  CANFD_timings_t config;
  config.clock = CLK_24MHz;
  config.baudrate =    500000;       // 500kbps arbitration rate
  config.baudrateFD = 2000000;      // 2000kbps data rate
  config.propdelay = 190;
  config.bus_length = 1;
  config.sample = 75;
  FD.setRegions(64);
  FD.setBaudRateAdvanced(config, 1, 1);
  FD.onReceive(canSniff);
  FD.setMBFilter(ACCEPT_ALL);
  FD.setMBFilter(MB13, 0x1);
  FD.setMBFilter(MB12, 0x1, 0x3);
  FD.setMBFilterRange(MB8, 0x1, 0x04);
  FD.enableMBInterrupt(MB8);
  FD.enableMBInterrupt(MB12);
  FD.enableMBInterrupt(MB13);
  FD.enhanceFilter(MB8);
  FD.enhanceFilter(MB10);
  FD.distribute();
  FD.mailboxStatus();
  

  can1.begin();
  can1.setBaudRate(500000);     // 500kbps data rate
  can1.enableFIFO();
  can1.enableFIFOInterrupt();
  can1.onReceive(FIFO, canSniff20);
  can1.mailboxStatus();

  timer.begin(sendframe, 500000); // Send frame every 500ms 
}

void sendframe()
{
  
  CAN_message_t msg2;
  msg2.id = 0x401;
  
  for ( uint8_t i = 0; i < 8; i++ ) {
    msg2.buf[i] = i + 1;
  }
  
  msg2.id = 0x402;
  msg2.buf[1] = d++;
  can1.write(msg2);       // write to can1
  
  CANFD_message_t msg;
  msg.len = 64;
  msg.id = 0x4fd;
  msg.seq = 1;
  for ( uint8_t i = 0; i < 64; i++ ) {
    msg.buf[i] = i + 1;
  }
  msg.buf[0] = d;
  FD.write( msg);         // write to can3

}


void canSniff(const CANFD_message_t &msg) {
  Serial.print("ISR - MB "); Serial.print(msg.mb);
  Serial.print("  OVERRUN: "); Serial.print(msg.flags.overrun);
  Serial.print("  LEN: "); Serial.print(msg.len);
  Serial.print(" EXT: "); Serial.print(msg.flags.extended);
  Serial.print(" TS: "); Serial.print(msg.timestamp);
  Serial.print(" ID: "); Serial.print(msg.id, HEX);
  Serial.print(" Buffer: ");
  for ( uint8_t i = 0; i < msg.len; i++ ) {
    Serial.print(msg.buf[i], HEX); Serial.print(" ");
  } Serial.println();
}

void canSniff20(const CAN_message_t &msg) { // global callback
  Serial.print("T4: ");
  Serial.print("MB "); Serial.print(msg.mb);
  Serial.print(" OVERRUN: "); Serial.print(msg.flags.overrun);
  Serial.print(" BUS "); Serial.print(msg.bus);
  Serial.print(" LEN: "); Serial.print(msg.len);
  Serial.print(" EXT: "); Serial.print(msg.flags.extended);
  Serial.print(" REMOTE: "); Serial.print(msg.flags.remote);
  Serial.print(" TS: "); Serial.print(msg.timestamp);
  Serial.print(" ID: "); Serial.print(msg.id, HEX);
  Serial.print(" IDHIT: "); Serial.print(msg.idhit);
  Serial.print(" Buffer: ");
  for ( uint8_t i = 0; i < msg.len; i++ ) {
    Serial.print(msg.buf[i], HEX); Serial.print(" ");
  } Serial.println();
}



void loop() {
  
  CANFD_message_t msg;
  can1.events();

  FD.events(); /* needed for sequential frame transmit and callback queues */
  if (  FD.readMB(msg) ) {
      Serial.print("MB: "); Serial.print(msg.mb);
      Serial.print("  OVERRUN: "); Serial.print(msg.flags.overrun);
      Serial.print("  ID: 0x"); Serial.print(msg.id, HEX );
      Serial.print("  EXT: "); Serial.print(msg.flags.extended );
      Serial.print("  LEN: "); Serial.print(msg.len);
      Serial.print("  BRS: "); Serial.print(msg.brs);
      Serial.print("  EDL: "); Serial.print(msg.edl);
      Serial.print(" DATA: ");
      for ( uint8_t i = 0; i <msg.len ; i++ ) {
        Serial.print(msg.buf[i]); Serial.print(" ");
      }
      Serial.print("  TS: "); Serial.println(msg.timestamp);
   }
}
