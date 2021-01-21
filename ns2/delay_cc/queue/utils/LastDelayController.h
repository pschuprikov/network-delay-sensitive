#ifndef NS2_LASTDELAYCONTROLLER_H
#define NS2_LASTDELAYCONTROLLER_H


class LastDelayController {
public:
    LastDelayController();

    void do_control(double slack, int mean_pkt_size, int qlim_in_bytes,
                    double link_bandwidth);
    int get_queue_limit_in_bytes(int orig_qlim_in_bytes) const;

private:
    void check_expiration();

private:
    int last_delay_bytes_;
    int min_above_last_delay_bytes_;
    double last_delay_set_time_;
};


#endif //NS2_LASTDELAYCONTROLLER_H
