#pragma once
#include "unordered_vector.hpp"
#include "Common3D.hpp"

namespace RavEngine {
    template<typename index_t,typename container_t>
    class UnorderedSparseSetGenericContainer {
    public: 
        constexpr static index_t default_index = std::numeric_limits<index_t>::max();
        constexpr static index_t INVALID_INDEX = default_index;
        using index_type = index_t;
        using value_type = container_t::value_type;
    private:
        container_t dense_set;
        std::vector<index_t> sparse_set{ default_index };

    public:
        std::vector<index_t> reverse_map;
        using const_iterator = typename decltype(dense_set)::const_iterator_type;

        template<typename ... A>
        inline void Emplace(index_t sparse_index, A&& ... args) {
            //if a record for this does not exist, create it
            if (!HasForSparseIndex(sparse_index)) {
                dense_set.emplace(args...);
                reverse_map.emplace_back(sparse_index);
                if (sparse_index >= sparse_set.size()) {
                    sparse_set.resize(closest_multiple_of(sparse_index + 1, 2), default_index);  //ensure there is enough space for this id
                }
                sparse_set[sparse_index] = static_cast<typename decltype(sparse_set)::value_type>(dense_set.size() - 1);
            }
        }

        inline void EraseAtSparseIndex(index_t sparse_index) {
            // get the record, then call erase on it
            assert(HasForSparseIndex(sparse_index));

            auto denseidx = SparseToDense(sparse_index);
            dense_set.erase(dense_set.begin() + denseidx);

            if (denseidx < dense_set.size()) {    // did a move happen during this deletion?
                auto ownerOfMoved = reverse_map.back();
                sparse_set[ownerOfMoved] = denseidx;
                reverse_map[denseidx] = reverse_map.back();
            }
            reverse_map.pop_back();
            sparse_set[sparse_index] = INVALID_INDEX;
        }

        inline auto& GetForSparseIndex(index_t sparse_index) {
            assert(HasForSparseIndex(sparse_index));
            return dense_set[SparseToDense(sparse_index)];
        }

        inline auto SparseToDense(index_t sparse_index) {
            return sparse_set[sparse_index];
        }

        inline bool HasForSparseIndex(index_t sparse_index) const {
            return sparse_index < sparse_set.size() && sparse_set[sparse_index] != default_index;
        }

        auto begin() {
            return dense_set.begin();
        }

        auto end() {
            return dense_set.end();
        }

        auto begin() const {
            return dense_set.begin();
        }

        auto end() const {
            return dense_set.end();
        }

        // get by dense index, not by entity ID
        value_type& Get(index_t idx) {
            return dense_set[idx];
        }

        // given a dense index, return its sparse index
        value_type& GetSparseIndexForDense(index_t idx) {
            return reverse_map[idx];
        }

        const value_type& Get(index_t idx) const {
            return Get(idx);
        }

        auto DenseSize() const {
            return dense_set.size();
        }

        auto GetDenseData() const {
            return dense_set.data();
        }

        inline auto& GetDense() {
            return dense_set;
        }
    };

    template<typename index_t, typename container_t>
    class UnorderedSparseSet : public UnorderedSparseSetGenericContainer<index_t, unordered_vector<container_t>> {};
}
