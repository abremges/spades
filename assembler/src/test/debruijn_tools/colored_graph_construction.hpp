#pragma once

namespace compare {

template<class Graph, class Mapper>
class CoveredRangesFinder {
	const Graph& g_;
	const Mapper mapper_;

	typedef typename Graph::EdgeId EdgeId;
	typedef restricted::map<EdgeId, vector<Range>> CoveredRanges;

	vector<Range> ProcessRange(Range new_range,
			const vector<Range>& curr_ranges) const {
		vector<Range> answer;
		size_t i = 0;
		while (i < curr_ranges.size()
				&& curr_ranges[i].end_pos < new_range.start_pos) {
			answer.push_back(curr_ranges[i]);
			++i;
		}

		size_t merge_start =
				(i != curr_ranges.size()) ?
						std::min(curr_ranges[i].start_pos,
								new_range.start_pos) :
						new_range.start_pos;

		size_t merge_end = new_range.end_pos;
		while (i < curr_ranges.size()
				&& curr_ranges[i].start_pos <= new_range.end_pos) {
			if (curr_ranges[i].end_pos > merge_end)
				merge_end = curr_ranges[i].end_pos;
			++i;
		}
		answer.push_back(Range(merge_start, merge_end));
		while (i < curr_ranges.size()) {
			answer.push_back(curr_ranges[i]);
			++i;
		}
		return answer;
	}

	void ProcessPath(const MappingPath<EdgeId>& path, CoveredRanges& crs) const {
		for (size_t i = 0; i < path.size(); ++i) {
			auto mapping = path[i];
			EdgeId edge = mapping.first;
			const vector<Range>& curr_ranges = crs[edge];
			Range mapping_range = mapping.second.mapped_range;
			VERIFY(g_.length(edge) >= mapping_range.end_pos);
			crs[edge] = ProcessRange(mapping_range, curr_ranges);
			VERIFY(g_.length(edge) >= crs[edge].back().end_pos);
		}
	}

public:

	CoveredRangesFinder(const Graph& g, const Mapper& mapper) :
			g_(g), mapper_(mapper) {

	}

	void FindCoveredRanges(CoveredRanges& crs, ContigStream& stream) const {
		io::SingleRead read;
		stream.reset();
//		NewExtendedSequenceMapper<gp_t::k_value + 1, Graph> mapper(gp_.g,
//				gp_.index, gp_.kmer_mapper);
		while (!stream.eof()) {
			stream >> read;
			ProcessPath(mapper_.MapSequence(read.sequence()), crs);
		}
	}

};

template<class Graph, class Mapper>
class ColoredGraphConstructor {
	typedef typename Graph::EdgeId EdgeId;
	typedef typename Graph::VertexId VertexId;
	typedef restricted::map<EdgeId, vector<Range>> CoveredRanges;
	typedef restricted::map<EdgeId, vector<size_t>> BreakPoints;

	Graph& g_;
	ColorHandler<Graph>& coloring_;
	const Mapper mapper_;

	void AddBreaks(set<size_t>& breaks, const vector<Range>& ranges) const {
		for (auto it = ranges.begin(); it != ranges.end(); ++it) {
			breaks.insert(it->start_pos);
			breaks.insert(it->end_pos);
		}
	}

	vector<size_t> PostProcessBreaks(const set<size_t>& tmp_breaks) const {
		vector<size_t> breaks(tmp_breaks.begin(), tmp_breaks.end());
		//breaks contain 0 and edge_length here!
		VERIFY(breaks.size() >= 2);
		//cleaning breaks from 0 and edge_length
		vector<size_t> final_breaks;
		for (size_t i = 1; i < breaks.size() - 1; ++i) {
			final_breaks.push_back(breaks[i]);
		}
		return final_breaks;
	}

	void FindBreakPoints(BreakPoints& bps, const vector<CoveredRanges>& crss) const {
		for (auto it = g_.SmartEdgeBegin(); !it.IsEnd(); ++it) {
			EdgeId e = *it;
			set<size_t> tmp_breaks;
			for (size_t i = 0; i < crss.size(); ++i) {
				auto crs_it = crss[i].find(e);
				if (crs_it != crss[i].end()) {
					AddBreaks(tmp_breaks, crs_it->second);
				}
			}
			bps[e] = PostProcessBreaks(tmp_breaks);
			VERIFY(bps[e].empty() || bps[e].back() < g_.length(e));
		}
	}

	void SplitEdge(const vector<size_t>& breaks, EdgeId e) {
		vector<size_t> shifts(breaks.size());
		if (!breaks.empty()) {
			shifts[0] = breaks[0];
			for (size_t i = 1; i < breaks.size(); ++i) {
				shifts[i] = breaks[i] - breaks[i - 1];
			}
		}
		EdgeId curr_e = e;
		for (size_t i = 0; i < breaks.size(); ++i) {
			auto split_result = g_.SplitEdge(curr_e, shifts[i]);
			curr_e = split_result.second;
		}
	}

//	void PrintCRS(const CoveredRanges& crs) {
//		for (auto it = crs.begin(); it != crs.end(); ++it) {
//			DEBUG(
//					"For edge " << gp_.g.str(it->first) << " ranges "
//							<< it->second);
//		}
//	}

	void SplitGraph(const vector<ContigStream*>& streams) {
		INFO("Determining covered ranges");
		CoveredRangesFinder<Graph, Mapper> crs_finder(g_, mapper_);
		vector<CoveredRanges> crss(streams.size());
		for (size_t i = 0; i < streams.size(); ++i) {
			crs_finder.FindCoveredRanges(crss[i], *streams[i]);
			//		DEBUG("Printing covered ranges for stream i");
			//		PrintCRS(crss[i]);
		}
		BreakPoints bps;
		INFO("Determining breakpoints");
		FindBreakPoints(bps, crss);

		INFO("Splitting graph");
		SplitGraph(bps);
	}

	void SplitGraph(/*const */BreakPoints& bps) {
		vector<EdgeId> initial_edges;
		for (auto it = g_.SmartEdgeBegin(); !it.IsEnd(); ++it) {
			initial_edges.push_back(*it);
		}
		for (auto it = SmartSetIterator<Graph, EdgeId>(g_,
				initial_edges.begin(), initial_edges.end()); !it.IsEnd();
				++it) {
			EdgeId e = *it;
			VERIFY(bps.find(e) != bps.end());
			VERIFY(bps[e].empty() || bps[e].back() < g_.length(e));
			//todo temporary fix!!!
			if (e == g_.conjugate(e))
				continue;
			SplitEdge(bps[e], e);
		}
	}

	void PaintGraph(ContigStream& stream, edge_type color) {
		io::SingleRead read;
		stream.reset();
		while (!stream.eof()) {
			stream >> read;
			PaintPath(mapper_.MapSequence(read.sequence()).simple_path(),
					color);
		}
	}

	void PaintGraph(const vector<pair<ContigStream*, edge_type>>& stream_mapping) {
		INFO("Coloring graph");
		for (auto it = stream_mapping.begin(); it != stream_mapping.end();
				++it) {
			PaintGraph(*(it->first), it->second);
		}
	}

	void PaintPath(const Path<EdgeId>& path, edge_type color) {
		for (size_t i = 0; i < path.size(); ++i) {
			coloring_.Paint(path[i], color);
			coloring_.Paint(g_.EdgeStart(path[i]), color);
			coloring_.Paint(g_.EdgeEnd(path[i]), color);
		}
	}

	void CompressGraph(Graph& g, ColorHandler<Graph>& coloring) {
		for (auto it = g.SmartVertexBegin(); !it.IsEnd(); ++it) {
			VertexId v = *it;
			if (g.CanCompressVertex(v)
					&& coloring.Color(g.GetUniqueOutgoingEdge(v))
							== coloring.Color(g.GetUniqueIncomingEdge(v))) {
				g.CompressVertex(v);
			}
		}
	}

public:
	ColoredGraphConstructor(Graph& g, ColorHandler<Graph>& coloring,
			const Mapper& mapper) :
			g_(g), coloring_(coloring), mapper_(mapper) {

	}

	void ConstructGraph(const vector<ContigStream*>& streams) {
		VERIFY(streams.size() == 2);

//		if (detailed_output) {
//			//saving for debug and investigation
//			SaveOldGraph(output_folder + "saves/init_graph");
//		}

		SplitGraph(streams);

//		if (detailed_output) {
//			//saving for debug and investigation
//			SaveOldGraph(output_folder + "saves/split_graph");
//		}

		vector<pair<ContigStream*, edge_type>> stream_mapping;
		stream_mapping.push_back(make_pair(streams[0], edge_type::red));
		stream_mapping.push_back(make_pair(streams[1], edge_type::blue));

		PaintGraph(stream_mapping);

		//situation in example 6 =)
		CompressGraph(g_, coloring_);
	}
};

}