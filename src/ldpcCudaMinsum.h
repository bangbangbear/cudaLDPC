#ifndef __LDPCCUDAMINSUM_H__
#define __LDPCCUDAMINSUM_H__

#include <stdexcept>
#include "ldpcMatQC.h"
#include "ldpcMinSumQC.h"

#define checkCudaErrors(call)                                           \
  do {                                                                  \
    cudaError_t err = call;                                             \
    if (cudaSuccess != err) {                                           \
      char errorBuff[512];                                              \
      snprintf(errorBuff, sizeof(errorBuff) - 1,			\
                "CUDA error '%s' in func '%s' line %d",			\
                cudaGetErrorString(err), __FUNCTION__, __LINE__);	\
      throw std::runtime_error(errorBuff);				\
    }                                                                   \
  } while (0)

#define checkCudaDriverErrors(call)                             \
  do {                                                          \
    CUresult err = call;                                        \
    if (CUDA_SUCCESS != err) {                                  \
      char errorBuff[512];                                      \
      snprintf(errorBuff, sizeof(errorBuff) - 1,                \
                "CUDA error DRIVER: '%d' in func '%s' line %d", \
                err, __FUNCTION__, __LINE__);                   \
      throw std::runtime_error(errorBuff);                      \
    }                                                           \
  } while (0)

#define executeTimer(call)                      \
  do {                                          \
    cudaEvent_t start, stop;                    \
    cudaEventCreate(&start);                    \
    cudaEventCreate(&stop);                     \
    cudaEventRecord(start);                     \
    call;                                       \
    cudaEventRecord(stop);                      \
    cudaEventSynchronize(stop);                 \
    float ms = 0;                               \
    static float total_time = 0;                \
    cudaEventElapsedTime(&ms, start, stop);     \
    printf("%s: this run: %f ms, total run: %f\n", #call, ms, total_time += ms); \
  } while (0)


class ldpcCudaMinsumDec : public ldpcMinSumQCDec {
public:
  ldpcCudaMinsumDec(ldpcMatrixQC const &mat);
  ~ldpcCudaMinsumDec();

  virtual std::vector<int> decode(std::vector<float> const &llr);
  virtual std::string get_name() {return "ldpcCudaMinsumDec";}

private:
  float *c2v_cuda, *v2c_cuda;
  float *llr_cuda;
  int *usc_cuda, *hd_cuda;
  ldpcMatrixQC::circ_entry *row_circs_cuda;
  ldpcMatrixQC::circ_entry *col_circs_cuda;
  int *row_weight_cuda;
  int *col_weight_cuda;
  int max_rowWeight, max_colWeight;

  std::vector<ldpcMatrixQC::circ_entry> row_circs_host;
  std::vector<ldpcMatrixQC::circ_entry> col_circs_host;
};


#endif
