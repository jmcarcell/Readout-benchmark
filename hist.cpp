///////////////////////////////////////////
//
// Custom histogram class
//
///////////////////////////////////////////

#include "hist.h"
#include <vector>
#include <string>
#include <ostream>
#include <fstream>
#include <iostream>


Hist::Hist(int steps, double low, double high)
  : fSteps(steps), fLow(low), fHigh(high)
{
  fEntries = std::vector<int> (steps, 0);
  fStepSize = (high - low) / steps;
}

int Hist::FindBin(double x){
  return (x - fLow) / fStepSize;
}
  
int Hist::Fill(double x){
  int bin = FindBin(x);
  // Underflow, do nothing
  if(bin < 0) return -1;

  // Overflow, do nothing
  if(bin >= fSteps) return -1;

  fEntries[bin]++;
  fNentries++;
  fSum += x;
  return bin;
}

void Hist::Save(std::string filename){
  std::ofstream file;
  file.open(filename);
  file << fSteps << " " << fLow << " " << fHigh << " " << std::endl;
  for (auto x: fEntries)
    file << x << " ";
  file << std::endl;
}

void Hist::Save(std::ofstream &filehandle){
  filehandle << fSteps << " " << fLow << " " << fHigh << " " << std::endl;
  for (auto x: fEntries)
    filehandle << x << " ";
  filehandle << std::endl;
}
