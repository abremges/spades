#pragma once 
#include <tr1/unordered_map>

template<class T, class Value, class Hash, class KeyEqual>
struct parallel_unordered_map
{
    private:
        typedef	std::tr1::unordered_map<T, Value, Hash, KeyEqual>				origin_map_t;
        typedef	std::vector<origin_map_t>							            map_arr_t;
        typedef typename origin_map_t::value_type      value_type;
    public:    
        parallel_unordered_map(size_t nthreads)
            : nthreads_		(nthreads)
              , buckets_		(nthreads, origin_map_t(100000)) {

        }

        parallel_unordered_map(const parallel_unordered_map<T, Value, Hash, KeyEqual>& map)
        {
            nthreads_ = map.get_threads_num();
            for (size_t i = 0; i < nthreads_; ++i) {
                buckets_.push_back(map.get_buckets()[i]);
            }
        }

        parallel_unordered_map<T, Value, Hash, KeyEqual> & operator=(const parallel_unordered_map<T, Value, Hash, KeyEqual>& map)
        { 
            if (this == &map) return *this;

            nthreads_ = map.get_threads_num();
            for (size_t i = 0; i < nthreads_; ++i) {
                buckets_.push_back(map.get_buckets()[i]);
            }
            return *this;                      
        }

        //bool operator== (const parallel_unordered_map<T, Value, Hash, KeyEqual>& map)
        //{
            //if (nthreads_ != map.get_threads_num) return false;
            //for (size_t i = 0;
        //}

        void insert(const value_type& value, size_t bucket_num)
        {
            buckets_[bucket_num].insert(value);
        }

        void merge_buckets(origin_map_t& om)
        {
            for (size_t i = 0; i<nthreads_; ++i) {
                om.insert(buckets_[i].begin(), buckets_[i].end());            
            }
        }

        const origin_map_t & operator[](size_t i) const 
        {
            return buckets_[i];
        }

        size_t get_threads_num() const 
        {
            return nthreads_;
        }

        const map_arr_t & get_buckets() const
        {
            return buckets_;   
        }

        void clear() {
            for (size_t i = 0; i < nthreads_; ++i) {
                buckets_[i].clear();
            }
        }

    private:
        size_t 		nthreads_;
        map_arr_t 	buckets_;
};
