// Copyright 2014 Google Inc. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "precompiled.h"
#include "fplbase/async_loader.h"
#include "fplbase/utilities.h"

#ifndef FPLBASE_BACKEND_SDL
#error This version of AsyncLoader depends on SDL.
#endif

namespace fplbase {

// Push this to signal the worker thread that it's time to quit.
class BookendAsyncResource : public AsyncAsset {
  static const char *kBookendFileName;

 public:
  BookendAsyncResource() : AsyncAsset(kBookendFileName) {}
  virtual ~BookendAsyncResource() {}
  virtual void Load() {}
  virtual bool Finalize() { return true; }
  virtual bool IsValid() { return true; }
  static bool IsBookend(const AsyncAsset &res) {
    return res.filename() == kBookendFileName;
  }
};

// static
const char *BookendAsyncResource::kBookendFileName = "bookend";

AsyncLoader::AsyncLoader() : worker_thread_(nullptr) {
  mutex_ = SDL_CreateMutex();
  job_semaphore_ = SDL_CreateSemaphore(0);
  assert(mutex_ && job_semaphore_);
}

AsyncLoader::~AsyncLoader() {
  Stop();
}

void AsyncLoader::Stop() {
  if (worker_thread_) {
    StopLoadingWhenComplete();
    SDL_WaitThread(static_cast<SDL_Thread *>(worker_thread_), nullptr);
    worker_thread_ = nullptr;

    if (mutex_) {
      SDL_DestroyMutex(static_cast<SDL_mutex *>(mutex_));
      mutex_ = nullptr;
    }
    if (job_semaphore_) {
      SDL_DestroySemaphore(static_cast<SDL_semaphore *>(job_semaphore_));
      job_semaphore_ = nullptr;
    }
  }
}

void AsyncLoader::QueueJob(AsyncAsset *res) {
  Lock([this, res]() { queue_.push_back(res); });
  SDL_SemPost(static_cast<SDL_semaphore *>(job_semaphore_));
}

void AsyncLoader::LoaderWorker() {
  for (;;) {
    auto res = LockReturn<AsyncAsset *>(
        [this]() { return queue_.empty() ? nullptr : queue_.front(); });
    if (!res) {
      SDL_SemWait(static_cast<SDL_semaphore *>(job_semaphore_));
      continue;
    }
    // Stop loading once we reach the bookend enqueued by
    // StopLoadingWhenComplete(). To start loading again, call StartLoading().
    if (BookendAsyncResource::IsBookend(*res)) break;
    LogInfo(kApplication, "async load: %s", res->filename_.c_str());
    res->Load();
    Lock([this, res]() {
      queue_.pop_front();
      done_.push_back(res);
    });
  }
}

int AsyncLoader::LoaderThread(void *user_data) {
  reinterpret_cast<AsyncLoader *>(user_data)->LoaderWorker();
  return 0;
}

void AsyncLoader::StartLoading() {
  worker_thread_ =
      SDL_CreateThread(AsyncLoader::LoaderThread, "FPL Loader Thread", this);
  assert(worker_thread_);
}

void AsyncLoader::PauseLoading() { assert(false); }

void AsyncLoader::StopLoadingWhenComplete() {
  // When the loader thread hits the bookend, it will exit.
  static BookendAsyncResource bookend;
  QueueJob(&bookend);
}

bool AsyncLoader::TryFinalize() {
  for (;;) {
    auto res = LockReturn<AsyncAsset *>(
        [this]() { return done_.empty() ? nullptr : done_.front(); });
    if (!res) break;
    bool ok = res->Finalize();
    if (!ok) {
      // Can't do much here, since res is already constructed. Caller has to
      // check IsValid() to know if resource can be used.
    }
    Lock([this]() { done_.pop_front(); });
  }
  return LockReturn<bool>([this]() { return queue_.empty() && done_.empty(); });
}

void AsyncLoader::Lock(const std::function<void()> &body) {
  auto err = SDL_LockMutex(static_cast<SDL_mutex *>(mutex_));
  (void)err;
  assert(err == 0);
  body();
  SDL_UnlockMutex(static_cast<SDL_mutex *>(mutex_));
}

}  // namespace fplbase
