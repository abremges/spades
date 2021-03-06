//***************************************************************************
//* Copyright (c) 2015 Saint Petersburg State University
//* Copyright (c) 2011-2014 Saint Petersburg Academic University
//* All Rights Reserved
//* See file LICENSE for details.
//***************************************************************************

#include "pe_config_struct.hpp"
#include "pipeline/config_common.hpp"

namespace path_extend {

void load(output_broken_scaffolds& obs, boost::property_tree::ptree const& pt, std::string const& key, bool complete) {
  if (complete || pt.find(key) != pt.not_found()) {
    std::string ep = pt.get<std::string>(key);
    obs = pe_config::output_broken_scaffolds_id(ep);
  }
}

void load(scaffolding_mode &sm, boost::property_tree::ptree const& pt, std::string const& key, bool complete) {
    if (complete || pt.find(key) != pt.not_found()) {
        std::string ep = pt.get<std::string>(key);
        sm = pe_config::scaffolding_mode_id(ep);
    }
}

void load(pe_config::ParamSetT::ScaffoldGraphParamsT& sg, boost::property_tree::ptree const& pt, bool /*complete*/) {
    using config_common::load;
    load(sg.construct,          pt, "construct"         );
    load(sg.output,             pt, "output"            );
    load(sg.always_add,         pt, "always_add"        );
    load(sg.never_add,          pt, "never_add"         );
    load(sg.relative_threshold,  pt, "relative_threshold" );
    load(sg.graph_connectivity, pt, "graph_connectivity");
    load(sg.max_path_length,    pt, "max_path_length"   );
}

void load(pe_config::OutputParamsT& o, boost::property_tree::ptree const& pt, bool complete) {
  using config_common::load;

  load(o.write_overlaped_paths,   pt, "write_overlaped_paths" , complete);
  load(o.write_paths,             pt, "write_paths"           , complete);
}

void load(pe_config::VisualizeParamsT& o, boost::property_tree::ptree const& pt, bool complete) {
  using config_common::load;
  load(o.print_overlaped_paths,   pt, "print_overlaped_paths" , complete);
  load(o.print_paths,             pt, "print_paths"           , complete);
}

void load(pe_config::ParamSetT::ExtensionOptionsT& es,
          boost::property_tree::ptree const& pt, bool complete) {
    using config_common::load;
    load(es.use_default_single_threshold, pt, "use_default_single_threshold", complete);
    load(es.priority_coeff, pt, "priority_coeff", complete);
    load(es.weight_threshold, pt, "weight_threshold", complete);
    load(es.single_threshold, pt, "single_threshold", complete);
    load(es.max_repeat_length, pt, "max_repeat_length", complete);
}

void load(pe_config::ParamSetT::LoopRemovalT& lr,
          boost::property_tree::ptree const& pt, bool complete) {
    using config_common::load;
    load(lr.max_loops, pt, "max_loops", complete);
    load(lr.mp_max_loops, pt, "mp_max_loops", complete);
}

void load(pe_config::ParamSetT::CoordinatedCoverageT& coord_cov,
          boost::property_tree::ptree const& pt, bool complete) {
    using config_common::load;
    load(coord_cov.max_edge_length_in_repeat, pt, "max_edge_length_repeat", complete);
    load(coord_cov.delta, pt, "delta", complete);
    load(coord_cov.min_path_len, pt, "min_path_len", complete);
}

void load(pe_config::ParamSetT::ScaffolderOptionsT& so, 
            boost::property_tree::ptree const& pt, bool complete)
{
  using config_common::load;
  load(so.on      , pt, "on"      , complete);
  load(so.cutoff      , pt, "cutoff", complete);
  load(so.rel_cutoff      , pt, "rel_cutoff", complete);
  load(so.sum_threshold      , pt, "sum_threshold", complete);

  load(so.cluster_info      , pt, "cluster_info", complete);
  load(so.cl_threshold      , pt, "cl_threshold", complete);

  load(so.fix_gaps      , pt, "fix_gaps", complete);
  load(so.use_la_gap_joiner      , pt, "use_la_gap_joiner", complete);
  load(so.min_gap_score      , pt, "min_gap_score", complete);
  load(so.max_must_overlap      , pt, "max_must_overlap", complete);
  load(so.max_can_overlap      , pt, "max_can_overlap", complete);
  load(so.short_overlap      , pt, "short_overlap", complete);
  load(so.artificial_gap      , pt, "artificial_gap", complete);
  load(so.use_old_score      , pt, "use_old_score", complete);
  load(so.min_overlap_length, pt, "min_overlap_length", complete);
  load(so.flank_addition_coefficient, pt, "flank_addition_coefficient", complete);
  load(so.flank_multiplication_coefficient, pt, "flank_multiplication_coefficient", complete);
}

void load(pe_config::ParamSetT& p, boost::property_tree::ptree const& pt, bool complete) {
    using config_common::load;
    load(p.sm, pt, "scaffolding_mode", complete);
    load(p.normalize_weight, pt,  "normalize_weight", complete);
    load(p.cut_all_overlaps, pt, "cut_all_overlaps", complete);
    load(p.remove_overlaps, pt, "remove_overlaps", complete);
    load(p.multi_path_extend, pt, "multi_path_extend", complete);
    load(p.split_edge_length, pt, "split_edge_length", complete);
    load(p.extension_options, pt, "extension_options", complete);
    load(p.mate_pair_options, pt, "mate_pair_options", complete);
    load(p.scaffolder_options, pt, "scaffolder", complete);
    load(p.loop_removal, pt, "loop_removal", complete);
    load(p.coordinated_coverage, pt, "coordinated_coverage", complete);
    load(p.use_coordinated_coverage, pt, "use_coordinated_coverage", complete);
    load(p.scaffolding2015, pt, "scaffolding2015", complete);
    load(p.scaffold_graph_params, pt, "scaffold_graph", complete);
}


void load(pe_config::LongReads& p, boost::property_tree::ptree const& pt,
          bool complete) {
    using config_common::load;
    load(p.filtering, pt, "filtering", complete);
    load(p.weight_priority, pt, "weight_priority", complete);
    load(p.unique_edge_priority, pt, "unique_edge_priority", complete);
    load(p.min_significant_overlap, pt, "min_significant_overlap", complete);

}

void load(pe_config::ParamSetT::Scaffolding2015& p, boost::property_tree::ptree const& pt,
          bool) {
    using config_common::load;
    load(p.autodetect, pt, "autodetect");
    load(p.min_unique_length, pt, "min_unique_length");
    load(p.unique_coverage_variation, pt, "unique_coverage_variation");
}

void load(pe_config::AllLongReads& p, boost::property_tree::ptree const& pt,
          bool complete) {
    using config_common::load;
    load(p.pacbio_reads, pt, "pacbio_reads", complete);
    load(p.single_reads, pt, "single_reads", complete);
    load(p.contigs, pt, "contigs", complete);
    load(p.meta_contigs, pt, "meta_untrusted_contigs", complete);
}

void load(pe_config::MainPEParamsT& p, boost::property_tree::ptree const& pt,
          bool complete) {
    using config_common::load;
    load(p.debug_output, pt, "debug_output", complete);
    load(p.output, pt, "output", complete);
    load(p.viz, pt, "visualize", complete);
    load(p.obs, pt, "output_broken_scaffolds", complete);
    load(p.param_set, pt, "params", complete);
    load(p.long_reads, pt, "long_reads", complete);
    if (!p.debug_output) {
        p.output.DisableAll();
        p.viz.DisableAll();
    }
    p.etc_dir = "path_extend";
}

//// main long contigs config load function
//void load(pe_config& pe_cfg, boost::property_tree::ptree const& pt, bool complete) {
//  using config_common::load;
//
//  load(pe_cfg.dataset_name           , pt, "dataset", complete);
//  load(pe_cfg.params                 , pt, "pe_params", complete);
//}

};

