// Copyright 2004-present Facebook. All Rights Reserved.

#include "src/NullRequestHandler.h"
#include "src/Stats.h"

#include <folly/ExceptionWrapper.h>

namespace reactivesocket {

void NullSubscriber::onSubscribe(std::shared_ptr<Subscription> subscription) {
  subscription->cancel();
}

void NullSubscriber::onNext(Payload /*element*/) {}

void NullSubscriber::onComplete() {}

void NullSubscriber::onError(folly::exception_wrapper /*ex*/) {}

void NullSubscription::request(size_t /*n*/){};

void NullSubscription::cancel() {}

std::shared_ptr<Subscriber<Payload>> NullRequestHandler::handleRequestChannel(
    Payload /*request*/,
    StreamId /*streamId*/,
    const std::shared_ptr<Subscriber<Payload>>& response) {
  // TODO(lehecka): get rid of onSubscribe call
  response->onSubscribe(std::make_shared<NullSubscription>());
  response->onError(std::runtime_error("NullRequestHandler"));
  return std::make_shared<NullSubscriber>();
}

void NullRequestHandler::handleRequestStream(
    Payload /*request*/,
    StreamId /*streamId*/,
    const std::shared_ptr<Subscriber<Payload>>& response) {
  // TODO(lehecka): get rid of onSubscribe call
  response->onSubscribe(std::make_shared<NullSubscription>());
  response->onError(std::runtime_error("NullRequestHandler"));
}

void NullRequestHandler::handleRequestSubscription(
    Payload /*request*/,
    StreamId /*streamId*/,
    const std::shared_ptr<Subscriber<Payload>>& response) {
  // TODO(lehecka): get rid of onSubscribe call
  response->onSubscribe(std::make_shared<NullSubscription>());
  response->onError(std::runtime_error("NullRequestHandler"));
}

void NullRequestHandler::handleRequestResponse(
    Payload /*request*/,
    StreamId /*streamId*/,
    const std::shared_ptr<Subscriber<Payload>>& response) {
  response->onSubscribe(std::make_shared<NullSubscription>());
  response->onError(std::runtime_error("NullRequestHandler"));
}

void NullRequestHandler::handleFireAndForgetRequest(
    Payload /*request*/,
    StreamId /*streamId*/) {}

void NullRequestHandler::handleMetadataPush(
    std::unique_ptr<folly::IOBuf> /*request*/) {}

std::shared_ptr<StreamState> NullRequestHandler::handleSetupPayload(
    ReactiveSocket& socket,
    ConnectionSetupPayload /*request*/) {
  return std::make_shared<StreamState>(Stats::noop());
}

bool NullRequestHandler::handleResume(
    ReactiveSocket& socket,
    const ResumeIdentificationToken& /*token*/,
    ResumePosition /*position*/) {
  return false;
}

void NullRequestHandler::handleCleanResume(
    std::shared_ptr<Subscription> /* response */) {}

void NullRequestHandler::handleDirtyResume(
    std::shared_ptr<Subscription> /* response */) {}

} // reactivesocket
