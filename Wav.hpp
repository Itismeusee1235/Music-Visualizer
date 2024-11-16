#ifndef WAV_HPP
#define WAV_HPP

#include <bits/stdc++.h>

using namespace std;

typedef struct WavHeader
{
  char RIFF[4];
  uint32_t ChunkSize;
  char WAVE[4];
  char fmt[4];
  uint32_t Subchunk1Size;
  uint16_t AudioFormat;
  uint16_t NumOfChan;
  uint32_t SamplesPerSec;
  uint32_t bytesPerSec;
  uint16_t blockAlign;
  uint16_t bitsPerSample;
  char Subchunk2ID[4];
  uint32_t Subchunk2Size;
} wav_header;

class Wav
{
public:
  WavHeader header;
  int16_t *data;

public:
  Wav();
  bool read(string fileName);
  bool write(string fileName);
};

#endif
