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
#include "timer_app.h"

#include <tbox/base/log.h>
#include <tbox/base/defines.h>
#include <tbox/base/json.hpp>
#include <tbox/event/timer_event.h>

TimerApp::TimerApp(tbox::main::Context &ctx) :
    Module("timer_demo", ctx),
    timer_(ctx.loop()->newTimerEvent())
{ }

TimerApp::~TimerApp()
{
    CHECK_DELETE_RESET_OBJ(timer_);
}

void TimerApp::onFillDefaultConfig(tbox::Json &cfg)
{
    cfg["interval_ms"] = 1000;
    cfg["message"] = "Tick from cpp-tbox";
}

bool TimerApp::onInit(const tbox::Json &cfg)
{
    if (timer_ == nullptr) {
        LogWarn("timer object is null");
        return false;
    }

    auto js_interval = cfg["interval_ms"];
    if (!js_interval.is_number_integer() && !js_interval.is_number_unsigned()) {
        LogWarn("interval_ms should be integer");
        return false;
    }
    int interval_ms = js_interval.get<int>();
    if (interval_ms <= 0) {
        LogWarn("interval_ms must be positive");
        return false;
    }
    interval_ = std::chrono::milliseconds(interval_ms);

    auto js_message = cfg["message"];
    if (js_message.is_string())
        message_ = js_message.get<std::string>();
    else
        message_ = "Tick from cpp-tbox";

    if (!timer_->initialize(interval_, tbox::event::Event::Mode::kPersist)) {
        LogWarn("failed to initialize timer");
        return false;
    }

    timer_->setCallback([this] {
        LogInfo("%s", message_.c_str());
    });

    LogInfo("timer configured: interval=%lld ms, message='%s'",
            static_cast<long long>(interval_.count()), message_.c_str());
    return true;
}

bool TimerApp::onStart()
{
    if (timer_ == nullptr)
        return false;

    if (!timer_->enable()) {
        LogWarn("failed to enable timer");
        return false;
    }

    LogInfo("timer started");
    return true;
}

void TimerApp::onStop()
{
    if (timer_ != nullptr) {
        timer_->disable();
        LogInfo("timer stopped");
    }
}

void TimerApp::onCleanup()
{
    CHECK_DELETE_RESET_OBJ(timer_);
    interval_ = std::chrono::milliseconds{0};
    message_.clear();
}
