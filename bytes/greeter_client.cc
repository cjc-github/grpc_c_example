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

    std::cout << "客户端发送的数据: Hello world" << std::endl;
    // // 1、字符串 -> bytes
    // std::string data = "hello world";
    // request.set_coverage_data(data);

    // // 2、u8* -> bytes
    // const uint8_t* data_ptr = reinterpret_cast<const uint8_t*>("Hello, World!");
    // size_t data_len = 13;

    // std::string data(reinterpret_cast<const char*>(data_ptr), data_len);
    // request.set_coverage_data(data);


    // //  3、几个文件
    // uint8_t array[] = {0xE4, 0xBD, 0xA0, 0xE5, 0xA5, 0xBD, 0xE5, 0xA5, 0xBD, 0x00};
    // size_t len = sizeof(array) / sizeof(array[0]) - 1;

    // // 4、试探超时
    // const int arraySize = 8388608;
    // 4194299是数据单次传递的最大值了，因为考虑裁剪
    const int arraySize = 4194299;
    // const int arraySize = 65536;

    uint8_t array[arraySize];

    uint8_t value = 0xE4;

    for(int i = 0; i < arraySize; i++){
      array[i] = value;
    }

    std::string str(reinterpret_cast<const char*>(array), arraySize);
    request.set_coverage_data(str);

    // 5、 可行
    // request.set_coverage_data("Hello world");

    NewReply reply;
    ClientContext context;
    // context.set_compression_algorithm(GRPC_COMPRESS_DEFLATE);
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

  grpc::ChannelArguments ch_args;
  // ch_args.SetMaxReceiveMessageSize(8 * 1024 * 1024);
  // ch_args.SetMaxSendMessageSize(8 * 1024 * 1024);
  // ch_args.SetInt(GRPC_ARG_MAX_SEND_MESSAGE_LENGTH, 100*1024*1024);
  ch_args.SetInt(GRPC_ARG_MAX_SEND_MESSAGE_LENGTH, -1);
  GreeterClient greeter(
      grpc::CreateCustomChannel(target_str, grpc::InsecureChannelCredentials(), ch_args));

  // GreeterClient greeter(
  //    grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()));

  std::string user("world");
  std::string reply = greeter.NewSayHello(user);
  std::cout << "Greeter received: " << reply << std::endl;

  return 0;
}
