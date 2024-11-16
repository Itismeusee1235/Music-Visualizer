#include <bits/stdc++.h>
#include <portaudio.h>

using namespace std;

#define SAMPLE_RATE (44100)
#define FRAMES_PER_BUFFER (512)
#define NUM_OF_SECONDS (10)
#define NUM_OF_CHANNELS (2)
#define SAMPLE int16_t
typedef struct
{
  int frameIndex;
  int maxFrameIndex;
  SAMPLE *data;
} audioData;

void CheckErr(PaError err)
{
  if (err != paNoError)
  {
    printf("PortAudio error: %s\n", Pa_GetErrorText(err));
    exit(EXIT_FAILURE);
  }
}

int recordCallback(const void *inputBuf, void *outputBuf, unsigned long framesPerBuffer,
                   const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags, void *userData)
{
  audioData *data = (audioData *)userData;
  const SAMPLE *rptr = (SAMPLE *)(inputBuf);
  SAMPLE *wptr = &data->data[data->frameIndex * NUM_OF_CHANNELS];
  long framesToCalc;
  long i;
  int finished;
  unsigned long framesLeft = data->maxFrameIndex - data->frameIndex;

  if (framesLeft < framesPerBuffer)
  {
    framesToCalc = framesLeft;
    finished = paComplete;
  }
  else
  {
    framesToCalc = framesPerBuffer;
    finished = paContinue;
  }

  if (inputBuf == NULL)
  {
    *wptr++ = 0.0f;
    if (NUM_OF_CHANNELS == 2)
      *wptr++ = 0.0f;
  }
  else
  {
    for (i = 0; i < framesToCalc; i++)
    {
      *wptr++ = *rptr++;
      if (NUM_OF_CHANNELS == 2)
        *wptr++ = *rptr++;
    }
  }

  data->frameIndex += framesToCalc;

  return finished;
}

int outputCallback(const void *intputbuf, void *outputBuf, unsigned long framesPerBuffer,
                   const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags, void *userData)
{
  audioData *data = (audioData *)userData;
  SAMPLE *wptr = (SAMPLE *)outputBuf;
  SAMPLE *rptr = &(data->data[data->frameIndex * NUM_OF_CHANNELS]);
  unsigned long i;
  int finished;
  unsigned int framesLeft = data->maxFrameIndex - data->frameIndex;

  if (framesLeft < framesPerBuffer)
  {
    for (i = 0; i < framesLeft; i++)
    {
      *wptr++ = *rptr++;
      if (NUM_OF_CHANNELS == 2)
      {
        *wptr++ = *rptr++;
      }
    }
    data->frameIndex += framesLeft;
    finished = paComplete;
  }
  else
  {
    for (i = 0; i < framesPerBuffer; i++)
    {
      *wptr++ = *rptr++;
      if (NUM_OF_CHANNELS == 2)
      {
        *wptr++ = *rptr++;
      }
    }
    data->frameIndex += framesPerBuffer;
    finished = paContinue;
  }
  return finished;
}

int main()
{
  PaStreamParameters inputParameters, outputParameters;
  PaStream *stream;
  audioData data;
  int i;
  int totalFrames;
  int numSamples;
  int numBytes;

  data.maxFrameIndex = totalFrames = SAMPLE_RATE * NUM_OF_SECONDS;
  data.frameIndex = 0;
  numSamples = totalFrames * NUM_OF_CHANNELS;
  numBytes = numSamples * sizeof(SAMPLE);
  printf("numSamples: %d\n", numSamples);
  printf("numBytes: %d\n", numBytes);

  data.data = (SAMPLE *)malloc(numBytes);

  PaError err;
  err = Pa_Initialize();
  CheckErr(err);

  inputParameters.device = Pa_GetDefaultInputDevice();
  inputParameters.channelCount = NUM_OF_CHANNELS;
  inputParameters.sampleFormat = paInt16;
  inputParameters.suggestedLatency = Pa_GetDeviceInfo(Pa_GetDefaultInputDevice())->defaultLowInputLatency;
  inputParameters.hostApiSpecificStreamInfo = NULL;

  err =
      Pa_OpenStream(&stream, &inputParameters, NULL, SAMPLE_RATE, FRAMES_PER_BUFFER, paClipOff, recordCallback, &data);
  CheckErr(err);

  err = Pa_StartStream(stream);
  CheckErr(err);
  printf("Started Recording\n");
  fflush(stdout);

  while ((err = Pa_IsStreamActive(stream)) == 1)
  {
    Pa_Sleep(1000);
    printf("Index: %d\n", data.frameIndex);
  }

  err = Pa_CloseStream(stream);
  CheckErr(err);

  data.frameIndex = 0;

  outputParameters.device = Pa_GetDefaultOutputDevice();
  outputParameters.channelCount = NUM_OF_CHANNELS;
  outputParameters.sampleFormat = paInt16;
  outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
  outputParameters.hostApiSpecificStreamInfo = NULL;

  err =
      Pa_OpenStream(&stream, NULL, &outputParameters, SAMPLE_RATE, FRAMES_PER_BUFFER, paClipOff, outputCallback, &data);
  CheckErr(err);

  err = Pa_StartStream(stream);
  CheckErr(err);

  while ((err = Pa_IsStreamActive(stream)) == 1)
  {
    printf("Waiting for playback to finished\n");
    fflush(stdout);
    Pa_Sleep(100);
  }

  err = Pa_CloseStream(stream);
  CheckErr(err);

  err = Pa_Terminate();
  CheckErr(err);

  free(data.data);

  return 0;
}
