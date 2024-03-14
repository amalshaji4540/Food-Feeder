#include <ESP32Servo.h>
#include "BluetoothSerial.h"
#include <EEPROM.h>
#include <Ultrasonic.h>
#define EEPROM_SIZE 3
//Bluetooth initialisation
//#define USE_PIN // Uncomment this to use PIN during pairing. The pin is specified on the line below
const char *pin = "1234"; // Change this to more secure PIN.

String device_name = "Food feeder";

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

#if !defined(CONFIG_BT_SPP_ENABLED)
#error Serial Bluetooth not available or not enabled. It is only available for the ESP32 chip.
#endif

BluetoothSerial SerialBT;
String my_str;              //for storing data coming from bluetooth

//Servo intialisation

Servo servo1;
Servo servo2;
Servo servo3;
int sinitialposition1=150;   //initial postion for servos 
int sinitialposition2=30;
int sinitialposition3=40;
int servopos1;             /*default positions of servo where food is delivered                             (servopos1 and servopos2 are global variables which can be varied and accessed from any where in the program)*/
int servopos2;
int servopos3;

int foodloss = sinitialposition2 + 15;

//Ultrasonic initialisation

#define TRIGGER_PIN 32
#define ECHO_PIN 33
Ultrasonic ultrasonic(TRIGGER_PIN, ECHO_PIN);

// Storing in eeprom


void setup() {
  
  Serial.begin(115200);
  SerialBT.begin(device_name); //Bluetooth device name
  Serial.printf("The device with name \"%s\" is started.\nNow you can pair it with Bluetooth!\n", device_name.c_str());
  //Serial.printf("The device with name \"%s\" and MAC address %s is started.\nNow you can pair it with Bluetooth!\n", device_name.c_str(), SerialBT.getMacString()); // Use this after the MAC method is implemented
  #ifdef USE_PIN
    SerialBT.setPin(pin);
    Serial.println("Using PIN");
  #endif


  servo1.attach(14);
  servo2.attach(27); 
  servo3.attach(25);
   
  servo1.write(sinitialposition1);          //moving servos to initial positions
  delay(50); 
  servo2.write(sinitialposition2);
  delay(50); 
  servo3.write(sinitialposition3);
  delay(50); 
   
      //for serial communication
      //For Eeprom
EEPROM.begin(EEPROM_SIZE);


}

void loop() 
{
   servopos1 = EEPROM.read(0);             /*default positions of servo where food is delivered                             (servopos1 and servopos2 are global variables which can be varied and accessed from any where in the program)*/
   servopos2 = EEPROM.read(1);
   servopos3 = EEPROM.read(2);
  initial(0);               /*call intial function for bringing servo to initial position.it is a user defined function.
                                  if we pass 0 as argument servo comes to intial position and set fuction is called
                                if we pass 1 as argument servo comes to initial position and reset function is called*/
}


void initial(int flag)  /*rotate servos to initial positions. 
                        if we pass 0 as argument servo comes to intial position and set function is called
   if we pass 1 as argument servo comes to initial position and reset function is called*/
{  
  Serial.println("initial function is called");
  rotateservo(sinitialposition1,1);
  rotateservo(sinitialposition2,2);
  rotateservo(sinitialposition3,3);
  
  if(flag==0)
  {
    set(servopos1,servopos2);
  }
  else if(flag==1)
  {
    reset();
  }
  else if (flag==2) {

    while(true) {

    }
  }
}

void set(int pos1,int pos2)    /*function for rotating servo repeatedly in loop.
                               First argument is servo 1 position,second argument is servo 2 position*/
{ 
  while(1)
  {
    Serial.println("set function is called");
   
    rotateservo(servopos3,3);   //rotate 3rd servo 90 degree(taking food)
    delay(500);

    rotateservo(pos2,2); /*rotate 2nd servo pos2 degree (moving food vertically up) 3rd servo also rotate with 2nd servo for keeping spoon straight*/
    delay(1000);

    rotateservo(pos1,1); //rotate 1st servo pos1 degree (moving food towards user)
    delay(4000);         //delay for taking food by patient
     
    rotateservo(sinitialposition1,1); //moving servo 1 to initial position
    delay(1000);
    rotateservo(foodloss,2); //moving servo 2 to intial position
    delay(1000);
    rotateservo(sinitialposition3,3); //moving servo 3 to initial position
    delay(1000);
    rotateservo(sinitialposition2,2); //moving servo 2 to initial position
    delay(1000);
  }    
}

void reset()                //function for resetting servo positions(user defined function).The postions are taken from mobile app
{ 
  Serial.println("reset function is called");
  while(1)
  {  
    if(SerialBT.available())              //checking data is available from blutooth
    {
      my_str=SerialBT.readString();          // reading data from blutooth as string
      if(my_str.startsWith("s1"))        //this condition is true when slider one is moved in mobile application
      {
         my_str = my_str.substring(2, my_str.length());   // Extract only the number.
         int servopos1temp=my_str.toInt();
         servopos1= 180-servopos1temp;                 // Convert the string into integer  
         Serial.println(servopos1);        
         rotateservo(servopos1,1);                        /*calling rotateservo function for rotation servo (user defined function).
                                                         first argument is angle and second argument is servo number */
      }
      else if(my_str.startsWith("s2"))                    //this condition is true when slider two is moved in mobile application
      {
         my_str = my_str.substring(2, my_str.length());  // Extract only the number.
         servopos2 = my_str.toInt();                     // Convert the string into integer  
         Serial.println(servopos2);        
         rotateservo(servopos2,2);
      }
      else if(my_str.startsWith("s3"))                    //this condition is true when slider two is moved in mobile application
      {
         my_str = my_str.substring(2, my_str.length());  // Extract only the number.
         servopos3 = my_str.toInt();                     // Convert the string into integer  
         Serial.println(servopos3);        
         rotateservo(servopos3,3);
      }
      else if(my_str.startsWith("SET"))                   //this condition is true when set button is pressed in mobile application
      {
         Serial.println(my_str);
         EEPROM.write(0,servopos1);
         delay(10);
         EEPROM.write(1,servopos2);
         delay(10);
         EEPROM.write(2,servopos3);
         delay(10);
         EEPROM.commit();
         initial(0);                                     //initial function is called    
      }
    }
  }
}           

int pause(){
  Serial.println("pause");   
  while(1)
  {
    if(SerialBT.available())            //checking data is available from bluetooth
        {
        my_str=SerialBT.readString();
          if(my_str.startsWith("RUN"))     //if condition became true when run button is pressed in app
          {
             Serial.println("run");      
             return 0;                
          }
        if(my_str.startsWith("RESET"))     //if condition became true when reset button is pressed in app
        {
          Serial.println(my_str);  
          my_str="";    
          initial(1);                 
        }
        if(my_str.startsWith("STOP"))
      {
        initial(2);
         my_str="";
      }
        
        }
  }
}


void rotateservo(int angle,int servo)   /*function for rotate servo.first argument is rotating angle ,second argument is servo number*/
{
  int pos;
  int initialpos3=servo3.read();        /*for knowing current position of servos*/
  int initialpos2=servo2.read();
  int initialpos1=servo1.read();
 
  if(servo==1)                          //rotate servo 1 
  { 
    Serial.println("servo1");
    if(initialpos1<angle)              //checking current position is less than given angle
    {
      for(pos=initialpos1;pos<=angle;pos++) 
      {
          scan();
          usscan();
          servo1.write(pos);
          Serial.println(pos);
          delay(25);  
      } 
    }
    else if(initialpos1>angle)         //checking current position greater than given angle
    {
      for(pos=initialpos1;pos>=angle;pos--)
      {
          scan();
          usscan();
          servo1.write(pos);
          Serial.println(pos);
          delay(25);
      }
    }
  }
  else if(servo==2)                    //rotate servo 1 and 2 at a time (for keeping spoon parallel to horizontal axis)
  {
    Serial.println("servo2");
    if(initialpos2<angle)
    {
      for(pos=initialpos2;pos<=angle;pos++)
      {   
        
          initialpos3=initialpos3-1;
          servo2.write(pos);
          scan();          //for rotating servo 2
          Serial.println(pos);
          servo3.write(initialpos3);  //for rotating servo 3
          delay(30);  
      } 
    }
    else if(initialpos2>angle)
    {
      for(pos=initialpos2;pos>=angle;pos--)
      {   
          scan();
          initialpos3=initialpos3+1;
          servo2.write(pos);
          Serial.println(pos);
          servo3.write(initialpos3);
          delay(30);
      }
    }

   }
    else if(servo==3)                          //rotate servo 3 alone 
  {
    Serial.println("servo3");
    if(initialpos3<angle)
    {
      for(pos=initialpos3;pos<=angle;pos++)
      {   
       
          servo3.write(pos);
          scan();
          Serial.println(pos);
          delay(30);  
      } 
    }
    else if(initialpos3>angle)
    {
      for(pos=initialpos3;pos>=angle;pos--)
      {   
          servo3.write(pos);
          scan();
          Serial.println(pos);
          delay(30);
      }
    }
  }
}


int scan()
{
  
   if(SerialBT.available())            //checking data is available from bluetooth
    {
    my_str=SerialBT.readString();
      if(my_str.startsWith("RESET"))     //if condition became true when reset button is pressed in app
      {
         Serial.println(my_str);  
         my_str="";    
         initial(1);                 
      }
      if(my_str.startsWith("PAUSE"))     //if condition became true when pause button is pressed in app
      {
             pause();
             my_str="";              
      }
      if(my_str.startsWith("STOP"))
      {
        initial(2);
         my_str="";
      }
    
    }
    return (0);
}
int usscan()
  {
  Serial.println("usscan is called");
  long distance = ultrasonic.read();
  Serial.print("Distance: ");
  Serial.println(distance);

  if (distance>22) 
    {
      return(0);
    }
  else 
    {
      delay(1000);
      usscan();
    }
  return(0);
  }
