############################################################################
# Copyright (c) 2015 Saint Petersburg State University
# Copyright (c) 2011-2014 Saint Petersburg Academic University
# All Rights Reserved
# See file LICENSE for details.
############################################################################

project(debruijn_test CXX)

add_executable(debruijn_test
               ${EXT_DIR}/include/teamcity_boost/teamcity_boost.cpp
               ${EXT_DIR}/include/teamcity_boost/teamcity_messages.cpp
               test.cpp)
target_link_libraries(debruijn_test spades_modules cityhash ssw ${COMMON_LIBRARIES})

add_executable(component_generator generate_component.cpp)
target_link_libraries(component_generator spades_modules cityhash ${COMMON_LIBRARIES})

add_executable(reference_fixer reference_fixer.cpp)
target_link_libraries(reference_fixer spades_modules cityhash ${COMMON_LIBRARIES})
