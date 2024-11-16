#include <bits/stdc++.h>
#include <cstdint>
#include <portaudio.h>

using namespace std;

#define SAMPLE_RATE 44100
#define FRAME_PER_BUFFER 512

static void checkErr(PaError err)
{
  if (err != paNoError)
  {
    printf("Portaudio error : %s\n", Pa_GetErrorText(err));
    exit(EXIT_FAILURE);
  }
}

static inline float max(float a, float b)
{
  return a > b ? a : b;
}
static inline float Abs(float a)
{
  return a > 0 ? a : -a;
}

static int PaTestCallback(const void *inputBuf, void *outputBuf, unsigned long framesPerBuffer,
                          const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags, void *userData)
{
  int16_t *in = (int16_t *)inputBuf;

  (void)outputBuf;

  int16_t dispSize = 200;
  printf("\r");

  int16_t vol_l = 0;
  int16_t vol_r = 0;

  for (unsigned long i = 0; i < framesPerBuffer * 2; i++)
  {
    vol_l = max(Abs(in[i]), vol_l);
    vol_r = max(Abs(in[i + 1]), vol_r);
  }
  vol_l = max(vol_l - 2000, 0);
  vol_r = max(vol_r - 2000, 0);
  float l = vol_l * 1000.0 / 32767;
  float r = vol_r * 1000.0 / 32767;

  // cout << l << " " << r << endl;

  for (int16_t i = 0; i < dispSize; i++)
  {
    float barProportion = i * 1000.0 / dispSize;
    if (barProportion <= l && barProportion <= r)
    {
      printf("█");
    }
    else if (barProportion <= l)
    {
      printf("▀");
    }
    else if (barProportion <= r)
    {
      printf("▄");
    }
    else
    {
      printf(" ");
    }
  }

  fflush(stdout);

  return 0;
}

int main()
{
  PaError err;

  err = Pa_Initialize();
  checkErr(err);

  int numDevices = Pa_GetDeviceCount();
  printf("Number of devices: %d\n", numDevices);

  if (numDevices < 0)
  {
    printf("Portaudio error in getting devices");
    exit(EXIT_FAILURE);
  }
  else if (numDevices == 0)
  {
    printf("No Devices");
    exit(EXIT_SUCCESS);
  }

  const PaDeviceInfo *deviceInfo;
  for (int i = 0; i < numDevices; i++)
  {
    deviceInfo = Pa_GetDeviceInfo(i);
    printf("Device number: %d\n", i);
    printf("Device Name: %s\n", deviceInfo->name);
    printf("Device number of input channels: %d\n", deviceInfo->maxInputChannels);
    printf("Device number of output channels: %d\n", deviceInfo->maxOutputChannels);
    printf("Device Sample Rate: %f\n", deviceInfo->defaultSampleRate);
  }

  int device = 11;
  float data;

  PaStreamParameters inputParameters;
  PaStreamParameters outputParameters;

  memset(&inputParameters, 0, sizeof(inputParameters));
  inputParameters.channelCount = 2;
  inputParameters.device = device;
  inputParameters.hostApiSpecificStreamInfo = NULL;
  inputParameters.sampleFormat = paInt16;
  inputParameters.suggestedLatency = Pa_GetDeviceInfo(device)->defaultLowInputLatency;

  memset(&outputParameters, 0, sizeof(outputParameters));
  outputParameters.channelCount = 2;
  outputParameters.device = device;
  outputParameters.hostApiSpecificStreamInfo = NULL;
  outputParameters.sampleFormat = paFloat32;
  outputParameters.suggestedLatency = Pa_GetDeviceInfo(device)->defaultLowInputLatency;

  PaStream *stream;
  err = Pa_OpenStream(&stream, // Address of pointer of stream
                      &inputParameters, &outputParameters, SAMPLE_RATE, FRAME_PER_BUFFER, paNoFlag, PaTestCallback,
                      &data // User Data
  );
  checkErr(err);

  err = Pa_StartStream(stream);
  checkErr(err);

  Pa_Sleep(20 * 1000);

  err = Pa_CloseStream(stream);
  checkErr(err);

  err = Pa_Terminate();
  checkErr(err);
  return 0;
}
