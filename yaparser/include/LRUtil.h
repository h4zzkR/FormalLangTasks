//
// Created by h4zzkr on 08.12.2021.
//
#ifndef YAPARSER_LRUTIL_H
#define YAPARSER_LRUTIL_H

namespace hashing {
    template <typename element_hash>
    struct sequence_hasher {
        template<class T>
        size_t operator()(const T& container) const {
            size_t prime_ = 3, hsh = 0;
            for (const auto & i : container) {
                hsh += element_hash{}(i) * prime_;
                prime_ *= prime_;
            }
            return hsh;
        }
    };

    template <typename element_hash>
    struct sequence_equal {
        template<class T>
        bool operator()(const T& container1, const T& container2) const {
            return sequence_hasher<element_hash>{}(container1) ==
                   sequence_hasher<element_hash>{}(container2);
        }
    };

    template <typename T, typename Flag = std::true_type>
    struct item_hasher {
        size_t operator()(const T& it) const { return it.hash(Flag()); }
    };
    template <typename T, typename Flag = std::true_type>
    struct item_equal {
        bool operator()(const T& it1, const T& it2) const {
            return it1.hash(Flag()) == it2.hash(Flag());
        }
    };
}

/*
 * s{i} - shift on i state
 * r{i} - reduce on i rule
 * a    - accept
 * c    - conflict in grammar
 */
struct ActionLabel {
    char type;
    size_t item_id;
    ActionLabel() = default;
    explicit ActionLabel(char type, size_t state_id = -1): type(type), item_id(state_id) {}
};

template <typename String>
std::vector<std::string> split(String&& s) {
    std::stringstream ss(std::forward<String>(s));
    std::istream_iterator<std::string> begin(ss);
    std::istream_iterator<std::string> end;
    std::vector<std::string> vstrings(begin, end);
    return vstrings;
}

#endif //YAPARSER_LRUTIL_H