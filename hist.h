///////////////////////////////////////////
//
// Custom histogram class
//
///////////////////////////////////////////

// It only supports one constructor with (steps, low limit, high limit)

#include <vector>
#include <string>

class Hist {

  int FindBin(double x);

public:
  double fLow, fHigh, fStepSize;
  int fNentries;
  double fSum;
  
  int fSteps;
  std::vector<int> fEntries;

  Hist(int steps, double low, double high);
  int Fill(double x);

  void Save(std::string filename);
  void Save(std::ofstream &filehandle);
};
