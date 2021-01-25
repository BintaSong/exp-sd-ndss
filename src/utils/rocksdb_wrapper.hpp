//
// Sophos - Forward Private Searchable Encryption
// Copyright (C) 2016 Raphael Bost
//
// This file is part of Sophos.
//
// Sophos is free software: you can redistribute it and/or modify
// it under the terms of the GNU Affero General Public License as
// published by the Free Software Foundation, either version 3 of the
// License, or (at your option) any later version.
//
// Sophos is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with Sophos.  If not, see <http://www.gnu.org/licenses/>.
//

#pragma once

#include "logger.hpp"
#include "utils.hpp"
#include <stdlib.h>
#include <rocksdb/db.h>
#include <rocksdb/table.h>
#include <rocksdb/memtablerep.h>
#include <rocksdb/options.h>

#include <list>
#include <iostream>
#include "Utilities.h"

namespace sse {
    namespace sophos {

        class RockDBWrapper {
        public:
            RockDBWrapper() = delete;
            inline RockDBWrapper(const std::string &path);
            inline ~RockDBWrapper();

            inline bool get(const std::string &key, std::string &data) const;
            inline bool put(const std::string &key, const std::string &data) const;
            inline bool put(const char* key, const char* data) const;
            inline bool remove(const std::string key) const;

            template <size_t N, typename V>
            inline bool get(const std::array<uint8_t, N> &key, V &data) const;

            template <typename V>
            inline bool get(const uint8_t *key, const uint8_t key_length, V &data) const;

            template <size_t N, typename V>
            inline bool put(const std::array<uint8_t, N> &key, const V &data);

            template <size_t N>
            inline bool remove(const std::array<uint8_t, N> &key);

            inline bool remove(const uint8_t *key, const uint8_t key_length);

            inline void flush(bool blocking = true);

            inline uint64_t approximate_size() const;

        private:
            rocksdb::DB* db_;

        };

        RockDBWrapper::RockDBWrapper(const std::string &path)
        : db_(NULL) {
            rocksdb::Options options;
            options.create_if_missing = true;


            rocksdb::CuckooTableOptions cuckoo_options;
            cuckoo_options.identity_as_first_hash = false;
            cuckoo_options.hash_table_ratio = 0.9;


            //        cuckoo_options.use_module_hash = false;
            //        cuckoo_options.identity_as_first_hash = true;

            options.table_cache_numshardbits = 4;
            options.max_open_files = -1;




            options.table_factory.reset(rocksdb::NewCuckooTableFactory(cuckoo_options));

            //            options.memtable_factory.reset(new rocksdb::VectorRepFactory());

            options.compression = rocksdb::kNoCompression;
            options.bottommost_compression = rocksdb::kDisableCompressionOption;

            options.compaction_style = rocksdb::kCompactionStyleLevel;
            options.info_log_level = rocksdb::InfoLogLevel::ERROR_LEVEL;


            //        options.max_grandparent_overlap_factor = 10;

            //            options.delayed_write_rate = 8388608;
            options.max_background_compactions = 20;

            //        options.disableDataSync = true;
            options.allow_mmap_reads = true;
            options.new_table_reader_for_compaction_inputs = true;

            options.allow_concurrent_memtable_write = options.memtable_factory->IsInsertConcurrentlySupported();

            //            options.max_bytes_for_level_base = 64*1024*1024;//8*1024;//4294967296; // 4 GB
            //            options.arena_block_size = 1024*1024;//128;//134217728; // 128 MB
            //            options.target_file_size_base = 1024*1024*8;//1024;//201327616;
            //            options.write_buffer_size = 1024*1024*8;//1024;//1073741824; // 1GB
            options.max_bytes_for_level_base = 3 * 1024 * 1024;
            options.write_buffer_size = options.max_bytes_for_level_base / 10;
            options.arena_block_size = options.write_buffer_size / 8;
            options.target_file_size_base = options.max_bytes_for_level_base / 10;

            options.level0_file_num_compaction_trigger = 10;
            options.level0_slowdown_writes_trigger = 16;
            options.hard_pending_compaction_bytes_limit = 137438953472; // 128 GB


            //        options.optimize_filters_for_hits = true;


            rocksdb::Status status = rocksdb::DB::Open(options, path, &db_);

            if (!status.ok()) {
                logger::log(logger::CRITICAL) << "Unable to open the database: " << status.ToString() << std::endl;
                db_ = NULL;
            }
        }

        RockDBWrapper::~RockDBWrapper() {
            if (db_) {
                delete db_;
            }
        }

        bool RockDBWrapper::get(const std::string &key, std::string &data) const {
            rocksdb::Status s = db_->Get(rocksdb::ReadOptions(), key, &data);

            return s.ok();
        }

        bool RockDBWrapper::put(const char* key, const char* data) const {
            rocksdb::Status s = db_->Put(rocksdb::WriteOptions(), key, data);

            return s.ok();
        }

        bool RockDBWrapper::put(const std::string &key, const std::string &data) const {
            rocksdb::Status s = db_->Put(rocksdb::WriteOptions(), key, data);

            return s.ok();
        }

        bool RockDBWrapper::remove(const std::string key) const {
            rocksdb::Status s = db_->Delete(rocksdb::WriteOptions(), key);

            return s.ok();
        }

        template <size_t N, typename V>
        bool RockDBWrapper::get(const std::array<uint8_t, N> &key, V &data) const {
            rocksdb::Slice k_s(reinterpret_cast<const char*> (key.data()), N);
            std::string value;

            rocksdb::Status s = db_->Get(rocksdb::ReadOptions(false, true), k_s, &value);

            if (s.ok()) {
                ::memcpy(&data, value.data(), sizeof (V));
            }

            return s.ok();
        }

        template <typename V>
        bool RockDBWrapper::get(const uint8_t *key, const uint8_t key_length, V &data) const {
            rocksdb::Slice k_s(reinterpret_cast<const char*> (key), key_length);
            std::string value;

            rocksdb::Status s = db_->Get(rocksdb::ReadOptions(false, true), k_s, &value);

            if (s.ok()) {
                ::memcpy(&data, value.data(), sizeof (V));
            }

            return s.ok();
        }

        template <size_t N, typename V>
        bool RockDBWrapper::put(const std::array<uint8_t, N> &key, const V &data) {
            rocksdb::Slice k_s(reinterpret_cast<const char*> (key.data()), N);
            rocksdb::Slice k_v(reinterpret_cast<const char*> (&data), sizeof (V));
            rocksdb::Status s = db_->Put(rocksdb::WriteOptions(), k_s, k_v);
            if (!s.ok()) {
                logger::log(logger::ERROR) << "Unable to insert pair in the database: " << s.ToString() << std::endl;
                logger::log(logger::ERROR) << "Failed on pair: key=" << hex_string(key) << ", data=" << hex_string(data) << std::endl;

            }
            //        assert(s.ok());

            return s.ok();
        }

        template <size_t N>
        bool RockDBWrapper::remove(const std::array<uint8_t, N> &key) {
            rocksdb::Slice k_s(reinterpret_cast<const char*> (key.data()), N);

            rocksdb::Status s = db_->Delete(rocksdb::WriteOptions(), k_s);

            return s.ok();
        }

        bool RockDBWrapper::remove(const uint8_t *key, const uint8_t key_length) {
            rocksdb::Slice k_s(reinterpret_cast<const char*> (key), key_length);

            rocksdb::Status s = db_->Delete(rocksdb::WriteOptions(), k_s);

            return s.ok();
        }

        void RockDBWrapper::flush(bool blocking) {
            rocksdb::FlushOptions options;

            options.wait = blocking;

            rocksdb::Status s = db_->Flush(options);

            if (!s.ok()) {
                logger::log(logger::ERROR) << "DB Flush failed: " << s.ToString() << std::endl;
            }

        }

        uint64_t RockDBWrapper::approximate_size() const {
            uint64_t v;
            bool flag = db_->GetIntProperty(rocksdb::DB::Properties::kEstimateNumKeys, &v);

            assert(flag);

            return v;
        }

        class RocksDBCounter {
        public:
            RocksDBCounter() = delete;
            RocksDBCounter(const std::string &path);

            inline ~RocksDBCounter() {
                if (db_) {
                    delete db_;
                }
            };

            bool get(const std::string &key, uint32_t &val) const;

            bool get_and_increment(const std::string &key, uint32_t &val);

            bool increment(const std::string &key, uint32_t default_value = 0);

            bool set(const std::string &key, uint32_t val);

            bool remove_key(const std::string &key);

            inline void flush(bool blocking = true);

            inline uint64_t approximate_size() const {
                uint64_t v;
                bool flag = db_->GetIntProperty(rocksdb::DB::Properties::kEstimateNumKeys, &v);

                assert(flag);

                return v;
            }

        private:
            rocksdb::DB* db_;

        };

        template<class T> struct serialization {
            std::string serialize(const T&);
            bool deserialize(std::string::iterator& begin, const std::string::iterator& end, T& out);
        };

        template <typename T, class Serializer = serialization<T>>
        class RockDBListStore {
        public:
            typedef Serializer serializer;

            RockDBListStore() = delete;
            inline RockDBListStore(const std::string &path);
            inline ~RockDBListStore();

            // find the list associated to key and append elements to data
            bool get(const std::string &key, std::list<T> &data, serializer& deser) const;

            bool get(const std::string &key, std::list<T> &data) const {
                serializer deser = serializer();
                return get(key, data, deser);
            }

            template <size_t N>
            inline bool get(const std::array<uint8_t, N> &key, std::list<T> &data, serializer& deser) const {
                return get(key.data(), N, data, deser);
            }

            template <size_t N>
            inline bool get(const std::array<uint8_t, N> &key, std::list<T> &data) const {
                serializer deser = serializer();
                return get(key, data, deser);
            }

            bool get(const uint8_t *key, const uint8_t key_length, std::list<T> &data, serializer& deser) const;

            inline bool get(const uint8_t *key, const uint8_t key_length, std::list<T> &data) const {
                serializer deser = serializer();
                return get(key, key_length, data, deser);
            }


            template <size_t N>
            bool put(const std::array<uint8_t, N> &key, const std::list<T> &data, serializer& ser);

            template <size_t N>
            inline bool put(const std::array<uint8_t, N> &key, const std::list<T> &data) {
                serializer ser = serializer();
                return put<N>(key, data, ser);
            }


            void flush(bool blocking = true);
        private:
            rocksdb::DB* db_;

        };

        template <typename T, class Serializer>
        RockDBListStore<T, Serializer>::RockDBListStore(const std::string &path) {
            rocksdb::Options options;
            options.create_if_missing = true;


            rocksdb::CuckooTableOptions cuckoo_options;
            cuckoo_options.identity_as_first_hash = false;
            cuckoo_options.hash_table_ratio = 0.9;

            options.table_cache_numshardbits = 4;
            options.max_open_files = -1;

            options.compression = rocksdb::kNoCompression;
            options.bottommost_compression = rocksdb::kDisableCompressionOption;

            options.compaction_style = rocksdb::kCompactionStyleLevel;
            options.info_log_level = rocksdb::InfoLogLevel::ERROR_LEVEL;

            options.delayed_write_rate = 8388608;
            options.max_background_compactions = 20;

            options.allow_mmap_reads = true;
            options.new_table_reader_for_compaction_inputs = true;

            options.allow_concurrent_memtable_write = options.memtable_factory->IsInsertConcurrentlySupported();

            options.max_bytes_for_level_base = 4294967296; // 4 GB
            options.arena_block_size = 134217728; // 128 MB
            options.level0_file_num_compaction_trigger = 10;
            options.level0_slowdown_writes_trigger = 16;
            options.hard_pending_compaction_bytes_limit = 137438953472; // 128 GB
            options.target_file_size_base = 201327616;
            options.write_buffer_size = 1073741824; // 1GB

            rocksdb::Status status = rocksdb::DB::Open(options, path, &db_);

            if (!status.ok()) {
                logger::log(logger::CRITICAL) << "Unable to open the database: " << status.ToString() << std::endl;
                db_ = NULL;
            }
        }

        template <typename T, class Serializer>
        RockDBListStore<T, Serializer>::~RockDBListStore() {
            if (db_) {
                delete db_;
            }
        }

        template <typename T, class Serializer>
        bool RockDBListStore<T, Serializer>::get(const std::string &key, std::list<T> &data, serializer& deser) const {
            std::string raw_string;
            rocksdb::Status s = db_->Get(rocksdb::ReadOptions(), key, &raw_string);

            if (s.ok()) {
                auto it = raw_string.begin();
                const auto end = raw_string.end();
                T elt;

                while (deser.deserialize(it, end, elt)) {
                    data.push_back(std::move<T>(elt));
                }
            }
            return s.ok();
        }

        //    template <typename T, class Serializer>
        //    template <size_t N>
        //    bool RockDBListStore<T, Serializer>::get(const std::array<uint8_t, N> &key, std::list<T> &data, serializer& deser) const
        //    {
        //        rocksdb::Slice k_s(reinterpret_cast<const char*>( key.data() ),N);
        //        std::string raw_string;
        //        
        //        rocksdb::Status s = db_->Get(rocksdb::ReadOptions(false,true), k_s, &raw_string);
        //        
        //        if(s.ok()){
        //            auto it = raw_string.begin();
        //            const auto end = raw_string.end();
        //            T elt;
        //            
        //            while (deser.deserialize(it, end, elt)) {
        //                data.push_back(std::move<T>(elt));
        //            }
        //        }
        //        return s.ok();
        //    }

        template <typename T, class Serializer>
        bool RockDBListStore<T, Serializer>::get(const uint8_t *key, const uint8_t key_length, std::list<T> &data, serializer& deser) const {
            rocksdb::Slice k_s(reinterpret_cast<const char*> (key), key_length);
            std::string raw_string;

            rocksdb::Status s = db_->Get(rocksdb::ReadOptions(false, true), k_s, &raw_string);

            if (s.ok()) {
                auto it = raw_string.begin();
                const auto end = raw_string.end();
                T elt;

                while (deser.deserialize(it, end, elt)) {
                    //                data.push_back(std::move<T>(elt));
                    data.push_back(elt);
                }
            }
            return s.ok();
        }

        template <typename T, class Serializer>
        template <size_t N>
        bool RockDBListStore<T, Serializer>::put(const std::array<uint8_t, N> &key, const std::list<T> &data, serializer& ser) {
            std::string serialized_list;

            for (T elt : data) {
                serialized_list += ser.serialize(elt);
            }

            rocksdb::Slice k_s(reinterpret_cast<const char*> (key.data()), N);
            //        rocksdb::Slice k_v(reinterpret_cast<const char*>(&serialized_list.data()), sizeof(V));
            rocksdb::Slice k_v(serialized_list.data(), serialized_list.size());

            rocksdb::Status s = db_->Put(rocksdb::WriteOptions(), k_s, k_v);

            if (!s.ok()) {
                logger::log(logger::ERROR) << "Unable to insert key in the database: " << s.ToString() << std::endl;
                //            logger::log(logger::ERROR) << "Failed on pair: key=" << hex_string(key) << ", data=" << hex_string(data) << std::endl;

            }

            return s.ok();
        }

        template <typename T, class Serializer>
        void RockDBListStore<T, Serializer>::flush(bool blocking) {
            rocksdb::FlushOptions options;

            options.wait = blocking;

            rocksdb::Status s = db_->Flush(options);

            if (!s.ok()) {
                logger::log(logger::ERROR) << "DB Flush failed: " << s.ToString() << std::endl;
            }

        }

    }
}
