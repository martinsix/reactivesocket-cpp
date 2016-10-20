// Copyright 2004-present Facebook. All Rights Reserved.

#pragma once

#include <memory>

#include <gmock/gmock.h>

#include "src/Payload.h"
#include "src/RequestHandler.h"

namespace reactivesocket {

class MockRequestHandler : public RequestHandler {
 public:
  MOCK_METHOD2(
      handleRequestChannel_,
      Subscriber<Payload>*(Payload& request, SubscriberFactory&));
  MOCK_METHOD2(
      handleRequestStream_,
      void(Payload& request, SubscriberFactory&));
  MOCK_METHOD2(
      handleRequestSubscription_,
      void(Payload& request, SubscriberFactory&));
  MOCK_METHOD2(
      handleRequestResponse_,
      void(Payload& request, SubscriberFactory&));
  MOCK_METHOD1(handleFireAndForgetRequest_, void(Payload& request));
  MOCK_METHOD1(
      handleMetadataPush_,
      void(std::unique_ptr<folly::IOBuf>& request));
  MOCK_METHOD1(handleSetupPayload_, void(ConnectionSetupPayload& request));

  Subscriber<Payload>& handleRequestChannel(
      Payload request,
      SubscriberFactory& subscriberFactory) override {
    return *handleRequestChannel_(request, subscriberFactory);
  }

  void handleRequestStream(
      Payload request,
      SubscriberFactory& subscriberFactory) override {
    handleRequestStream_(request, subscriberFactory);
  }

  void handleRequestSubscription(
      Payload request,
      SubscriberFactory& subscriberFactory) override {
    handleRequestSubscription_(request, subscriberFactory);
  }

  void handleRequestResponse(
      Payload request,
      SubscriberFactory& subscriberFactory) override {
    handleRequestResponse_(request, subscriberFactory);
  }

  void handleFireAndForgetRequest(Payload request) override {
    handleFireAndForgetRequest_(request);
  }

  void handleMetadataPush(std::unique_ptr<folly::IOBuf> request) override {
    handleMetadataPush_(request);
  }

  void handleSetupPayload(ConnectionSetupPayload request) override {
    handleSetupPayload_(request);
  }
};
}
