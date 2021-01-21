Agent/DelayTransport instproc set_callback {tcp_pair} {
    $self instvar ctrl
    $self set ctrl $tcp_pair
}

Agent/DelayTransport instproc done_data {} {
    global ns sink
    $self instvar ctrl
    if { [info exists ctrl] } {
        $ctrl fin_notify
    }
}
