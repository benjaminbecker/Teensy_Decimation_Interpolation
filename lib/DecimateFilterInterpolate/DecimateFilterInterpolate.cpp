#include "DecimateFilterInterpolate.h"

void AudioEffectDecimateFilterInterpolate::update(void)
{
  audio_block_t *blockINPUT, *blockOUTPUT, *blockBUFFER;

  blockINPUT = receiveReadOnly(0);

  if (!blockINPUT) {
    release(blockINPUT);
    return;
  }

  blockBUFFER = allocate();

  if (true){
    // decimate
    arm_fir_decimate_q15(
      &decimate_fir,
      (q15_t *) blockINPUT->data,
      (q15_t *) blockBUFFER->data,
      AUDIO_BLOCK_SAMPLES);
    // convert to q31
    arm_q15_to_q31 ((q15_t *) blockBUFFER->data, buffer_q31_1, DOWNSAMPLING_BLOCK_SAMPLES);
    // scale down by 1/8
    arm_scale_q31(buffer_q31_1, 0x7FFFFFFF, -3, buffer_q31_2, DOWNSAMPLING_BLOCK_SAMPLES);

    // buffer_q31_2 must not be changed inside following for loop

    for (uint8_t id = 0; id < BIQUAD_MAX_CASCADES; id++){
      if (biquad_cascade_on[id]){
        // filter (biquad cascades)
        arm_biquad_cas_df1_32x64_q31(
          &(biquad_cascade[id]),
          buffer_q31_2,
          buffer_q31_1,
          DOWNSAMPLING_BLOCK_SAMPLES);
        // scale up by 8
        arm_scale_q31(buffer_q31_1, 0x7FFFFFFF, 3, buffer_q31_1, DOWNSAMPLING_BLOCK_SAMPLES);
        // convert to q15
        arm_q31_to_q15 (buffer_q31_1, (q15_t *) blockBUFFER->data, DOWNSAMPLING_BLOCK_SAMPLES);
        // allocate output block
        blockOUTPUT = allocate();
        // interpolate
        // todo: every channel needs its own state
        arm_fir_interpolate_q15(
          &(interpolate_fir[id]),
          (q15_t *) blockBUFFER->data,
          (q15_t *) blockOUTPUT->data,
          DOWNSAMPLING_BLOCK_SAMPLES);
        arm_scale_q15(blockOUTPUT->data, 0x7FFF, 3, blockOUTPUT->data, AUDIO_BLOCK_SAMPLES);
        transmit(blockOUTPUT, id);
        release(blockOUTPUT);
      }
    }
  }
  else  // error allocating memory, give up
  {
    transmit(blockINPUT);
  }
  release(blockINPUT);
  release(blockBUFFER);

}
