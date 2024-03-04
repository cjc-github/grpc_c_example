#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/strings/str_format.h"

#include <grpcpp/grpcpp.h>

#ifdef BAZEL_BUILD
#include "examples/protos/helloworld.grpc.pb.h"
#else
#include "helloworld.grpc.pb.h"
#endif

ABSL_FLAG(uint16_t, port, 50051, "Server port for the service");

using grpc::CallbackServerContext;
using grpc::Server;
using grpc::ServerBidiReactor;
using grpc::ServerBuilder;
using grpc::Status;
using helloworld::Greeter;
using helloworld::HelloReply;
using helloworld::HelloRequest;

// Logic behind the server's behavior.
class KeyValueStoreServiceImpl final : public Greeter::CallbackService {
  ServerBidiReactor<HelloRequest, HelloReply>* SayHelloBidiStream(
      CallbackServerContext* context) override {
    class Reactor : public ServerBidiReactor<HelloRequest, HelloReply> {
     public:
      explicit Reactor() { StartRead(&request_); }

      // 收到客户端的请求时,会生成一个响应消息，并调用StartWrite方法将响应发送回客户端
      void OnReadDone(bool ok) override {
        if (!ok) {
          // 当客户端发送拒绝信号
          std::cout << "OnReadDone Cancelled!" << std::endl;
          return Finish(grpc::Status::CANCELLED);
        }
        // 发送消息Count x ACK
        response_.set_message(absl::StrCat(request_.name(), " Ack"));

        StartWrite(&response_);
        std::this_thread::sleep_for(std::chrono::seconds(1));
      }

      void OnWriteDone(bool ok) override {
        std::cout << "OnWriteDone!" << std::endl;
        if (!ok) {
          // Client cancelled it
          std::cout << "OnWriteDone Cancelled!" << std::endl;
          return Finish(grpc::Status::CANCELLED);
        }
        StartRead(&request_);
      }

      void OnDone() override { delete this; }

     private:
      HelloRequest request_;
      HelloReply response_;
    };

    return new Reactor();
  }
};

// 运行
void RunServer(uint16_t port) {
  std::string server_address = absl::StrFormat("0.0.0.0:%d", port);
  KeyValueStoreServiceImpl service;

  ServerBuilder builder;
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);

  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;

  server->Wait();
}

int main(int argc, char** argv) {
  absl::ParseCommandLine(argc, argv);
  RunServer(absl::GetFlag(FLAGS_port));
  return 0;
}
