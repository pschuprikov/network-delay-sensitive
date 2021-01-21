#include <algorithm>
#include "PIASDelayAssigner.h"

double PIASDelayAssigner::assign_delay(int msg_len) {
    if (msg_len <= 100 * 1024) {
        return 4.e-4;
    } else if (msg_len <= 10 * 1024 * 1024) {
        return 6e-2;
    } else {
        return 0.150;
    }
}

PIASDelayAssigner::PIASDelayAssigner(int const *) {

}
