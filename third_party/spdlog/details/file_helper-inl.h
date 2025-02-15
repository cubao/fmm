// Copyright(c) 2015-present, Gabi Melman & spdlog contributors.
// Distributed under the MIT License (http://opensource.org/licenses/MIT)

#pragma once

#ifndef SPDLOG_HEADER_ONLY
#include "spdlog/details/file_helper.h"
#endif

#include "spdlog/details/os.h"
#include "spdlog/common.h"

#include <cerrno>
#include <chrono>
#include <cstdio>
#include <string>
#include <thread>
#include <tuple>

namespace spdlog
{
namespace details
{

SPDLOG_INLINE file_helper::~file_helper() { close(); }

SPDLOG_INLINE void file_helper::open(const filename_t &fname, bool truncate)
{
    close();
    auto *mode = truncate ? SPDLOG_FILENAME_T("wb") : SPDLOG_FILENAME_T("ab");
    _filename = fname;
    for (int tries = 0; tries < open_tries; ++tries) {
        if (!os::fopen_s(&fd_, fname, mode)) {
            return;
        }

        details::os::sleep_for_millis(open_interval);
    }

    SPDLOG_THROW(spdlog_ex("Failed opening file " +
                               os::filename_to_str(_filename) + " for writing",
                           errno));
}

SPDLOG_INLINE void file_helper::reopen(bool truncate)
{
    if (_filename.empty()) {
        SPDLOG_THROW(
            spdlog_ex("Failed re opening file - was not opened before"));
    }
    open(_filename, truncate);
}

SPDLOG_INLINE void file_helper::flush() { std::fflush(fd_); }

SPDLOG_INLINE void file_helper::close()
{
    if (fd_ != nullptr) {
        std::fclose(fd_);
        fd_ = nullptr;
    }
}

SPDLOG_INLINE void file_helper::write(const fmt::memory_buffer &buf)
{
    size_t msg_size = buf.size();
    auto data = buf.data();
    if (std::fwrite(data, 1, msg_size, fd_) != msg_size) {
        SPDLOG_THROW(spdlog_ex(
            "Failed writing to file " + os::filename_to_str(_filename), errno));
    }
}

SPDLOG_INLINE size_t file_helper::size() const
{
    if (fd_ == nullptr) {
        SPDLOG_THROW(spdlog_ex("Cannot use size() on closed file " +
                               os::filename_to_str(_filename)));
    }
    return os::filesize(fd_);
}

SPDLOG_INLINE const filename_t &file_helper::filename() const
{
    return _filename;
}

SPDLOG_INLINE bool file_helper::file_exists(const filename_t &fname)
{
    return os::file_exists(fname);
}

//
// return file path and its extension:
//
// "mylog.txt" => ("mylog", ".txt")
// "mylog" => ("mylog", "")
// "mylog." => ("mylog.", "")
// "/dir1/dir2/mylog.txt" => ("/dir1/dir2/mylog", ".txt")
//
// the starting dot in filenames is ignored (hidden files):
//
// ".mylog" => (".mylog". "")
// "my_folder/.mylog" => ("my_folder/.mylog", "")
// "my_folder/.mylog.txt" => ("my_folder/.mylog", ".txt")
SPDLOG_INLINE std::tuple<filename_t, filename_t>
file_helper::split_by_extension(const filename_t &fname)
{
    auto ext_index = fname.rfind('.');

    // no valid extension found - return whole path and empty string as
    // extension
    if (ext_index == filename_t::npos || ext_index == 0 ||
        ext_index == fname.size() - 1) {
        return std::make_tuple(fname, filename_t());
    }

    // treat casese like "/etc/rc.d/somelogfile or "/abc/.hiddenfile"
    auto folder_index = fname.rfind(details::os::folder_sep);
    if (folder_index != filename_t::npos && folder_index >= ext_index - 1) {
        return std::make_tuple(fname, filename_t());
    }

    // finally - return a valid base and extension tuple
    return std::make_tuple(fname.substr(0, ext_index), fname.substr(ext_index));
}
} // namespace details
} // namespace spdlog
