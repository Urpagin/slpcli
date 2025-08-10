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
    Impl(const SlpOptions &opt, Callback cb) : opts(opt), disp(std::move(cb), opts.worker_thread_count) {}
};

slp::slp(SlpOptions opt, Callback cb) : p_(std::make_unique<Impl>(opt, std::move(cb))) {}

slp::~slp() {
    try {
        p_->disp.seal();
        p_->disp.finish();
    } catch (const std::exception &e) {
        std::cerr << "Error: failed to destruct Slp object: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "Error: failed to destruct Slp object, unknown exception" << std::endl;
    }
}

void slp::submit(ServerQuery q) const { p_->disp.submit(std::move(q)); }

void slp::seal() const { p_->disp.seal(); }

void slp::finish() const { p_->disp.finish(); }
