// Copyright 2004-present Facebook. All Rights Reserved.

#include "src/automata/StreamSubscriptionResponderBase.h"

namespace reactivesocket {

void StreamSubscriptionResponderBase::onSubscribeImpl(
    std::shared_ptr<Subscription> subscription) {
  Base::onSubscribe(subscription);
}

void StreamSubscriptionResponderBase::onNextImpl(Payload response) {
  debugCheckOnNextOnCompleteOnError();
  switch (state_) {
    case State::RESPONDING:
      Base::onNext(std::move(response));
      break;
    case State::CLOSED:
      break;
  }
}

void StreamSubscriptionResponderBase::onCompleteImpl() {
  debugCheckOnNextOnCompleteOnError();
  switch (state_) {
    case State::RESPONDING: {
      state_ = State::CLOSED;
      connection_->outputFrameOrEnqueue(
          Frame_RESPONSE::complete(streamId_).serializeOut());
      connection_->endStream(streamId_, StreamCompletionSignal::GRACEFUL);
    } break;
    case State::CLOSED:
      break;
  }
}

void StreamSubscriptionResponderBase::onErrorImpl(folly::exception_wrapper ex) {
  debugCheckOnNextOnCompleteOnError();
  switch (state_) {
    case State::RESPONDING: {
      state_ = State::CLOSED;
      auto msg = ex.what().toStdString();
      connection_->outputFrameOrEnqueue(
          Frame_ERROR::applicationError(streamId_, msg).serializeOut());
      connection_->endStream(streamId_, StreamCompletionSignal::ERROR);
    } break;
    case State::CLOSED:
      break;
  }
}

void StreamSubscriptionResponderBase::endStream(StreamCompletionSignal signal) {
  switch (state_) {
    case State::RESPONDING:
      // Spontaneous ::endStream signal means an error.
      DCHECK(StreamCompletionSignal::GRACEFUL != signal);
      state_ = State::CLOSED;
      break;
    case State::CLOSED:
      break;
  }
  Base::endStream(signal);
}

void StreamSubscriptionResponderBase::onNextFrame(Frame_CANCEL&& frame) {
  switch (state_) {
    case State::RESPONDING:
      state_ = State::CLOSED;
      connection_->endStream(streamId_, StreamCompletionSignal::GRACEFUL);
      break;
    case State::CLOSED:
      break;
  }
}
}
