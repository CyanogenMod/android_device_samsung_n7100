/*
 * Copyright (C) 2011 The Android Open Source Project
 * Copyright (C) 2012 Wolfson Microelectronics plc
 * Copyright (C) 2012 The CyanogenMod Project
 *               Daniel Hillenbrand <codeworkx@cyanogenmod.com>
 *               Guillaume "XpLoDWilD" Lesniak <xplodgui@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/* ALSA cards for WM1811 */
#define CARD_DEFAULT  0

#define PORT_PLAYBACK 0
#define PORT_MODEM    1
#define PORT_BT       2
#define PORT_CAPTURE  3

#define PCM_WRITE pcm_write

#define PLAYBACK_PERIOD_SIZE  880
#define PLAYBACK_PERIOD_COUNT 8
#define PLAYBACK_SHORT_PERIOD_COUNT 2

#define CAPTURE_PERIOD_SIZE   1024
#define CAPTURE_PERIOD_COUNT  4

#define SHORT_PERIOD_SIZE 192

//
// deep buffer
//
/* screen on */
#define DEEP_BUFFER_SHORT_PERIOD_SIZE 1056
#define PLAYBACK_DEEP_BUFFER_SHORT_PERIOD_COUNT 4
/* screen off */
#define DEEP_BUFFER_LONG_PERIOD_SIZE 880
#define PLAYBACK_DEEP_BUFFER_LONG_PERIOD_COUNT 8


/* minimum sleep time in out_write() when write threshold is not reached */
#define MIN_WRITE_SLEEP_US 5000

#define RESAMPLER_BUFFER_FRAMES (PLAYBACK_PERIOD_SIZE * 2)
#define RESAMPLER_BUFFER_SIZE (4 * RESAMPLER_BUFFER_FRAMES)

#define DEFAULT_OUT_SAMPLING_RATE 44100
#define MM_LOW_POWER_SAMPLING_RATE 44100
#define MM_FULL_POWER_SAMPLING_RATE 44100
#define DEFAULT_IN_SAMPLING_RATE 44100

/* sampling rate when using VX port for narrow band */
#define VX_NB_SAMPLING_RATE 8000
/* sampling rate when using VX port for wide band */
#define VX_WB_SAMPLING_RATE 16000

/* product-specific defines */
#define PRODUCT_DEVICE_PROPERTY "ro.product.device"
#define PRODUCT_NAME_PROPERTY   "ro.product.name"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

#define STRING_TO_ENUM(string) { #string, string }

struct string_to_enum {
    const char *name;
    uint32_t value;
};

const struct string_to_enum out_channels_name_to_enum_table[] = {
    STRING_TO_ENUM(AUDIO_CHANNEL_OUT_STEREO),
    STRING_TO_ENUM(AUDIO_CHANNEL_OUT_5POINT1),
    STRING_TO_ENUM(AUDIO_CHANNEL_OUT_7POINT1),
};

enum pcm_type {
    PCM_NORMAL = 0,
    PCM_SPDIF,
    PCM_HDMI,
    PCM_TOTAL,
};

enum output_type {
    OUTPUT_DEEP_BUF,      // deep PCM buffers output stream
    OUTPUT_LOW_LATENCY,   // low latency output stream
    OUTPUT_HDMI,
    OUTPUT_TOTAL
};

enum tty_modes {
    TTY_MODE_OFF,
    TTY_MODE_VCO,
    TTY_MODE_HCO,
    TTY_MODE_FULL
};

struct mixer_ctls
{
    struct mixer_ctl *mixinl_in1l_volume;
    struct mixer_ctl *mixinl_in2l_volume;
};

struct route_setting
{
    char *ctl_name;
    int intval;
    char *strval;
};

struct route_setting voicecall_default[] = {
    { .ctl_name = "AIF2 Mode", .intval = 0, },
    { .ctl_name = "DAC1L Mixer AIF1.1 Switch", .intval = 1, },
    { .ctl_name = "DAC1R Mixer AIF1.1 Switch", .intval = 1, },
    { .ctl_name = "DAC1L Mixer AIF2 Switch", .intval = 1, },
    { .ctl_name = "DAC1R Mixer AIF2 Switch", .intval = 1, },
    { .ctl_name = "AIF2DAC Mux", .strval = "AIF2DACDAT", },
    { .ctl_name = NULL, },
};

struct route_setting voicecall_default_disable[] = {
    { .ctl_name = "AIF2 Mode", .intval = 0, },
    { .ctl_name = "DAC1L Mixer AIF2 Switch", .intval = 0, },
    { .ctl_name = "DAC1R Mixer AIF2 Switch", .intval = 0, },
    { .ctl_name = "AIF2DAC Mux", .strval = "AIF3DACDAT", },
    { .ctl_name = "Main Mic Switch", .intval = 0, },
    { .ctl_name = "MIXINL IN2L Switch", .intval = 0, },
    { .ctl_name = "Sub Mic Switch", .intval = 0, },
    { .ctl_name = "MIXINR IN1R Switch", .intval = 0, },
    { .ctl_name = NULL, },
};

struct route_setting default_input[] = {
    { .ctl_name = "Main Mic Switch", .intval = 1, },
    { .ctl_name = "MainMicBias Mode", .intval = 1, },
    { .ctl_name = "IN2L Volume", .intval = 28, },
    { .ctl_name = "MIXINL IN2L Switch", .intval = 1, },
    { .ctl_name = "MIXINL IN2L Volume", .intval = 0, },
    { .ctl_name = "AIF1ADC1 HPF Mode", .intval = 0, },
    { .ctl_name = "AIF1ADC1 HPF Switch", .intval = 1, },
    { .ctl_name = NULL, },
};

struct route_setting default_input_disable[] = {
    { .ctl_name = "Main Mic Switch", .intval = 0, },
    { .ctl_name = "IN2L Volume", .intval = 4, },
    { .ctl_name = "MIXINL IN2L Switch", .intval = 0, },
    { .ctl_name = "AIF1ADC1 HPF Switch", .intval = 0, },
    { .ctl_name = NULL, },
};

struct route_setting noise_suppression[] = {
    { .ctl_name = "Sub Mic Switch", .intval = 1, },
    { .ctl_name = "IN1R Volume", .intval = 25, },
    { .ctl_name = "MIXINR IN1R Switch", .intval = 1, },
    { .ctl_name = "MIXINR IN1R Volume", .intval = 0, },
    { .ctl_name = "AIF1ADCR Source", .intval = 1, },
    { .ctl_name = NULL, },
};

struct route_setting noise_suppression_disable[] = {
    { .ctl_name = "Sub Mic Switch", .intval = 0, },
    { .ctl_name = "IN1R Volume", .intval = 7, },
    { .ctl_name = "MIXINR IN1R Switch", .intval = 0, },
    { .ctl_name = "MIXINR IN1R Volume", .intval = 0, },
    { .ctl_name = NULL, },
};

struct route_setting headset_input[] = {
    { .ctl_name = "MIXINL IN2L Switch", .intval = 0, },
    { .ctl_name = "MIXINR IN1R Switch", .intval = 0, },
    { .ctl_name = "Headset Mic Switch", .intval = 1, },
    { .ctl_name = "IN1L Volume", .intval = 18, },
    { .ctl_name = "MIXINL IN1L Switch", .intval = 1, },
    { .ctl_name = "MIXINL IN1L Volume", .intval = 0, },
    { .ctl_name = "AIF1ADC1 HPF Mode", .intval = 1, },
    { .ctl_name = "AIF1ADC1 HPF Switch", .intval = 1, },
    { .ctl_name = "AIF1ADC1 Volume", .intval = 96, },
    { .ctl_name = "AIF1ADCL Source", .intval = 0, },
    { .ctl_name = "AIF1ADCR Source", .intval = 0, },
    { .ctl_name = NULL, },
};

struct route_setting headset_input_disable[] = {
    { .ctl_name = "Headset Mic Switch", .intval = 0, },
    { .ctl_name = "MIXINL IN1L Switch", .intval = 0, },
    { .ctl_name = "AIF1ADC1 HPF Mode", .intval = 0, },
    { .ctl_name = "AIF1ADC1 HPF Switch", .intval = 0, },
    { .ctl_name = NULL, },
};

struct route_setting bt_output[] = {
    { .ctl_name = "AIF1DAC1 Volume", .intval = 96, },
    { .ctl_name = "AIF1 Boost Volume", .intval = 0, },
    { .ctl_name = "DAC2 Volume", .intval = 96, },
    { .ctl_name = "AIF2ADC Volume", .intval = 96, },
    { .ctl_name = "DAC1L Mixer AIF1.1 Switch", .intval = 1, },
    { .ctl_name = "DAC1R Mixer AIF1.1 Switch", .intval = 1, },
    { .ctl_name = "AIF3ADC Mux", .intval = 1, },
    { .ctl_name = "AIF2DAC2L Mixer AIF1.1 Switch", .intval = 1, },
    { .ctl_name = "AIF2DAC2R Mixer AIF1.1 Switch", .intval = 1, },
    { .ctl_name = "AIF2DAC Volume", .intval = 96, },
    { .ctl_name = "MIXINL IN1L Volume", .intval = 1, },
    { .ctl_name = "IN2L Volume", .intval = 28, },
    { .ctl_name = "IN1R Volume", .intval = 28, },
    { .ctl_name = "LINEOUT1N Switch", .intval = 0, },
    { .ctl_name = "LINEOUT1P Switch", .intval = 0, },
    { .ctl_name = "AIF1ADC1 HPF Switch", .intval = 0, },
    { .ctl_name = "AIF2ADC HPF Mode", .intval = 3, },
    { .ctl_name = "AIF2ADC HPF Switch", .intval = 1, },
    { .ctl_name = "AIF2DAC Mux", .strval = "AIF2DACDAT", },
    { .ctl_name = "AIF2DAC2R Mixer AIF2 Switch", .intval = 1, },
    { .ctl_name = "AIF2DAC2L Mixer AIF2 Switch", .intval = 1, },
    { .ctl_name = NULL, },
};

struct route_setting bt_input[] = {
    { .ctl_name = "AIF2ADC Mux", .intval = 1, },
    { .ctl_name = "AIF1ADCL Source", .intval = 0, },
    { .ctl_name = "AIF1ADCR Source", .intval = 1, },
    { .ctl_name = "DAC1L Mixer AIF2 Switch", .intval = 1, },
    { .ctl_name = "DAC1R Mixer AIF2 Switch", .intval = 1, },
    { .ctl_name = "AIF1ADC1R Mixer AIF2 Switch", .intval = 1, },
    { .ctl_name = "AIF1ADC1L Mixer AIF2 Switch", .intval = 1, },
    { .ctl_name = "AIF1ADC1 Volume", .intval = 96, },
    { .ctl_name = "AIF2DAC Volume", .intval = 96, },
    { .ctl_name = NULL, },
};

struct route_setting bt_disable[] = {
    { .ctl_name = "AIF1DAC1 Volume", .intval = 96, },
    { .ctl_name = "AIF1 Boost Volume", .intval = 0, },
    { .ctl_name = "DAC2 Volume", .intval = 96, },
    { .ctl_name = "AIF2ADC Volume", .intval = 96, },
    { .ctl_name = "AIF2ADC Mux", .intval = 0, },
    { .ctl_name = "MIXINL IN1L Volume", .intval = 0, },
    { .ctl_name = "LINEOUT1N Switch", .intval = 1, },
    { .ctl_name = "LINEOUT1P Switch", .intval = 1, },
    { .ctl_name = "AIF2ADC HPF Mode", .intval = 0, },
    { .ctl_name = "AIF2ADC HPF Switch", .intval = 0, },
    { .ctl_name = "AIF2DAC2R Mixer AIF2 Switch", .intval = 0, },
    { .ctl_name = "AIF2DAC2L Mixer AIF2 Switch", .intval = 0, },
    { .ctl_name = "AIF1ADC1R Mixer AIF2 Switch", .intval = 0, },
    { .ctl_name = "AIF1ADC1L Mixer AIF2 Switch", .intval = 0, },
    { .ctl_name = NULL, },
};
