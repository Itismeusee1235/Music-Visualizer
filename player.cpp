#include "Wav.hpp"
#include <bits/stdc++.h>
#include <portaudio.h>

using namespace std;

#define SAMPLE int16_t

typedef struct
{
  int maxFrameIndex;
  int frameIndex;
  SAMPLE *data;

} audioData;

int playCallback(const void *inputBuff, void *outputBuff, unsigned long framesPerBuffer,
                 const PaStreamCallbackTimeInfo *timeInfo, PaStreamCallbackFlags statusFlags, void *userData)
{
  audioData *data = (audioData *)userData;
  SAMPLE *rptr = &data->data[data->frameIndex * 2];
  SAMPLE *wtpr = (SAMPLE *)outputBuff;
  unsigned long i;
  int finished;
  unsigned long framesLeft = data->maxFrameIndex - data->frameIndex;

  if (framesLeft < framesPerBuffer)
  {
    for (i = 0; i < framesLeft; i++)
    {
      *wtpr++ = *rptr++;
      *wtpr++ = *rptr++;
    }
    finished = paComplete;
    data->frameIndex += framesLeft;
  }
  else
  {
    for (i = 0; i < framesPerBuffer; i++)
    {
      *wtpr++ = *rptr++;
      *wtpr++ = *rptr++;
    }
    finished = paContinue;
    data->frameIndex += framesPerBuffer;
  }
  return finished;
}

void CheckErr(PaError err)
{
  if (err != paNoError)
  {
    printf("Portaudio error: %s\n", Pa_GetErrorText(err));
    exit(EXIT_FAILURE);
  }
}

void displayAsTime(long seconds)
{
  long hours = seconds / 3600;
  long minutes = (seconds % 3600) / 60;
  seconds = (seconds % 60);
  cout << setfill('0') << setw(2) << hours << ":" << setfill('0') << setw(2) << minutes << ":" << setfill('0')
       << setw(2) << seconds;
}

int main()
{
  Wav file;
  audioData data;
  file.read("./Music/song.wav");
  unsigned long totalFrames;
  unsigned long numSamples;
  unsigned long numBytes;
  float duration;

  numBytes = file.header.Subchunk2Size;
  numSamples = numBytes / (file.header.bitsPerSample / 8);
  data.maxFrameIndex = totalFrames = numSamples / file.header.NumOfChan;
  data.frameIndex = 0;
  data.data = file.data;
  duration = numBytes * 1.0 / file.header.bytesPerSec;

  PaError err;
  PaStream *stream;
  PaStreamParameters outputParameters;

  err = Pa_Initialize();
  CheckErr(err);

  outputParameters.device = Pa_GetDefaultOutputDevice();
  outputParameters.channelCount = file.header.NumOfChan;
  outputParameters.sampleFormat = paInt16;
  outputParameters.suggestedLatency = Pa_GetDeviceInfo(outputParameters.device)->defaultLowOutputLatency;
  outputParameters.hostApiSpecificStreamInfo = NULL;

  err = Pa_OpenStream(&stream, NULL, &outputParameters, file.header.SamplesPerSec, 512, paClipOff, playCallback, &data);
  CheckErr(err);

  err = Pa_StartStream(stream);
  CheckErr(err);
  while ((err = Pa_IsStreamActive(stream)) == 1)
  {
    system("clear");
    float ratio = data.frameIndex * 1.0 / data.maxFrameIndex;
    printf("Playing... \n");
    displayAsTime(static_cast<long>(ratio * duration));
    cout << "/";
    displayAsTime(static_cast<long>(duration));
    printf("(%d%%)\n", static_cast<int>(ratio * 100));

    Pa_Sleep(100);
  }
  system("clear");
  float ratio = data.frameIndex * 1.0 / data.maxFrameIndex;
  printf("Playing... \n");
  displayAsTime(static_cast<long>(ratio * duration));
  cout << "/";
  displayAsTime(static_cast<long>(duration));
  printf("(%d%%)\n", static_cast<int>(ratio));

  err = Pa_CloseStream(stream);
  CheckErr(err);

  err = Pa_Terminate();
  CheckErr(err);

  return 0;
}
