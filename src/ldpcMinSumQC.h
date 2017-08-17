#ifndef __LDPCMINSUMQC_H__
#define __LDPCMINSUMQC_H__

#include "ldpcMinSum.h"
#include "ldpcMatQC.h"

class ldpcMinSumQCDec : public ldpcMinSumDec {
public:
  ldpcMinSumQCDec(ldpcMatrixQC const &mat);
  ~ldpcMinSumQCDec() {}

  std::vector<int> decode(std::vector<float> const &llr);

protected:
  int update_v2c(int i);
  int update_c2v(int j);
  
  int circSize;
  int numCircRow, numCircCol;
  std::vector<std::vector<ldpcMatrixQC::circ_entry>> const &row_circs;
  std::vector<std::vector<ldpcMatrixQC::circ_entry>> const &col_circs;

  std::vector<std::vector<std::vector<float>>> c2v, v2c;
};


#endif
