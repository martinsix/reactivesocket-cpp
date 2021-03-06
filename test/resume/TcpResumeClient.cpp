// Copyright 2004-present Facebook. All Rights Reserved.

#include <folly/Memory.h>
#include <folly/io/async/EventBaseManager.h>
#include <folly/io/async/ScopedEventBaseThread.h>
#include <gmock/gmock.h>
#include "src/ClientResumeStatusCallback.h"
#include "src/FrameTransport.h"
#include "src/NullRequestHandler.h"
#include "src/StandardReactiveSocket.h"
#include "src/folly/FollyKeepaliveTimer.h"
#include "src/framed/FramedDuplexConnection.h"
#include "src/tcp/TcpDuplexConnection.h"
#include "test/simple/PrintSubscriber.h"
#include "test/simple/StatsPrinter.h"

using namespace ::testing;
using namespace ::reactivesocket;
using namespace ::folly;

DEFINE_string(host, "localhost", "host to connect to");
DEFINE_int32(port, 9898, "host:port to connect to");

namespace {
class Callback : public AsyncSocket::ConnectCallback {
 public:
  virtual ~Callback() = default;

  void connectSuccess() noexcept override {}

  void connectErr(const AsyncSocketException& ex) noexcept override {
    std::cout << "TODO error" << ex.what() << " " << ex.getType() << "\n";
  }
};

class ResumeCallback : public ClientResumeStatusCallback {
  void onResumeOk() override {
    LOG(INFO) << "resumeOk";
  }

  // Called when an ERROR frame with CONNECTION_ERROR is received during
  // resuming operation
  void onResumeError(folly::exception_wrapper ex) override {
    LOG(INFO) << "resumeError: " << ex.what();
  }

  // Called when the resume operation was interrupted due to network
  // the application code may try to resume again.
  void onConnectionError(folly::exception_wrapper ex) override {
    LOG(INFO) << "connectionError: " << ex.what();
  }
};
}

int main(int argc, char* argv[]) {
  FLAGS_logtostderr = true;
  FLAGS_minloglevel = 0;

  google::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);
  google::InstallFailureSignalHandler();

  ScopedEventBaseThread eventBaseThread;

  std::unique_ptr<StandardReactiveSocket> reactiveSocket;
  Callback callback;
  StatsPrinter stats;

  auto token = ResumeIdentificationToken::generateNew();

  eventBaseThread.getEventBase()->runInEventBaseThreadAndWait([&]() {
    folly::SocketAddress addr(FLAGS_host, FLAGS_port, true);

    folly::AsyncSocket::UniquePtr socket(
        new folly::AsyncSocket(eventBaseThread.getEventBase()));
    socket->connect(&callback, addr);

    LOG(INFO) << "attempting connection to " << addr.describe();

    std::unique_ptr<DuplexConnection> connection =
        folly::make_unique<TcpDuplexConnection>(std::move(socket), stats);
    std::unique_ptr<DuplexConnection> framedConnection =
        folly::make_unique<FramedDuplexConnection>(std::move(connection));
    std::unique_ptr<RequestHandler> requestHandler =
        folly::make_unique<DefaultRequestHandler>();

    reactiveSocket = StandardReactiveSocket::disconnectedClient(
        *eventBaseThread.getEventBase(),
        std::move(requestHandler),
        stats,
        folly::make_unique<FollyKeepaliveTimer>(
            *eventBaseThread.getEventBase(), std::chrono::seconds(10)));

    reactiveSocket->onConnected([](ReactiveSocket& socket) {
      LOG(INFO) << "socket connected " << &socket;
    });
    reactiveSocket->onDisconnected([](ReactiveSocket& socket) {
      LOG(INFO) << "socket disconnect " << &socket;
    });
    reactiveSocket->onClosed([](ReactiveSocket& socket) {
      LOG(INFO) << "socket closed " << &socket;
    });

    LOG(INFO) << "requestStream:";
    reactiveSocket->requestStream(
        Payload("from client"), std::make_shared<PrintSubscriber>());

    LOG(INFO) << "connecting RS ...";
    reactiveSocket->clientConnect(
        std::make_shared<FrameTransport>(std::move(framedConnection)),
        ConnectionSetupPayload(
            "text/plain", "text/plain", Payload("meta", "data"), true, token));
  });

  std::string input;
  std::getline(std::cin, input);

  eventBaseThread.getEventBase()->runInEventBaseThreadAndWait([&]() {
    LOG(INFO) << "disconnecting RS ...";
    reactiveSocket->disconnect();
    LOG(INFO) << "requestStream:";
    reactiveSocket->requestStream(
        Payload("from client2"), std::make_shared<PrintSubscriber>());
  });

  std::getline(std::cin, input);

  eventBaseThread.getEventBase()->runInEventBaseThreadAndWait([&]() {
    folly::SocketAddress addr(FLAGS_host, FLAGS_port, true);

    LOG(INFO) << "new TCP connection ...";
    folly::AsyncSocket::UniquePtr socketResume(
        new folly::AsyncSocket(eventBaseThread.getEventBase()));
    socketResume->connect(&callback, addr);

    std::unique_ptr<DuplexConnection> connectionResume =
        folly::make_unique<TcpDuplexConnection>(std::move(socketResume), stats);
    std::unique_ptr<DuplexConnection> framedConnectionResume =
        folly::make_unique<FramedDuplexConnection>(std::move(connectionResume));

    LOG(INFO) << "try resume ...";
    reactiveSocket->tryClientResume(
        token,
        std::make_shared<FrameTransport>(std::move(framedConnectionResume)),
        folly::make_unique<ResumeCallback>());
  });

  std::getline(std::cin, input);

  // TODO why need to shutdown in eventbase?
  eventBaseThread.getEventBase()->runInEventBaseThreadAndWait(
      [&reactiveSocket]() {
        LOG(INFO) << "releasing RS";
        reactiveSocket.reset(nullptr);
      });

  return 0;
}
