/**************************************************************************/
/*!
@file     MQ135.cpp
@author   G.Krocker (Mad Frog Labs)
@license  GNU GPLv3
First version of an Arduino Library for the MQ135 gas sensor
TODO: Review the correction factor calculation. This currently relies on
the datasheet but the information there seems to be wrong.
@section  HISTORY
v1.0 - First release
*/
/**************************************************************************/

#include <math.h>
#include "MQ135.h"


/**************************************************************************/
/*!
@brief  Get the correction factor to correct for temperature and humidity
@param[in] t  The ambient air temperature
@param[in] h  The relative humidity
@return The calculated correction factor
*/
/**************************************************************************/
ICACHE_FLASH_ATTR float getCorrectionFactor(float t, float h) {
  return CORA * t * t - CORB * t + CORC - (h-33.)*CORD;
}

/**************************************************************************/
/*!
@brief  Get the resistance of the sensor, ie. the measurement value
@return The sensor resistance in kOhm
*/
/**************************************************************************/
ICACHE_FLASH_ATTR float getResistance() {
  int val = system_adc_read();
  return ((1023./(float)val) * 5. - 1.)*RLOAD;
}

/**************************************************************************/
/*!
@brief  Get the resistance of the sensor, ie. the measurement value corrected
        for temp/hum
@param[in] t  The ambient air temperature
@param[in] h  The relative humidity
@return The corrected sensor resistance kOhm
*/
/**************************************************************************/
ICACHE_FLASH_ATTR float getCorrectedResistance(float t, float h) {
  return getResistance()/getCorrectionFactor(t, h);
}

/**************************************************************************/
/*!
@brief  Get the ppm of CO2 sensed (assuming only CO2 in the air)
@return The ppm of CO2 in the air
*/
/**************************************************************************/
ICACHE_FLASH_ATTR float getPPM() {
  return PARA * pow((getResistance()/RZERO), -PARB);
}

/**************************************************************************/
/*!
@brief  Get the ppm of CO2 sensed (assuming only CO2 in the air), corrected
        for temp/hum
@param[in] t  The ambient air temperature
@param[in] h  The relative humidity
@return The ppm of CO2 in the air
*/
/**************************************************************************/
ICACHE_FLASH_ATTR float getCorrectedPPM(float t, float h) {
  return PARA * pow((getCorrectedResistance(t, h)/RZERO), -PARB);
}

/**************************************************************************/
/*!
@brief  Get the resistance RZero of the sensor for calibration purposes
@return The sensor resistance RZero in kOhm
*/
/**************************************************************************/
ICACHE_FLASH_ATTR float getRZero() {
  return getResistance() * pow((ATMOCO2/PARA), (1./PARB));
}

/**************************************************************************/
/*!
@brief  Get the corrected resistance RZero of the sensor for calibration
        purposes
@param[in] t  The ambient air temperature
@param[in] h  The relative humidity
@return The corrected sensor resistance RZero in kOhm
*/
/**************************************************************************/
ICACHE_FLASH_ATTR float getCorrectedRZero(float t, float h) {
  return getCorrectedResistance(t, h) * pow((ATMOCO2/PARA), (1./PARB));
}
