/*
 *
 * Copyright 2018 gRPC authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include <iostream>
#include <memory>
#include <string>

#include <grpcpp/grpcpp.h>

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
using helloworld::HelloReply;
using helloworld::HelloRequest;


// typedef enum {
//   GRPC_COMPRESS_NONE = 0,
//   GRPC_COMPRESS_DEFLATE = 1,
//   GRPC_COMPRESS_GZIP = 2,
//   /* EXPERIMENTAL: Stream compression is currently experimental. */
//   GRPC_COMPRESS_STREAM_GZIP = 3,
//   /* TODO(ctiller): snappy */
//   GRPC_COMPRESS_ALGORITHMS_COUNT = 4
// } grpc_compression_algorithm;

class GreeterServiceImpl final : public Greeter::Service {
  Status SayHello(ServerContext* context, const HelloRequest* request,
                  HelloReply* reply) override {
    // 设置压缩级别
    context->set_compression_algorithm(GRPC_COMPRESS_GZIP);
    // 打印出压缩算法
    std::cout << " [+] compression algorithm: " << context->compression_algorithm() << std::endl;

    // 设置返回信息
    std::string prefix("Hello ");
    reply->set_message(prefix + request->name());

    // 打印结果
    std::cout << " [+] Server return: " << prefix << request->name() << std::endl;
    return Status::OK;
  }
};

void RunServer() {
  std::string server_address("0.0.0.0:50051");
  GreeterServiceImpl service;

  ServerBuilder builder;
  // channel级别的压缩算法，但是会被调用时的压缩算法给覆盖掉
  builder.SetDefaultCompressionAlgorithm(GRPC_COMPRESS_DEFLATE);
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  builder.RegisterService(&service);

  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;
  server->Wait();
}

int main(int argc, char** argv) {
  RunServer();

  return 0;
}
