#pragma once

#include "splitters.hpp"
namespace omnigraph {

template<class Graph>
class AbstractLoopKiller {
public:
	typedef typename Graph::VertexId VertexId;
	typedef typename Graph::EdgeId EdgeId;
private:
	Graph &graph_;

	VertexId FindStart(set<VertexId> component_set) {
		VertexId result;
		for(auto it = component_set.begin(); it != component_set.end(); ++it) {
			vector<EdgeId> incoming = graph_.IncomingEdges(*it);
			for(auto eit = incoming.begin(); eit != incoming.end(); ++eit) {
				if(component_set.count(graph_.EdgeStart(*eit)) == 0) {
					if(result != VertexId()) {
						return VertexId();
					}
					result = *it;
				}
			}
		}
		return result;
	}

	VertexId FindFinish(set<VertexId> component_set) {
		VertexId result;
		for(auto it = component_set.begin(); it != component_set.end(); ++it) {
			vector<EdgeId> outgoing = graph_.OutgoingEdges(*it);
			for(auto eit = outgoing.begin(); eit != outgoing.end(); ++eit) {
				if(component_set.count(graph_.EdgeEnd(*eit)) == 0) {
					if(result != VertexId()) {
						return VertexId();
					}
					result = *it;
				}
			}
		}
		return result;
	}

protected:
	const size_t splitting_edge_length_;
	const size_t max_component_size_;

	Graph &g() {
		return graph_;
	}

public:

	AbstractLoopKiller(Graph &graph, size_t splitting_edge_length,
			size_t max_component_size) :
			graph_(graph), splitting_edge_length_(splitting_edge_length), max_component_size_(
					max_component_size) {
	}

	virtual ~AbstractLoopKiller() {
	}



	void KillAllLoops() {
		LongEdgesExclusiveSplitter<Graph> splitter(graph_, splitting_edge_length_);
		while(!splitter.Finished()) {
			vector<VertexId> component = splitter.NextComponent();
			if(component.size() > max_component_size_)
				continue;
			set<VertexId> component_set(component.begin(), component.end());
			VertexId start = FindStart(component_set);
			VertexId finish = FindFinish(component_set);
			if(start == VertexId() || finish == VertexId()) {
				continue;
			}
			KillLoop(start, finish, component_set);
		}
		Compressor<Graph> compressor(graph_);
		compressor.CompressAllVertices();
	}

	virtual void KillLoop(VertexId start, VertexId finish, const set<VertexId> &component) = 0;
};

template<class Graph>
class SimpleLoopKiller : public AbstractLoopKiller<Graph> {

public:
	typedef typename Graph::VertexId VertexId;
	typedef typename Graph::EdgeId EdgeId;

private:
	vector<EdgeId> FindPath(VertexId start, VertexId finish) {
		set<VertexId> was;
		vector<EdgeId> rr = FindPath(start, finish, was);
		return vector<EdgeId>(rr.rbegin(), rr.rend());
	}

	vector<EdgeId> FindPath(VertexId start, VertexId finish, set<VertexId> &was) {
		was.insert(start);
		vector<EdgeId> outgoing = this->g().OutgoingEdges(start);
		for(auto it = outgoing.begin(); it != outgoing.end(); ++it) {
			VertexId next = this->g().EdgeEnd(*it);
			if(next == finish) {
				return {*it};
			}
			if(was.count(next) == 0) {
				vector<EdgeId> result = FindPath(next, finish, was);
				if(result.size() > 0) {
					result.push_back(*it);
					return result;
				}
			}
		}
		return {};
	}

	bool CheckNotMuchRemoved(set<EdgeId> edges, set<VertexId> component) {
		size_t sum = 0;
		for(auto it = component.begin(); it != component.end(); ++it) {
			vector<EdgeId> outgoing = this->g().OutgoingEdges(*it);
			for(auto eit = outgoing.begin(); eit != outgoing.end(); ++eit) {
				if(component.count(this->g().EdgeEnd(*eit)) == 1 && edges.count(*eit) == 0 ) {
					sum += this->g().length(*eit);
				}
			}
		}
		return sum <= this->splitting_edge_length_;
	}

	void RemoveExtraEdges(set<EdgeId> edges, set<VertexId> component) {
		vector<VertexId> comp(component.begin(), component.end());
		vector<EdgeId> to_delete;
		for(auto it = comp.begin(); it != comp.end(); ++it) {
			vector<EdgeId> outgoing = this->g().OutgoingEdges(*it);
			for(auto eit = outgoing.begin(); eit != outgoing.end(); ++eit) {
				if(component.count(this->g().EdgeEnd(*eit)) == 1 && edges.count(*eit) == 0 ) {
					to_delete.push_back(*eit);
				}
			}
		}
		SmartSetIterator<Graph, EdgeId> s(this->g(), to_delete.begin(), to_delete.end());
		while(!s.IsEnd()) {
			this->g().DeleteEdge(*s);
			++s;
		}
	}

	void RemoveIsolatedVertices(set<VertexId> component) {
		SmartSetIterator<Graph, VertexId> s(this->g(), component.begin(), component.end());
		while(!s.IsEnd()) {
			if(this->g().IsDeadStart(*s) && this->g().IsDeadEnd(*s)) {
				this->g().DeleteVertex(*s);
			}
			++s;
		}
	}

public:
	SimpleLoopKiller(Graph &graph, size_t splitting_edge_length, size_t max_component_size) :
			AbstractLoopKiller<Graph>(graph, splitting_edge_length, max_component_size) {
	}

	virtual void KillLoop(VertexId start, VertexId finish, const set<VertexId> &component) {
		vector<EdgeId> path = FindPath(start, finish);
		set<EdgeId> edges(path.begin(), path.end());
		if(path.size() > 0) {
			if(!CheckNotMuchRemoved(edges, component)) {
				return;
			}
			RemoveExtraEdges(edges, component);
			RemoveIsolatedVertices(component);
		}
	}

};
}