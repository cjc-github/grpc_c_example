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

using grpc::Channel;
using grpc::ChannelArguments;
using grpc::ClientContext;
using grpc::Status;
using helloworld::Greeter;
using helloworld::HelloReply;
using helloworld::HelloRequest;

class GreeterClient {
 public:
  GreeterClient(std::shared_ptr<Channel> channel)
      : stub_(Greeter::NewStub(channel)) {}


  std::string SayHello(const std::string& user) {
    HelloRequest request;
    request.set_name(user);
    std::cout << " [+] Client send: " << user << std::endl;
    HelloReply reply;
    ClientContext context;

    // 设置压缩算法
    context.set_compression_algorithm(GRPC_COMPRESS_DEFLATE);

    // 打印出压缩算法
    std::cout << " [+] compression algorithm: " << context.compression_algorithm() << std::endl;


    // 调用SayHello
    Status status = stub_->SayHello(&context, request, &reply);

    // 返回
    if (status.ok()) {
      return reply.message();
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
  ChannelArguments args;

  // channel级别的压缩算法，但是会被调用时的压缩算法给覆盖掉
  args.SetCompressionAlgorithm(GRPC_COMPRESS_GZIP);
  GreeterClient greeter(grpc::CreateCustomChannel(
      "localhost:50051", grpc::InsecureChannelCredentials(), args));

  // 调用SayHello函数
  std::string user("world world world world");
  std::string reply = greeter.SayHello(user);
  std::cout << " [+] Greeter received: " << reply << std::endl;
  return 0;
}
