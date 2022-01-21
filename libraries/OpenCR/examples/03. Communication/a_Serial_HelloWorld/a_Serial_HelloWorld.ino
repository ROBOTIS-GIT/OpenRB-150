/*
 * Serial_HelloWorld
*/
volatile int nCount=0;

void setup() {
  Serial.begin(57600);
  Serial1.begin(57600);
  Serial2.begin(57600);
  Serial3.begin(57600);
  while(!Serial);
}

void loop() {
  //print "Hello World!!" to PC though USB Virtual COM port
  Serial.println(nCount);
  Serial1.println(nCount);
  Serial2.println(nCount);
  Serial3.println(nCount);
  // Serial.print("nCount : "); // display nCount variable and increase nCount.
  // Serial.println(nCount++);
  nCount++;
  delay(1000);
}


