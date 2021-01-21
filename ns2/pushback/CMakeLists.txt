add_library(libpushback OBJECT
            agg-spec.cc
            agg-spec.h
            ident-tree.cc
            ident-tree.h
            logging-data-struct.cc
            logging-data-struct.h
            pushback.cc
            pushback.h
            pushback-constants.h
            pushback-message.h
            pushback-queue.cc
            pushback-queue.h
            rate-estimator.cc
            rate-estimator.h
            rate-limit.cc
            rate-limit.h
            rate-limit-strategy.cc
            rate-limit-strategy.h
        )
target_include_directories(libpushback PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        $<TARGET_PROPERTY:libcommon,INTERFACE_INCLUDE_DIRECTORIES>
        )
