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
#include "examples/protos/helloworld.grpc.pb.h"
#else
#include "helloworld.grpc.pb.h"
#endif

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using helloworld::Greeter;
// 接口
using helloworld::NewRepeat;
using helloworld::NewReply;
using helloworld::MapData;

ABSL_FLAG(uint16_t, port, 50051, "Server port for the service");


class GreeterServiceImpl final : public Greeter::Service {
  Status NewSayHello(ServerContext* context, const NewRepeat* request,
                  NewReply* reply) override {

   // 基础类型的repeated
    std::string user = request->field(0);
    std::cout << "来自客户端的数据: " << user << std::endl;

    const MapData& md1 = request->md(0);
    std::cout << "来自客户端的数据: " << md1.index() << std::endl;
    std::cout << "来自客户端的数据: " << md1.count() << std::endl;


    // 返回信息
    std::vector<std::string> messages = {"Hello", "World"};
    for (const auto& message : messages) {
        reply->add_field(message);
    }

    MapData* md2 = reply->add_md();
    md2->set_index(md1.index() + 1);
    md2->set_count(md1.count() + 1);

    return Status::OK;
  }
};

void RunServer(uint16_t port) {
  std::string server_address = absl::StrFormat("0.0.0.0:%d", port);
  GreeterServiceImpl service;

  grpc::EnableDefaultHealthCheckService(true);
  grpc::reflection::InitProtoReflectionServerBuilderPlugin();
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
