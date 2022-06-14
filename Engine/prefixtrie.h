#ifndef PREFIX_TRIE_H_
#define PREFIX_TRIE_H_

#include <string>
#include <unordered_map>
#include <vector>

struct TrieNode {
    void *next_index;
    size_t value_pos;
};

typedef std::unordered_map<char, TrieNode> TrieIndexMap;

template<typename OBJECT>
struct PTrieEntry {
    std::string key;
    OBJECT value;

    PTrieEntry() {}

    PTrieEntry(const std::string & _key)
        : key(_key) {}
};

template<typename OBJECT>
class PTrieMap {
public:
    PTrieMap() : trie_index(new TrieIndexMap) {}

    // interface used in KoPLEngine
    size_t find(const std::string &key) const {
        const char * ch = key.c_str();
        auto idx = this->trie_index;
        size_t pos = 0;
        while (idx != nullptr) {
            auto res = idx->find(*ch);
            if (res == idx->end()) {
                // miss
                return this->kvs.size();
            }

            pos = (*idx)[*ch].value_pos;
            idx = (TrieIndexMap *) (*idx)[*ch].next_index;

            ++ch;
        }

        // check if hit
        if (kvs[pos]->key == key) {
            // hit
            return pos;
        }

        // miss
        return kvs.size();
    }

    OBJECT & operator [] (const std::string &key) {
        return kvs[add(key)]->value;
    }

    OBJECT && operator [] (const std::string &key) const {
        return kvs[this->find(key)]->value;
    }

    OBJECT at(const std::string &key) const {
        return kvs[this->find(key)]->value;
    }

    size_t end() const {
        return this->kvs.size();
    }

private:
    ssize_t add(const std::string & key) {
        const char *ch  = key.c_str();
        size_t k = 0;

        auto idx = this->trie_index;
        auto last_idx = idx;
        size_t pos = 0;

        // try to add the key
        while (idx != nullptr) {
            last_idx = idx;

            auto res = idx->find(*ch);
            if (res == idx->end()) {
                // miss
                (*idx)[*ch].next_index = nullptr;
                (*idx)[*ch].value_pos = pos = kvs.size();
                kvs.emplace_back(new PTrieEntry<OBJECT>(key));

                return pos;
            }

            pos = (*idx)[*ch].value_pos;
            idx = (TrieIndexMap *) (*idx)[*ch].next_index;

            ++ch;
            ++k;
        }

        // now idx must be nullptr
        // ckeck if hit
        if (kvs[pos]->key == key) {
            // hit
            return pos;
        }

        // miss
        // then insert the longest prefix by expand last_idx
        const auto conflict_key = kvs[pos]->key;
        const char *conflict_p = conflict_key.c_str() + k - 1;
        const auto conflict_pos = pos;
        --ch;
        while (*ch == *conflict_p) {
            (*last_idx)[*ch].next_index = new TrieIndexMap;
            last_idx = (TrieIndexMap *) (*last_idx)[*ch].next_index;
            ++ch;
            ++conflict_p;
        }

        (*last_idx)[*conflict_p].next_index = nullptr;
        (*last_idx)[*conflict_p].value_pos = conflict_pos;

        (*last_idx)[*ch].next_index = nullptr;
        (*last_idx)[*ch].value_pos = pos = kvs.size();

        kvs.emplace_back(new PTrieEntry<OBJECT>(key));

        return pos;
    }

    TrieIndexMap *trie_index;

    std::vector<PTrieEntry<OBJECT> *> kvs;
};

#endif
