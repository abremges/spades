############################################################################
# Copyright (c) 2015 Saint Petersburg State University
# Copyright (c) 2011-2014 Saint Petersburg Academic University
# All Rights Reserved
# See file LICENSE for details.
############################################################################

project(path_extend CXX)

add_library(path_extend STATIC pe_config_struct.cpp
                    scaffolder2015/extension_chooser2015.cpp
                    scaffolder2015/scaffold_graph.cpp
                    scaffolder2015/scaffold_graph_constructor.cpp
                    scaffolder2015/scaffold_graph_visualizer.cpp
                    scaffolder2015/connection_condition2015.cpp)

target_link_libraries(path_extend graph_support)

