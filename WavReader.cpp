#include <fstream>
#include <iostream>
#include <stdint.h>
using namespace std;

struct WavHeader {
  char header[4];
  uint32_t chunkSize;
  char format[4];

  char subChunk1ID[4];
  uint32_t subChunk1Size;
  uint16_t audioFormat;
  uint16_t numChannels;
  uint32_t sampleRate;
  uint32_t byteRate;
  uint16_t blockAlign;
  uint16_t bitsPerSample;

  char subChunk2ID[4];
  uint32_t subChunk2Size;
};

int main() {

  ifstream file("output.wav", ios::in | ios::binary);

  WavHeader wav;
  file.read(reinterpret_cast<char *>(&wav), sizeof(wav));

  cout << wav.header[0] << wav.header[1] << wav.header[2] << wav.header[3]
       << endl;
  cout << wav.sampleRate << endl;

  return 0;
}
