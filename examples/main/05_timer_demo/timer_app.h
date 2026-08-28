/*
 *     .============.
 *    //  M A K E  / \
 *   //  C++ DEV  /   \
 *  //  E A S Y  /  \/ \
 * ++ ----------.  \/\\  .
 *  \\     \\     \\ /\\  /
 *   \\     \\     \\   /
 *    \\     \\     \\ /
 *     -============'
 *
 * Copyright (c) 2018 Hevake and contributors, all rights reserved.
 *
 * This file is part of cpp-tbox (https://github.com/cpp-main/cpp-tbox)
 * Use of this source code is governed by MIT license that can be found
 * in the LICENSE file in the root of the source tree. All contributing
 * project authors may be found in the CONTRIBUTORS.md file in the root
 * of the source tree.
 */
#ifndef TBOX_MAIN_EXAMPLE_TIMER_APP_H_20240607
#define TBOX_MAIN_EXAMPLE_TIMER_APP_H_20240607

#include <string>
#include <chrono>

#include <tbox/main/main.h>

namespace tbox::event {
class TimerEvent;
}

class TimerApp : public tbox::main::Module {
  public:
    explicit TimerApp(tbox::main::Context &ctx);
    ~TimerApp() override;

  protected:
    void onFillDefaultConfig(tbox::Json &cfg) override;
    bool onInit(const tbox::Json &cfg) override;
    bool onStart() override;
    void onStop() override;
    void onCleanup() override;

  private:
    tbox::event::TimerEvent *timer_ = nullptr;
    std::chrono::milliseconds interval_{0};
    std::string message_;
};

#endif // TBOX_MAIN_EXAMPLE_TIMER_APP_H_20240607
