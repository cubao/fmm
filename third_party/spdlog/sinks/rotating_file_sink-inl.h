// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#ifndef SPDLOG_HEADER_ONLY
#include "spdlog/sinks/rotating_file_sink.h"
#endif

#include "spdlog/common.h"

#include "spdlog/details/file_helper.h"
#include "spdlog/details/null_mutex.h"
#include "spdlog/fmt/fmt.h"

#include <cerrno>
#include <chrono>
#include <ctime>
#include <mutex>
#include <string>
#include <tuple>

namespace spdlog
{
namespace sinks
{

template <typename Mutex>
SPDLOG_INLINE rotating_file_sink<Mutex>::rotating_file_sink(
    filename_t base_filename, std::size_t max_size, std::size_t max_files,
    bool rotate_on_open)
    : base_filename_(std::move(base_filename)), max_size_(max_size),
      max_files_(max_files)
{
    file_helper_.open(calc_filename(base_filename_, 0));
    current_size_ = file_helper_.size(); // expensive. called only once
    if (rotate_on_open && current_size_ > 0) {
        rotate_();
    }
}

// calc filename according to index and file extension if exists.
// e.g. calc_filename("logs/mylog.txt, 3) => "logs/mylog.3.txt".
template <typename Mutex>
SPDLOG_INLINE filename_t rotating_file_sink<Mutex>::calc_filename(
    const filename_t &filename, std::size_t index)
{
    typename std::conditional<std::is_same<filename_t::value_type, char>::value,
                              fmt::memory_buffer, fmt::wmemory_buffer>::type w;
    if (index != 0u) {
        filename_t basename, ext;
        std::tie(basename, ext) =
            details::file_helper::split_by_extension(filename);
        fmt::format_to(w, SPDLOG_FILENAME_T("{}.{}{}"), basename, index, ext);
    } else {
        fmt::format_to(w, SPDLOG_FILENAME_T("{}"), filename);
    }
    return fmt::to_string(w);
}

template <typename Mutex>
SPDLOG_INLINE const filename_t &rotating_file_sink<Mutex>::filename() const
{
    return file_helper_.filename();
}

template <typename Mutex>
SPDLOG_INLINE void
rotating_file_sink<Mutex>::sink_it_(const details::log_msg &msg)
{
    fmt::memory_buffer formatted;
    base_sink<Mutex>::formatter_->format(msg, formatted);
    current_size_ += formatted.size();
    if (current_size_ > max_size_) {
        rotate_();
        current_size_ = formatted.size();
    }
    file_helper_.write(formatted);
}

template <typename Mutex> SPDLOG_INLINE void rotating_file_sink<Mutex>::flush_()
{
    file_helper_.flush();
}

// Rotate files:
// log.txt -> log.1.txt
// log.1.txt -> log.2.txt
// log.2.txt -> log.3.txt
// log.3.txt -> delete
template <typename Mutex>
SPDLOG_INLINE void rotating_file_sink<Mutex>::rotate_()
{
    using details::os::filename_to_str;
    file_helper_.close();
    for (auto i = max_files_; i > 0; --i) {
        filename_t src = calc_filename(base_filename_, i - 1);
        if (!details::file_helper::file_exists(src)) {
            continue;
        }
        filename_t target = calc_filename(base_filename_, i);

        if (!rename_file(src, target)) {
            // if failed try again after a small delay.
            // this is a workaround to a windows issue, where very high rotation
            // rates can cause the rename to fail with permission denied
            // (because of antivirus?).
            details::os::sleep_for_millis(100);
            if (!rename_file(src, target)) {
                file_helper_.reopen(
                    true); // truncate the log file anyway to prevent it to grow
                           // beyond its limit!
                current_size_ = 0;
                SPDLOG_THROW(spdlog_ex("rotating_file_sink: failed renaming " +
                                           filename_to_str(src) + " to " +
                                           filename_to_str(target),
                                       errno));
            }
        }
    }
    file_helper_.reopen(true);
}

// delete the target if exists, and rename the src file  to target
// return true on success, false otherwise.
template <typename Mutex>
SPDLOG_INLINE bool
rotating_file_sink<Mutex>::rename_file(const filename_t &src_filename,
                                       const filename_t &target_filename)
{
    // try to delete the target file in case it already exists.
    (void)details::os::remove(target_filename);
    return details::os::rename(src_filename, target_filename) == 0;
}

} // namespace sinks
} // namespace spdlog
