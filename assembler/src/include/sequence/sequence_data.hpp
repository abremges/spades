//***************************************************************************
//* Copyright (c) 2011-2012 Saint-Petersburg Academic University
//* All Rights Reserved
//* See file LICENSE for details.
//****************************************************************************

/**
 * @file    sequence_data.hpp
 * @author  vyahhi
 * @version 1.0
 *
 * @section LICENSE
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * @section DESCRIPTION
 *
 * Sequence Data with ref counter
 */

#ifndef SEQUENCE_DATA_HPP_
#define SEQUENCE_DATA_HPP_

#include <vector>
#include <string>
#include "log.hpp"

class SequenceData {
private:
    friend class Sequence;
    // type to store Seq in Sequences
    typedef u_int64_t ST;
    // number of nucleotides in ST
    static const size_t STN = (sizeof(ST) * 4);
    // number of bits in STN (for faster div and mod)
    static const size_t STNbits = log_<STN, 2>::value;
    // ref counter
    size_t count; // should be volatile some day for multi-threading
    // sequence (actual data for what it's for)
    Seq<STN, ST> *bytes_;
    // methods:
    SequenceData(const SequenceData &sd); // forbidden
    SequenceData& operator=(const SequenceData&); // forbidden
    void Grab() {
        ++count;
    }
    void Release() {
        --count;
        if (count == 0) {
            delete this;
        }
    }

    //template<typename S>
    //string SubString(const S &s, size_t offset, size_t size) {
        //string str;
        //for (size_t i = offset; i < size; ++i) {
            //str += s[i];
        //}
        //return str;
    //}
public:
    /**
	 * Sequence initialization (arbitrary size string)
	 *
	 * @param s ACGT or 0123-string
	 */
    template<typename S>
    SequenceData(const S &s, size_t size) : count(0) 
    {
        TRACE("New constructor seqdata " << size);
        size_t bytes_size = (size + STN - 1) >> STNbits;
        bytes_ = (Seq<STN, ST>*) malloc(bytes_size * sizeof(Seq<STN, ST> )); // it's a bit faster than new
        for (size_t i = 0; i < (size >> STNbits); ++i) {
            bytes_[i] = Seq<STN, ST> (s, i << STNbits);
        }

        if (size & (STN - 1)) 
            bytes_[size >> STNbits] = Seq<STN, ST> (s, (size & ~(STN - 1)), (size & (STN - 1)));

        //if (size & (STN - 1)) {
            //// fill with As for not breaking contract of Seq
            ////todo think of something better than using string
            //string s2 = SubString(s, size & ~(STN - 1), size);
            //size_t count = STN - (size & (STN - 1));
            //s2.append(count, 'A');
            //bytes_[size >> STNbits] = Seq<STN, ST> (s2);
        //}
    }

    SequenceData(size_t size_): count(0) {
        size_t size = size_;
        size_t bytes_size = (size + STN - 1) >> STNbits;
        bytes_ = (Seq<STN, ST>*) malloc(bytes_size * sizeof(Seq<STN, ST> )); // it's a bit faster than new
        for (size_t i = 0; i < bytes_size; ++i) {
            bytes_[i] = Seq<STN, ST>();
        }
    }

    bool BinRead(std::istream& file, size_t size) {
        size_t bytes_size = (size + STN - 1) >> STNbits;
        for (size_t i = 0; i < bytes_size; ++i) {
            bytes_[i].BinRead(file);
        }
        return !file.fail();
    }

    bool BinWrite(std::ostream& file, size_t size) {
        size_t bytes_size = (size + STN - 1) >> STNbits;
        for (size_t i = 0; i < bytes_size; ++i) {
            bytes_[i].BinWrite(file);
        }
        return !file.fail();
    }

    ~SequenceData() {
        free(bytes_);
    }

    char operator[](const size_t i) const {
        return bytes_[i >> STNbits][i & (STN - 1)];
    }
};

#endif /* REFCOUNT_HPP_ */
