//#include <esp_adc/adc_oneshot.h>
//#include <esp_adc/adc_cali.h>
//#include <esp_adc/adc_cali_scheme.h>


float MAX_BATTERY_VOLTAGE = 4.2; // Max LiPoly voltage of a 3.7 battery is 4.2
float MIN_BATTERY_VOLTAGE = 3.0; // Minimum voltage of a discharged battery


const float voltagePin () {
  // A13 pin is not exposed on Huzzah32 board because it's tied to
  // measuring voltage level of battery. Note: you must
  // multiply the analogRead value by 2x to get the true battery
  // level. See: 
  // https://learn.adafruit.com/adafruit-huzzah32-esp32-feather/esp32-faq
  //int rawValue = analogRead(A13);
  int rawValue = analogRead(34);

  // Reference voltage on ESP32 is 1.1V
  // https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/peripherals/adc.html#adc-calibration
  // See also: https://bit.ly/2zFzfMT
  float voltageLevel = (rawValue / 4095.0) * 2 * 1.1 * 3.3; // calculate voltage level
  float batteryFraction = voltageLevel / MAX_BATTERY_VOLTAGE;

  float voltage = rawValue * (3.3 / 4095.0) * 2; // Assuming a voltage divider with equal resistors
  float batteryPercentage = (voltage - MAX_BATTERY_VOLTAGE) / (MAX_BATTERY_VOLTAGE - MIN_BATTERY_VOLTAGE) * 100.0;

  float voltagePin = rawValue / 4096.0 * 2 * 1.1 * 3.3; // au lieu de 7,46

  return voltagePin;
}

const float batteryPercentageLevel () {
  //Régression polynomiale à partir d'une extrapolation du ratio voltage/pourcentage fourni par chatGPT
  //float batteryPercentagePin = 6.53953043*pow(10,-12)*pow(voltagePin,5)-0.000000000119222341*pow(voltagePin,4)-0.000000000867930581*pow(voltagePin,3)-0.00000000315376758*pow(voltagePin,2)+100*voltagePin-320;
  /*
  voltage = np.array([4.20, 4.15, 4.10, 4.05, 4.00, 3.95, 3.90, 3.85, 3.80, 3.75, 3.70, 3.65, 3.60, 3.55, 3.50, 3.45, 3.40, 3.35, 3.30, 3.25, 3.20])
  percentage = np.array([100, 95, 90, 85, 80, 75, 70, 65, 60, 55, 50, 45, 40, 35, 30, 25, 20, 15, 10, 5, 0])
  */

  float voltageLevel = voltagePin();

  float batteryPercentagePin = -186.396348951606*pow(voltageLevel,7)+3646.70044633217*pow(voltageLevel,6)-26889.5529074022*pow(voltageLevel,5)+79856.542204405*pow(voltageLevel,4)+27320.3505157116*pow(voltageLevel,3)-779854.340256534*pow(voltageLevel,2)+1841903.14340668*voltageLevel-1429703.41983678;

  return batteryPercentagePin;

}


const char* battery_level() {
  static char buffer[20];
 
  /* Nouvelle équation tirée d'une déchage classique de LiPo avec régression polynomiale à 7 niveaux */
  float voltageLevel = voltagePin();
  float batteryPercentagePin = batteryPercentageLevel();

  //batteryPercentage = constrain(batteryPercentage * 100, 0, 100); // Constrain percentage to be between 0 and 100

  batteryPercentagePin = constrain(batteryPercentagePin, 0, 100); // Constrain percentage to be between 0 and 100
 
  //Serial.println((String)"Raw method 1:" + rawValue + " Voltage:" + voltageLevel + "V Percent: " + (batteryFraction * 100) + "%");
  //Serial.println((String)"Raw method 2:" + rawValue + " Voltage:" + voltage + "V Percent: " + batteryPercentage + "%");
  //Serial.println((String)"Raw method 3:" + rawValue + " Voltage:" + voltageLevel + "V Percent: " + batteryPercentagePin + "%");
  
  //snprintf(buffer, sizeof(buffer), "%.1f %s -- %.1f %s", batteryFraction * 100, "%", batteryPercentagePin, "%");
  snprintf(buffer, sizeof(buffer), "%.2f%s | %.1f%s", voltageLevel, "V", batteryPercentagePin, "%");

  Serial.print("Battery level : ");
  Serial.println(buffer); 

  return buffer;
}

/*
const int MAX_ANALOG_VAL = 4095;
const char* battery_level() {
  static char buffer[10];
  uint32_t value = 0;
  int rounds = 11;
  esp_adc_cal_characteristics_t adc_chars;

  //battery voltage divided by 2 can be measured at GPIO34, which equals ADC1_CHANNEL6
  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(ADC1_CHANNEL_6, ADC_ATTEN_DB_11);
  switch(esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc_chars)) {
    case ESP_ADC_CAL_VAL_EFUSE_TP:
      Serial.println("Characterized using Two Point Value");
      break;
    case ESP_ADC_CAL_VAL_EFUSE_VREF:
      Serial.printf("Characterized using eFuse Vref (%d mV)\r\n", adc_chars.vref);
      break;
    default:
      Serial.printf("Characterized using Default Vref (%d mV)\r\n", 1100);
  }

  //to avoid noise, sample the pin several times and average the result
  for(int i=1; i<=rounds; i++) {
    value += adc1_get_raw(ADC1_CHANNEL_6);
  }
  value /= (uint32_t)rounds;

  //due to the voltage divider (1M+1M) values must be multiplied by 2
  //and convert mV to V

  snprintf(buffer, sizeof(buffer), "%4.3f V", esp_adc_cal_raw_to_voltage(value, &adc_chars)*2.0/1000.0);
  return buffer;
}*/