#include <algorithm>
#include "ldpcMinSumQC.h"

ldpcMinSumQCDec::ldpcMinSumQCDec(ldpcMatrixQC const &mat) :
  ldpcMinSumDec(mat),
  circSize(mat.get_circSize()),
  numCircRows(mat.get_numCircRows()),
  numCircCols(mat.get_numCircCols()),
  row_circs(mat.get_row_circulants()),
  col_circs(mat.get_col_circulants()),
  c2v(numCircRows, std::vector<std::vector<float>>(numCircCols, std::vector<float>(circSize, 0.0))),
  v2c(numCircRows, std::vector<std::vector<float>>(numCircCols, std::vector<float>(circSize, 0.0)))
{
}

std::vector<int> ldpcMinSumQCDec::decode(std::vector<float> const &llr)
{
  int usc = 0, iter = 0;
  c2v.assign(numCircRows, std::vector<std::vector<float>>(numCircCols, std::vector<float>(circSize, 0.0)));

  do {
    usc = 0;
    for(int i = 0; i < numCircCols; i++) {
      update_v2c(i, llr);
    }
    for(int j = 0; j < numCircRows; j++) {
      usc += update_c2v(j);
    }
  } while(usc && ++iter < maxIter);

  return hd;
}

int ldpcMinSumQCDec::update_v2c(int circ_i, std::vector<float> const &llrIn)
{
  std::vector<float> sum_llr(&llrIn[circ_i*circSize], &llrIn[(circ_i+1) * circSize]);

  for(size_t j = 0; j < col_circs[circ_i].size(); j++) {
    int row = col_circs[circ_i][j].ind;
    int offset = col_circs[circ_i][j].offset;
    for(int i = 0; i < offset; i++) {
      sum_llr[i] += c2v[row][circ_i][circSize+i-offset];
    }
    for(int i = offset; i < circSize; i++) {
      sum_llr[i] += c2v[row][circ_i][i-offset];
    }
  }

  for(size_t j = 0; j < col_circs[circ_i].size(); j++) {
    int row = col_circs[circ_i][j].ind;
    int offset = col_circs[circ_i][j].offset;
    for(int i = 0; i < circSize - offset; i++) {
      v2c[row][circ_i][i] = sum_llr[i+offset] - c2v[row][circ_i][i];
    }
    for(int i = circSize - offset; i < circSize; i++) {
      v2c[row][circ_i][i] = sum_llr[i+offset - circSize] - c2v[row][circ_i][i];
    }
  }

  std::transform(sum_llr.begin(), sum_llr.end(), hd.begin() + circ_i * circSize, std::bind2nd(std::less<float>(), 0.0));

  return 0;
}

int ldpcMinSumQCDec::update_c2v(int circ_j)
{
  std::vector<int> sign(circSize, 1);
  std::vector<float> min1(circSize, 1e32), min2(circSize, 1e32);
  std::vector<int> min_ind(circSize, -1);

  for(size_t i = 0; i < row_circs[circ_j].size(); i++) {
    int col = row_circs[circ_j][i].ind;
    for(int j = 0; j < circSize; j++) {
      float mag = v2c[circ_j][col][j];
      if(mag < 0) {
        mag = -mag;
        sign[j] = -sign[j];
      }
      if(mag < min1[j]) {
        min2[j] = min1[j];
        min1[j] = mag;
        min_ind[j] = col;
      } else if(mag < min2[j]) {
        min2[j] = mag;
      }
    }
  }

  for(size_t i = 0; i < row_circs[circ_j].size(); i++) {
    int col = row_circs[circ_j][i].ind;
    for(int j = 0; j < circSize; j++) {
      c2v[circ_j][col][j] = (((sign[j] * v2c[circ_j][col][j]) < 0) ? -scale : scale) * ((min_ind[j] == col) ? min2[j] : min1[j]);
    }
  }

  return std::count(sign.begin(), sign.end(), -1);
}
