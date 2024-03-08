#include <iostream>
#include <memory>
#include <string>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/strings/str_format.h"

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#ifdef BAZEL_BUILD
#include "examples/protos/helloworld_bytes.grpc.pb.h"
#else
#include "helloworld_bytes.grpc.pb.h"
#endif

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using helloworld::Greeter;
// 接口
using helloworld::NewRepeat;
using helloworld::NewReply;

ABSL_FLAG(uint16_t, port, 50051, "Server port for the service");


class GreeterServiceImpl final : public Greeter::Service {
  Status NewSayHello(ServerContext* context, const NewRepeat* request,
                  NewReply* reply) override {
    // context->set_compression_algorithm(GRPC_COMPRESS_GZIP);
    const std::string& coverage_data = request->coverage_data();
    // std::cout << "来自客户端的数据: " << coverage_data << std::endl; 

    size_t coverage_data_length = coverage_data.length(); // 或者使用 size() 方法
    std::cout << "coverage_data 的长度为: " << coverage_data_length << std::endl;

    reply->set_coverage_data(coverage_data);

    return Status::OK;
  }
};

void RunServer(uint16_t port) {
  std::string server_address = absl::StrFormat("0.0.0.0:%d", port);
  GreeterServiceImpl service;

  grpc::EnableDefaultHealthCheckService(true);
  grpc::reflection::InitProtoReflectionServerBuilderPlugin();
  ServerBuilder builder;

  // 服务端设置大小
  builder.SetMaxReceiveMessageSize(10 * 1024 * 1024);
  builder.SetMaxSendMessageSize(10 * 1024 * 1024);
  builder.SetMaxMessageSize(10 * 1024 * 1024);
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
