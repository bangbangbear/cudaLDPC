#ifndef __LDPCMINSUMQC_H__
#define __LDPCMINSUMQC_H__

#include "ldpcMinSum.h"
#include "ldpcMatQC.h"

class ldpcMinSumQCDec : public ldpcMinSumDec {
public:
  ldpcMinSumQCDec(ldpcMatrixQC const &mat);
  ~ldpcMinSumQCDec() {}

  virtual std::vector<int> decode(std::vector<float> const &llr);
  virtual std::string get_name() {return "ldpcMinSumQCDec";}

protected:
  int update_v2c(int i, std::vector<float> const &llrIn);
  int update_c2v(int j);
  
  int circSize;
  int numCircRow, numCircCol;
  std::vector<std::vector<ldpcMatrixQC::circ_entry>> const &row_circs;
  std::vector<std::vector<ldpcMatrixQC::circ_entry>> const &col_circs;

  std::vector<std::vector<std::vector<float>>> c2v, v2c;
};


#endif
