#include "scaff_supplementary.hpp"
#include <algorithm>

using namespace std;
namespace path_extend {


void ScaffoldingUniqueEdgeAnalyzer::SetCoverageBasedCutoff() {
    vector <pair<double, size_t>> coverages;
    map <EdgeId, size_t> long_component;
    size_t total_len = 0, short_len = 0, cur_len = 0;

    for (auto iter = gp_.g.ConstEdgeBegin(); !iter.IsEnd(); ++iter) {
        if (gp_.g.length(*iter) > length_cutoff_) {
            coverages.push_back(make_pair(gp_.g.coverage(*iter), gp_.g.length(*iter)));
            total_len += gp_.g.length(*iter);
            long_component[*iter] = 0;
        } else {
            short_len += gp_.g.length(*iter);
        }
    }
    if (total_len == 0) {
        WARN("not enough edges longer than "<< length_cutoff_);
        return;
    }
    sort(coverages.begin(), coverages.end());
    size_t i = 0;
    while (cur_len < total_len / 2 && i < coverages.size()) {
        cur_len += coverages[i].second;
        i++;
    }
    median_coverage_ = coverages[i].first;
}


void ScaffoldingUniqueEdgeAnalyzer::FillUniqueEdgeStorage(ScaffoldingUniqueEdgeStorage &storage_) {
    storage_.unique_edges_.clear();
    size_t total_len = 0;
    size_t unique_len = 0;
    size_t unique_num = 0;
    storage_.SetMinLength(length_cutoff_);
    for (auto iter = gp_.g.ConstEdgeBegin(); !iter.IsEnd(); ++iter) {
        size_t tlen = gp_.g.length(*iter);
        total_len += tlen;
        if (gp_.g.length(*iter) >= length_cutoff_ && gp_.g.coverage(*iter) > median_coverage_ * (1 - relative_coverage_variation_)
                && gp_.g.coverage(*iter) < median_coverage_ * (1 + relative_coverage_variation_) ) {
            storage_.unique_edges_.insert(*iter);
            unique_len += tlen;
            unique_num ++;
        }
    }
    for (auto iter = storage_.begin(); iter != storage_.end(); ++iter) {
        DEBUG (gp_.g.int_id(*iter) << " " << gp_.g.coverage(*iter) << " " << gp_.g.length(*iter) );
    }
    INFO ("With length cutoff: " << length_cutoff_ <<", median long edge coverage: " << median_coverage_ << ", and maximal unique coverage: " <<
                                                                                                            relative_coverage_variation_);
    INFO("Unique edges quantity: " << unique_num << ", unique edges length " << unique_len <<", total edges length" << total_len);
    if (unique_len * 2 < total_len) {
        WARN("Less than half of genome in unique edges!");
    }

}



}
