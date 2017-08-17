#ifndef __LDMCMAT_H__
#define __LDMCMAT_H__

#include <iostream>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

class ldpcMatrix {
public:
  ldpcMatrix(std::string const &fname) {
    loadFromFile(fname);
  }
  virtual ~ldpcMatrix() {}

  // Load file with a special format.
  // File Format:
  // column index (starting from 1) of non-zero elem in each row.
  void loadFromFile(std::string const &fname) {
    std::ifstream fs(fname.c_str(), std::istream::in);
    std::istream_iterator<int> start(fs), end;
    std::vector<int> fdata(start, end);
    int r = 0, last_val = 100000, c = 0;
    for(auto it = fdata.begin(); it != fdata.end(); ++it) {
      if(*it <= 0) continue;
      if(*it < last_val) r++;
      if(*it > c) c= *it;
      entry e = {r-1, *it-1};
      graph.push_back(e);
      last_val = *it;
    }
    numRows = r;
    numCols = c;
  }

  struct entry {int row; int col;};

  std::vector<entry> const & get_edges() const { return graph; }
  int get_numRows() const { return numRows; }
  int get_numCols() const { return numCols; }

protected:
  std::vector<entry> graph;
  int numRows, numCols;
};

#endif
