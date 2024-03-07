#include <iostream>
#include <memory>
#include <string>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"

#include <grpcpp/grpcpp.h>

#ifdef BAZEL_BUILD
#include "examples/protos/helloworld_bytes.grpc.pb.h"
#else
#include "helloworld_bytes.grpc.pb.h"
#endif

ABSL_FLAG(std::string, target, "localhost:50051", "Server address");

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using helloworld::Greeter;
// 接口
using helloworld::NewRepeat;
using helloworld::NewReply;

class GreeterClient {
 public:
  GreeterClient(std::shared_ptr<Channel> channel)
      : stub_(Greeter::NewStub(channel)) {}

    std::string NewSayHello(const std::string& user) {

    NewRepeat request;
    request.set_coverage_data(user);

    NewReply reply;
    ClientContext context;

    Status status = stub_->NewSayHello(&context, request, &reply);

    if (status.ok()) {
      // std::cout << "服务端返回的数据: " << reply.coverage_data() << std::endl;
      return "true";
    } else {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      return "RPC failed";
    }
  }

 private:
  std::unique_ptr<Greeter::Stub> stub_;
};

int main(int argc, char** argv) {
  absl::ParseCommandLine(argc, argv);
  std::string target_str = absl::GetFlag(FLAGS_target);

  GreeterClient greeter(
      grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()));

  // 初始化
  const int arraySize = 8 * 1024 * 1024;
  uint8_t* array = new uint8_t[arraySize];

  uint8_t value = 0xE4;
  for(int i = 0; i < arraySize; i++){
    array[i] = value;
  }
  
  // 切割
  const int chunkSize = 1 * 1024 * 1024;
  int numChunks = (arraySize + chunkSize - 1) / chunkSize;
  
  // 循环
  for(int i = 0; i < numChunks; i++){
    int offset = i * chunkSize;
    int size = chunkSize;
    if (offset + size > arraySize) {
        size = arraySize - offset;
    }

    uint8_t* chunk = array + offset;
    std::string str(reinterpret_cast<char*>(chunk), size);


    std::cout << "Num: " << i << " data: " << str.length() << std::endl;
    std::string reply = greeter.NewSayHello(str);
    std::cout << "Greeter received: " << reply << std::endl;
  }
  delete[] array;

  return 0;
}
