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
  std::cout << bin << std::endl;
  // Underflow
  if(bin < 0) return -1;
  // Overflow
  if(bin >= fSteps) return -1;
  std::cout << fEntries.size() << std::endl;
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
