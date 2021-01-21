add_library(libdiffusion OBJECT
        diff_header.h
        diff_prob.h
        diff_prob.cc
        diff_rate.h
        diff_rate.cc
        diff_sink.h
        diff_sink.cc
        diffusion.h
        diffusion.cc
        flooding.h
        flooding.cc
        hash_table.h
        hash_table.cc
        iflist.h
        iflist.cc
        omni_mcast.h
        omni_mcast.cc
        routing_table.h
        routing_table.cc
        )
target_link_libraries(libdiffusion PRIVATE libcommon)
target_compile_definitions(libdiffusion INTERFACE WITH_DIFFUSION)
