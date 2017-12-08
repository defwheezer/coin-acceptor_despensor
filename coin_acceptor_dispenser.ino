const int COIN_PIN = 3; // use one of the arduino interrupt pins (0 = pin 2, 1 = pin 3)
const int coinHopperPin = 2; // use one of the arduino interrupt pins (0 = pin 2, 1 = pin 3)
const int ledPin = 13;  // Add if needed to PWM pin for LED level adj.

//coin hopper stuff
int relayPin = 12;  // solid state relay for cube hopper 12v motor
bool relay = LOW;  // motor off
int counter = 0;
int count;

//save strings to ROM to save SRAM, failsafe for coins entered
#include <avr/pgmspace.h>
char const coin_0[] PROGMEM = "Received Penny"; // "Received Penny" etc are strings to store - change to suit.
char const coin_1[] PROGMEM = "Received  Nickel";
char const coin_2[] PROGMEM = "Received  Dime";
char const coin_3[] PROGMEM = "Received Quarter";
char const coin_4[] PROGMEM = "Unknown Coin";
// Then set up a table to refer to your strings.
PGM_P const string_table[] PROGMEM = // change "string_table" name to suit
{
  coin_0,
  coin_1,
  coin_2,
  coin_3,
  coin_4, };
  
char buffer[30]; // make sure this is large enough for the largest string it must hold

int totalMoneyOut = 0;
int totalMoneyIn = 0;

int ledState = LOW; 


// total amount of money collected;
int money = 0;
int prevMoney = 0;
// gets incremented by the ISR;
// gets reset when coin was recognized (after train of pulses ends);
volatile int pulses = 0;
volatile long timeLastPulse = 0;

//coin hopper
volatile long pulseStartTime;
volatile long pulseStopTime;

void setup() {
  // Debugging output
  Serial.begin(115200);
 //coin acceptor
  pinMode(COIN_PIN, INPUT);
  attachInterrupt(1, coinISR, RISING);  // COIN wire connected to PIN 'COIN_PIN' = interrupt 1;
  Serial.println("Initializations done.");
  Serial.println("Ready to accept coins.");
  // coin hopper coin sensor is high when no coin, goes low when coin present
  // count coin at fall from 5v, then stop motor when coin clears (sensor raises back to 5v)
  attachInterrupt(0, countCoins, FALLING); // count when falling from 5v
  digitalWrite(coinHopperPin, INPUT_PULLUP); //set internal pull up resistor
  pinMode(relayPin, OUTPUT); //relay for hopper motor

}

// executed for every pulse;
void coinISR()
{
  pulses++;
  timeLastPulse = millis();
}

void countCoins() {
    pulseStartTime = millis();
    counter++;
}

void payOut(int coins) {
 while (coins > counter) {
      digitalWrite(relayPin, HIGH);  //start motor
      relay = HIGH; //motor is on
      }
      
      delay(50); // clear coin from shoot
      digitalWrite(relayPin, LOW);  //stop motor
      relay = LOW; //motor is off
      counter = 0;  // reset number of coins per cycle
      totalMoneyIn = 0;
}      

void loop()
{
  //used to check if we have any coins inserted
  long timeFromLastPulse = millis() - timeLastPulse;
  //Serial.println(timeFromLastPulse);
  //only do if done detecting pulses for a while (usere done entering coins)
  if(timeFromLastPulse > 2000 && totalMoneyIn != 0) { 
     Serial.print("Total In ");
     Serial.println(totalMoneyIn);
     totalMoneyOut = (totalMoneyIn/25); //convert to quarters
     Serial.print("Total Out ");
     Serial.println(totalMoneyOut);
     payOut(totalMoneyOut);
  }
  
  if (pulses > 0 && timeFromLastPulse > 250) {
    int moneyIn = 0;
    // sequence of pulses stopped; determine the coin type;
    if (pulses == 1)   {
      //Serial.println("1 pulse (misread?)");
      //ignore errant pulse
      moneyIn = 0;
    }
    if (pulses == 2)   {
      Serial.println("Received penny");
      moneyIn = 1;
    }
    else if (pulses == 4)   {
      Serial.println("Received Nickel");
      moneyIn = 5;
    }
    else if (pulses == 6)   {
      Serial.println("Received dime");
      moneyIn = 10;
    }
    else if (pulses == 7)   {
      //Serial.println("Received quarter (misread as 7 pulses)");
      Serial.println("Received quarter");
      moneyIn = 25;
    }
    else if (pulses == 8)   {
      Serial.println("Received quarter");
      moneyIn = 25;
    }
    else  {
      //Serial.print("ERROR - Unknown coin: ");
      //Serial.print(pulses);
      //Serial.println(" pulses");
    }
    
    Serial.print("Adding: ");
    Serial.println(moneyIn);
    pulses = 0;
    totalMoneyIn = totalMoneyIn + moneyIn;
  }
}
