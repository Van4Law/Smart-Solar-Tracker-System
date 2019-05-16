#include <SD.h> // підключення бібліотеки керування мікропроцесором модуля запису даних 
#include <Servo.h>  // підключення бібліотеки керування серводвигунами
#include <SPI.h>  // підключення бібліотеки
#include <RTC.h> // підключення бібліотеки керування мікросхемою модуля реального часу DS3231
#include <Thread.h> // підключення бібліотеки створення віртуальних потоків
#include <ThreadController.h> // підключення бібліотеки керування віртуальними потоками
RTC    time;  // ініціалізація модуля реального часу DS3231
const int CS_PIN = 4; // ініціалізація виводу «CS» модуля SD карти до цифрового контакту «4» плати Arduino
long datet; // використання змінної типу long для збереження значень  datet розмірністю 32біта(4байта)
float SolarInput = A7; // ініціалізація аналогового контакту «7» на зчитування сигналів від сонячної панелі
float Vin = 0;
Servo horizontal; 
int servoh = 90; // встановлення стартового положення плеча горизонтального сервопривода під кутом 90°          
Servo vertical;   
int servov = 90; // встановлення стартового положення плеча вертикального сервопривода під кутом 90°     
int ldrlt = 0; // ініціалізація верхнього лівого фоторезистора до аналогового контакту «0» плати Arduino
int ldrrt = 1; // ініціалізація верхнього правого фоторезистора до аналогового контакту «1» плати Arduino
int ldrld = 2; // ініціалізація нижнього лівого фоторезистора до аналогового контакту «2» плати Arduino
int ldrrd = 3; // ініціалізація нижнього правого фоторезистора до аналогового контакту «3» плати Arduino
ThreadController controll = ThreadController();
Thread myThread = Thread(); // ініціалізація та створення управління над віртуальними потоками
void setup() {
Serial.begin(9600); // встановлення швидкості передачі даних по послідовному інтерфейсі 9600 бод
Serial.println("Initializing Card"); // вивід інформації через послідовний порт про ініціалізацію SD карти  
pinMode(CS_PIN, OUTPUT);
time.begin(RTC_DS3231); // ініціалізація початку роботи модуля реального часу DS3231       
//time.settime(0,2,14,7,5,19); // встановлення часу модуля DS3231 у форматі (секунди, хвилини, години, день, місяць, рік)
horizontal.attach(9); // ініціалізація цифрового виводу «9» з підтримкою ШІМ для управління горизонтальним серводвигуном 
vertical.attach(10);  // ініціалізація цифрового виводу «10» з підтримкою ШІМ для управління вертикальним серводвигуном
//prepareVoltageMeasuringThread(); // підготовка до роботи віртуального потоку для вимірювання напруги з сонячної панелі
prepareWritingThread(); //підготовка до роботи віртуального потоку для запису даних на SD карту

  if (!SD.begin(CS_PIN))
  {
    Serial.println("Card Failure"); // вивід інформації через послідовний порт про неможливість ініціалізації SD карти 
    return; // припиняє виконання функції і повертає значення з перерваної функції в викликаючу, якщо це потрібно
  }
  Serial.println("Card Ready");
    
    File logFile = SD.open("LOG.csv", FILE_WRITE);
  if (logFile)
  {
    logFile.println(" , "); //Just a leading blank line, incase there was previous data
    String header = "DATE,                VOLTAGE";
    logFile.println(header);
    logFile.close();
    Serial.println(header);
  }
  else
  {
    Serial.println("Couldn't open log file");
  }
}
void prepareWritingThread()
{
  myThread.onRun(WritingThread); // запуск віртуального потоку запису інформації на SD карту
  myThread.setInterval(5000); // встановлення періоду 900с(15хв)  для послідовного запису даних 
  controll.add(&myThread);
}
/*void prepareVoltageMeasuringThread()
{
    myThread.onRun(measureVoltage); // запуск віртуального потоку вимірювання значень напруги з сонячної панелі
    myThread.setInterval(250); // встановлення періоду 1с для послідовного вимірювання значень напруги з сонячної панелі
    controll.add(&myThread);
  }
void measureVoltage()
{
  int Vin = map(analogRead(SolarInput), 0, 1023, 0, 16);// вимірювання значень напруги з сонячної панелі та конвертація їх в формат string
  }*/
void WritingThread()
{

   Vin = float (analogRead(SolarInput)/63.95); 
   String Time = String(time.gettime("d-m-Y, H:i:s"));
   String dataString = String(Time) + ", " + Vin;// запис часу на SD карту  у форматі (секунди, хвилини, години, день, місяць, рік)
 File logFile = SD.open("LOG.csv", FILE_WRITE);
  if (logFile)
  {
    logFile.println(dataString);
    logFile.close();
    Serial.println(dataString);
  }
  else
  {
    Serial.println("ERROR! Couldn't acces file");
  }
  }
  void runAllThreads()
{
    controll.run(); // запуск всіх паралельних віртуальних потоків
}  
void loop()
{
    controll.run();

    int lt = analogRead(ldrlt); // зчитування значень з верхнього лівого фоторезистора
    int rt = analogRead(ldrrt); // зчитування значень з верхнього правого фоторезистора
    int ld = analogRead(ldrld); // зчитування значень з нижнього лівого фоторезистора
    int rd = analogRead(ldrrd); // зчитування значень з нижнього лівого фоторезистора
    int tol = analogRead(6)/4; // зчитування та обробка значень які задають чутливість руху системи
    int avt = (lt + rt) / 2; // визначення середнього значення верхніх фоторезисторів
    int avd = (ld + rd) / 2; // визначення середнього значення нижніх фоторезисторів
    int avl = (lt + ld) / 2; // визначення середнього значення фоторезисторів з лівої сторони
    int avr = (rt + rd) / 2; // визначення середнього значення фоторезисторів з правої сторони
    
    int dvert = avt - avd; // обчислення різниці між верхніми та нижніми значеннями фоторезисторів 
    int dhoriz = avl - avr; // обчислення різниці між значеннями фоторезисторів лівої та правої сторони 
    
    if (-1*tol > dvert || dvert > tol) // зміна кута вертикального серводвигуна якщо різниця значень не є допустимою
    {
        if (avt > avd)
        {
            servov = --servov;
             if (servov > 170) 
             { 
                servov = 170;
             }
        }
        else if (avt < avd)
        {
            servov= ++servov;
            if (servov < 10)
            {
                servov = 10;
            }
        }
        vertical.write(servov); // рух вертикального серводвигуна на значення servov
    }
    
    if (-1*tol > dhoriz || dhoriz > tol) // зміна кута горизонтального серводвигуна якщо різниця значень не є допустимою
    {
        if (avl > avr)
        {
            servoh = --servoh;
            if (servoh < 10)
            {
                servoh = 10;
            }
        }
        else if (avl < avr)
        {
            servoh = ++servoh;
             if (servoh > 170)
             {
                servoh = 170;
             }
        }
        else if (avl = avr)
        {
            // рух серводвигунів не відбувається
        }
        horizontal.write(servoh); // рух горизонтального серводвигуна на значення servoh
    }
 

      delay(50); // часова затримка системи 50мкс 
}
