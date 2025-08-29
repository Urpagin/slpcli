//
// Created by urpagin on 8/10/25.
//

#include <utility>

#include "../include/slp.h"

#include <iostream>

#include "Dispatcher.h"


struct slp::Impl {
    SlpOptions opts;
    Dispatcher disp;
    Impl(const SlpOptions &opt, Callback cb) : opts(opt), disp(std::move(cb), opts) {}
};

slp::slp(SlpOptions opt, Callback cb) : p_(std::make_unique<Impl>(opt, std::move(cb))) {}

slp::~slp() {
    try {
        p_->disp.seal_and_wait();
    } catch (const std::exception &e) {
        err("Failed to destruct Slp object: ", e.what());
    } catch (...) {
        err("Failed to destruct Slp object, unknown exception");
    }
}

void slp::submit(ServerQuery q) const { p_->disp.submit(std::move(q)); }


void slp::seal_and_wait() const { p_->disp.seal_and_wait(); }
