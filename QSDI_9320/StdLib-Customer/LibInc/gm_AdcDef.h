#ifndef __ADC_DEF_H__
#define __ADC_DEF_H__

// values from "Sequoia BandGap based ADC calibration programming model", Harish Gogna 070613
#define POST_OFFSET1_CALIB_LOW_GAIN_1_3V        0x52	// ideal gain for calibrating a signal of 1.3V (low gain mode)
#define POST_OFFSET1_CALIB_HIGH_GAIN_0_7V_Y     0x80	// ideal Y gain for calibrating a signal of 0.7V (high gain mode)
#define POST_OFFSET1_CALIB_HIGH_GAIN_0_7V_PbPr  0x90	// ideal PbPr gain for calibrating a signal of 0.7V (high gain mode)
#define POST_OFFSET1_CALIB_HIGH_GAIN_0_7V       0x82	// ideal gain for calibrating a non-component signal of 0.7V (high gain mode)

#define NR_OF_SAMPLES				32		// Number of PixelGrab samples 

#define MIN_OFFSET1_VALUE			0x00	// Minimum ADC Offset1 value
#define MAX_OFFSET1_VALUE			0x7F	// Maximum ADC Offset1 value
#define MID_OFFSET1_VALUE			0x40	// Mid ADC Offset1 value

// STDP93xx XZS2 is a 9-bit DC offset, but offset2 MSB, XZS2[8], is not used 
#define MIN_OFFSET2_VALUE			0x00	// Minimum ADC Offset2 value
#define MAX_OFFSET2_VALUE			0xFF	// Maximum ADC Offset2 value
#define MID_OFFSET2_VALUE			0x80	// Mid ADC Offset2 value

#define MIN_ADCGAIN_VALUE			0x00	// Minimum ADC gain value
#define MAX_ADCGAIN_VALUE			0xFF	// Maximum ADC gain  value
#define MID_ADCGAIN_VALUE			0x80	// Mid ADC gain  value

#endif
