//***************************************************************************
//* Copyright (c) 2015 Saint Petersburg State University
//* Copyright (c) 2011-2014 Saint Petersburg Academic University
//* All Rights Reserved
//* See file LICENSE for details.
//***************************************************************************

#ifndef SMOOTHING_DISTANCE_ESTIMATION_HPP_
#define SMOOTHING_DISTANCE_ESTIMATION_HPP_

#include "paired_info.hpp"
#include "data_divider.hpp"
#include "peak_finder.hpp"
#include "weighted_distance_estimation.hpp"

namespace omnigraph {

namespace de {

template<class Graph>
class SmoothingDistanceEstimator : public WeightedDistanceEstimator<Graph> {
    //FIXME configure
    static const size_t OVERLAP_TOLERANCE = 1000;
protected:
    typedef WeightedDistanceEstimator<Graph> base;
    typedef typename base::InPairedIndex InPairedIndex;
    typedef typename base::OutPairedIndex OutPairedIndex;
    typedef typename base::InHistogram InHistogram;
    typedef typename base::OutHistogram OutHistogram;
    typedef typename InPairedIndex::Histogram TempHistogram;

public:
    SmoothingDistanceEstimator(const Graph &graph,
                               const InPairedIndex &histogram,
                               const GraphDistanceFinder<Graph> &dist_finder,
                               std::function<double(int)> weight_f,
                               size_t linkage_distance, size_t max_distance, size_t threshold,
                               double range_coeff, double delta_coeff,
                               size_t cutoff,
                               size_t min_peak_points,
                               double inv_density,
                               double percentage,
                               double derivative_threshold,
                               bool only_scaffolding = false) :
            base(graph, histogram, dist_finder, weight_f, linkage_distance, max_distance),
            threshold_(threshold),
            range_coeff_(range_coeff),
            delta_coeff_(delta_coeff),
            cutoff_((int) cutoff),
            min_peak_points_(min_peak_points),
            inv_density_(inv_density),
            percentage_(percentage),
            deriv_thr(derivative_threshold),
            only_scaffolding_(only_scaffolding),
            gap_distances(0) { }

    virtual ~SmoothingDistanceEstimator() { }

protected:
    typedef typename Graph::EdgeId EdgeId;
    typedef pair<EdgeId, EdgeId> EdgePair;
    typedef vector<pair<int, double> > EstimHist;
    typedef vector<PairInfo<EdgeId> > PairInfos;
    typedef vector<size_t> GraphLengths;

    EstimHist EstimateEdgePairDistances(EdgePair /*ep*/,
                                        const InHistogram & /*raw_data*/,
                                        const vector<size_t> & /*forward*/) const override {
        VERIFY_MSG(false, "Sorry, the SMOOOOTHING estimator is not available anymore." <<
                          "SPAdes is going to terminate");

        return EstimHist();
    }

private:
    typedef pair<size_t, size_t> Interval;

    size_t threshold_;
    double range_coeff_;
    double delta_coeff_;
    int cutoff_;
    size_t min_peak_points_;
    double inv_density_;
    double percentage_;
    double deriv_thr;
    bool only_scaffolding_;
    mutable size_t gap_distances;

    EstimHist FindEdgePairDistances(EdgePair ep,
                                    const TempHistogram &raw_hist) const {
        size_t first_len = this->graph().length(ep.first);
        size_t second_len = this->graph().length(ep.second);
        TRACE("Lengths are " << first_len << " " << second_len);
        TempHistogram data;
        for (auto I = raw_hist.begin(), E = raw_hist.end(); I != E; ++I) {
            Point p = *I;
            if (math::ge(2 * (long) rounded_d(p) + (long) second_len, (long) first_len)) if (
                    (long) rounded_d(p) + (long) OVERLAP_TOLERANCE >= (long) first_len)
                data.insert(p);
        }
        EstimHist result;
        double picture_weight = 0.;
        for (auto I = data.begin(), E = data.end(); I != E; ++I)
            picture_weight += I->weight;
        if (math::ls(picture_weight, 3.))
            return result;

        DataDivider<EdgeId> data_divider(threshold_,
                                         vector<Point>(data.begin(), data.end()));

        PairInfos infos;
        infos.reserve(data.size());
        const vector<Interval> &clusters =
                data_divider.DivideAndSmoothData(ep, infos, this->weight_f_);
        DEBUG("Seeking for distances");
        TRACE("size " << infos.size());

        for (size_t i = 0; i < clusters.size(); ++i) {
            size_t begin = clusters[i].first;
            size_t end = clusters[i].second;
            TRACE("begin " << begin << " at " << rounded_d(infos[begin])
                  << ", " << " end " << end << " at " << rounded_d(infos[end - 1]));
            size_t data_length = rounded_d(infos[end - 1]) - rounded_d(infos[begin]) + 1;
            TRACE("data length " << data_length);
            if (end - begin > min_peak_points_) {
                size_t range = (size_t) math::round((double) data_length * range_coeff_);
                size_t delta = (size_t) math::round((double) data_length * delta_coeff_);
                PeakFinder<EdgeId> peakfinder(infos, begin, end, range, delta, percentage_, deriv_thr);
                DEBUG("Processing window : " << rounded_d(infos[begin])
                      << " " << rounded_d(infos[end - 1]));
                peakfinder.FFTSmoothing(cutoff_);
                TRACE("Listing peaks");
                const EstimHist &peaks = peakfinder.ListPeaks();
                //for (auto iter = peaks.begin(); iter != peaks.end(); ++iter) {
                //TRACE("PEAKS " << iter->first << " " << iter->second);
                //}
                if (peaks.size() == 0)
                    continue;
                size_t index_of_max_weight = 0;
                for (size_t i = 0; i < peaks.size(); ++i)
                    if (math::ls(peaks[index_of_max_weight].second, peaks[i].second))
                        index_of_max_weight = i;
                result.push_back(peaks[index_of_max_weight]);
            }
        }

        if (result.size() == 0)
            return result;
        size_t index_of_max_weight = 0;
        for (size_t i = 0; i < result.size(); ++i)
            if (math::ls(result[index_of_max_weight].second, result[i].second))
                index_of_max_weight = i;

        EstimHist new_result;
        for (size_t i = 0; i < result.size(); ++i)
            if (result[i].second > .5 * result[index_of_max_weight].second)
                new_result.push_back(result[i]);
        return new_result;
    }

    void ProcessEdge(EdgeId e1,
                     const InPairedIndex &pi,
                     PairedInfoBuffer<Graph> &result) const override {
        typename base::LengthMap second_edges;
        auto inner_map = pi.GetHalf(e1);
        for (auto I : inner_map)
            second_edges[I.first];

        this->FillGraphDistancesLengths(e1, second_edges);

        for (const auto &entry: second_edges) {
            EdgeId e2 = entry.first;
            EdgePair ep(e1, e2);

            VERIFY(ep <= pi.ConjugatePair(ep));

            TRACE("Processing edge pair " << this->graph().int_id(e1)
                  << " " << this->graph().int_id(e2));
            const GraphLengths &forward = entry.second;

            auto hist = pi.Get(e1, e2).Unwrap();
            EstimHist estimated;
            //DEBUG("Extending paired information");
            //DEBUG("Extend left");
            //this->base::ExtendInfoLeft(e1, e2, hist, 1000);
            DEBUG("Extend right");
            this->ExtendInfoRight(e1, e2, hist, 1000);
            if (forward.size() == 0) {
                estimated = FindEdgePairDistances(ep, hist);
                ++gap_distances;
            } else if (forward.size() > 0 && (!only_scaffolding_)) {
                //TODO: remove THIS
                InPairedIndex temp_index(this->graph());
                temp_index.AddMany(e1, e2, hist);
                auto hist = temp_index.Get(e1, e2);
                estimated = this->base::EstimateEdgePairDistances(ep, hist, forward);
            }
            DEBUG(gap_distances << " distances between gap edge pairs have been found");
            OutHistogram res = this->ClusterResult(ep, estimated);
            this->AddToResult(res, ep, result);
        }
    }

    bool IsTipTip(EdgeId e1, EdgeId e2) const {
        return (this->graph().OutgoingEdgeCount(this->graph().EdgeEnd(e1)) == 0 &&
                this->graph().IncomingEdgeCount(this->graph().EdgeEnd(e1)) == 1 &&
                this->graph().IncomingEdgeCount(this->graph().EdgeStart(e2)) == 0 &&
                this->graph().OutgoingEdgeCount(this->graph().EdgeStart(e2)) == 1);
    }

    void ExtendInfoRight(EdgeId e1, EdgeId e2, TempHistogram &data, size_t max_shift) const {
        ExtendRightDFS(e1, e2, data, 0, max_shift);
    }

    void MergeInto(const InHistogram &what, TempHistogram &where, int shift) const {
        // assuming they are sorted already
        if (what.size() == 0)
            return;

        if (where.size() == 0) {
            for (auto to_be_added : what) {
                to_be_added.d += shift;
                where.insert(to_be_added);
            }

            return;
        }

        // Check, whether two histograms intersect. If not, we can just merge them
        // straightforwardly.
        if (math::ls(where.rbegin()->d, what.min().d + shift) ||
            math::gr(where.begin()->d, what.max().d + shift)) {
            for (auto to_be_added : what) {
                to_be_added.d += shift;
                where.insert(to_be_added);
            }
        } else {
            for (auto to_be_added : what) {
                to_be_added.d += shift;
                auto low_bound = std::lower_bound(where.begin(), where.end(), to_be_added);
                if (to_be_added == *low_bound) {
                    to_be_added.weight += low_bound->weight;
                    where.erase(to_be_added);
                    where.insert(to_be_added);
                } else
                    where.insert(low_bound, to_be_added);
            }
        }
    }

    void ExtendRightDFS(const EdgeId &first, EdgeId current, TempHistogram &data, int shift,
                        size_t max_shift) const {
        auto end = this->graph().EdgeEnd(current);
        if (current == first)
            return;
        if (this->graph().IncomingEdgeCount(end) > 1)
            return;

        for (EdgeId next : this->graph().OutgoingEdges(end)) {
            auto hist = this->index().Get(first, next);
            if (-shift < (int) max_shift)
                ExtendRightDFS(first, next, data, shift - (int) this->graph().length(current), max_shift);

            //auto filtered_infos = FilterPositive(hist, this->graph().length(first), this->graph().length(next));
            //if (filtered_infos.size() > 0)
            //  MergeInto(filtered_infos, data, shift - (int) this->graph().length(current));
            MergeInto(hist, data, shift - (int) this->graph().length(current));
        }
    }

    const string Name() const override {
        static const string my_name = "SMOOTHING";
        return my_name;
    }

    DECL_LOGGER("SmoothingDistanceEstimator")
};

}

}

#endif /* SMOOTHING_DISTANCE_ESTIMATION_HPP_ */
