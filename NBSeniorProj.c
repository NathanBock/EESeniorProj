/**********************************************************************************
 EE493
 Senior Project
 Aquatic Timing System
 
 Includes a User Interface with 2 buttons and LCD. Button 1 starts a 5 second
 count down then starts a timer. ADC0 input stops the timer. Then the elapsed
 time is displayed to the LCD along with temperature and number of times the
 timer has been started.

 Special thanks to Todd Morton for the SysTick and LCD modules.

 Nathan Bock 6/2/2020
**********************************************************************************/
#include <stdio.h>
#include "MK40D10.h"
#include "MCUType.h"
#include "LCD.h"
#include "SysTickDelay.h"
#include "ADC.h"
#include "NBSeniorProj.h"

#define ENABLE 1
#define ROW2 2
#define COL12 12
#define SW1_PORT GPIOE->PDIR
#define SW2_PORT GPIOC->PDIR
#define SW1_BIT 0x10000000
#define SW2_BIT 0x00040000
#define SW1_PRESSED  0x0
#define SW1_RELEASED 0x10000000
#define SW2_PRESSED  0x0
#define SW2_RELEASED 0x00040000

/*****************************************************
    Typedefs
******************************************************/

typedef enum  {SW_OFF, SW_EDGE, SW_VERF} SWSTATES;
typedef enum  {RESET, START, COUNT, STOP} TIMER_STATES;

typedef struct{
    INT8U ms;
    INT8U tms;
    INT8U hms;
    INT8U secs;
    INT8U mins;
} ETIME;

/*****************************************************
    Function Prototypes
******************************************************/

static void GPIOInit(void);

static void LPTimerInit(void);

/******************************************************************************
    Global Variables
******************************************************************************/
static ETIME swETime;                   /* Elapsed time structure */

static const INT8C Go[] =  "Go";        /* Stored constant string */

static TIMER_STATES TimeState = RESET;  /* State Machine variable */

void main(void) {

    INT32U cursw1;            //Current switch positions
    INT32U cursw2;

    SWSTATES swstate;         //Current switch debounce state
    INT8U swcount;            // Switch press counter
    swcount = 0;
    swstate = SW_OFF;

    INT8U eventcount = 0;

    INT32S oldtempcels;
    INT32S tempcels;                // Calculated Temperature

    INT16U calibrated;
    INT16U sample;

    LPTimerInit();
    ADC_Init();
    LcdInit();
    GPIOInit();
    (void)SysTickDlyInit();


    while(1){
        switch(TimeState){

            case RESET:             //Reset state, clear LCD and set values back to 0
                LcdClrDisp();
                eventcount = 0;
                TimeState = STOP;
                break;

            case START:
                LcdMoveCursor(1,8);             // 5 Second count down
                for(INT32U i = 5;i != 0; i--){
                    SysTickDelay(1000);				
                    LcdDispDecWord(i,1);
                }
                SysTickDelay(1000);
                LcdDispStrg(Go);                // Prompt user to Go!
                TimeState = COUNT;
                break;

            case COUNT:
                swETime.ms = 0;
				swETime.tms = 0;                // Timer state, set elapsed time to 0
                swETime.hms = 0;
                swETime.secs = 0;
                swETime.mins = 0;


                NVIC_ClearPendingIRQ(ADC0_IRQn);    //Enable Touchpad Interrupt to stop the timer
                NVIC_EnableIRQ(ADC0_IRQn);

                NVIC_ClearPendingIRQ(LPTMR0_IRQn);	//Enable LPT Interrupt for time keeping
                NVIC_EnableIRQ(LPTMR0_IRQn);

                calibrated = ADCGetSample();

                while(TimeState == COUNT){
                    if(swETime.ms == 10){
                        swETime.tms++;                          // Tens of milliseconds
                        swETime.ms = 0;
                        if(swETime.tms == 10){                  // Hundreds of milliseconds
                            swETime.tms = 0;
                            swETime.hms++;
                            if(swETime.hms == 10){              // Seconds
                                swETime.secs++;
                                swETime.hms = 0;
                                if(swETime.secs == 60){         // Update minutes
                                    swETime.mins++;
                                    swETime.secs = 0;
                                    if(swETime.mins == 100){    /* Overflow,              */
                                        swETime.mins = 0;       /*   roll to back to zero */
                                    }else{
                                    }
                                }else{
                                }
                            }else{
                            }
                        }else{
                        }
                    }else{
                    }
                                                                // If a touch is detected go back to stop state
                                                                // Disable LPTMR interrupt
                    sample = ADCGetSample();
                    if((sample > (calibrated + 10000)) || (sample< (calibrated - 10000))){
                        NVIC_DisableIRQ(LPTMR0_IRQn);
                        eventcount ++;
                        TimeState = STOP;
                    }
                }
                break;

            case STOP:                                          // Stop state, disable unnecessary interrupts
                NVIC_ClearPendingIRQ(ADC1_IRQn);				// Enable Temp. Sensor
                NVIC_EnableIRQ(ADC1_IRQn);

                NVIC_DisableIRQ(ADC0_IRQn);						// Disable Touchpad


                LcdClrDisp();
                                                                // Update display with elapsed time
                LcdDispDecWord(swETime.mins,1);
                LcdDispChar(':');
                LcdDispDecWord(swETime.secs,1);
                LcdDispChar(':');
                LcdDispDecWord(swETime.hms,1);
                LcdDispDecWord(swETime.tms,1);

                LcdMoveCursor(2,1);                             // Update display with number of events
                LcdDispDecWord(eventcount,1);


                oldtempcels = 0;                                // Ensure value gets written
                while(TimeState == STOP){
                    SysTickWaitEvent(10);                       //polling rate=10ms

                    tempcels = ADCGetTemp();
                    if(oldtempcels != tempcels){                // Update temperature only if it's changed
                        oldtempcels = tempcels;
                        LcdMoveCursor(2,10);
                        LcdDispDecWord(tempcels,1);
                        LcdDispChar(223);                       // Display Degrees symbol
                        LcdDispChar('C');
                    }
                    else{}

                    cursw1 = SW1_PORT & SW1_BIT;                //   Get current switch position
                    cursw2 = SW2_PORT & SW2_BIT;
                    switch (swstate) {
                      case SW_OFF:                              // wait for switch edge
                          if(cursw1 == SW1_PRESSED||cursw2 == SW2_PRESSED){
                              swstate = SW_EDGE;
                          }
                          else{
                            }
                            break;
                        case SW_EDGE:                           //   Verify switch press
                            if(cursw1 == SW1_PRESSED||cursw2 == SW2_PRESSED){
                                swstate = SW_VERF;              // Switch press verified
                                swcount++;                      //   Change stopwatch mode
                                if(cursw1 == SW1_PRESSED){
                                    TimeState = START;
                                    NVIC_DisableIRQ(ADC1_IRQn);	//Disable Temp. Sensor

                                }
                                else if (cursw2 == SW2_PRESSED){
                                    TimeState = RESET;
                                }
                                else{}
                            }else{                              //   False switch press, ignore
                                swstate = SW_OFF;
                            }
                            break;
                        case SW_VERF:                           // Wait for release
                            if(cursw1== SW1_RELEASED||cursw2 == SW2_RELEASED){
                                swstate = SW_OFF;
                            }else{
                            }
                            break;
                        default:
                            swstate = SW_OFF;
                    }

                }

                break;
            default:
                break;

        }

    }


}






/************************************************************************
* LPTMR0_IRQHandler() - Handles interrupt that occurs LPTMR Compare value
*   is reached
* Returns: None
* Arguments: None
*************************************************************************/
void LPTMR0_IRQHandler(void){
    LPTMR0->CSR |= LPTMR_CSR_TCF(ENABLE);
    swETime.ms++;
}

/************************************************************************
* LPTimerInit() - Initializes the Low Power Timer Module
*
* Returns: None
* Arguments: cmp Pulse Count compare value
*************************************************************************/
void LPTimerInit(void){


    SIM->SCGC5 |= SIM_SCGC5_LPTIMER(ENABLE) | SIM_SCGC5_PORTC(ENABLE);

    PORTC->PCR[5]|= PORT_PCR_MUX(3);

    LPTMR0->CSR |= LPTMR_CSR_TIE(ENABLE) | LPTMR_CSR_TMS(ENABLE) | LPTMR_CSR_TPS(2);
    LPTMR0->CMR |= LPTMR_CMR_COMPARE(10000);


    LPTMR0->CSR |= LPTMR_CSR_TEN(ENABLE);




    NVIC_ClearPendingIRQ(LPTMR0_IRQn);
    NVIC_EnableIRQ(LPTMR0_IRQn);
}


/************************************************************************
* GPIOInit() - Initializes GPIO Ports/Pins
* Returns: None
* Arguments: None
*************************************************************************/
static void GPIOInit(void){

    SIM->SCGC5 |= SIM_SCGC5_PORTE(ENABLE);
    SIM->SCGC5 |= SIM_SCGC5_PORTC(ENABLE);
    PORTE->PCR[28]|= PORT_PCR_MUX(1);
    PORTC->PCR[18]|= PORT_PCR_MUX(1);

}


