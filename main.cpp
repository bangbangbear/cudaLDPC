#include "ldpcMat.h"
#include "ldpcMinSum.h"
#include <random>
#include <algorithm>
#include <iostream>
#include <chrono>

int main(int argc, const char *argv[])
{
  ldpcMatrix mat(argv[1]);
  ldpcMinSumDec dec(mat);

  double ber = 0.004;
  std::mt19937 rng;
  std::bernoulli_distribution bsc_dist(ber);
  std::vector<int> cwd(mat.get_numCols());
  std::vector<double> llr(mat.get_numCols());

  int r = 0, u = 0, n = 0;
  double total_time = 0;
  for(n = 0; n < 10000; n++) {
    std::generate(cwd.begin(), cwd.end(), [&](){return bsc_dist(rng);});
    std::transform(cwd.begin(), cwd.end(), llr.begin(), std::bind1st(std::minus<double>(), 0.5));
    r += std::count(cwd.begin(), cwd.end(), 1);
    auto start = std::chrono::high_resolution_clock::now();
    std::vector<int> hd = dec.decode(llr);
    auto end = std::chrono::high_resolution_clock::now();
    total_time += std::chrono::duration<double, std::milli>(end - start).count();
    u += std::count(hd.begin(), hd.end(), 1);
  }
  std::cout << "ber = " << ber <<": raw ber = " << r / (cwd.size() * n * 1.0)
            <<", final ber = " << u / (cwd.size() * n * 1.0)
            <<std::endl;
  std::cout << "decoder takes " << total_time <<" ms." <<std::endl;
  
  return 0;
}
