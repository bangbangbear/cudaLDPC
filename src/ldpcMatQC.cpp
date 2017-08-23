#include "ldpcMatQC.h"

ldpcMatrixQC::ldpcMatrixQC(std::string const &fname) :
  ldpcMatrix(fname)
{
  circSize = 128;
  
  row_circs.resize(numRows/circSize);
  col_circs.resize(numCols/circSize);

  for(auto it = graph.begin(); it != graph.end(); ++it) {
    if(it->row % circSize == 0) {
      circ_entry e1 = {it->row/circSize, it->col % circSize};
      circ_entry e2 = {it->col/circSize, it->col % circSize};
      col_circs[it->col/circSize].push_back(e1);
      row_circs[it->row/circSize].push_back(e2);
    }
  }

  for(auto it = row_circs.begin(); it != row_circs.end(); ++it) {
    row_weight.push_back(it->size());
  }
  for(auto it = col_circs.begin(); it != col_circs.end(); ++it) {
    col_weight.push_back(it->size());
  }
}
