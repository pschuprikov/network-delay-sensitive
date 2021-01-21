#ifndef DELAY_TRANSPORT_ERROR_H
#define DELAY_TRANSPORT_ERROR_H

#include <exception>
#include <string>

namespace delay_transport {

class Error : std::exception {
public:
    explicit Error(std::string const &reason);

    char const * what() const noexcept override;

private:
    std::string reason;
};


}

#endif //DELAY_TRANSPORT_ERROR_H
