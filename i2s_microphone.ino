#include <driver/i2s.h>
#include <limits.h>
#include "filter1.h"
#include <arduinoFFT.h>
#include "config.h"

const i2s_port_t I2S_PORT = I2S_NUM_0;

filter1Type * f1;
arduinoFFT FFT = arduinoFFT();

struct rms_state {
  float rms;
  int nbr_of_samples;
  float sum_squares;
};

struct rms_state rms_fast_t = {RMS_INITIAL, RMS_SAMPLES, 1UL * RMS_SAMPLES * RMS_INITIAL * RMS_INITIAL};

int32_t samples[I2S_BLOCK_SIZE];
float samples_mirror[I2S_BLOCK_SIZE];
float samples_filtered[I2S_BLOCK_SIZE];

double vReal[I2S_BLOCK_SIZE];
double vImag[I2S_BLOCK_SIZE];
double fft_bins[N_PIXELS];

void i2s_init() {
  xTaskCreatePinnedToCore(
        i2s_loop, /* Function to implement the task */
        "I2S", /* Name of the task */
        10000,  /* Stack size in words */
        NULL,  /* Task input parameter */
        4 ,  /* Priority of the task */
        NULL,  /* Task handle. */
        0); /* Core where the task should run */  
}

void i2s_setup() {
  //Serial.begin(2000000);
  Serial.println("Configuring I2S...");
  esp_err_t err;

  // The I2S config as per the example
  const i2s_config_t i2s_config = {
    .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX), // Receive, not transfer
    .sample_rate = 16000,
    .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT, // could only get it to work with 32bits
    .channel_format = I2S_CHANNEL_FMT_ONLY_RIGHT, // although the SEL config should be left, it seems to transmit on right
    .communication_format = i2s_comm_format_t(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_LSB),
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,     // Interrupt level 1
    .dma_buf_count = 4,                           // number of buffers
    .dma_buf_len = I2S_BLOCK_SIZE                     // samples per buffer
  };

  // The pin config as per the setup
  const i2s_pin_config_t pin_config = {
    .bck_io_num = 14,   // BCKL
    .ws_io_num = 15,    // LRCL
    .data_out_num = -1, // not used (only for speakers)
    .data_in_num = 32   // DOUT
  };

  // Configuring the I2S driver and pins.
  // This function must be called before any I2S driver read/write operations.
  err = i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
  if (err != ESP_OK) {
    Serial.printf("Failed installing driver: %d\n", err);
    while (true);
  }
  err = i2s_set_pin(I2S_PORT, &pin_config);
  if (err != ESP_OK) {
    Serial.printf("Failed setting pin: %d\n", err);
    while (true);
  }
  Serial.println("I2S driver installed.");

  Serial.println("Create audio filter.");
  f1 = filter1_create();

}

float rms_filter(float sample, struct rms_state * state)
{
  state->sum_squares -= state->sum_squares / state->nbr_of_samples;
  state->sum_squares += sample * sample;
  if (state->rms == 0) state->rms = 1;    /* do not divide by zero */
  state->rms = (state->rms + state->sum_squares / state->nbr_of_samples / state->rms) / 2.0f;
  return state->rms;
}

void fft_stuff(float * data, size_t size)
{
  for (int i = 0; i < size; i++)
  {
    vReal[i] = data[i];
    vImag[i] = 0.0;
  }
  for (int i = 0; i < N_PIXELS; i++)
  {
    fft_bins[i] = 0.0;
  }

  FFT.Windowing(vReal, size, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.Compute(vReal, vImag, size, FFT_FORWARD);
  FFT.ComplexToMagnitude(vReal, vImag, size);

  const int max_size = size / 4;
  double fft_data[max_size];
  memcpy(fft_data, vReal + 2, max_size*sizeof(double));

  if (N_PIXELS > max_size)
  {
    for (int i = 0; i < N_PIXELS; i++) {
      int bin = lroundf(1.0f * max_size * i / N_PIXELS);
      fft_bins[i] += fft_data[bin];
    }    
  }
  else
  {
    for (int i = 0; i < max_size; i++) {
      int bin = lroundf(1.0f * N_PIXELS * i / max_size);
      fft_bins[bin] += fft_data[i];
    }
  }

  copy_to_fft(fft_bins);

#if 0
  for (int i = 0; i < N_PIXELS; i++)
  {
    Serial.printf("%2.1f ", fft_bins[i]);
  }
  Serial.println();
#endif
}

void i2s_loop(void *) {
  i2s_setup();
  for(;;) {
    size_t samples_read = 0;

    int status = i2s_read(I2S_PORT,
                          samples,
                          I2S_BLOCK_SIZE,     // the doc says bytes, but its elements.
                          &samples_read,
                          portMAX_DELAY); // no timeout
  
    if (samples_read != I2S_BLOCK_SIZE)
    {
      Serial.println("Wrong size read");
      continue;
    }
  
    samples_read /= 4;
    if (samples_read > 0) {
      static float maxenvelope = 0;
  
      float maxval = -2147483648;
      float minval = 2147483648;
      for (int i = 0; i < samples_read; i++) {
        samples_mirror[i] = samples[i] / 3518234624.0f;
      }
  
      filter1_filterBlock( f1, samples_mirror, samples_filtered, samples_read );
      fft_stuff(samples_mirror, samples_read);
  
  
      for (int i = 0; i < samples_read; i++) {
        rms_filter(samples_filtered[i], &rms_fast_t);
        //Serial.println(samples_filtered[i]);
  
        minval = fmin(minval, samples_filtered[i]);
        maxval = fmax(maxval, samples_filtered[i]);
      }
  
      float envelope = (maxval - minval);
      rms_filter(envelope, &rms_fast_t);
    }
  }
}

// actually we would need to call `i2s_driver_uninstall(I2S_PORT)` upon exit.
