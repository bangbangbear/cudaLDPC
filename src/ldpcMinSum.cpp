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

std::vector<int> ldpcMinSumDec::decode(std::vector<float> const &llr)
{
  hd.assign(hd.size(), 1);
  llrIn = llr;
  c2v.assign(c2v.size(), 0);

  int usc = -1;
  for(int i = 0; i < maxIter && usc; i++) {
    update_v2c();
    usc = update_c2v();
  }

  return hd;
}


int ldpcMinSumDec::update_v2c()
{
  std::vector<float> sum_llr = llrIn;

  int ind = 0;
  for(auto it = g.begin(); it != g.end(); ++it, ++ind) {
    sum_llr[it->col] += c2v[ind];
  }
  ind = 0;
  for(auto it = g.begin(); it != g.end(); ++it, ++ind) {
    v2c[ind] = sum_llr[it->col] - c2v[ind];
  }

  std::transform(sum_llr.begin(), sum_llr.end(), hd.begin(), std::bind2nd(std::less<float>(), 0.0));

  return 0;
}

int ldpcMinSumDec::update_c2v()
{
  std::vector<int> sign(numRows, 1);
  std::vector<float> min1(numRows, 1e32), min2(numRows, 1e32);
  std::vector<int> min_ind(numRows, -1);

  int ind = 0;
  for(auto it = g.begin(); it != g.end(); ++it, ++ind) {
    float mag = v2c[ind];
    if(mag < 0) {
      mag = -mag;
      sign[it->row] = -sign[it->row];
    }
    if(mag < min1[it->row]) {
      min2[it->row] = min1[it->row];
      min1[it->row] = mag;
      min_ind[it->row] = ind;
    } else if (mag < min2[it->row]) {
      min2[it->row] = mag;
    }
  }

  ind = 0;
  for(auto it = g.begin(); it != g.end(); ++it, ++ind) {
    c2v[ind] = ((sign[it->row] * v2c[ind] < 0) ? -scale : scale) * (min_ind[it->row] == ind ? min2[it->row] : min1[it->row]);
  }
  
  return std::count(sign.begin(), sign.end(), -1);
}
