#define TIMER_INTERRUPT_DEBUG 0
#define _TIMERINTERRUPT_LOGLEVEL_ 0

#define USE_TIMER_2 true
#warning Using Timer2


#include "TimerInterrupt.h"
#include "ISR_Timer.h"
#include <SimpleTimer.h>              // https://github.com/schinken/SimpleTimer


#ifndef LED_BUILTIN
  #define LED_BUILTIN       13
#endif

ISR_Timer ISR_timer;

#define LED_TOGGLE_INTERVAL_MS        1000L

// You have to use longer time here if having problem because Arduino AVR clock is low, 16MHz => lower accuracy.
// Tested OK with 1ms when not much load => higher accuracy.
#define TIMER_INTERVAL_MS            5L

volatile uint32_t startMillis = 0;

void TimerHandler()
{
  static bool toggle  = false;
  static int timeRun  = 0;

  ISR_timer.run();

  // Toggle LED every LED_TOGGLE_INTERVAL_MS = 2000ms = 2s
  if (++timeRun == ((LED_TOGGLE_INTERVAL_MS) / TIMER_INTERVAL_MS) )
  {
    timeRun = 0;

    //timer interrupt toggles pin LED_BUILTIN
    digitalWrite(LED_BUILTIN, toggle);
    toggle = !toggle;
  }
}

/////////////////////////////////////////////////

#define NUMBER_ISR_TIMERS         8

typedef void (*irqCallback)  (void);

/////////////////////////////////////////////////

#define USE_COMPLEX_STRUCT      true

#if USE_COMPLEX_STRUCT

  typedef struct 
  {
    irqCallback   irqCallbackFunc;
    uint32_t      TimerInterval;
    unsigned long deltaMillis;
    unsigned long previousMillis;
  } ISRTimerData;
  
  // In NRF52, avoid doing something fancy in ISR, for example Serial.print()
  // The pure simple Serial.prints here are just for demonstration and testing. Must be eliminate in working environment
  // Or you can get this run-time error / crash
  
  void doingSomething(int index);

#else

  volatile unsigned long deltaMillis    [NUMBER_ISR_TIMERS] = { 0, 0, 0, 0, 0, 0, 0, 0};
  volatile unsigned long previousMillis [NUMBER_ISR_TIMERS] = { 0, 0, 0, 0, 0, 0, 0, 0};
  
  // You can assign any interval for any timer here, in milliseconds
  uint32_t TimerInterval[NUMBER_ISR_TIMERS] =
  {
    5000L,  10000L,  15000L,  20000L,  25000L,  30000L,  35000L,  40000L
  };
  
  void doingSomething(int index)
  {
    unsigned long currentMillis  = millis();
    
    deltaMillis[index]    = currentMillis - previousMillis[index];
    previousMillis[index] = currentMillis;
  }

#endif

////////////////////////////////////
// Shared
////////////////////////////////////

void doingSomething0()
{
  doingSomething(0);
}

void doingSomething1()
{
  doingSomething(1);
}

void doingSomething2()
{
  doingSomething(2);
}

void doingSomething3()
{
  doingSomething(3);
}

void doingSomething4()
{
  doingSomething(4);
}

void doingSomething5()
{
  doingSomething(5);
}

void doingSomething6()
{
  doingSomething(6);
}

void doingSomething7()
{
  doingSomething(7);
}

#if USE_COMPLEX_STRUCT

  ISRTimerData curISRTimerData[NUMBER_ISR_TIMERS] =
  {
    //irqCallbackFunc, TimerInterval, deltaMillis, previousMillis
    { doingSomething0,    5000L, 0, 0 },
    { doingSomething1,   10000L, 0, 0 },
    { doingSomething2,   15000L, 0, 0 },
    { doingSomething3,   20000L, 0, 0 },
    { doingSomething4,   25000L, 0, 0 },
    { doingSomething5,   30000L, 0, 0 },
    { doingSomething6,   35000L, 0, 0 },
    { doingSomething7,   40000L, 0, 0 }
  };
  
  void doingSomething(int index)
  {
    unsigned long currentMillis  = millis();
    
    curISRTimerData[index].deltaMillis    = currentMillis - curISRTimerData[index].previousMillis;
    curISRTimerData[index].previousMillis = currentMillis;
  }

#else

  irqCallback irqCallbackFunc[NUMBER_ISR_TIMERS] =
  {
    doingSomething0,  doingSomething1,  doingSomething2,  doingSomething3,
    doingSomething4,  doingSomething5,  doingSomething6,  doingSomething7
  };

#endif

////////////////////////////////////////////////


#define SIMPLE_TIMER_MS        2000L

// Init SimpleTimer
SimpleTimer simpleTimer;

// Here is software Timer, you can do somewhat fancy stuffs without many issues.
// But always avoid
// 1. Long delay() it just doing nothing and pain-without-gain wasting CPU power.Plan and design your code / strategy ahead
// 2. Very long "do", "while", "for" loops without predetermined exit time.
void simpleTimerDoingSomething2s()
{
  static unsigned long previousMillis = startMillis;

  unsigned long currMillis = millis();

  Serial.print(F("SimpleTimer : "));Serial.print(SIMPLE_TIMER_MS / 1000);
  Serial.print(F(", ms : ")); Serial.print(currMillis);
  Serial.print(F(", Dms : ")); Serial.println(currMillis - previousMillis);

  for (uint16_t i = 0; i < NUMBER_ISR_TIMERS; i++)
  {
#if USE_COMPLEX_STRUCT    
    Serial.print(F("Timer : ")); Serial.print(i);
    Serial.print(F(", programmed : ")); Serial.print(curISRTimerData[i].TimerInterval);
    Serial.print(F(", actual : ")); Serial.println(curISRTimerData[i].deltaMillis);
#else
    Serial.print(F("Timer : ")); Serial.print(i);
    Serial.print(F(", programmed : ")); Serial.print(TimerInterval[i]);
    Serial.print(F(", actual : ")); Serial.println(deltaMillis[i]);
#endif    
  }

  previousMillis = currMillis;
}

void setup() 
{
  pinMode(LED_BUILTIN, OUTPUT);

  Serial.begin(115200);
  while (!Serial);

  Serial.print(F("\nStarting ISR_16_Timers_Array_Complex on "));
  Serial.println(BOARD_TYPE);
  Serial.println(TIMER_INTERRUPT_VERSION);
  Serial.print(F("CPU Frequency = ")); Serial.print(F_CPU / 1000000); Serial.println(F(" MHz"));

  // Timer0 is used for micros(), millis(), delay(), etc and can't be used
  // Select Timer 1-2 for UNO, 1-5 for MEGA, 1,3,4 for 16u4/32u4
  // Timer 2 is 8-bit timer, only for higher frequency
  // Timer 4 of 16u4 and 32u4 is 8/10-bit timer, only for higher frequency
  
#if USE_TIMER_2

  ITimer2.init();

  // Using ATmega328 used in UNO => 16MHz CPU clock ,

  if (ITimer2.attachInterruptInterval(TIMER_INTERVAL_MS, TimerHandler))
  {
    Serial.print(F("Starting  ITimer2 OK, millis() = ")); Serial.println(millis());
  }
  else
    Serial.println(F("Can't set ITimer2. Select another freq. or timer"));
    
#endif

  //ISR_timer.setInterval(2000L, doingSomething2s);
  //ISR_timer.setInterval(5000L, doingSomething5s);

  // Just to demonstrate, don't use too many ISR Timers if not absolutely necessary
  // You can use up to 16 timer for each ISR_Timer
  for (uint16_t i = 0; i < NUMBER_ISR_TIMERS; i++)
  {
#if USE_COMPLEX_STRUCT
    curISRTimerData[i].previousMillis = startMillis;
    ISR_timer.setInterval(curISRTimerData[i].TimerInterval, curISRTimerData[i].irqCallbackFunc);
#else
    previousMillis[i] = startMillis;
    ISR_timer.setInterval(TimerInterval[i], irqCallbackFunc[i]);
#endif    
  }

  // You need this timer for non-critical tasks. Avoid abusing ISR if not absolutely necessary.
  simpleTimer.setInterval(SIMPLE_TIMER_MS, simpleTimerDoingSomething2s);
}

#define BLOCKING_TIME_MS      10000L

void loop()
{
  // This unadvised blocking task is used to demonstrate the blocking effects onto the execution and accuracy to Software timer
  // You see the time elapse of ISR_Timer still accurate, whereas very unaccurate for Software Timer
  // The time elapse for 2000ms software timer now becomes 3000ms (BLOCKING_TIME_MS)
  // While that of ISR_Timer is still prefect.
  delay(BLOCKING_TIME_MS);

  // You need this Software timer for non-critical tasks. Avoid abusing ISR if not absolutely necessary
  // You don't need to and never call ISR_Timer.run() here in the loop(). It's already handled by ISR timer.
  simpleTimer.run();
}