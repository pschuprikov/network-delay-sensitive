source [file join [file dirname [info script]] "tcp-common-opt.tcl"]

set ns [new Simulator]
puts "Date: [clock seconds]"
set sim_start [clock seconds]

set next_arg_idx 0

proc next_arg {} {
    global next_arg_idx argv argc
    set cur_arg_idx $next_arg_idx
    incr next_arg_idx 

    if {$cur_arg_idx >= $argc} {
        puts "Not enough arguments: $argc"
        exit 1
    }

    return [lindex $argv $cur_arg_idx]
}

set sim_end [next_arg]
set link_rate [next_arg]
set mean_link_delay [next_arg]
set host_delay [next_arg]
set queueSize [next_arg]
set connections_per_pair [next_arg]
set flow_arrival_command [next_arg]

#### Multipath
set enableMultiPath [next_arg]
set perflowMP [next_arg]

#### Transport settings options
set sourceAlg [next_arg] ; # Sack or DCTCP-Sack
set initWindow [next_arg]
set ackRatio [next_arg]
set slowstartrestart [next_arg]
set DCTCP_g [next_arg] ; # DCTCP alpha estimation gain
set min_rto [next_arg]
set prob_cap_ [next_arg] ; # Threshold of consecutive timeouts to trigger probe mode

#### Switch side options
set switchAlg [next_arg] ; # DropTail (pFabric), RED (DCTCP) or Priority (PIAS)
set DCTCP_K [next_arg]
set drop_prio_ [next_arg]
set prio_scheme_ [next_arg]
set deque_prio_ [next_arg]
set keep_order_ [next_arg]
set prio_num_ [next_arg]
set ECN_scheme_ [next_arg]
set pias_thresh_0 [next_arg]
set pias_thresh_1 [next_arg]
set pias_thresh_2 [next_arg]
set pias_thresh_3 [next_arg]
set pias_thresh_4 [next_arg]
set pias_thresh_5 [next_arg]
set pias_thresh_6 [next_arg]

#### topology
set topology_spt [next_arg]
set topology_tors [next_arg]
set topology_spines [next_arg]
set topology_x [next_arg]

### result file
set flowlog [open [next_arg] w]

set delay_assigner_filename [next_arg]
set use_fifo_processing_order [next_arg]
set use_deadline [next_arg]
set packet_properties_assigner_filename [next_arg]
set expiration_time_controller [next_arg]
set enable_delay [next_arg]
set enable_dupack [next_arg]
set enable_early_expiration [next_arg]

if {$next_arg_idx < $argc} {
    puts "[expr $argc - $next_arg_idx] unconsumed arguments"
    exit 1
}

#### Packet size is in bytes.
set pktSize 1460
#### trace frequency
set queueSamplingInterval 0.0001
#set queueSamplingInterval 1

puts "Simulation input:"
puts "Dynamic Flow - Pareto"
puts "topology: spines server per rack = $topology_spt, x = $topology_x"
puts "sim_end $sim_end"
puts "link_rate $link_rate Gbps"
puts "link_delay $mean_link_delay sec"
puts "host_delay $host_delay sec"
puts "queue size $queueSize pkts"
puts "connections_per_pair $connections_per_pair"
puts "enableMultiPath=$enableMultiPath, perflowMP=$perflowMP"
puts "source algorithm: $sourceAlg"
puts "TCP initial window: $initWindow"
puts "ackRatio $ackRatio"
puts "DCTCP_g $DCTCP_g"
puts "slow-start Restart $slowstartrestart"
puts "switch algorithm $switchAlg"
puts "DCTCP_K_ $DCTCP_K"
puts "pktSize(payload) $pktSize Bytes"
puts "pktSize(include header) [expr $pktSize + 40] Bytes"

puts " "

################# Transport Options ####################
Agent/TCP set ecn_ 1
Agent/TCP set old_ecn_ 1
Agent/TCP set packetSize_ $pktSize
Agent/TCP/FullTcp set segsize_ $pktSize
Agent/TCP/FullTcp set spa_thresh_ 0
Agent/TCP set slow_start_restart_ $slowstartrestart
Agent/TCP set windowOption_ 0
Agent/TCP set minrto_ $min_rto
Agent/TCP set tcpTick_ 0.000001
Agent/TCP set maxrto_ 64
Agent/TCP set lldct_w_min_ 0.125
Agent/TCP set lldct_w_max_ 2.5
Agent/TCP set lldct_size_min_ 204800
Agent/TCP set lldct_size_max_ 1048576

Agent/TCP/FullTcp set link_rate_ $link_rate

Agent/TCP/FullTcp set nodelay_ true; # disable Nagle
Agent/TCP/FullTcp set segsperack_ $ackRatio;
Agent/TCP/FullTcp set interval_ 0.000006

if {$ackRatio > 2} {
    Agent/TCP/FullTcp set spa_thresh_ [expr ($ackRatio - 1) * $pktSize]
}

if {[string compare $sourceAlg "DCTCP-Sack"] == 0} {
    set myAgent "Agent/TCP/FullTcp/Sack/D"
    Agent/TCP set ecnhat_ true
    Agent/TCPSink set ecnhat_ true
    Agent/TCP set ecnhat_g_ $DCTCP_g
    Agent/TCP set lldct_ false
} elseif {[string compare $sourceAlg "LLDCT-Sack"] == 0} {
    set myAgent "Agent/TCP/FullTcp/Sack/D"
    Agent/TCP set ecnhat_ true
    Agent/TCPSink set ecnhat_ true
    Agent/TCP set ecnhat_g_ $DCTCP_g;
    Agent/TCP set lldct_ true
} elseif {[string compare $sourceAlg "DDTCP"] == 0} {
    set myAgent "Agent/TCP/FullTcp/Sack/DDTCP/D";
    Agent/TCP set ecnhat_ true
    Agent/TCPSink set ecnhat_ true
    Agent/TCP set ecnhat_g_ $DCTCP_g
    Agent/TCP set lldct_ false
} elseif {[string compare $sourceAlg "MinTCP-Sack"] == 0} {
    set myAgent "Agent/TCP/FullTcp/Sack/MinTCP/D"
    Agent/TCP set ecnhat_ true
    Agent/TCPSink set ecnhat_ true
    Agent/TCP set ecnhat_g_ $DCTCP_g
    Agent/TCP set lldct_ false
}

$myAgent set enable_early_expiration_ $enable_early_expiration

#Shuang
Agent/TCP/FullTcp set prio_scheme_ $prio_scheme_;
if {$enable_dupack} {
    Agent/TCP/FullTcp set dynamic_dupack_ 0;
} else {
    Agent/TCP/FullTcp set dynamic_dupack_ 1000000;
}
Agent/TCP set window_ 1000000
Agent/TCP set windowInit_ $initWindow
Agent/TCP set rtxcur_init_ $min_rto;
Agent/TCP/FullTcp/Sack set clear_on_timeout_ false;
Agent/TCP/FullTcp/Sack set sack_rtx_threshmode_ 2;
Agent/TCP/FullTcp set prob_cap_ $prob_cap_;

Agent/TCP/FullTcp set enable_pias_ false
Agent/TCP/FullTcp set pias_prio_num_ 0
Agent/TCP/FullTcp set pias_debug_ false
Agent/TCP/FullTcp set pias_thresh_0 0
Agent/TCP/FullTcp set pias_thresh_1 0
Agent/TCP/FullTcp set pias_thresh_2 0
Agent/TCP/FullTcp set pias_thresh_3 0
Agent/TCP/FullTcp set pias_thresh_4 0
Agent/TCP/FullTcp set pias_thresh_5 0
Agent/TCP/FullTcp set pias_thresh_6 0

#Whether we enable PIAS
if {[string compare $switchAlg "Priority"] == 0 } {
    Agent/TCP/FullTcp set enable_pias_ true
    Agent/TCP/FullTcp set pias_prio_num_ $prio_num_
    Agent/TCP/FullTcp set pias_debug_ false
    Agent/TCP/FullTcp set pias_thresh_0 $pias_thresh_0
    Agent/TCP/FullTcp set pias_thresh_1 $pias_thresh_1
    Agent/TCP/FullTcp set pias_thresh_2 $pias_thresh_2
    Agent/TCP/FullTcp set pias_thresh_3 $pias_thresh_3
    Agent/TCP/FullTcp set pias_thresh_4 $pias_thresh_4
    Agent/TCP/FullTcp set pias_thresh_5 $pias_thresh_5
    Agent/TCP/FullTcp set pias_thresh_6 $pias_thresh_6
}

if {$queueSize > $initWindow } {
    Agent/TCP set maxcwnd_ [expr $queueSize - 1];
} else {
    Agent/TCP set maxcwnd_ $initWindow
}

################# Switch Options ######################
Queue set limit_ $queueSize

Queue/DropTail set queue_in_bytes_ true
Queue/DropTail set mean_pktsize_ [expr $pktSize+40]
Queue/DropTail set drop_prio_ $drop_prio_
Queue/DropTail set deque_prio_ $deque_prio_
Queue/DropTail set keep_order_ $keep_order_
Queue/DropTail/D set delay_estimation_mode_ 2
Queue/DropTail/D set expiration_time_controller_enabled_ $expiration_time_controller
Queue/DropTail set thresh_ $DCTCP_K
Queue/DropTail set ecn_enable_ [expr $ECN_scheme_ != 0]


Queue/RED set bytes_ false
Queue/RED set queue_in_bytes_ true
Queue/RED set mean_pktsize_ [expr $pktSize+40]
Queue/RED set setbit_ true
Queue/RED set gentle_ false
Queue/RED set q_weight_ 1.0
Queue/RED set mark_p_ 1.0
Queue/RED set thresh_ $DCTCP_K
Queue/RED set maxthresh_ $DCTCP_K
Queue/RED set drop_prio_ $drop_prio_
Queue/RED set deque_prio_ $deque_prio_

Queue/Priority set queue_num_ $prio_num_
Queue/Priority set thresh_ $DCTCP_K
Queue/Priority set mean_pktsize_ [expr $pktSize+40]
Queue/Priority set marking_scheme_ $ECN_scheme_

Queue/LA set thresh_ $DCTCP_K
Queue/LA set ecn_enable_ [expr $ECN_scheme_ != 0]
Queue/LA set drop_strategy_ 1

Queue/PriorityEDF set use_fifo_processing_order_ $use_fifo_processing_order

############## Multipathing ###########################
if {$enableMultiPath == 1} {
    $ns rtproto DV
    Agent/rtProto/DV set advertInterval	[expr 2*$sim_end]
    Node set multiPath_ 1
    if {$perflowMP != 0} {
        Classifier/MultiPath set perflow_ 1
        Agent/TCP/FullTcp set dynamic_dupack_ 0; # enable duplicate ACK
    }
}

############# Topoplgy #########################
set S [expr $topology_spt * $topology_tors] ; #number of servers
set UCap [expr $link_rate * $topology_spt / $topology_spines / $topology_x] ; #uplink rate

puts "UCap: $UCap"

for {set i 0} {$i < $S} {incr i} {
    set s($i) [$ns node]
}

for {set i 0} {$i < $topology_tors} {incr i} {
    set n($i) [$ns node]
}

for {set i 0} {$i < $topology_spines} {incr i} {
    set a($i) [$ns node]
}

############ Edge links ##############
for {set i 0} {$i < $S} {incr i} {
    set j [expr $i/$topology_spt]
    $ns duplex-link $s($i) $n($j) [set link_rate]Gb [expr $host_delay + $mean_link_delay] $switchAlg
}

############ Core links ##############
for {set i 0} {$i < $topology_tors} {incr i} {
    for {set j 0} {$j < $topology_spines} {incr j} {
        $ns duplex-link $n($i) $a($j) [set UCap]Gb $mean_link_delay $switchAlg
    }
}

#############  Agents ################
puts "Setting up connections ..."; flush stdout

set flow_gen 0
set flow_fin 0

set init_fid 0
for {set j 0} {$j < $S } {incr j} {
    for {set i 0} {$i < $S } {incr i} {
        if {$i != $j} {
                set agtagr($i,$j) [new Agent_Aggr_pair]
                $agtagr($i,$j) setup $s($i) $s($j) "$i $j" $connections_per_pair $init_fid "TCP_pair"
                $agtagr($i,$j) attach-logfile $flowlog

                puts  "($i,$j) :  [expr 17*$i+1244*$j] [expr 33*$i+4369*$j]"

                $agtagr($i,$j) {*}$flow_arrival_command [expr 17*$i+1244*$j] [expr 33*$i+4369*$j]

                $ns at 0.1 "$agtagr($i,$j) warmup 0.5 5"
                $ns at 1 "$agtagr($i,$j) init_schedule"

                set init_fid [expr $init_fid + $connections_per_pair];
            }
        }
}

puts "Initial agent creation done";flush stdout
puts "Simulation started!"

$ns run
