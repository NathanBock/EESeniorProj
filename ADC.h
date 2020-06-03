/**********************************************************************************
 ADC Header File


 Nathan Bock 6/2/2020
 **********************************************************************************/

#ifndef ADC_H_
#define ADC_H_

/************************************************************************
* ADC0_IRQHandler() - Handles interrupt that occurs when ADC 0 conversion
*  is complete.
* Returns: None
* Arguments: None
*************************************************************************/
void ADC0_IRQHandler(void);

/************************************************************************
* ADC1_IRQHandler() - Handles interrupt that occurs when ADC 1 conversion
*  is complete.
* Returns: None
* Arguments: None
*************************************************************************/
void ADC1_IRQHandler(void);

/************************************************************************
* ADC_Init() - Initializes ADC
*
* Returns: None
* Arguments: None
*************************************************************************/
void ADC_Init(void);

/************************************************************************
* ADCGetTemp() - Returns Temperature in Celsius
*
* Returns: INT32U
* Arguments: None
*************************************************************************/
INT32S ADCGetTemp(void);

/************************************************************************
* ADCGetSample() - Returns ADC Sample
*
* Returns: INT16U
* Arguments: None
*************************************************************************/
INT16U ADCGetSample(void);

#endif /* ADC_H_ */
