// Copyright 2004-present Facebook. All Rights Reserved.

#pragma once

#include "src/ConnectionSetupPayload.h"
#include "src/Payload.h"
#include "src/ReactiveStreamsCompat.h"

namespace folly {
class Executor;
}

namespace reactivesocket {

class SubscriberFactory {
 public:
  virtual ~SubscriberFactory() = default;
  virtual Subscriber<Payload>& createSubscriber() = 0;
  virtual Subscriber<Payload>& createSubscriber(folly::Executor& executor) = 0;
};

class RequestHandler {
 public:
  virtual ~RequestHandler() = default;

  /// Handles a new Channel requested by the other end.
  ///
  /// Modelled after Publisher::subscribe, hence must synchronously call
  /// Subscriber::onSubscribe, and provide a valid Subscription.
  virtual Subscriber<Payload>& handleRequestChannel(
      Payload request,
      SubscriberFactory& subscriberFactory) = 0;

  /// Handles a new Stream requested by the other end.
  virtual void handleRequestStream(
      Payload request,
      SubscriberFactory& subscriberFactory) = 0;

  /// Handles a new inbound Subscription requested by the other end.
  virtual void handleRequestSubscription(
      Payload request,
      SubscriberFactory& subscriberFactory) = 0;

  /// Handles a new inbound RequestResponse requested by the other end.
  virtual void handleRequestResponse(
      Payload request,
      SubscriberFactory& subscriberFactory) = 0;

  /// Handles a new fire-and-forget request sent by the other end.
  virtual void handleFireAndForgetRequest(Payload request) = 0;

  /// Handles a new metadata-push sent by the other end.
  virtual void handleMetadataPush(std::unique_ptr<folly::IOBuf> request) = 0;

  /// Temporary home - this should eventually be an input to asking for a
  /// RequestHandler so negotiation is possible
  virtual void handleSetupPayload(ConnectionSetupPayload request) = 0;
};
}
