#ifndef decimate_filter_interpolate_h_
#define decimate_filter_interpolate_h_

#include <Arduino.h>
#include "AudioStream.h"
#include "utility/dspinst.h"
#include "arm_math.h"

#define FIR_MAX_COEFFS 200
#define FIR_PASSTHRU ((const short *) 1)
#define BIQUAD_MAX_STAGES 10 // the number of stages per biquad cascade
#define BIQUAD_MAX_CASCADES 10 // the number of parallel cascades

class AudioEffectDecimateFilterInterpolate : public AudioStream
{
  public:
    AudioEffectDecimateFilterInterpolate() : AudioStream(1, inputQueueArray) {
      // switch all biquad cascades off at initialization
      for (uint8_t id = 0; id < BIQUAD_MAX_CASCADES; id++) biquad_cascade_on[id] = false;
    }

    // begin( decimate_coeffs, interpolate_coeffs, interpolate_coeffs, interpolate_coeffs, decimation_factor, interpolate_factor

    void begin(const short *coeff_decimate, int decimate_coeffs, const short *coeff_interpolate, int interpolation_coeffs, uint8_t decimation_factor) {
      d_factor = decimation_factor;
      DOWNSAMPLING_BLOCK_SAMPLES = AUDIO_BLOCK_SAMPLES/decimation_factor;

      // Initialize FIR decimate&interpolate instance (ARM DSP Math Library)
      arm_fir_decimate_init_q15(
        &decimate_fir,
        decimate_coeffs,
        decimation_factor,
        (q15_t *)coeff_decimate,
        &dec_state[0],
        AUDIO_BLOCK_SAMPLES);

      for (uint8_t id = 0; id < BIQUAD_MAX_CASCADES; id++){
        arm_fir_interpolate_init_q15 (
          &(interpolate_fir[id]),
          decimation_factor,
          interpolation_coeffs,
          (q15_t *)coeff_interpolate,
          &(interp_state[(AUDIO_BLOCK_SAMPLES + FIR_MAX_COEFFS)*id]),
          DOWNSAMPLING_BLOCK_SAMPLES);
      }
    }

    void setCoefficients(int8_t id, const q31_t *coeff_biquad, uint8_t numStagesBiquad, int8_t postShift){
      // Initialize Biquad filter
      arm_biquad_cas_df1_32x64_init_q31(&(biquad_cascade[id]), numStagesBiquad, (q31_t *)coeff_biquad, &(biquad_state[id*BIQUAD_MAX_STAGES*4]), postShift);
      // turn Biquad cascade on
      biquad_cascade_on[id] = true;
    }



    virtual void update(void);

  private:
    // input to audio effect
    audio_block_t *inputQueueArray[1];

    // pointer to current coefficients or NULL or FIR_PASSTHRU
    const short *coeff_decimate;

    // instance structures for decimation, interpolation and biquad
    arm_fir_decimate_instance_q15 decimate_fir;
    arm_fir_interpolate_instance_q15 interpolate_fir[BIQUAD_MAX_CASCADES];
    arm_biquad_cas_df1_32x64_ins_q31 biquad_cascade[BIQUAD_MAX_CASCADES];

    // state arrays for decimation, interpolation and biquad
    q15_t dec_state[AUDIO_BLOCK_SAMPLES + FIR_MAX_COEFFS];
    q15_t interp_state[(AUDIO_BLOCK_SAMPLES + FIR_MAX_COEFFS)*BIQUAD_MAX_CASCADES];
    q63_t biquad_state[BIQUAD_MAX_CASCADES*BIQUAD_MAX_STAGES*4];

    q31_t buffer_q31_1[AUDIO_BLOCK_SAMPLES];
    q31_t buffer_q31_2[AUDIO_BLOCK_SAMPLES];

    // boolean array to store which biquad cascades are used
    bool biquad_cascade_on[BIQUAD_MAX_CASCADES];

    // decimation factor
    uint8_t d_factor;

    // block length for downsampled signal
    uint16_t DOWNSAMPLING_BLOCK_SAMPLES;
};


#endif
