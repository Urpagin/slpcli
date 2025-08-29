//
// Created by Urpagin on 2025-08-28.
//
// A simple logger that does the work inside a dedicated thread so as
// not to block other operations too much.
//
// Licence: MIT
//
// "Multiple Producers, Single Consumer" type of logger.
#pragma once

#include <atomic>
#include <condition_variable>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <queue>
#include <string_view>
#include <thread>

namespace logger::detail {
    using item = std::tuple<std::string, bool>;

    // if true, does not print anything.
    static std::atomic<bool> is_silent{false};

    static std::condition_variable cv;
    static std::queue<item> log_q;
    static std::mutex log_q_guard{};

    // Mainly for the log() function warning, less for the log_kill_consumer where
    // I could've used a sentinel value, where I feel it could've been faster.
    static std::atomic<bool> do_stop{false};

    static std::thread worker;


    /// @brief Returns a human-readable timestamp.
    inline std::string make_timestamp() {
        using namespace std::chrono;
        const std::time_t time = system_clock::to_time_t(system_clock::now());
        const std::tm tm = *std::localtime(&time);
        std::ostringstream oss;
        oss << std::put_time(&tm, "%H:%M:%S");
        return oss.str();
    }

    static constexpr std::string_view RED{"\033[31m"};
    static constexpr std::string_view BLUE{"\033[34m"};
    static constexpr std::string_view RESET{"\033[0m"};

    /// @brief Colours the string.
    inline std::string do_colour(const std::string &msg, const bool is_err) {
        std::ostringstream oss;
        if (is_err) {
            oss << RED << msg << RESET;
        } else {
            oss << BLUE << msg << RESET;
        }
        return oss.str();
    }

    /// @brief Formats the string to be pretty~
    inline std::string do_format(const std::string &msg, const bool is_err) {
        std::ostringstream oss;
        if (is_err) {
            oss << '[' << make_timestamp() << ']' << " [ERR] " << msg;
            return do_colour(oss.str(), is_err);
        }
        oss << '[' << make_timestamp() << ']' << " [INFO] " << msg;
        return do_colour(oss.str(), is_err);
    }

    /// @brief App logger, instead of std::{cout,cerr} each time.
    static void log(const std::string_view msg, const bool is_err) {
        if (do_stop)
            return;
        // Push to queue, making sure to guard.
        {
            std::lock_guard lk(log_q_guard);
            // clone the msg, having a reference put into another thread is very unsafe.
            log_q.emplace(std::string(msg), is_err);
        }
        // notify the worker
        cv.notify_one();
    }
} // namespace logger::detail

namespace logger {
    namespace d = detail;


    /// @brief Starts the logger consumer.
    /// Logic inspired from https://en.cppreference.com/w/cpp/thread/condition_variable.html
    static void log_start() {
        // Our worker thread
        d::worker = std::thread([&]() {
            while (!d::do_stop || !d::log_q.empty()) {
                std::unique_lock lk(d::log_q_guard);
                d::cv.wait(lk, [&] { return !d::log_q.empty() || d::do_stop; });
                // After the wait, we own the lock.

                // We don't allow an empty queue to be .front()'ed
                if (d::log_q.empty() && d::do_stop)
                    break;

                auto [msg, is_stderr] = d::log_q.front();
                d::log_q.pop();
                lk.unlock();
                (is_stderr ? std::cerr : std::cout) << d::do_format(msg, is_stderr) << '\n';
            }
        });
        // Don't detach because detaching and joining afterwards is UB.
    }

    /// @brief Kills logging - seals the log() function and waits for the queue to empty.
    static void log_end() {
        d::do_stop = true;
        d::cv.notify_all();
        // Finally stop the thread.
        d::worker.join();
    }

    /// @brief Changes the detail::is_silent flag to the arg.
    static void log_set_silent(const bool is_silent) { d::is_silent = is_silent; }


    /// @brief Logs a message with a blue colour, INFO mode. To stdout.
    /// Behavior: Appends a newline after argument expansions.
    /// Usage: info(a, b, c, d) E.g., err("hello", "world", a_var)
    template<class... Ts>
    static void info(const Ts &...xs) {
        if (d::is_silent)
            return;

        std::ostringstream oss;
        (oss << ... << xs);
        d::log(oss.view(), false);
    }

    /// @brief Logs a message with a red colour, ERR mode. To stdout.
    /// Behavior: Appends a newline after argument expansions.
    /// Usage: err(a, b, c, d) E.g., err("hello", "world", a_var)
    template<class... Ts>
    static void err(const Ts &...xs) {
        if (d::is_silent)
            return;

        std::ostringstream oss;
        (oss << ... << xs);
        d::log(oss.view(), true);
    }
} // namespace logger
