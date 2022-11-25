#ifndef CB_NN_H_
#define CB_NN_H_

#include <vector>

class TfLiteInterpreter;
class TfLiteDelegate;

namespace cb {

class NN {
 public:
  NN() = default;
  ~NN();
  int Init(const std::string& model_buffer, int threads = 1,
           bool use_xnnpack = 0);
  int InitFromFile(const char* path, int threads = 1, bool use_xnnpack = 0);

  void* input_tensor(int index);
  template <typename T>
  T* input_tensor(int index) {
    return static_cast<T*>(input_tensor(index));
  }

  const void* output_tensor(int index, size_t* bytes = nullptr);
  template <typename T>
  const T* output_tensor(int index, size_t* bytes = nullptr) {
    return static_cast<const T*>(output_tensor(index, bytes));
  }

  int Invoke();

 private:
  std::string model_buffer_;
  TfLiteInterpreter* interpreter_ = nullptr;
  TfLiteDelegate* xnnpack_delegate_ = nullptr;
};

}  // namespace cb

#endif  // CB_NN_H_
