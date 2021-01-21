#ifndef DELAY_TRANSPORT_RAWLOGGER_H
#define DELAY_TRANSPORT_RAWLOGGER_H

#include <string>
#include <memory>
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>

namespace delay_transport {

class RawLogger {
public:
    explicit RawLogger(std::string const& name) :
        logger_(spdlog::basic_logger_mt(name, "logs/" + std::string(name), true)) {
    }

    [[nodiscard]] auto get() const -> spdlog::logger * {
        return logger_.get();
    }

private:
    std::shared_ptr<spdlog::logger> logger_;
};

}

#endif //DELAY_TRANSPORT_RAWLOGGER_H
