//***************************************************************************
//* Copyright (c) 2015 Saint Petersburg State University
//* All Rights Reserved
//* See file LICENSE for details.
//***************************************************************************

//
// Created by lab42 on 8/19/15.
//

#ifndef GENOME_STORAGE_HPP_
#define GENOME_STORAGE_HPP_

#include <string>
#include "data_structures/sequence/sequence.hpp"
namespace debruijn_graph {
    class GenomeStorage {
    private:
        std::string s_;
    public:
        GenomeStorage():s_(""){
        }

        GenomeStorage(const std::string &s): s_(s){
        }

        Sequence GetSequence() const;
        void SetSequence(const Sequence &s);
        std::string str() const;
        size_t size() const;
    };
}
#endif //PROJECT_GENOME_STORAGE_HPP
