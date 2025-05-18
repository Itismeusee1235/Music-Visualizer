#ifndef WAVHEADER_HPP
#define WAVHEADER_HPP

#include <fstream>
#include <stdint.h>
using namespace std;

struct WavHeader {
  char chunkID[4]; // Contains RIFF in ASCII form
  uint32_t
      chunkSize;  // Size of entire file in bytes - 8 bytes from first 2 fields
  char format[4]; // Contains letter WAVE

  char subChunk1ID[4]; // Contains the letters 'fmt '
  uint32_t
      subChunk1Size;    // Contains size of the rest of the subchunk1 usually 16
  uint16_t audioFormat; // Contains the format, usually 1, values other than 1
                        // mean some sort of compression
  uint16_t numChannels; // Number of channels 1, for mono and 2 for stereo
  uint32_t sampleRate;  // Samples per sec
  uint32_t
      byteRate; // Bytes per sec = sampleRate * numChannels * bitsPerSample/8
  uint16_t blockAlign; // Number of bytes for 1 sample, includes all channels =
                       // numChannels * bitsPerSample/8
  uint16_t bitsPerSample; // Size of each sample in bits

  char subChunk2ID[4];    // Contains the letters 'data'
  uint32_t subChunk2Size; // number of bytes of data = Num of samples *
                          // numChannels * bitsPerSample/8
};

class Wav {
  WavHeader header;
  // data
  char *filename;

public:
  Wav(char *filename);
  void print();
  int getSize();
  float getDuration();
  ~Wav();

private:
  void readBytes(ifstream &file, char *buffer, int size);
  int readHeader(ifstream &file);
};

#endif
