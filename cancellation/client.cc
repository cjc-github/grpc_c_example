#include <condition_variable>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/strings/str_cat.h"

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
using grpc::StatusCode;
using helloworld::Greeter;
using helloworld::HelloReply;
using helloworld::HelloRequest;


class KeyValueStoreClient
    : public grpc::ClientBidiReactor<HelloRequest, HelloReply> {
 public:
  KeyValueStoreClient(std::shared_ptr<Channel> channel)
      : stub_(Greeter::NewStub(channel)) {
    stub_->async()->SayHelloBidiStream(&context_, this);
    request_.set_name("Begin");
    
    // 启动异步写, 发送Begin
    StartWrite(&request_);
    // 启动与服务器的双向流式RPC调用
    StaErtCall();
  }

  // 将打印请求的名称和响应的消息，并发送下一个请求; 当发送10个请求时，则使用context_TryCancel来取消PRC
  void OnReadDone(bool ok) override {
    if (ok) {
      // 打印出
      std::cout << request_.name() << " : " << response_.message() << std::endl;

      if (++counter_ < 100) {
        request_.set_name(absl::StrCat("Count ", counter_));
        
        StartWrite(&request_);
        std::this_thread::sleep_for(std::chrono::seconds(1));
      } else {
        // Cancel after sending 10 messages
        context_.TryCancel();
      }
    }
  }

  // 准备写入
  void OnWriteDone(bool ok) override {
    if (ok) {
      StartRead(&response_);
    }
  }

  // 返回拒绝信息
  void OnDone(const grpc::Status& status) override {
    if (!status.ok()) {
      // 如果状态为拒绝状态
      if (status.error_code() == StatusCode::CANCELLED) {
        // Eventually client will know here that call is cancelled.
        std::cout << "RPC Cancelled!" << std::endl;
      } else {
        std::cout << "RPC Failed: " << status.error_code() << ": "
                  << status.error_message() << std::endl;
      }
    }
    std::unique_lock<std::mutex> l(mu_);
    done_ = true;
    cv_.notify_all();
  }

  // 最开始就执行
  void Await() {
    std::unique_lock<std::mutex> l(mu_);
    while (!done_) {
      cv_.wait(l);
    }
  }

 private:
  std::unique_ptr<Greeter::Stub> stub_;
  size_t counter_ = 0;
  ClientContext context_;
  bool done_ = false;
  HelloRequest request_;
  HelloReply response_;
  std::mutex mu_;
  std::condition_variable cv_;
};

int main(int argc, char** argv) {
  absl::ParseCommandLine(argc, argv);
  std::string target_str = absl::GetFlag(FLAGS_target);
  KeyValueStoreClient client(
      grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials()));
  client.Await();
  return 0;
}
