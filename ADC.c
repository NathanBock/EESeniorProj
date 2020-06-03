/*
 * ADC.c
 *
 *  Created on: Jun 2, 2020
 *      Author: Nathan
 */
#include "MCUType.h"
#include "ADC.h"
#include "MK40D10.h"

#include "NBSeniorProj.h"


#define ENABLE 1
#define CHAN 8

#define CLK_DIV 2
#define SIXTEEN_BIT 3
#define AVERAGE 3
#define COUNT_VALUE 20971
#define CLEAR 1

#define PIT_TRIG 5
#define HARDWARE_TRIG 1


static INT32S TempCels;                // Calculated Temperature
static INT16U Sample;                   /* ADC0(Touchpad) values            */

/************************************************************************
* ADCGetSample() - Returns ADC Sample
*
* Returns: INT16U
* Arguments: None
*************************************************************************/
INT16U ADCGetSample(void){
    return Sample;
}

/************************************************************************
* ADCGetTemp() - Returns Temperature in Celsius
*
* Returns: INT32U
* Arguments: None
*************************************************************************/
INT32S ADCGetTemp(void){
    return TempCels;
}

/************************************************************************
* ADC0_IRQHandler() - Handles interrupt that occurs when ADC 0 conversion
*  is complete.
* Returns: None
* Arguments: None
*************************************************************************/
void ADC0_IRQHandler(void){

    Sample = ADC0->R[0];   /* Read ADC data */

}

/************************************************************************
* ADC1_IRQHandler() - Handles interrupt that occurs when ADC 1 conversion
*  is complete.
* Returns: None
* Arguments: None
*************************************************************************/
void ADC1_IRQHandler(void){
    INT16U tsample;

    tsample = ADC1->R[0];
    TempCels = (INT32S)(((33*tsample)/10) - 26214)/1278 ;

}

/************************************************************************
* ADC_Init() - Initializes ADC
*
* Returns: None
* Arguments: None
*************************************************************************/
void ADC_Init(void){

    // ADC0 Initialization
    SIM->SCGC6 |= SIM_SCGC6_ADC0_MASK;   /*Turn on ADC0 clock           */
    SIM->SOPT7 |= SIM_SOPT7_ADC0TRGSEL(PIT_TRIG)|SIM_SOPT7_ADC0ALTTRGEN(ENABLE);
    SIM->SCGC6 |= SIM_SCGC6_PIT(ENABLE);        /* Initialize PIT and ADC clocks*/
    PIT->MCR &= ~PIT_MCR_MDIS_MASK;

                                                /* Clock Divide by 8,
                                                 * 16bit samples, Hardware  trigger,
                                                 * 32 Bit hardware averaging           */
    ADC0->CFG1 |= ADC_CFG1_ADIV(CLK_DIV)|ADC_CFG1_MODE(SIXTEEN_BIT);
    ADC0->SC2 |= ADC_SC2_ADTRG(HARDWARE_TRIG);
    ADC0->SC3 |= ADC_SC3_AVGE(ENABLE) | ADC_SC3_AVGS(ENABLE);


                                                /* Load count value for 1000 Hz and enable */
    PIT->CHANNEL[1].LDVAL = PIT_LDVAL_TSV(COUNT_VALUE);
    PIT->CHANNEL[1].TCTRL |= PIT_TCTRL_TEN(ENABLE);
                                                /*Select ADC0 channel 8                */
    ADC0->SC1[0] = ADC_SC1_ADCH(CHAN) | ADC_SC1_AIEN(ENABLE);


    // ADC1 Initialization


    SIM->SCGC3 |= SIM_SCGC3_ADC1_MASK;
    SIM->SOPT7 |= SIM_SOPT7_ADC1TRGSEL(6)|SIM_SOPT7_ADC1ALTTRGEN(ENABLE);


                                                /* Clock Divide by 8,
                                                 * 16bit samples, Hardware  trigger,
                                                 * 32 Bit hardware averaging           */
    ADC1->CFG1 |= ADC_CFG1_ADIV(CLK_DIV)|ADC_CFG1_MODE(SIXTEEN_BIT);
    ADC1->SC2 |= ADC_SC2_ADTRG(HARDWARE_TRIG);
    ADC1->SC3 |= ADC_SC3_AVGE(ENABLE) | ADC_SC3_AVGS(ENABLE);


                                                /* Load count value for .5 Hz and enable */
    PIT->CHANNEL[2].LDVAL = PIT_LDVAL_TSV(41942799);
    PIT->CHANNEL[2].TCTRL |= PIT_TCTRL_TEN(ENABLE);
    ADC1->SC1[0] = ADC_SC1_ADCH(16) | ADC_SC1_AIEN(ENABLE);   /*Also starts a conversion       */
    NVIC_ClearPendingIRQ(ADC1_IRQn);
    NVIC_EnableIRQ(ADC1_IRQn);



}
