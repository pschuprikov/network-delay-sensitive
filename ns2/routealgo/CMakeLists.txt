add_library(libroutealgo OBJECT
        bfs.h
        bfs.cc
        dijkstra.h
        dijkstra.cc
        rbitmap.h
        rbitmap.cc
        rlookup.h
        rlookup.cc
        rnode.h
        rnode.cc
        routealgo.h
        routealgo.cc
        tnode.h
        tnode.cc
    )
if(WITH_NIX)
    target_link_libraries(libroutealgo PUBLIC libnix)
endif()
