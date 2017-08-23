#include "ldpcMat.h"
#include "ldpcMatQC.h"
#include "ldpcMinSum.h"
#include "ldpcMinSumQC.h"
#include "ldpcCudaMinsum.h"
#include <random>
#include <algorithm>
#include <iostream>
#include <chrono>

int main(int argc, const char *argv[])
{
  ldpcMatrixQC mat(argv[1]);
  ldpcDecoder *dec;
  if(argc > 2 && std::string(argv[2]) == "QC") {
    dec = new ldpcMinSumQCDec(mat);
  } else if(argc > 2 && std::string(argv[2]) == "cuda") {
    dec = new ldpcCudaMinsumDec(mat);
  } else {
    dec = new ldpcMinSumDec(mat);
  }
  std::cout <<dec->get_name() <<" is instantiated. " <<std::endl;

  float ber = 0.003;
  std::mt19937 rng;
  std::bernoulli_distribution bsc_dist(ber);
  std::vector<int> cwd(mat.get_numCols());
  std::vector<float> llr(mat.get_numCols());

  int r = 0, u = 0, n = 0;
  float total_time = 0;
  for(n = 0; n < 1000; n++) {
    std::generate(cwd.begin(), cwd.end(), [&](){return bsc_dist(rng);});
    // cwd[0] = 1;
    std::transform(cwd.begin(), cwd.end(), llr.begin(), std::bind1st(std::minus<float>(), 0.5));
    r += std::count(cwd.begin(), cwd.end(), 1);
    auto start = std::chrono::high_resolution_clock::now();
    std::vector<int> hd = dec->decode(llr);
    auto end = std::chrono::high_resolution_clock::now();
    total_time += std::chrono::duration<float, std::milli>(end - start).count();
    u += std::count(hd.begin(), hd.end(), 1);
  }
  std::cout << "ber = " << ber <<": raw ber = " << r / (cwd.size() * n * 1.0)
            <<", final ber = " << u / (cwd.size() * n * 1.0)
            <<std::endl;
  std::cout << "decoder takes " << total_time <<" ms." <<std::endl;

  delete dec;
  return 0;
}
