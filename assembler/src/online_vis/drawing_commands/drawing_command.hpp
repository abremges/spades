#pragma once

#include "../environment.hpp"
#include "../command.hpp"
#include "../command_type.hpp"
#include "../errors.hpp"
#include "../argument_list.hpp"

namespace online_visualization {

    class DrawingCommand : public LocalCommand {
        protected:
            void DrawPicture(Environment& curr_env, VertexId vertex, string label = "") const {
                make_dir(curr_env.folder_);
                stringstream namestream;
                namestream << curr_env.folder_ << "/" << curr_env.picture_counter_ << "_" << curr_env.file_name_base_ << "_" << label << "_" << ".dot";
                string file_name = namestream.str();
                //stringstream linkstream;
                //linkstream  << curr_env.folder_ << "/" << curr_env.file_name_base_ << "_latest.dot";
                VertexNeighborhoodFinder<Graph> splitter(curr_env.graph(), vertex, curr_env.max_vertices_, curr_env.edge_length_bound_);
                //EdgePosGraphLabeler<Graph> labeler(curr_env.graph(), gp_.edge_pos);
                WriteComponents<Graph>(curr_env.graph(), splitter, file_name, *DefaultColorer(curr_env.graph(), curr_env.coloring_), curr_env.tot_lab_);
                //WriteComponents <Graph> (curr_env.graph(), splitter, linkstream.str(), *DefaultColorer(curr_env.graph(), curr_env.coloring_), curr_env.tot_lab_);
                cout << "The picture is written to " << file_name << endl;
                
                curr_env.picture_counter_++;
            }

            void DrawPicturesAlongPath(Environment& curr_env, const MappingPath<EdgeId>& path, string label = "") const {
                make_dir(curr_env.folder_);
                stringstream namestream;
                namestream << curr_env.folder_ << "/" << curr_env.picture_counter_ << "_" << curr_env.file_name_base_ << "_" << label << "_" << ".dot";
                string file_name = namestream.str();
                ReliableSplitterAlongPath<Graph> splitter(curr_env.graph(), curr_env.max_vertices_, curr_env.edge_length_bound_, path);
                WriteComponents<Graph>(curr_env.graph(), splitter, file_name, *DefaultColorer(curr_env.graph(), curr_env.coloring_), curr_env.tot_lab_);
                cout << "The picture is written to " << file_name << endl;
                
                curr_env.picture_counter_++;
            }

              
            //TODO: copy zgrviewer 
            int ShowPicture(Environment& curr_env, VertexId vertex, string label = "") const {
                DrawPicture(curr_env, vertex, label);
                stringstream command_line_string;
                command_line_string << "gnome-open " << curr_env.folder_ << "/" << curr_env.file_name_base_ 
                                    << "_" << label << "_" << curr_env.picture_counter_ 
                                    << "_*_.dot & > /dev/null < /dev/null";
                int result = system(command_line_string.str().c_str());

                return result;
            }

            void DrawVertex(Environment& curr_env, size_t vertex_id, string label = "") const {
                DrawPicture(curr_env, curr_env.int_ids().ReturnVertexId(vertex_id), label);
            }


        public:
            DrawingCommand(CommandType command_type) : LocalCommand(command_type)
            {
            }

            virtual ~DrawingCommand()
            {
            }
    };
}