#include <iostream>
#include <memory>
#include <string>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"

#include <grpcpp/grpcpp.h>

#ifdef BAZEL_BUILD
#include "examples/protos/helloworld.grpc.pb.h"
#else
#include "helloworld.grpc.pb.h"
#endif

ABSL_FLAG(std::string, target, "localhost:50051", "Server address");

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using helloworld::Greeter;
// 接口
using helloworld::NewRepeat;
using helloworld::NewReply;
using helloworld::MapData;

class GreeterClient {
 public:
  GreeterClient(std::shared_ptr<Channel> channel)
      : stub_(Greeter::NewStub(channel)) {}

   std::string NewSayHello(const std::string& user) {

    NewRepeat request;
    // 基础类型的repeated
    request.add_field(user);
    std::cout << "发送的数据: " << user << std::endl;

    // 结构体的repeated
    MapData *md1 = request.add_md();
    md1->set_index(3);
    md1->set_count(4);



    NewReply reply;
    ClientContext context;
    Status status = stub_->NewSayHello(&context, request, &reply);


    if (status.ok()) {
      for (int i = 0; i < reply.field_size(); ++i) {
        std::string user = reply.field(i);
        std::cout << "服务端返回的数据: " << user << std::endl;
      }

      for (int i = 0; i < reply.md_size(); ++i) {
        MapData md = reply.md(i);
        std::cout << "服务端返回的数据: " << md.index() << md.count() << std::endl;
      }
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
  std::string user("world");
  std::string reply = greeter.NewSayHello(user);
  std::cout << "Greeter received: " << reply << std::endl;

  return 0;
}
