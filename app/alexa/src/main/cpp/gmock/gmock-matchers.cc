#include <string.h>
#include <sstream>
#include <string>
#include "gmock-matchers.h"
#include "gmock-generated-matchers.h"

namespace testing {
    Matcher<const internal::string&>::Matcher(const internal::string& s) {
        *this = Eq(s);
    }
    Matcher<const internal::string&>::Matcher(const char* s) {
        *this = Eq(internal::string(s));
    }
    Matcher<internal::string>::Matcher(const internal::string& s) { *this = Eq(s); }
    Matcher<internal::string>::Matcher(const char* s) {
        *this = Eq(internal::string(s));
    }
#if GTEST_HAS_STRING_PIECE_
    Matcher<const StringPiece&>::Matcher(const internal::string& s) {
        *this = Eq(s);
    }
    Matcher<const StringPiece&>::Matcher(const char* s) {
        *this = Eq(internal::string(s));
    }
    Matcher<const StringPiece&>::Matcher(StringPiece s) {
        *this = Eq(s.ToString());
    }
    Matcher<StringPiece>::Matcher(const internal::string& s) {
        *this = Eq(s);
    }
    Matcher<StringPiece>::Matcher(const char* s) {
        *this = Eq(internal::string(s));
    }
    Matcher<StringPiece>::Matcher(StringPiece s) {
        *this = Eq(s.ToString());
    }
#endif
    namespace internal {
        GTEST_API_ string JoinAsTuple(const Strings& fields) {
            switch(fields.size()) {
                case 0: return "";
                case 1: return fields[0];
                default:
                    string result = "(" + fields[0];
                    for (size_t i = 1; i < fields.size(); i++) {
                        result += ", ";
                        result += fields[i];
                    }
                    result += ")";
                    return result;
            }
        }
        GTEST_API_ string FormatMatcherDescription(bool negation, const char* matcher_name, const Strings& param_values) {
            string result = ConvertIdentifierNameToWords(matcher_name);
            if (param_values.size() >= 1) result += " " + JoinAsTuple(param_values);
            return negation ? "not (" + result + ")" : result;
        }
        class MaxBipartiteMatchState {
        public:
            explicit MaxBipartiteMatchState(const MatchMatrix& graph) : graph_(&graph), left_(graph_->LhsSize(), kUnused),
                                                                        right_(graph_->RhsSize(), kUnused) {}
            ElementMatcherPairs Compute() {
                ::std::vector<char> seen;
                for (size_t ilhs = 0; ilhs < graph_->LhsSize(); ++ilhs) {
                    GTEST_CHECK_(left_[ilhs] == kUnused) << "ilhs: " << ilhs << ", left_[ilhs]: " << left_[ilhs];
                    seen.assign(graph_->RhsSize(), 0);
                    TryAugment(ilhs, &seen);
                }
                ElementMatcherPairs result;
                for (size_t ilhs = 0; ilhs < left_.size(); ++ilhs) {
                    size_t irhs = left_[ilhs];
                    if (irhs == kUnused) continue;
                    result.push_back(ElementMatcherPair(ilhs, irhs));
                }
                return result;
            }
        private:
            static const size_t kUnused = static_cast<size_t>(-1);
            bool TryAugment(size_t ilhs, ::std::vector<char>* seen) {
                for (size_t irhs = 0; irhs < graph_->RhsSize(); ++irhs) {
                    if ((*seen)[irhs]) continue;
                    if (!graph_->HasEdge(ilhs, irhs)) continue;
                    (*seen)[irhs] = 1;
                    if (right_[irhs] == kUnused || TryAugment(right_[irhs], seen)) {
                        left_[ilhs] = irhs;
                        right_[irhs] = ilhs;
                        return true;
                    }
                }
                return false;
            }
            const MatchMatrix* graph_;
            ::std::vector<size_t> left_;
            ::std::vector<size_t> right_;
            GTEST_DISALLOW_ASSIGN_(MaxBipartiteMatchState);
        };
        const size_t MaxBipartiteMatchState::kUnused;
        GTEST_API_ ElementMatcherPairs FindMaxBipartiteMatching(const MatchMatrix& g) {
            return MaxBipartiteMatchState(g).Compute();
        }
        static void LogElementMatcherPairVec(const ElementMatcherPairs& pairs, ::std::ostream* stream) {
            typedef ElementMatcherPairs::const_iterator Iter;
            ::std::ostream& os = *stream;
            os << "{";
            const char *sep = "";
            for (Iter it = pairs.begin(); it != pairs.end(); ++it) {
                os << sep << "\n  (" << "element #" << it->first << ", " << "matcher #" << it->second << ")";
                sep = ",";
            }
            os << "\n}";
        }
        GTEST_API_ bool FindPairing(const MatchMatrix& matrix, MatchResultListener* listener) {
            ElementMatcherPairs matches = FindMaxBipartiteMatching(matrix);
            size_t max_flow = matches.size();
            bool result = (max_flow == matrix.RhsSize());
            if (!result) {
                if (listener->IsInterested()) {
                    *listener << "where no permutation of the elements can satisfy all matchers, and the closest match is " << max_flow << " of " << matrix.RhsSize()
                              << " matchers with the pairings:\n";
                    LogElementMatcherPairVec(matches, listener->stream());
                }
                return false;
            }
            if (matches.size() > 1) {
                if (listener->IsInterested()) {
                    const char *sep = "where:\n";
                    for (size_t mi = 0; mi < matches.size(); ++mi) {
                    *listener << sep << " - element #" << matches[mi].first << " is matched by matcher #" << matches[mi].second;
                    sep = ",\n";
                    }
                }
            }
            return true;
        }
        bool MatchMatrix::NextGraph() {
            for (size_t ilhs = 0; ilhs < LhsSize(); ++ilhs) {
                for (size_t irhs = 0; irhs < RhsSize(); ++irhs) {
                    char &b = matched_[SpaceIndex(ilhs, irhs)];
                    if (!b) {
                        b = 1;
                        return true;
                    }
                    b = 0;
                }
            }
            return false;
        }
        void MatchMatrix::Randomize() {
            for (size_t ilhs = 0; ilhs < LhsSize(); ++ilhs) {
                for (size_t irhs = 0; irhs < RhsSize(); ++irhs) {
                    char& b = matched_[SpaceIndex(ilhs, irhs)];
                    b = static_cast<char>(rand() & 1);
                }
            }
        }
        string MatchMatrix::DebugString() const {
            ::std::stringstream ss;
            const char *sep = "";
            for (size_t i = 0; i < LhsSize(); ++i) {
                ss << sep;
                for (size_t j = 0; j < RhsSize(); ++j) ss << HasEdge(i, j);
                sep = ";";
            }
            return ss.str();
        }
        void UnorderedElementsAreMatcherImplBase::DescribeToImpl(::std::ostream* os) const {
            if (matcher_describers_.empty()) {
                *os << "is empty";
                return;
            }
            if (matcher_describers_.size() == 1) {
                *os << "has " << Elements(1) << " and that element ";
                matcher_describers_[0]->DescribeTo(os);
                return;
            }
            *os << "has " << Elements(matcher_describers_.size()) << " and there exists some permutation of elements such that:\n";
            const char* sep = "";
            for (size_t i = 0; i != matcher_describers_.size(); ++i) {
                *os << sep << " - element #" << i << " ";
                matcher_describers_[i]->DescribeTo(os);
                sep = ", and\n";
            }
        }
        void UnorderedElementsAreMatcherImplBase::DescribeNegationToImpl(::std::ostream* os) const {
            if (matcher_describers_.empty()) {
                *os << "isn't empty";
                return;
            }
            if (matcher_describers_.size() == 1) {
                *os << "doesn't have " << Elements(1) << ", or has " << Elements(1) << " that ";
                matcher_describers_[0]->DescribeNegationTo(os);
                return;
            }
            *os << "doesn't have " << Elements(matcher_describers_.size()) << ", or there exists no permutation of elements such that:\n";
            const char* sep = "";
            for (size_t i = 0; i != matcher_describers_.size(); ++i) {
                *os << sep << " - element #" << i << " ";
                matcher_describers_[i]->DescribeTo(os);
                sep = ", and\n";
            }
        }
        bool UnorderedElementsAreMatcherImplBase::VerifyAllElementsAndMatchersAreMatched(const ::std::vector<string>& element_printouts,
                                                                                         const MatchMatrix& matrix,
                                                                                         MatchResultListener* listener) const {
            bool result = true;
            ::std::vector<char> element_matched(matrix.LhsSize(), 0);
            ::std::vector<char> matcher_matched(matrix.RhsSize(), 0);
            for (size_t ilhs = 0; ilhs < matrix.LhsSize(); ilhs++) {
                for (size_t irhs = 0; irhs < matrix.RhsSize(); irhs++) {
                    char matched = matrix.HasEdge(ilhs, irhs);
                    element_matched[ilhs] |= matched;
                    matcher_matched[irhs] |= matched;
                }
            }
            {
                const char* sep = "where the following matchers don't match any elements:\n";
                for (size_t mi = 0; mi < matcher_matched.size(); ++mi) {
                    if (matcher_matched[mi]) continue;
                    result = false;
                    if (listener->IsInterested()) {
                        *listener << sep << "matcher #" << mi << ": ";
                        matcher_describers_[mi]->DescribeTo(listener->stream());
                        sep = ",\n";
                    }
                }
            }
            {
                const char* sep = "where the following elements don't match any matchers:\n";
                const char* outer_sep = "";
                if (!result) outer_sep = "\nand ";
                for (size_t ei = 0; ei < element_matched.size(); ++ei) {
                    if (element_matched[ei]) continue;
                    result = false;
                    if (listener->IsInterested()) {
                        *listener << outer_sep << sep << "element #" << ei << ": " << element_printouts[ei];
                        sep = ",\n";
                        outer_sep = "";
                    }
                }
            }
            return result;
        }
    }
}