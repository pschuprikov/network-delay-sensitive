#ifndef NS2_LOGGER_H
#define NS2_LOGGER_H

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>

struct Logger {
    explicit Logger(std::string const& name)
            : logger_(spdlog::basic_logger_mt(name, name + ".log")) {
        logger_->set_pattern("%v");
        logger_->set_level(spdlog::level::off);
    }

    auto get() const -> spdlog::logger * {
        return logger_.get();
    }
private:
    std::shared_ptr<spdlog::logger> logger_;
};

#endif //NS2_LOGGER_H
