#ifndef __LDPCMATQC_H__
#define __LDPCMATQC_H__

#include "ldpcMat.h"

class ldpcMatrixQC : public ldpcMatrix {
public:
  ldpcMatrixQC(std::string const &fname);
  virtual ~ldpcMatrixQC() {}

  struct circ_entry {int ind; int offset;};
  std::vector<std::vector<circ_entry>> const &get_row_circulants() const {return row_circs;}
  std::vector<std::vector<circ_entry>> const &get_col_circulants() const {return col_circs;}

  int get_circSize() const {return circSize;}
  int get_numCircRows() const {return numRows/circSize;}
  int get_numCircCols() const {return numCols/circSize;}

protected:
  std::vector<circ_entry> circ_graph;
  std::vector<std::vector<circ_entry>> row_circs, col_circs;

  int numCircRows, numCircCols;
  int circSize;
};

#endif
