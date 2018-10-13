// Test von arm_fir_decimate_q15 und arm_fir_interpolate_q15
// Die SampleRate des Signals (Sinus, 100 Hz) wird zuerst um den Faktor 4 herabgesetzt
// und anschließend wieder um den Faktor 4 heraufgesetzt

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>

#include <DecimateFilterInterpolate.h>

// GUItool: begin automatically generated code
AudioSynthWaveformSine   sine1;          //xy=55,269
AudioSynthWaveformSine   sine2; //xy=61,313
AudioMixer4              mixer1;         //xy=244,257
AudioEffectDecimateFilterInterpolate           decimation;           //xy=412,204
AudioEffectDelay         delay1;         //xy=414,302
AudioOutputUSB           usb2;           //xy=835,224
AudioOutputI2S           i2s1;           //xy=835,298
AudioConnection          patchCord1(sine1, 0, mixer1, 0);
AudioConnection          patchCord2(sine2, 0, mixer1, 1);
AudioConnection          patchCord3(mixer1, decimation);
AudioConnection          patchCord4(mixer1, delay1);
AudioConnection          patchCord5(decimation, 0, usb2, 0);
AudioConnection          patchCord6(decimation, 2, usb2, 1);
// AudioConnection          patchCord6(delay1, 0, usb2, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=418,436
// GUItool: end automatically generated code



/*

FIR filter designed with
http://t-filter.appspot.com

sampling frequency: 44117 Hz

fixed point precision: 16 bits

* 0 Hz - 1000 Hz
  gain = 1
  desired ripple = 0.5 dB
  actual ripple = n/a

* 3000 Hz - 22050 Hz
  gain = 0
  desired attenuation = -40 dB
  actual attenuation = n/a

*/

// todo: design shorter filter

#define FILTER_TAP_NUM_DEC 64

static q15_t filter_taps_decimation[FILTER_TAP_NUM_DEC] = {
  35, 36, 50, 63, 74, 79, 75, 61, 33, -8, -62, -127, -198, -269, -333, -379, -399,
  -382, -321, -210, -45, 174, 442, 752, 1091, 1444, 1794, 2123, 2413, 2647, 2811,
  2895, 2895, 2811, 2647, 2413, 2123, 1794, 1444, 1091, 752, 442, 174, -45, -210,
  -321, -382, -399, -379, -333, -269, -198, -127, -62, -8, 33, 61, 75, 79, 74, 63,
  50, 36, 35
};

/*

FIR filter designed with
http://t-filter.appspot.com

sampling frequency: 44100 Hz

fixed point precision: 16 bits

* 0 Hz - 1000 Hz
  gain = 16
  desired ripple = 1 dB
  actual ripple = n/a

* 2000 Hz - 22050 Hz
  gain = 0
  desired attenuation = -40 dB
  actual attenuation = n/a

*/

#define FILTER_TAP_NUM_INTERP 128

static q15_t filter_taps_interpolation[FILTER_TAP_NUM_INTERP] = {
  -123, -96, -129, -163, -196, -227, -252, -267, -269, -254, -217, -157, -69, 46, 191, 362, 558, 773, 1000, 1232, 1458, 1666, 1843, 1977, 2053, 2060, 1986, 1822, 1562,
  1203, 748, 202, -424, -1114, -1845, -2593, -3327, -4013, -4616, -5098, -5423, -5553, -5457, -5106, -4477, -3554, -2331, -808, 1003, 3082, 5398, 7913, 10578, 13339, 16136,
   18905, 21580, 24095, 26388, 28399, 30076, 31374, 32260, 32709, 32709, 32260, 31374, 30076, 28399, 26388, 24095, 21580, 18905, 16136, 13339, 10578, 7913, 5398, 3082, 1003,
    -808, -2331, -3554, -4477, -5106, -5457, -5553, -5423, -5098, -4616, -4013, -3327, -2593, -1845, -1114, -424, 202, 748, 1203, 1562, 1822, 1986, 2060, 2053, 1977, 1843,
     1666, 1458, 1232, 1000, 773, 558, 362, 191, 46, -69, -157, -217, -254, -269, -267, -252, -227, -196, -163, -129, -96, -123
};

// octave bandpass 125 Hz
static uint8_t filter1NumStages = 10;
static q31_t filter1Coefficients[] = {
  53433806, 0, -53433806, 2082832655, -1051644118,
  53433806, 0, -53433806, 2125286334, -1062472756,
  52666543, 0, -52666543, 2046357703, -1012019932,
  52666543, 0, -52666543, 2102387853, -1040111187,
  52039249, 0, -52039249, 2021958535, -982800038,
  52039249, 0, -52039249, 2078882853, -1017895034,
  51597297, 0, -51597297, 2012271174, -967419986,
  51597297, 0, -51597297, 2055164665, -996431716,
  51369419, 0, -51369419, 2033181698, -977926095,
  51369419, 0, -51369419, 2017141007, -966648623
};

// octave bandpass 250 Hz
static uint8_t filter2NumStages = 10;
static q31_t filter2Coefficients[] = {
  105927207, 0, -105927207, 1937322159, -1030421981,
  105927207, 0, -105927207, 2081577971, -1051208202,
  103005980, 0, -103005980, 1876478361, -954687099,
  103005980, 0, -103005980, 2035903863, -1007194117,
  100702521, 0, -100702521, 1841960164, -900109296,
  100702521, 0, -100702521, 1988530417, -964375280,
  99122651, 0, -99122651, 1939825122, -923875769,
  99122651, 0, -99122651, 1835569270, -871468694,
  98321330, 0, -98321330, 1892865690, -889720821,
  98321330, 0, -98321330, 1854916562, -869484548
};

// octave bandpass 500 Hz
static uint8_t filter3NumStages = 10;
static q31_t filter3Coefficients[] = {
  207794311, 0, -207794311, 1436391189, -992105720,
  207794311, 0, -207794311, 1932095488, -1028226288,
  197268893, 0, -197268893, 1370837211, -855021815,
  197268893, 0, -197268893, 1843145963, -942131840,
  189472091, 0, -189472091, 1749097753, -861839615,
  189472091, 0, -189472091, 1355612429, -758599378,
  184353910, 0, -184353910, 1649343094, -789100416,
  184353910, 0, -184353910, 1386030675, -706501245,
  181824207, 0, -181824207, 1547442443, -730750744,
  181824207, 0, -181824207, 1454362361, -699118969,
};


void setup() {
    // put your setup code here, to run once:
    // turn off LED
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
    // Audio Memory
    AudioMemory(100);
    // delay parallel to decimated signal to prevent phase difference
    // delay by FILTER_TAP_NUM_DEC samples, FILTER_TAP_NUM_DEC/2 for decimation and
    // FILTER_TAP_NUM_DEC/2 for interpolation
    float delayValue = FILTER_TAP_NUM_DEC/AUDIO_SAMPLE_RATE_EXACT*1000; //in ms
    delay1.delay(0, delayValue); //channels 0
    // decimation filter with reduction by factor 4
    int decimation_factor = 8;
    decimation.begin(filter_taps_decimation, FILTER_TAP_NUM_DEC, filter_taps_decimation, FILTER_TAP_NUM_DEC, decimation_factor);
    int8_t postShift = 1;
    decimation.setCoefficients(0, filter1Coefficients, filter1NumStages, postShift);
    decimation.setCoefficients(1, filter2Coefficients, filter2NumStages, postShift);
    decimation.setCoefficients(2, filter3Coefficients, filter3NumStages, postShift);
    // enable audio board
    sgtl5000_1.enable();
    sgtl5000_1.volume(0.5);
    sine1.frequency(125);
    sine1.amplitude(0.5);
    sine2.frequency(613);
    sine2.amplitude(0.5);
    mixer1.gain(0,1.0);
    mixer1.gain(1,1.0);
}

void loop() {
    // put your main code here, to run repeatedly:
}
