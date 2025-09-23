#include <Arduino.h>
#include "driver/i2s.h"
#include "freertos/ringbuf.h"

#define MAX98357_I2S_NUM  I2S_NUM_0 // Which I2S port to use
#define SAMPLE_RATE       16000  // Audio sampling rate
#define MAX98357_DOUT     38  // max98357 pin, please refer to: https://www.yuque.com/welinklab/pbihut/sdnm396nt3rmcfne
#define MAX98357_LRC      40
#define MAX98357_BCLK     39

#define MICROPHONE_I2S_NUM             I2S_NUM_1
#define AUDIO_SAMPLE_RATE              16000
#define MICROPHONE_I2S_BCLK            42
#define MICROPHONE_I2S_LRC             2
#define MICROPHONE_I2S_DOUT            1

#define  READ_SAMPLE_COUNT 80000  // Define the recording length, a total of 80K samples, for 16K sampling rate, it is to record 5 seconds

int16_t buffer[READ_SAMPLE_COUNT];
size_t bytesRead, bytesWritten;

RingbufHandle_t ringBuffer;

void playAudio(void* ptr);

void setup()
{
    // Build i2s configuration structure
    constexpr i2s_config_t max98357_i2s_config = {
        // I2S sends data to digital amplifier, so it is TX mode
        .mode = static_cast<i2s_mode_t>(I2S_MODE_MASTER | I2S_MODE_TX),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT, // 16-bit width
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT, // Left channel playback
        .communication_format = I2S_COMM_FORMAT_STAND_I2S, // Standard I2S protocol
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1, // Interrupt priority. If real-time requirements are high, priority can be raised.
        .dma_buf_count = 4, // Number of DMA buffers
        .dma_buf_len = 1024, // The number of audio samples that can be saved in each buffer. If the value is too large, there will be a delay in playing the audio. If the value is too small, it may cause the audio playback to be stuttered.
        .tx_desc_auto_clear = true // Automatically clean up DMA descriptors after data transmission is completed, which is simple and convenient, and can prevent memory leakage or DMA buffer overflow
    };

    // Define max98357 related pins
    constexpr i2s_pin_config_t max98357_gpio_config = {
        .bck_io_num = MAX98357_BCLK,
        .ws_io_num = MAX98357_LRC,
        .data_out_num = MAX98357_DOUT,
        .data_in_num = -1
    };

    // Start the I2S driver
    i2s_driver_install(MAX98357_I2S_NUM, &max98357_i2s_config, 0, nullptr);
    // Make relevant configurations effective
    i2s_set_pin(MAX98357_I2S_NUM, &max98357_gpio_config);


    constexpr i2s_config_t mic_i2s_config = {
        .mode = static_cast<i2s_mode_t>(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = AUDIO_SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT, // The left and right channels here should be consistent with the circuit
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 4,
        .dma_buf_len = 1024,
        .use_apll = false
    };
    constexpr i2s_pin_config_t mic_gpio_config = {
        .bck_io_num = MICROPHONE_I2S_BCLK,
        .ws_io_num = MICROPHONE_I2S_LRC,
        .data_out_num = -1,
        .data_in_num = MICROPHONE_I2S_DOUT
    };

    i2s_driver_install(MICROPHONE_I2S_NUM, &mic_i2s_config, 0, nullptr);
    i2s_set_pin(MICROPHONE_I2S_NUM, &mic_gpio_config);

    Serial.begin(9600);

    ringBuffer = xRingbufferCreate(160000 * sizeof(int16_t), RINGBUF_TYPE_BYTEBUF);
    if (ringBuffer == nullptr)
    {
        Serial.println("ringBuffer creation failed");
        ESP.restart();
    }
    // Start the audio playback task
    xTaskCreate(playAudio, "playAudioTask", 64000, nullptr, 1, nullptr);
}

// Get the recorded audio data from the RingBuffer, perform volume gain, and finally playback
void playAudio(void* ptr)
{
    size_t readItemSize;
    while (true)
    {
        void* buffer = xRingbufferReceive(ringBuffer, &readItemSize, pdMS_TO_TICKS(1000));
        if (buffer != nullptr)
        {
            auto* audioData = static_cast<int16_t*>(buffer);
            for (int i = 0; i < readItemSize; i++)
            {
                // Because the recording volume is small (maybe the motherboard recording hole is too small, it will affect it)
                // So gain the audio and limit it to the effective range
                // Note that when int32_t is used here, it is mainly to prevent overflow
                auto value = static_cast<int32_t>(audioData[i] * 10.0);
                if (value > 32767) value = 32767;
                if (value < -32768) value = -32768;
                audioData[i] = static_cast<int16_t>(value);
            }
            // Write data to the digital amplifier I2S channel through i2s_write to perform audio playback
            i2s_write(MAX98357_I2S_NUM, audioData, bytesRead,
                      &bytesWritten, portMAX_DELAY);
            // After processing, remember to return the buffer memory
            vRingbufferReturnItem(ringBuffer, buffer);
        }
    }
    vTaskDelete(nullptr);
}

void loop()
{
    if (Serial.available()) // Determine whether there is data input on the serial port, and start recording with input input.
    {
        Serial.readStringUntil('\n');
        Serial.println("Recording...");
        // Record audio from I2S channel through the i2s_read function
        // and save it to the buffer array, bytesRead is the final number of bytes read
        const esp_err_t err = i2s_read(MICROPHONE_I2S_NUM, buffer,
                                       80000 * sizeof(int16_t),
                                       &bytesRead, portMAX_DELAY);
        if (err != ESP_OK)
        {
            Serial.println("I2S read failed");
        }
        else
        {
            // Write recording data to ringBuffer
            xRingbufferSend(ringBuffer, buffer, bytesRead, portMAX_DELAY);
        }
    }
}
