#include "ldpcMinSum.h"

ldpcMinSumDec::ldpcMinSumDec(ldpcMatrix const &mat) :
  ldpcDecoder(mat)
{
  v2c.resize(g.size(), 0);
  c2v.resize(g.size(), 0);

  hd.resize(numCols, 0);
  llrIn.resize(numCols, 0);

  maxIter = 10;
  scale = 0.75;
}

std::vector<int> ldpcMinSumDec::decode(std::vector<double> const &llr)
{
  hd.assign(hd.size(), 1);
  llrIn = llr;
  int err;

  std::cout <<"maxIter = " << maxIter <<std::endl;
  for(int i = 0; i < maxIter; i++) {
    update_v2c();
    update_c2v();
    if((err = check_parity(hd)) == 0) {
      break;
    }
    std::cout <<err <<" errors remain after iter " << i <<std::endl;
  }

  return hd;
}


void ldpcMinSumDec::update_v2c()
{
  std::vector<double> sum_llr = llrIn;

  int ind = 0;
  for(auto it = g.begin(); it != g.end(); it++) {
    sum_llr[it->col] += c2v[ind++];
  }
  ind = 0;
  for(auto it = g.begin(); it != g.end(); it++) {
    v2c[ind] = sum_llr[it->col] - c2v[ind];
    ind++;
  }

  std::transform(sum_llr.begin(), sum_llr.end(), hd.begin(), std::bind2nd(std::less<double>(), 0.0));
}

void ldpcMinSumDec::update_c2v()
{
  std::vector<int> sign(numRows, 1);
  std::vector<double> min1(numRows, 1e32), min2(numRows, 1e32);
  std::vector<int> min_ind(numRows, -1);

  int ind = 0;
  for(auto it = g.begin(); it != g.end(); it++) {
    double mag = fabs(v2c[ind]);
    sign[it->row] *= (v2c[ind] < 0) ? -1 : 1;
    if(mag < min1[it->row]) {
      min2[it->row] = min1[it->row];
      min1[it->row] = mag;
      min_ind[it->row] = ind;
    } else if (mag < min2[it->row]) {
      min2[it->row] = mag;
    }
  }

  ind = 0;
  for(auto it = g.begin(); it != g.end(); it++) {
    c2v[ind] = sign[it->row] * ((v2c[ind] < 0) ? -1 : 1) * scale;
    c2v[ind] *= min_ind[it->row] == ind ? min2[it->row] : min1[it->row];
  }
  
}
