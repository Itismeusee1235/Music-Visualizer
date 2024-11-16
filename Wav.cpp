#include "./Wav.hpp"
#include <fstream>

using namespace std;
Wav::Wav()
{
}

bool Wav::read(string fileName)
{
  ifstream rf;
  rf.open(fileName, ios::in | ios::binary);

  if (!rf)
  {
    printf("Invalid Filename !!!\n");
    return false;
  }

  rf.read(reinterpret_cast<char *>(&header), sizeof(header));

  while (string(header.Subchunk2ID, 4) != "data")
  {
    rf.seekg(header.Subchunk2Size, ios::cur);
    rf.read(reinterpret_cast<char *>(&header.Subchunk2ID), sizeof(header.Subchunk2ID));
    rf.read(reinterpret_cast<char *>(&header.Subchunk2Size), sizeof(header.Subchunk2Size));
  }

  size_t size = header.Subchunk2Size;
  data = (int16_t *)malloc(size);

  rf.read(reinterpret_cast<char *>(data), header.Subchunk2Size);

  rf.close();

  return true;
}

bool Wav::write(string fileName)
{
  ofstream wf;
  wf.open(fileName, ios::out | ios::binary);
  if (!wf)
  {
    printf("Error opening file");
    return false;
  }
  wf.write(reinterpret_cast<char *>(&header), sizeof(header));
  wf.write(reinterpret_cast<char *>(data), header.Subchunk2Size);
  wf.close();
  return true;
}
