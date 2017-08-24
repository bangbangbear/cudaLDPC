#include "ldpcCudaMinsum.h"

#include "cuda.h"
#include "cuda_runtime.h"
#include "cuda_profiler_api.h"
#include "device_launch_parameters.h"
#include "device_functions_decls.h"

#define CIRC_SIZE 128

__global__ void update_v2c_kernel(float *v2c, float * c2v, int *hd, const float *llr, const ldpcMatrixQC::circ_entry *col_circ, const int *col_weight, int max_colWeight, int numCircRows, int numCircCols)
{
  const int col = blockIdx.x * blockDim.x + threadIdx.x;
  __shared__ float sum_llr[CIRC_SIZE];
  const int weight = col_weight[blockIdx.x];
  const ldpcMatrixQC::circ_entry *this_col = col_circ + max_colWeight * blockIdx.x;
  int i = 0;

  sum_llr[threadIdx.x] = llr[col];
  for(i = 0; i < weight; i++) {
    int offset = (threadIdx.x - this_col[i].offset) & 0x7f;
    int circ_ind = this_col[i].ind * numCircCols + blockIdx.x;
    sum_llr[threadIdx.x] += c2v[circ_ind * blockDim.x + offset];
  }

  for(i = 0; i < weight; i++) {
    int offset = (threadIdx.x - this_col[i].offset) & 0x7f;
    int circ_ind = this_col[i].ind * numCircCols + blockIdx.x;
    v2c[circ_ind * blockDim.x + offset] = sum_llr[threadIdx.x] - c2v[circ_ind * blockDim.x + offset];
  }
  hd[col] = sum_llr[threadIdx.x] < 0;
}

__global__ void update_c2v_kernel(float *v2c, float *c2v, int *usc, ldpcMatrixQC::circ_entry *circ_row, int *row_weight, int max_colWeight, int numCircRows, int numCircCols)
{
  float min1 = 1e32, min2;
  int min_ind, sign = 1, i;
  int weight = row_weight[blockIdx.x];
  ldpcMatrixQC::circ_entry *this_row = circ_row + max_colWeight * blockIdx.x;

  if(blockIdx.x == 0 && threadIdx.x == 0) {
    usc[0] = 0;
  }

  for(i=0; i < weight; i++) {
    int col = this_row[i].ind;
    int circ_ind = blockIdx.x * numCircCols + col;
    float mag = v2c[circ_ind * blockDim.x + threadIdx.x];
    if(mag < 0) {
      sign = -sign;
      mag = -mag;
    }
    if(mag < min1) {
      min2 = min1;
      min1 = mag;
      min_ind = col;
    } else if (mag < min2) {
      min2 = mag;
    }
  }

  for(i = 0; i < weight; i++) {
    int col = this_row[i].ind;
    int circ_ind = blockIdx.x * numCircCols + col;
    c2v[circ_ind * blockDim.x + threadIdx.x] = (((sign * v2c[circ_ind * blockDim.x + threadIdx.x]) < 0) ? -0.75 : 0.75) * (min_ind == col ? min2 : min1);
  }

  if(sign < 0) usc[0] = 1; //just want to see if usc is non-zero, so no need to use atomicAdd
}


ldpcCudaMinsumDec::ldpcCudaMinsumDec(ldpcMatrixQC const &mat) :
  ldpcMinSumQCDec(mat)
{
  // prepare host data
  std::vector<std::vector<ldpcMatrixQC::circ_entry>> row_circs = mat.get_row_circulants();
  std::vector<int> row_weight = mat.get_row_weight();
  max_rowWeight = *std::max_element(row_weight.begin(), row_weight.end());
  for(auto it = row_circs.begin(); it != row_circs.end(); ++it) {
    for(int i = 0; i < max_rowWeight; i++) {
      if(i < (int)it->size()) {
        row_circs_host.push_back((*it)[i]);
      } else {
        ldpcMatrixQC::circ_entry e = {-1, -1};
        row_circs_host.push_back(e);
      }
    }
  }

  std::vector<std::vector<ldpcMatrixQC::circ_entry>> col_circs = mat.get_col_circulants();
  std::vector<int> col_weight = mat.get_col_weight();
  max_colWeight = *std::max_element(col_weight.begin(), col_weight.end());
  for(auto it = col_circs.begin(); it != col_circs.end(); ++it) {
    for(int i = 0; i < max_colWeight; i++) {
      if(i < (int)it->size()) {
        col_circs_host.push_back((*it)[i]);
      } else {
        ldpcMatrixQC::circ_entry e = {-1, -1};
        col_circs_host.push_back(e);
      }
    }
  }
  
  int device_id = 0; // Force to use my only GPU
  checkCudaErrors(cudaSetDevice(device_id));
  checkCudaErrors(cudaDeviceReset());
  checkCudaErrors(cudaSetDeviceFlags(cudaDeviceScheduleBlockingSync));

  // Allocate memory on GPU
  checkCudaErrors(cudaMalloc((void **)&row_circs_cuda, row_circs_host.size() * sizeof(row_circs_host[0])));
  checkCudaErrors(cudaMemcpy(row_circs_cuda, &row_circs_host[0], row_circs_host.size() * sizeof(row_circs_host[0]), cudaMemcpyHostToDevice));
  checkCudaErrors(cudaMalloc((void **)&col_circs_cuda, col_circs_host.size() * sizeof(col_circs_host[0])));
  checkCudaErrors(cudaMemcpy(col_circs_cuda, &col_circs_host[0], col_circs_host.size() * sizeof(col_circs_host[0]), cudaMemcpyHostToDevice));
  checkCudaErrors(cudaMalloc((void **)&row_weight_cuda, row_weight.size() * sizeof(int)));
  checkCudaErrors(cudaMemcpy(row_weight_cuda, &row_weight[0], row_weight.size() * sizeof(int), cudaMemcpyHostToDevice));
  checkCudaErrors(cudaMalloc((void **)&col_weight_cuda, col_weight.size() * sizeof(int)));
  checkCudaErrors(cudaMemcpy(col_weight_cuda, &col_weight[0], col_weight.size() * sizeof(int), cudaMemcpyHostToDevice));

  // work space
  checkCudaErrors(cudaMalloc((void **)&v2c_cuda, numCircRows * numCircCols * circSize * sizeof(float)));
  checkCudaErrors(cudaMalloc((void **)&c2v_cuda, numCircRows * numCircCols * circSize * sizeof(float)));
  checkCudaErrors(cudaMalloc((void **)&llr_cuda, numCols  * sizeof(float)));
  checkCudaErrors(cudaMalloc((void **)&hd_cuda, numCols * sizeof(int)));
  checkCudaErrors(cudaMalloc((void **)&usc_cuda, sizeof(int)));
}

ldpcCudaMinsumDec::~ldpcCudaMinsumDec()
{
  checkCudaErrors(cudaFree(v2c_cuda));
  checkCudaErrors(cudaFree(c2v_cuda));
  checkCudaErrors(cudaFree(llr_cuda));
  checkCudaErrors(cudaFree(hd_cuda));

  checkCudaErrors(cudaFree(row_weight_cuda));
  checkCudaErrors(cudaFree(row_circs_cuda));
  checkCudaErrors(cudaFree(col_weight_cuda));
  checkCudaErrors(cudaFree(col_circs_cuda));

  checkCudaErrors(cudaDeviceReset());
}

std::vector<int> ldpcCudaMinsumDec::decode(std::vector<float> const &llr)
{
  int usc_host = -1;
  std::vector<int> hd_host(numCols);
  checkCudaErrors(cudaMemcpy(llr_cuda, &llr[0], llr.size() * sizeof(float), cudaMemcpyHostToDevice));
  // checkCudaErrors(cudaMemset(v2c_cuda, 0, numCircRows * numCircCols * CIRC_SIZE * sizeof(float)));
  checkCudaErrors(cudaMemset(c2v_cuda, 0, numCircRows * numCircCols * CIRC_SIZE * sizeof(float)));

  for(int iter = 0; iter < maxIter && usc_host; iter++) {
    update_v2c_kernel<<<numCols/circSize, circSize>>>(v2c_cuda, c2v_cuda, hd_cuda, llr_cuda, col_circs_cuda, col_weight_cuda, max_colWeight, numCircRows, numCircCols);
    update_c2v_kernel<<<numRows/circSize, circSize>>>(v2c_cuda, c2v_cuda, usc_cuda, row_circs_cuda, row_weight_cuda, max_rowWeight, numCircRows, numCircCols);

    checkCudaErrors(cudaMemcpy(&usc_host, usc_cuda, sizeof(int), cudaMemcpyDeviceToHost));
  }

  checkCudaErrors(cudaMemcpy(&hd_host[0], hd_cuda, numCols * sizeof(int), cudaMemcpyDeviceToHost));


  return hd_host;
}
