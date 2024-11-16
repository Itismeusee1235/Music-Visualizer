#include <bits/stdc++.h>
#include <fstream>

using namespace std;

typedef struct WAV_HEADER
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

} wav_hdr;

void getFileSize(string name)
{
  ifstream rf;
  rf.open(name, ios::in | ios::binary);

  if (!rf)
  {
    cout << "Invalid file" << endl;
    return;
  }

  rf.seekg(0, ios::end);
  cout << "File size is : " << rf.tellg() << " bytes" << endl;
  rf.close();
}

vector<int16_t> resample(vector<int16_t> *data, float ratio)
{
  // ratio = target samplerate/original samplerate
  const size_t originalSize = data->size();
  cout << originalSize << " " << ratio << endl;
  size_t targetSize = originalSize * ratio;
  vector<int16_t> output;
  output.reserve(targetSize);

  for (size_t i = 0; i < targetSize; i++)
  {
    float srcIndex = i / ratio; // the index of the element in the original sample data that corresponds to this sample
    size_t indexLow = static_cast<size_t>(srcIndex); // if srcIndex is a decimal then we take ceil and floor;
    size_t indexHigh = indexLow + 1;

    if (indexHigh >= originalSize)
    {
      output.push_back(data->back());
    }
    else
    {
      float frac = srcIndex - indexLow;
      float interpolated_sample =
          (1.0f - frac) * static_cast<float>((*data)[indexLow]) + static_cast<float>((*data)[indexHigh]) * frac;
      interpolated_sample = clamp(interpolated_sample, INT16_MIN * 1.0f, INT16_MAX * 1.0f);
      output.push_back(static_cast<int16_t>(interpolated_sample));
    }
  }
  return output;
}

int main()
{
  string filename = "/home/fenrir/Programming/C++/Music Visualizer/portaudio/song.wav";
  getFileSize(filename);
  ifstream rf;
  rf.open(filename, ios::binary | ios::out);

  WAV_HEADER header;
  rf.read((char *)(&header), sizeof(header));
  rf.seekg(header.Subchunk2Size, ios::cur);
  rf.read((char *)(&header.Subchunk2ID), sizeof(header.Subchunk2ID));
  rf.read((char *)(&header.Subchunk2Size), sizeof(header.Subchunk2Size));
  cout << header.ChunkSize << " " << header.Subchunk2Size << " " << header.bitsPerSample << endl
       << header.Subchunk2ID[0] << header.Subchunk2ID[1] << header.Subchunk2ID[2] << header.Subchunk2ID[3] << endl;

  size_t size = header.Subchunk2Size;

  vector<int16_t> ldata;
  vector<int16_t> rdata;
  ldata.reserve(size / 2);
  rdata.reserve(size / 2);

  for (int i = 0; i < size / 2; i++)
  {
    int16_t sample;
    rf.read((char *)(&sample), sizeof(int16_t));
    ldata.push_back(sample);
    rf.read((char *)(&sample), sizeof(int16_t));
    rdata.push_back(sample);
  }
  cout << ldata.size() << " " << rdata.size() << endl;
  rf.close();

  vector<int16_t> newlData = resample(&ldata, 44100.0f / 48000);
  vector<int16_t> newrData = resample(&rdata, 44100.0f / 48000);
  vector<int16_t> newData;
  cout << newlData.size() << " " << newrData.size() << " " << newData.size() << endl;

  for (int i = 0; i < newlData.size(); i++)
  {
    newData.push_back(newlData[i]);
    newData.push_back(newrData[i]);
  }
  cout << newlData.size() << " " << newrData.size() << " " << newData.size() << endl;

  header.SamplesPerSec = 44100;
  header.bytesPerSec = header.SamplesPerSec * header.NumOfChan * header.bitsPerSample / 8;
  header.Subchunk2Size = 2 * newlData.size();
  header.ChunkSize = header.Subchunk2Size + 44 - 8;

  ofstream of("audio.wav", ios::binary | ios::in);
  if (!of)
  {
    cout << "error" << endl;
  }
  of.write(reinterpret_cast<char *>(&header), sizeof(header));
  cout << header.Subchunk2Size << " " << newData.size() << " " << header.ChunkSize << endl;
  of.write(reinterpret_cast<char *>(newData.data()), header.Subchunk2Size);
  of.close();

  return 0;
}
