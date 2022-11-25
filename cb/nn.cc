#include <fstream>

#include <tensorflow/lite/c/c_api_experimental.h>
#include <tensorflow/lite/delegates/xnnpack/xnnpack_delegate.h>

#include "logging.hpp"
#include "nn.h"

namespace cb {

NN::~NN() {
  if (interpreter_ != nullptr) TfLiteInterpreterDelete(interpreter_);
  if (xnnpack_delegate_ != nullptr)
    TfLiteXNNPackDelegateDelete(xnnpack_delegate_);
}

int NN::Init(const std::string& model_buffer, int threads, bool use_xnnpack) {
  model_buffer_ = model_buffer;
  TfLiteModel* model =
      TfLiteModelCreate(model_buffer_.data(), model_buffer_.size());
  TfLiteInterpreterOptions* options = TfLiteInterpreterOptionsCreate();
  TfLiteInterpreterOptionsSetNumThreads(options, threads);
  interpreter_ = TfLiteInterpreterCreate(model, options);
  TfLiteInterpreterOptionsDelete(options);
  TfLiteModelDelete(model);

  if (use_xnnpack) {
    TfLiteXNNPackDelegateOptions xnnpack_options =
        TfLiteXNNPackDelegateOptionsDefault();
    xnnpack_options.num_threads = threads;
    xnnpack_delegate_ = TfLiteXNNPackDelegateCreate(&xnnpack_options);
    if (xnnpack_delegate_ == nullptr) {
      LOG(WARN) << "tflite xnnpack delegate create failed!";
    } else {
      auto status = TfLiteInterpreterModifyGraphWithDelegate(interpreter_,
                                                             xnnpack_delegate_);
      if (status != kTfLiteOk) {
        LOG(WARN) << "tflite xnnpack not support!";
      } else {
        VLOG(1) << "tflite use xnnpack.";
      }
    }
  }

  TfLiteStatus status = TfLiteInterpreterAllocateTensors(interpreter_);
  if (status != kTfLiteOk) {
    LOG(ERROR) << "tflite allocate tensor error!";
    return -1;
  }
  return 0;
}

int NN::InitFromFile(const char* path, int threads, bool use_xnnpack) {
  std::ifstream fin(path);
  auto model_buffer = std::string(std::istreambuf_iterator<char>(fin),
                                  std::istreambuf_iterator<char>());
  fin.close();
  return Init(model_buffer, threads, use_xnnpack);
}

void* NN::input_tensor(int index) {
  TfLiteTensor* tensor = TfLiteInterpreterGetInputTensor(interpreter_, index);
  return TfLiteTensorData(tensor);
}

const void* NN::output_tensor(int index, size_t* bytes) {
  const TfLiteTensor* tensor =
      TfLiteInterpreterGetOutputTensor(interpreter_, index);
  if (bytes != nullptr) {
    *bytes = TfLiteTensorByteSize(tensor);
  }
  return TfLiteTensorData(tensor);
}

int NN::Invoke() {
  TfLiteStatus status = TfLiteInterpreterInvoke(interpreter_);
  if (status != kTfLiteOk) {
    LOG(ERROR) << "tflite invoke failed";
    return -1;
  }
  return 0;
}

}  // namespace butu
