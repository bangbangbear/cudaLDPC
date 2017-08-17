#ifndef __LDPCDECODER_H__
#define __LDPCDECODER_H__

#include <algorithm>
#include "ldpcMat.h"

class ldpcDecoder {
public:
  ldpcDecoder(ldpcMatrix const &mat) :
    g(mat.get_edges()),
    numRows(mat.get_numRows()),
    numCols(mat.get_numCols()),
    numEdges(mat.get_edges().size())
  {
  }

  virtual ~ldpcDecoder() {}

  virtual std::vector<int> decode(std::vector<float> const &llr) = 0;

  // input hd: binary vector of hard decision
  // return: number of unsatisfied check
  virtual int check_parity(std::vector<int> &hd) {
    std::vector<int> synd(numRows, 0);
    for(auto it = g.begin(); it != g.end(); ++it) {
      synd[it->row] ^= hd[it->col];
    }

    return std::count(synd.begin(), synd.end(), 1);
  }

  virtual void set_maxIter(int iter) {maxIter = iter;}
    
protected:
  std::vector<ldpcMatrix::entry> const &g;
  int numRows, numCols, numEdges;

  int maxIter;
};

#endif
