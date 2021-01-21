add_library(libnix OBJECT
        classifier-nix.h
        classifier-nix.cc
        hdr_nv.h
        hdr_nv.cc
        nixnode.h
        nixnode.cc
        nixroute.cc
        nixvec.h
        nixvec.cc
        )

target_link_libraries(libnix PUBLIC libcommon)
target_compile_definitions(libnix INTERFACE WITH_NIX)
