#include "common.hpp"

#include "constructHashTable.hpp"
#include "graphConstruction.hpp"
#include "graphSimplification.hpp"
#include "graphio.hpp"
#include "readTracing.hpp"
#include "sequence.hpp"
#include "readsReformatter.hpp"
#include "read_generator.hpp"

using namespace paired_assembler;
LOGGER("p.main");

PairedGraph graph;

void init() {
	initConstants(ini_file);
	initGlobal();
	freopen(error_log.c_str(), "w", stderr);
	INFO("Constants inited...");
	cerr << l << " " << k;
}

void run() {
	char str[100];
	//	forgetQualityPairedData("I:/bioinf/eas20_8/s_6_1.cor.fastq.gz", "I:/bioinf/eas20_8/s_6_2.cor.fastq.gz", "data/paireddebruijn/reads_100_200_corr.txt" );
	//	forgetQualityPairedData("data/paireddebruijn/s_6_1.fastq.gz", "data/paireddebruijn/s_6_2.fastq.gz", "/media/605005E05005BDB2/data/realreads.txt" );
	//	LOG_ASSERT(1 == 0, "Something wrong");

	if (needPairs) {
		cerr << endl << " constructing pairs" << endl;
		if (needRevertedPairs)
			readsToPairs(parsed_reads, parsed_l_k_mers, true);
		else
			readsToPairs(parsed_reads, parsed_k_l_mers, false);
	}
	if (needLmers) {
		cerr << endl << " constructing Lmers" << endl;
		pairsToLmers(parsed_k_l_mers, parsed_l_mers);
	}

	if (needSequences) {
		cerr << endl << " constructing Sequences" << endl;
		pairsToSequences(parsed_k_l_mers, parsed_l_mers, parsed_k_sequence);
	}
	//	map<>sequencesToMap(parsed_k_sequence);

	if (needGraph) {
		cerr << endl << " constructing Graph" << endl;
		constructGraph(graph);
		save(folder+"graph.txt", graph);
		outputLongEdges(graph.longEdges, graph,
				folder+"beforeExpand.dot");
	}

	if (useExpandDefinite) {
		INFO("Expand definite...");
		if (!needGraph) {
			load(folder+"graph.txt", graph);
			graph.removeLowCoveredEdges(graph.longEdges, 3);
			graph.RebuildVertexMap();
			graph.recreateVerticesInfo(graph.VertexCount, graph.longEdges);
		}
		expandDefinite(graph.longEdges, graph, graph.VertexCount, true);
		outputLongEdges(graph.longEdges, graph,
				folder+"afterExpand.dot");
		//		outputLongEdgesThroughGenome(graph, "data/paireddebruijn/afterExpand_g.dot");
		save(folder+"expandedGraph.txt", graph);
	}

	if (useTraceReads) {
		INFO("Trace reads...");
		if (!useExpandDefinite) {
			INFO("Loading graph...");
			load(folder+"expandedGraph.txt", graph);
			INFO("Graph loaded!");
			graph.RebuildVertexMap();
			graph.recreateVerticesInfo(graph.VertexCount, graph.longEdges);
		}
		traceReads(graph.verts, graph.longEdges, graph, graph.VertexCount,
				graph.EdgeId);
		outputLongEdges(graph.longEdges, folder+"ReadsTraced.dot");
		//	outputLongEdgesThroughGenome(graph, "data/paireddebruijn/ReadsTraced_g.dot");
		save(folder+"tracedGraph.txt", graph);
	}

	if (useProcessLower) {
		INFO("Process lowers");

		if (!useTraceReads) {
			INFO("Load");
			load(folder+"tracedGraph.txt", graph);
			INFO("Rebuild");
			graph.RebuildVertexMap();
		}

		graph.recreateVerticesInfo(graph.VertexCount, graph.longEdges);
		while (processLowerSequence(graph.longEdges, graph, graph.VertexCount)) {
			graph.recreateVerticesInfo(graph.VertexCount, graph.longEdges);
			expandDefinite(graph.longEdges, graph, graph.VertexCount);
			INFO("one more");
		}
		outputLongEdges(graph.longEdges, folder+"afterLowers.dot");
		outputLongEdgesThroughGenome(graph,
				folder+"afterLowers_g.dot");

		graph.recreateVerticesInfo(graph.VertexCount, graph.longEdges);
		outputLongEdges(graph.longEdges, graph,
				folder+"afterLowers_info.dot");
		save(folder+"afterLowerGraph.txt", graph);
	}

	if (useExtractDefinite) {
		INFO("extractDefinite RIGHT Start");
		if (!useProcessLower) {
			load(folder+"afterLowerGraph.txt", graph);
			graph.RebuildVertexMap();
			DEBUG("loaded");
		}
		graph.recreateVerticesInfo(graph.VertexCount, graph.longEdges);
		extractDefinite(graph, RIGHT);
		outputLongEdges(graph.longEdges, graph,
				folder+"afterExtractDefinite1.dot");
		//	outputLongEdgesThroughGenome(graph, "data/paireddebruijn/afterExtractDefinite1_g.dot");
		INFO ("extractDefinite LEFT Start");
		graph.recreateVerticesInfo(graph.VertexCount, graph.longEdges);
		extractDefinite(graph, LEFT);
		outputLongEdges(graph.longEdges, graph,
				folder+"afterExtractDefinite2.dot");
	}
	cerr << "\n Finished";
	INFO("Finished");
}

void generate() {
//	generateReads<SmoothPositionChooser> ("data/paireddebruijn/generated1.txt",
//			"data/input/MG1655-K12_cut.fasta", 20, 200, 0, 0);
//	generateReads<RandomPositionChooser> ("data/paireddebruijn/generated2.txt",
//			"data/input/MG1655-K12_cut.fasta", 20, 200, 0, 0);
	generateReads<SmoothPositionChooser> ("data/paireddebruijn/generated3.txt",
			"data/input/MG1655-K12_cut.fasta", 20, 200, 0, 6);
	generateReads<RandomPositionChooser> ("data/paireddebruijn/generated4.txt",
			"data/input/MG1655-K12_cut.fasta", 20, 200, 0, 6);
}

int main() {
	//generate();
	init();
	run();
	return 0;
}
