#include "ldpcMat.h"
#include "ldpcMinSum.h"
#include <random>
#include <algorithm>
#include <iostream>

int main(int argc, const char *argv[])
{
  ldpcMatrix mat(argv[1]);
  ldpcMinSumDec dec(mat);

  double ber = 0.001;
  std::mt19937 rng;
  std::bernoulli_distribution bsc_dist(ber);
  std::vector<int> cwd(mat.get_numCols());
  std::vector<double> llr(mat.get_numCols());

  
  std::generate(cwd.begin(), cwd.end(), [&](){return bsc_dist(rng);});
  std::transform(cwd.begin(), cwd.end(), llr.begin(), std::bind1st(std::minus<double>(), 0.5));
  std::cout <<std::count(cwd.begin(), cwd.end(), 1) <<" errors before decoding." <<std::endl;  
  std::vector<int> hd = dec.decode(llr);
  std::cout <<std::count(hd.begin(), hd.end(), 1) <<" errors after decoding." <<std::endl;
  
  return 0;
}
