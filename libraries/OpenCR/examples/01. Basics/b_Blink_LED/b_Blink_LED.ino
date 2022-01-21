/*
 * Blink(LED)
 */

/*
#define BDPIN_LED_USER_1        22
#define BDPIN_LED_USER_2        23
#define BDPIN_LED_USER_3        24
#define BDPIN_LED_USER_4        25
 */

int led_pin = 8;

void setup() {
  // Set up the built-in LED pin as an output:
  pinMode(led_pin, OUTPUT);
  Serial.begin(115200);

}

void loop() {

  digitalWrite(led_pin, HIGH);  // set to as HIGH LED is turn-off
  delay(100);                   // Wait for 0.1 second
  digitalWrite(led_pin, LOW);   // set to as LOW LED is turn-on
  delay(100);                   // Wait for 0.1 second
}

