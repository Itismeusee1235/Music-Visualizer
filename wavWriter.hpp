#ifndef WAVWRITER_HPP
#define WAVWRITER_HPP

#include <fstream>
#include <iostream>
#include <string>

using namespace std;

typedef struct WavHeader
{
  string ChunkID;
  int ChunkSize;
  string Format;
  string SubChunk1ID;
  int SubChunk1Size;
  int AudioFormat;
  int NumOfChannels;
  int SampleRate;
  int ByteRate;
  int BlockAlign;
  int BitsPerSample;
  string SubChunk2ID;
  int SubChunk2Size;
};

void writeAsBytes(ofstream &filestream, int value, int size)
{
  filestream.write(reinterpret_cast<const char *>(&value), size);
}

void WriteWav(const char *filename, const int *data, int dataSize, int AudioFormat = 1, int NumOfChannels = 2,
              int SampleRate = 44100, int BitsPerSample = 16)
{
  WavHeader header;
  header.ChunkID = "RIFF";
  header.ChunkSize = 0;
  header.Format = "WAVE";
  header.SubChunk1ID = "fmt ";
  header.SubChunk1Size = 16;
  header.AudioFormat = AudioFormat;
  header.NumOfChannels = NumOfChannels;
  header.SampleRate = 44100;
  header.ByteRate = SampleRate * NumOfChannels * BitsPerSample / 8;
  header.BlockAlign = NumOfChannels * BitsPerSample / 8;
  header.BitsPerSample = BitsPerSample;
  header.SubChunk2ID = "data";
  header.SubChunk2Size = 0;

  ofstream file;
  file.open(filename, ios::binary);

  if (file.is_open())
  {
    file << header.ChunkID;
    writeAsBytes(file, header.ChunkSize, 4);
    file << header.Format;
    file << header.SubChunk1ID;

    writeAsBytes(file, header.SubChunk1Size, 4);
    writeAsBytes(file, header.AudioFormat, 2);
    writeAsBytes(file, header.NumOfChannels, 2);
    writeAsBytes(file, header.SampleRate, 4);
    writeAsBytes(file, header.ByteRate, 4);
    writeAsBytes(file, header.BlockAlign, 2);
    writeAsBytes(file, header.BitsPerSample, 2);

    file << header.SubChunk2ID;
    writeAsBytes(file, header.SubChunk2Size, 4);

    int startAudio = file.tellp();

    for (int i = 0; i < dataSize; i += 2)
    {
      // cout << data[i] << " " << data[i + 1] << endl;
      writeAsBytes(file, data[i], header.BitsPerSample / 8);
      writeAsBytes(file, data[i + 1], header.BitsPerSample / 8);
    }

    int endAudio = file.tellp();
    file.seekp(startAudio - 4);
    writeAsBytes(file, endAudio - startAudio, 4);
    file.seekp(4, ios::beg);
    writeAsBytes(file, endAudio - 8, 4);
  }
  file.close();
}

#endif
