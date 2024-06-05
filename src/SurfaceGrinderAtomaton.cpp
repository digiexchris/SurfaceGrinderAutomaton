#include <stdio.h>
#include "hardware/watchdog.h"
#include "pico/stdlib.h"
#include <string>
#include <string.h>
#include <iostream>
//#include "hardware/adc.h"
// #include "hardware/pwm.h"
// #include "hardware/irq.h"
#include "stepper.pio.h"

#include "blink.pio.h"

#include "drivers/Motor/Stepper.hpp"

using namespace std;
const uint LED_PIN = PICO_DEFAULT_LED_PIN;

char inbuff[100];
int buffcounter = 0;
bool dataavailable = false;
string inString = "";
string outstr = "";

#define STM_EN 15

float fullRevolutionAngle = 360.0;
int directionChangeDelay = 50; // ms

/****************************************************/

Stepper* stepper;

// void setupPIO();
// void setupGPIO();
// uint32_t construct32bits(int pulsecnt);
// void moveMECHPART(StepperDevice *mpart);
// void enableStepper(bool act);
// void getStepperSteps(StepperDevice *mpart, float degree, int dir);
// void readtobuffer();
// void processinData();
// float countAngle(float currentAng, float counterval, bool dir);
// bool changeMotorDirection(StepperDevice *mpart);
// static inline void put_steps(uint32_t steps);

void blink_pin_forever(PIO pio, uint sm, uint offset, uint pin, uint freq)
{
	blink_program_init(pio, sm, offset, pin);
	pio_sm_set_enabled(pio, sm, true);

	printf("Blinking pin %d at %d Hz\n", pin, freq);

	// PIO counter program takes 3 more cycles in total than we pass as
	// input (wait for n + 1; mov; jmp)
	pio->txf[sm] = (125000000 / (2 * freq)) - 3;
}

int main()
{

    stdio_init_all();

	sleep_ms(1000); // time for uart to connect for debugging, comment out later
	printf("\n\n--------------\n\n");
	// Watchdog example code
	if (watchdog_caused_reboot())
	{
		printf("Rebooted by Watchdog!\n");
		// Whatever action you may take if a watchdog caused a reboot
	}

	// Lastly, watchdog and the led

	// PIO Blinking example
	PIO pio = pio0;
	uint offset = pio_add_program(pio, &blink_program);
	printf("Loaded program at %d\n", offset);

#ifdef PICO_DEFAULT_LED_PIN
	blink_pin_forever(pio, 0, offset, PICO_DEFAULT_LED_PIN, 3);
#else
	blink_pin_forever(pio, 0, offset, 6, 3);
#endif

	// Enable the watchdog, requiring the watchdog to be updated every 100ms or the chip will reboot
	// second arg is pause on debug which means the watchdog will pause when stepping through code
	//watchdog_enable(12000, 1);

	// You need to call this function at least more often than the 100ms in the enable call to prevent a reboot
	watchdog_update();

	printf("System Clock Frequency is %d Hz\n", clock_get_hz(clk_sys));
	printf("USB Clock Frequency is %d Hz\n", clock_get_hz(clk_usb));

    stepper = new Stepper(11, 10, 9);

    uint64_t addStepsCounter = 0;

    while (true)
    {
		watchdog_update();
        if ((time_us_64() / 1000) - addStepsCounter > 500)
        {
            addStepsCounter = time_us_64() / 1000;
            stepper->MoveRelative(1000, Stepper::DirectionState::POS);
        }
        
        while(stepper->Update())
        {
            watchdog_update();
            printf("Steps Remaining in queue: %d\n", stepper->GetRemainingSteps());
        }
    }
}

// bool privChangeMotorDirection()
// {
//     if (myStepperDevice->activedir != myStepperDevice->dirchange && myStepperDevice->directionChangeDelayCounter == 0) // first change detected
//     {
//         if (pio_sm_is_tx_fifo_empty(myStepperDevice->stm_pio, myStepperDevice->stm_sm))
//         {
//             myStepperDevice->directionChangeDelayCounter = (time_us_64() / 1000);
//         }
//         return false;
//     }
//     else if (myStepperDevice->activedir != myStepperDevice->dirchange && myStepperDevice->directionChangeDelayCounter != 0)
//     {
//         if (((time_us_64() / 1000) - myStepperDevice->directionChangeDelayCounter) > myStepperDevice->directionChangeDelay) // pre change delay
//         {
//             myStepperDevice->activedir = myStepperDevice->dirchange;
//             gpio_put(myStepperDevice->DIR_PIN, !myStepperDevice->activedir);
//             myStepperDevice->directionChangeDelayCounter = (time_us_64() / 1000);
//         }
//         return false;
//     }
//     else if (myStepperDevice->activedir == myStepperDevice->dirchange && myStepperDevice->directionChangeDelayCounter != 0)
//     {
//         if (((time_us_64() / 1000) - myStepperDevice->directionChangeDelayCounter) > myStepperDevice->directionChangeDelay) // post change delay
//         {
//             myStepperDevice->directionChangeDelayCounter = 0;
//             return true;
//         }
//         return false;
//     }

//     return true;
// }
// void setupPIO()
// {
//     // stepper 1 pio
//     uint stm_offset = pio_add_program(stepperMotor1.stm_pio, &stepper_1_program);
//     stepper_1_program_init(stepperMotor1.stm_pio, stepperMotor1.stm_sm, stm_offset, stepperMotor1.STEP_PIN, 10000, true);
// }
// void setupGPIO()
// {
//     stepperMotor1.DIR_PIN = 11;
//     stepperMotor1.stm_sm = 1;
//     stepperMotor1.STEP_PIN = 10;
//     stepperMotor1.FULLSTEP = 800;
//     // stepper enable pin
//     gpio_init(STM_EN);
//     gpio_set_dir(STM_EN, GPIO_OUT);
//     // gpio_put(STM_EN, 0);

//     // wheel 1 stepper dir pin
//     gpio_init(stepperMotor1.DIR_PIN);
//     gpio_set_dir(stepperMotor1.DIR_PIN, GPIO_OUT);
//     // gpio_put(stepperMotor1.DIR_PIN, 1);

//     // inbuilt led
//     gpio_init(LED_PIN);
//     gpio_set_dir(LED_PIN, GPIO_OUT);
// }
// uint32_t construct32bits(int pulsecnt)
// {
//     uint32_t outbits = 0b0;
//     for (size_t i = 0; i < pulsecnt; i++)
//     {
//         outbits = (outbits << 1) | 0b1;
//     }
//     return outbits;
// }
// void moveMECHPART(StepperDevice *mpart)
// {

//     if (myStepperDevice->StepsToTake > 0 && pio_sm_is_tx_fifo_empty(myStepperDevice->stm_pio, myStepperDevice->stm_sm))
//     {
//         if (changeMotorDirection(mpart))
//         {
//             int remBits = 32;
//             if (myStepperDevice->StepsToTake < 32)
//             {
//                 remBits = myStepperDevice->StepsToTake;
//             }

//             gpio_put(LED_PIN, 1);
//             uint32_t BitRes;
//             BitRes = construct32bits(remBits);
//             put_steps(BitRes);
//             myStepperDevice->StepsToTake -= remBits;
//         }
//     }
//     else
//     {
//         gpio_put(LED_PIN, 0);
//     }
// }
// float countAngle(float currentAng, float counterval, bool dir)
// {
//     float anglecounter = currentAng;
//     if (dir) // count up
//     {
//         if ((counterval + anglecounter) > 360)
//         {
//             anglecounter = (counterval + anglecounter) - 360;
//         }
//         else
//         {
//             anglecounter += counterval;
//         }
//     }
//     else
//     { // count down
//         if ((anglecounter - counterval) < 0)
//         {
//             anglecounter = 360 + (anglecounter - counterval);
//         }
//         else
//         {
//             anglecounter -= counterval;
//         }
//     }
//     return anglecounter;
// }
// void getStepperSteps(StepperDevice *mpart, float degree, int dir) //(degree position, steppernumber, direction to move (0-fastest route,1-forward,2-backward))
// {
//     if (degree > 0 && abs(degree - myStepperDevice->ActiveAngle) != 360 && degree <= 360)
//     { // no negative angle
//         float mStepRes = fullRevolutionAngle / float(myStepperDevice->FULLSTEP);
//         float precRange = 0.55;
//         bool mostRecentDir = myStepperDevice->dirchange;
//         int stepsToTake = 0;
//         int i = 0;
//         int j = 0;
//         bool ibool = true;
//         bool jbool = false;
//         // while (myStepperDevice->StepsToTake != 0)
//         // {
//         //     moveMECHPART(mpart);
//         // }

//         if (myStepperDevice->StepsToTake != 0) // take care of unreached position
//         {
//             //printf("remsteps %d activeangle %f dir %d", myStepperDevice->StepsToTake, myStepperDevice->ActiveAngle, myStepperDevice->activedir);
//             for (size_t k = 0; k < myStepperDevice->StepsToTake; k++)
//             {
//                 if (myStepperDevice->dirchange == ibool)
//                 {
//                     myStepperDevice->ActiveAngle = countAngle(myStepperDevice->ActiveAngle, mStepRes, false);
//                 }
//                 else if (myStepperDevice->dirchange == jbool)
//                 {
//                     myStepperDevice->ActiveAngle = countAngle(myStepperDevice->ActiveAngle, mStepRes, true);
//                 }
//             }
//             myStepperDevice->StepsToTake = 0;
//             //printf(" //adjangle %f \n", myStepperDevice->ActiveAngle);
//         }
//         float i_tempdeg = myStepperDevice->ActiveAngle;
//         float j_tempdeg = myStepperDevice->ActiveAngle;

//         if (dir == 0 || dir == 1)
//         {
//             // degDelta = abs(i_tempdeg - degree);
//             while ((abs(i_tempdeg - degree) / mStepRes) > precRange)
//             {
//                 i_tempdeg = countAngle(i_tempdeg, mStepRes, true);
//                 i += 1;
//                 // degDelta = abs(i_tempdeg - degree);
//             }
//         }
//         if (dir == 0 || dir == 2)
//         {
//             // degDelta = abs(j_tempdeg - degree);
//             while ((abs(j_tempdeg - degree) / mStepRes) > precRange)
//             {
//                 j_tempdeg = countAngle(j_tempdeg, mStepRes, false);
//                 j += 1;
//                 // degDelta = abs(j_tempdeg - degree);
//             }
//         }

//         if (i != 0 && j != 0)
//         {
//             if (i <= j)
//             { // move forward to angle
//                 stepsToTake = i;
//                 myStepperDevice->ActiveAngle = i_tempdeg;
//                 myStepperDevice->StepsToTake += stepsToTake;
//                 myStepperDevice->dirchange = ibool;
//             }
//             else
//             { // move backward to angle
//                 stepsToTake = j;
//                 myStepperDevice->ActiveAngle = j_tempdeg;
//                 myStepperDevice->StepsToTake += stepsToTake;
//                 myStepperDevice->dirchange = jbool;
//             }
//         }
//         else if (i > 0)
//         { // move forward to angle
//             stepsToTake = i;
//             myStepperDevice->ActiveAngle = i_tempdeg;
//             myStepperDevice->StepsToTake += stepsToTake;
//             myStepperDevice->dirchange = ibool;
//         }
//         else if (j > 0)
//         { // move backward to angle
//             stepsToTake = j;
//             myStepperDevice->ActiveAngle = j_tempdeg;
//             myStepperDevice->StepsToTake += stepsToTake;
//             myStepperDevice->dirchange = jbool;
//         }
//         enableStepper(true);
//         //printf(" calcDir %d \n", myStepperDevice->dirchange);
//     }
// }

// void enableStepper(bool act)
// {
//     if (act)
//     {
//         gpio_put(STM_EN, 0);
//     }
//     else
//     {
//         gpio_put(STM_EN, 1);
//     }
// }

// void connecttoPC(string str)
// {
//     inString = str;
//     outstr = inString;
//     // printf(outstr.c_str());
//     if (inString != "")
//     {

//         if (inString.find("moveto=") == 0)
//         {
//             inString.replace(inString.find("moveto="), 7, "");
//             getStepperSteps(&stepperMotor1, stof(inString), 0);
//             printf("activeangle %f stepstotake %d dir %d \n", stepperMotor1.ActiveAngle, stepperMotor1.StepsToTake, stepperMotor1.activedir);
//             inString = "";
//         }
//         else
//         {
//             inString = "";
//         }
//     }
// }

// void readtobuffer()
// {
//     char chr = getchar_timeout_us(0);
//     if (chr != 255)
//     {
//         if (chr == '*')
//         {
//             dataavailable = true;
//         }
//         // cout << chr << buffcounter << std::endl;
//         if (buffcounter < sizeof(inbuff))
//         {
//             inbuff[buffcounter] = chr;
//             buffcounter += 1;
//         }
//         else
//         {
//             buffcounter = 0;
//         }
//     }
//     else
//     {
//         dataavailable = false;
//     }
// }

// void processinData()
// {
//     if (dataavailable)
//     {
//         // cout << inbuff << std::endl;
//         int tempcnt = 0;
//         while (inbuff[tempcnt] != '*')
//         {
//             inString += inbuff[tempcnt];
//             tempcnt += 1;
//         }
//         // cout << inString << std::endl;
//         connecttoPC(inString);
//         buffcounter = 0;
//         dataavailable = false;
//         memset(inbuff, 0, sizeof(inbuff));
//     }
// }

// static inline void put_steps(uint32_t steps)
// {
//     pio_sm_put_blocking(pio0, 1, steps);
// }