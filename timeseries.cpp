///////////////////////////////////////////
//
// Custom time series class
//
///////////////////////////////////////////

#include "timeseries.h"
#include <vector>
#include <string>
#include <ostream>
#include <fstream>
#include <iostream>


TimeSeries::TimeSeries(uint64_t start, uint64_t end, int npoints) 
//  : fStart(start), fEnd(end), fNPoints(npoints)
{
  fStart = start;
  fEnd = end;
  fNPoints = npoints;

  uint64_t npoints64 = (uint64_t) npoints;
  //fIncSize = (end - start)/npoints64;
  fIncSize = 25;
  fData = std::vector<double> (npoints, 0);
}

CArray TimeSeries::FourierPrep(std::vector<double> input)
{
  std::valarray<Complex> output (input.size());
  Complex val;
  for (int i = 0; i < input.size(); i++)
  {
    val = input[i];
    output[i] = val;
  }
  return output;
}

CArray TimeSeries::FourierRebin(CArray input, double factor)
{
  int newsize = (int) (input.size()/factor);
  std::valarray<Complex> output (newsize);
  for (int i = 0; i < input.size(); i++)
  {
    int k = (int) i/factor;
    //std::cout << "i = " << i << ", k = " << k << std::endl;
    output[k] += input[i];
  }
  return output;
}

// Cooley-Tukey FFT (in-place, breadth-first, decimation-in-frequency)
void TimeSeries::FastFourierTransform(CArray &x)
{
  // DFT
  unsigned int N = x.size(), k = N, n;
  double thetaT = 3.14159265358979323846264338328L / N;
  Complex phiT = Complex(cos(thetaT), -sin(thetaT)), T;
  while (k > 1)
  {
    n = k;
    k >>= 1;
    phiT = phiT * phiT;
    T = 1.0L;
    for (unsigned int l = 0; l < k; l++)
    {
      for (unsigned int a = l; a < N; a += n)
      {
        unsigned int b = a + k;
        Complex t = x[a] - x[b];
        x[a] += x[b];
        x[b] = t * T;
      }
      T *= phiT;
    }
  }
  // Decimate
  unsigned int m = (unsigned int)log2(N);
  for (unsigned int a = 0; a < N; a++)
  {
    unsigned int b = a;
    // Reverse bits
    b = (((b & 0xaaaaaaaa) >> 1) | ((b & 0x55555555) << 1));
    b = (((b & 0xcccccccc) >> 2) | ((b & 0x33333333) << 2));
    b = (((b & 0xf0f0f0f0) >> 4) | ((b & 0x0f0f0f0f) << 4));
    b = (((b & 0xff00ff00) >> 8) | ((b & 0x00ff00ff) << 8));
    b = ((b >> 16) | (b << 16)) >> (32 - m);
    if (b > a)
    {
      Complex t = x[a];
      x[a] = x[b];
      x[b] = t;
    }
  }
}

void TimeSeries::ComputeFourier(double rebin_factor)
{
  //std::cout << "Computing FT" << std::endl;
  CArray input = FourierPrep(fData);
  //std::cout << "Input prepared" << std::endl;
  FastFourierTransform(input);
  //fData.clear();
  //std::cout << "Transform performed" << std::endl;
  int newsize = (int) input.size()/rebin_factor;
  //std::cout << "Size of array after rebinning = " << newsize << std::endl;
  CArray out_array (newsize);
  //std::cout << "CArray set up" << std::endl;
  out_array = FourierRebin(input, rebin_factor);
  //std::cout << "Rebinning complete" << std::endl;
  fRebinFactor = rebin_factor;
  //std::cout << "Rebin factor saved" << std::endl;
  fFourierTransform.resize(newsize);
  //std::cout << "Beginning loop" << std::endl;
  for (int i = 0; i < out_array.size(); i++)
  {
    //std::cout << "i = " << i << std::endl;
    double val = (double) out_array[i].real();
    //std::cout << "val = " << val << std::endl;
    fFourierTransform[i] = val;
    //std::cout << "Pushed back" << std::endl;
  }
  //std::cout << "Completing" << std::endl;
}
 
int TimeSeries::Enter(double value, uint64_t time)
{
  uint64_t index64 = (time - fStart)/fIncSize;
  if (index64 > fNPoints) 
  {
    std::cout << "-------OVERSPILL---------" << std::endl;
    std::cout << "index = ( time (" << time << ") - fStart (" << fStart << ") ) / fIncSize (" << fIncSize << ") = " << index64 << ", should be less than " << fNPoints << std::endl;
    std::cout << "Curr. time = " << time << ", should be less than end time = " << fEnd << ". Difference = " << fEnd - time << std::endl;
    std::cout << "fIncSize = ( end (" << fEnd << ") - start (" << fStart << ") ) / npoints (" << fNPoints << ")" << std::endl;
  }
  int index = (int) index64;
  if (index > fNPoints)
  {
    std::cerr << "Cannot enter a time that lies outside scope of series." << std::endl;
    return -1;
  }
  else fData[index] = value;
  return 1;
}

double TimeSeries::FindValue(uint64_t time)
{
  uint64_t index64 = (time - fStart)/fIncSize;
  int index = (int) index64;
  if ((index > fNPoints) || (index < 0))
  {
    std::cerr << "Cannot find a time that lies outside scope of series." << std::endl;
    return -9999;
  }
  else return fData[index];
}

void TimeSeries::Save(std::string filename)
{
  std::ofstream file;
  file.open(filename);
  file << fStart << " " << fEnd << " " << fIncSize << " " << fNPoints << " " << std::endl;
  for (auto x: fData)
    file << x << " ";
  file << std::endl;
}

void TimeSeries::Save(std::ofstream &filehandle)
{
  filehandle << fStart << " " << fEnd << " " << fIncSize << " " << fNPoints << " " << std::endl;
  for (auto x: fData)
    filehandle << x << " ";
  filehandle << std::endl;
}

void TimeSeries::SaveFourier(std::string filename)
{
  std::ofstream file;
  file.open(filename);
  file << fStart << " " << fEnd << " " << fIncSize << " " << fNPoints << " " << fRebinFactor << std::endl;
  for (auto x: fFourierTransform)
    file << x << " ";
  file << std::endl;
}

void TimeSeries::SaveFourier(std::ofstream &filehandle)
{
  filehandle << fStart << " " << fEnd << " " << fIncSize << " " << fNPoints << " " << fRebinFactor << std::endl;
  for (auto x: fFourierTransform)
    filehandle << x << " ";
  filehandle << std::endl;
}


