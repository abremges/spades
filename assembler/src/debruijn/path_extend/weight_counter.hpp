/*
 * weight_counter.hpp
 *
 *  Created on: Feb 19, 2012
 *      Author: andrey
 */

#ifndef WEIGHT_COUNTER_HPP_
#define WEIGHT_COUNTER_HPP_

#include "bidirectional_path.hpp"
#include "paired_library.hpp"
#include <algorithm>

namespace path_extend {

struct EdgeWithPairedInfo {
    size_t e_;
    double pi_;

    EdgeWithPairedInfo(size_t e_, double pi): e_(e_), pi_(pi) {

    }
};


struct EdgeWithDistance {
    EdgeId e_;
    int d_;

    EdgeWithDistance(EdgeId e, size_t d): e_(e), d_(d) {
    }
};


class ExtentionAnalyzer {

protected:
    Graph& g_;
    PairedInfoLibrary& lib_;

public:
    ExtentionAnalyzer(Graph& g, PairedInfoLibrary& l): g_(g), lib_(l) {
    }

    void FindCoveredEdges(const BidirectionalPath& path, EdgeId candidate, std::vector<EdgeWithPairedInfo>& edges) {
        edges.clear();
        for (int i = (int) path.Size() - 1; i >= 0; --i) {
            double w = lib_.IdealPairedInfo(path[i], candidate, path.LengthAt(i));
            if (math::gr(w, 0.)) {
                edges.push_back(EdgeWithPairedInfo(i, w));
            }
        }
    }


    void FindForwardEdges(const BidirectionalPath& path, EdgeId candidate, std::vector<EdgeWithDistance>& edges) {
        edges.clear();
        edges.push_back(EdgeWithDistance(candidate, 0));

        size_t i = 0;
        while (i < edges.size()) {
            size_t currentDistance = edges[i].d_ + g_.length(edges[i].e_);
            auto nextEdges = g_.OutgoingEdges(g_.EdgeEnd(edges[i].e_));

            if (edges[i].d_ < (int) lib_.insert_size_) {
                for (auto edge = nextEdges.begin(); edge != nextEdges.end(); ++edge) {
                    edges.push_back(EdgeWithDistance(*edge, currentDistance));
                }
            }
            ++i;
        }
    }
};


class WeightCounter {

protected:
    Graph& g_;
    PairedInfoLibraries& libs_;
    std::vector<ExtentionAnalyzer *> analyzers_;
    double avrageLibWeight_;

    double threshold_;
    bool normalizeWeight_;
    bool normalizeWightByCoverage_;

    bool tryDeepSearch_;

    std::set<int> excludedEdges_;

public:

    WeightCounter(Graph& g, PairedInfoLibraries& libs, double threshold = 0.0): g_(g), libs_(libs),
                threshold_(threshold), normalizeWeight_(true), normalizeWightByCoverage_(true), tryDeepSearch_(false), excludedEdges_()   {

        avrageLibWeight_ = 0.0;
        analyzers_.reserve(libs_.size());
        for (auto iter = libs_.begin(); iter != libs_.end(); ++iter) {
            analyzers_.push_back(new ExtentionAnalyzer(g_, **iter));
            avrageLibWeight_ += (*iter)->coverage_coeff_;
        }
        avrageLibWeight_ /= max(libs_.size(), (size_t) 1);
    }

    virtual ~WeightCounter() {
        for (auto iter = analyzers_.begin(); iter != analyzers_.end(); ++iter) {
            delete *iter;
        }
        analyzers_.clear();
    }

    virtual double CountWeight(BidirectionalPath& path, EdgeId e, int gapLength = 0) = 0;

    virtual void GetDistances(EdgeId e1, EdgeId e2, std::vector<int>& dist, std::vector<double>& w) = 0;

    virtual double CountIdealInfo(EdgeId e1, EdgeId e2, size_t dist) = 0;

    virtual double CountIdealInfo(const BidirectionalPath& p, EdgeId e, size_t gap) = 0;

    virtual bool IsExtensionPossible(BidirectionalPath& path, EdgeId e) {
        return IsExtensionPossible(CountWeight(path, e)) ? true : false;
    }

    virtual bool IsExtensionPossible(double weight) {
        return math::ge(weight, threshold_) ? true : false;
    }


    std::set<int> & GetExcludedEdges() {
        return excludedEdges_;
    }


    double getThreshold() const
    {
        return threshold_;
    }

    bool isNormalizeWeight() const
    {
        return normalizeWeight_;
    }

    bool isNormalizeWightByCoverage() const
    {
        return normalizeWightByCoverage_;
    }

    void setNormalizeWeight(bool normalizeWeight)
    {
        this->normalizeWeight_ = normalizeWeight;
    }

    void setNormalizeWightByCoverage(bool normalizeWightByCoverage)
    {
        this->normalizeWightByCoverage_ = normalizeWightByCoverage;
    }


    void setThreshold(double threshold)
    {
        this->threshold_ = threshold;
    }

    void setDeepSearch(bool value) {
        tryDeepSearch_ = value;
    }

    bool isDeepSearch() {
        return tryDeepSearch_;
    }

    PairedInfoLibraries& getLibs() {
        return libs_;
    }

};


class ReadCountWeightCounter: public WeightCounter {

protected:

    double CountSingleLib(int libIndex, BidirectionalPath& path, EdgeId e, int additionalGapLength = 0.0) {

        PairedInfoLibrary& pairedInfoLibrary = *libs_[libIndex];
        double weight = 0.0;

        std::vector<EdgeWithPairedInfo> coveredEdges;
        analyzers_[libIndex]->FindCoveredEdges(path, e, coveredEdges);

        for (auto iter = coveredEdges.begin(); iter != coveredEdges.end(); ++iter) {
            if (excludedEdges_.count(iter->e_) > 0) {
                continue;
            }

            double w = libs_[libIndex]->CountPairedInfo(path[iter->e_], e, path.LengthAt(iter->e_) + additionalGapLength);

            if (normalizeWeight_) {
                w /= iter->pi_;
            }

            weight += w;

        }

        return normalizeWightByCoverage_ ? pairedInfoLibrary.normalizeByCoverage(weight) * avrageLibWeight_ : weight;
    }

public:

    ReadCountWeightCounter(Graph& g_, PairedInfoLibraries& libs_, double threshold_ = 0.0): WeightCounter(g_, libs_, threshold_) {
    }

    virtual void GetDistances(EdgeId e1, EdgeId e2, std::vector<int>& dist, std::vector<double>& w){
		 for (size_t i = 0; i < libs_.size(); ++i) {
			 libs_[i]->CountDistances(e1, e2, dist, w);
		 }
	}

    virtual double CountIdealInfo(EdgeId e1, EdgeId e2, size_t dist){
    	 double res = 0.0;
		 for (size_t i = 0; i < libs_.size(); ++i) {
			 res += libs_[i]->IdealPairedInfo(e1, e2, dist);
		 }
         return res;
	}

    virtual double CountIdealInfo(const BidirectionalPath& p, EdgeId e, size_t gap) {
           double w = 0.0;
           for (int i = (int) p.Size() - 1; i >= 0; --i) {
               w += CountIdealInfo(p[i], e, gap + p.LengthAt(i));
           }
           return w;
       }

    virtual double CountWeight(BidirectionalPath& path, EdgeId e, int gapLength = 0) {
        double weight = 0.0;
        std::vector<EdgeWithDistance> edges;

        for (size_t i = 0; i < libs_.size(); ++i) {
            if (tryDeepSearch_) {
                analyzers_[i]->FindForwardEdges(path, e, edges);

                for (auto edge = edges.begin(); edge != edges.end(); ++edge) {
                    weight += CountSingleLib(i, path, edge->e_, edge->d_ + gapLength);
                }
            }
            else {
                weight += CountSingleLib(i, path, e, gapLength);
            }
        }

        return weight;
    }

};


class PathCoverWeightCounter: public WeightCounter {

protected:

    double singleThreshold;

    double CountSingleLib(int libIndex, BidirectionalPath& path, EdgeId e, int additionalGapLength = 0.0) {
        PairedInfoLibrary& pairedInfoLibrary = *libs_[libIndex];
        double weight = 0.0;
        double idealWeight = 0.0;

        std::vector<EdgeWithPairedInfo> coveredEdges;
        analyzers_[libIndex]->FindCoveredEdges(path, e, coveredEdges);

        for (auto iter = coveredEdges.begin(); iter != coveredEdges.end(); ++iter) {
            if (excludedEdges_.count(iter->e_) > 0) {
                continue;
            }

            double singleWeight = libs_[libIndex]->CountPairedInfo(path[iter->e_], e, path.LengthAt(iter->e_) + additionalGapLength);

            if (normalizeWeight_) {
                singleWeight /= iter->pi_;
            }
            if (normalizeWightByCoverage_) {
                singleWeight = pairedInfoLibrary.normalizeByCoverage(singleWeight) * avrageLibWeight_;
            }

            if (math::ge(singleWeight, singleThreshold)) {
                weight += iter->pi_;
            }
            idealWeight += iter->pi_;
        }

        return math::gr(idealWeight, 0.0) ? weight / idealWeight : 0.0;
    }

public:

    PathCoverWeightCounter(Graph& g_, PairedInfoLibraries& libs_, double threshold_ = 0.0, double singleThreshold_ = 0.0): WeightCounter(g_, libs_, threshold_), singleThreshold(singleThreshold_) {

    }

    virtual void GetDistances(EdgeId e1, EdgeId e2, std::vector<int>& dist, std::vector<double>& w){
    	 for (size_t i = 0; i < libs_.size(); ++i) {
    	     libs_[i]->CountDistances(e1, e2, dist, w);
    	 }
    }

    virtual double CountIdealInfo(EdgeId e1, EdgeId e2, size_t dist){
		 double res = 0.0;
		 for (size_t i = 0; i < libs_.size(); ++i) {
			 res += libs_[i]->IdealPairedInfo(e1, e2, dist);
		 }
		 return res;
	}

    virtual double CountIdealInfo(const BidirectionalPath& p, EdgeId e, size_t gap) {
           double w = 0.0;
           for (int i = (int) p.Size() - 1; i >= 0; --i) {
               w += g_.length(p[i]) ? CountIdealInfo(p[i], e, gap + p.LengthAt(i)) > 0 : 0;
           }
           return w;
       }



    virtual double CountWeight(BidirectionalPath& path, EdgeId e, int gapLength = 0) {
        double weight = 0.0;
        //excludedEdges_.clear();

        for (size_t i = 0; i < libs_.size(); ++i) {
            if (tryDeepSearch_) {
                //TODO
                VERIFY(false);
            }
            else {
                weight += CountSingleLib(i, path, e, gapLength);
            }
        }

        return weight / max(libs_.size(), (size_t) 1);
    }

};


}

#endif /* WEIGHT_COUNTER_HPP_ */