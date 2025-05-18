#include "WavReader.hpp"
#include <fstream>
#include <iostream>
#include <stdint.h>
#include <string.h>
using namespace std;

Wav::Wav(char *filename) {
  ifstream file(filename, ios::in | ios::binary);
  readHeader(file);
  readData(file);
  file.close();
}

void Wav::readBytes(ifstream &file, char *buffer, int size) {
  file.read(buffer, size);
}

int Wav::readHeader(ifstream &file) {

  readBytes(file, reinterpret_cast<char *>(&header.chunkID), 4);
  if (strncmp(header.chunkID, "RIFF", 4) != 0) {
    return -1;
  }
  readBytes(file, reinterpret_cast<char *>(&header.chunkSize), 4);
  readBytes(file, reinterpret_cast<char *>(&header.format), 4);
  readBytes(file, reinterpret_cast<char *>(&header.subChunk1ID), 4);
  readBytes(file, reinterpret_cast<char *>(&header.subChunk1Size), 4);
  readBytes(file, reinterpret_cast<char *>(&header.audioFormat), 2);
  readBytes(file, reinterpret_cast<char *>(&header.numChannels), 2);
  readBytes(file, reinterpret_cast<char *>(&header.sampleRate), 4);
  readBytes(file, reinterpret_cast<char *>(&header.byteRate), 4);
  readBytes(file, reinterpret_cast<char *>(&header.blockAlign), 2);
  readBytes(file, reinterpret_cast<char *>(&header.bitsPerSample), 2);

  readBytes(file, reinterpret_cast<char *>(&header.subChunk2ID), 4);
  readBytes(file, reinterpret_cast<char *>(&header.subChunk2Size), 4);

  while (strncmp(header.subChunk2ID, "data", 4) != 0 && !file.eof()) {
    file.seekg(header.subChunk2Size, ios::cur);
    readBytes(file, reinterpret_cast<char *>(&header.subChunk2ID), 4);
    readBytes(file, reinterpret_cast<char *>(&header.subChunk2Size), 4);
  }

  return 0;
}

void Wav::readData(ifstream &file) {
  int num_frames =
      (header.subChunk2Size * 8) / (header.numChannels * header.bitsPerSample);
  l_data = new float[num_frames];
  r_data = new float[num_frames];

  int num_bytes = header.bitsPerSample / 8;
  if (num_bytes == 2) {
    cout << "2 bytes" << endl;
    int16_t left, right;
    for (int i = 0; i < num_frames; i++) {
      readBytes(file, reinterpret_cast<char *>(&left), num_bytes);
      readBytes(file, reinterpret_cast<char *>(&right), num_bytes);
      l_data[i] = left / 32767.0f;
      r_data[i] = right / 32767.0f;
    }
  } else if (num_bytes == 4) {
    int32_t left, right;
    for (int i = 0; i < num_frames; i++) {
      readBytes(file, reinterpret_cast<char *>(&left), num_bytes);
      readBytes(file, reinterpret_cast<char *>(&right), num_bytes);
      l_data[i] = left / 2147483647.0f;
      r_data[i] = right / 2147483647.0f;
    }
  } else {
    printf("Exceeded bit depth");
  }
}

void Wav::print() {
  cout << header.chunkID[0] << header.chunkID[1] << header.chunkID[2]
       << header.chunkID[3] << endl;
  cout << header.chunkSize << endl;
  cout << header.format[0] << header.format[1] << header.format[2]
       << header.format[3] << endl;
  cout << header.subChunk1ID[0] << header.subChunk1ID[1]
       << header.subChunk1ID[2] << header.subChunk1ID[3] << endl;
  cout << header.subChunk1Size << " -" << endl;
  cout << header.audioFormat << endl;
  cout << header.numChannels << endl;
  cout << header.sampleRate << endl;
  cout << header.byteRate << endl;
  cout << header.blockAlign << endl;
  cout << header.bitsPerSample << endl;
  cout << header.subChunk2ID[0] << header.subChunk2ID[1]
       << header.subChunk2ID[2] << header.subChunk2ID[3] << endl;
  cout << header.subChunk2Size << endl;

  cout << getSize() << endl;
  cout << getDuration() << endl;

  // for (int i = 99900; i < 99900 + 100; i++) {
  //   cout << l_data[i] << " " << r_data[i] << endl;
  // }
}

int Wav::getSize() { return header.chunkSize + 8; }

float Wav::getDuration() {
  return static_cast<float>(header.subChunk2Size) / header.byteRate;
}

Wav::~Wav() {

  free(l_data);
  free(r_data);
}
