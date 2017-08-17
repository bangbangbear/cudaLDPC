#ifndef __LDPCMINSUM_H__
#define __LDPCMINSUM_H__

#include "ldpcMat.h"
#include "ldpcDecoder.h"

class ldpcMinSumDec : public ldpcDecoder {
public:
  ldpcMinSumDec(ldpcMatrix const &mat);
  ~ldpcMinSumDec() {}

  std::vector<int> decode(std::vector<float> const &llr);

protected:

  int update_v2c();
  int update_c2v();
  
  std::vector<float> v2c;
  std::vector<float> c2v;

  std::vector<int> hd;
  std::vector<float> llrIn;

  float scale;
};

#endif
