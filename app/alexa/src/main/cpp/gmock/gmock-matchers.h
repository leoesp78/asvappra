#ifndef GMOCK_INCLUDE_GMOCK_GMOCK_MATCHERS_H_
#define GMOCK_INCLUDE_GMOCK_GMOCK_MATCHERS_H_

#include <math.h>
#include <algorithm>
#include <iterator>
#include <limits>
#include <ostream>
#include <sstream>
#include <string>
#include <utility>
#include <vector>
#include <initializer_list>
#include "../gtest/gtest.h"
#include "internal/gmock-internal-utils.h"
#include "internal/gmock-port.h"

namespace testing {
    class MatchResultListener {
    public:
        explicit MatchResultListener(::std::ostream* os) : stream_(os) {}
        virtual ~MatchResultListener() = 0;
        template <typename T> MatchResultListener& operator<<(const T& x) {
            if (stream_ != NULL) *stream_ << x;
            return *this;
        }
        ::std::ostream* stream() { return stream_; }
        bool IsInterested() const { return stream_ != NULL; }
    private:
        ::std::ostream* const stream_;
        GTEST_DISALLOW_COPY_AND_ASSIGN_(MatchResultListener);
    };
    inline MatchResultListener::~MatchResultListener() {}
    class MatcherDescriberInterface {
    public:
        virtual ~MatcherDescriberInterface() {}
        virtual void DescribeTo(::std::ostream* os) const = 0;
        virtual void DescribeNegationTo(::std::ostream* os) const {
            *os << "not (";
            DescribeTo(os);
            *os << ")";
        }
    };
    template <typename T> class MatcherInterface : public MatcherDescriberInterface {
    public:
        virtual bool MatchAndExplain(T x, MatchResultListener* listener) const = 0;
    };
    class StringMatchResultListener : public MatchResultListener {
    public:
        StringMatchResultListener() : MatchResultListener(&ss_) {}
        internal::string str() const { return ss_.str(); }
        void Clear() { ss_.str(""); }
    private:
        ::std::stringstream ss_;
        GTEST_DISALLOW_COPY_AND_ASSIGN_(StringMatchResultListener);
    };
    namespace internal {
        struct AnyEq {
            template <typename A, typename B> bool operator()(const A& a, const B& b) const { return a == b; }
        };
        struct AnyNe {
            template <typename A, typename B> bool operator()(const A& a, const B& b) const { return a != b; }
        };
        struct AnyLt {
            template <typename A, typename B> bool operator()(const A& a, const B& b) const { return a < b; }
        };
        struct AnyGt {
            template <typename A, typename B> bool operator()(const A& a, const B& b) const { return a > b; }
        };
        struct AnyLe {
            template <typename A, typename B> bool operator()(const A& a, const B& b) const { return a <= b; }
        };
        struct AnyGe {
            template <typename A, typename B> bool operator()(const A& a, const B& b) const { return a >= b; }
        };
        class DummyMatchResultListener : public MatchResultListener {
        public:
            DummyMatchResultListener() : MatchResultListener(NULL) {}
        private:
            GTEST_DISALLOW_COPY_AND_ASSIGN_(DummyMatchResultListener);
        };
        class StreamMatchResultListener : public MatchResultListener {
        public:
            explicit StreamMatchResultListener(::std::ostream* os) : MatchResultListener(os) {}
        private:
            GTEST_DISALLOW_COPY_AND_ASSIGN_(StreamMatchResultListener);
        };
        template <typename T> class MatcherBase {
        public:
            bool MatchAndExplain(T x, MatchResultListener* listener) const {
                return impl_->MatchAndExplain(x, listener);
            }
            bool Matches(T x) const {
            DummyMatchResultListener dummy;
                return MatchAndExplain(x, &dummy);
            }
            void DescribeTo(::std::ostream* os) const { impl_->DescribeTo(os); }
            void DescribeNegationTo(::std::ostream* os) const {
                impl_->DescribeNegationTo(os);
            }
            void ExplainMatchResultTo(T x, ::std::ostream* os) const {
                StreamMatchResultListener listener(os);
                MatchAndExplain(x, &listener);
            }
            const MatcherDescriberInterface* GetDescriber() const {
                return impl_.get();
            }
        protected:
            MatcherBase() {}
            explicit MatcherBase(const MatcherInterface<T>* impl) : impl_(impl) {}
            virtual ~MatcherBase() {}
        private:
            ::testing::internal::linked_ptr<const MatcherInterface<T> > impl_;
        };
    }
    template <typename T> class Matcher : public internal::MatcherBase<T> {
    public:
        explicit Matcher() {}
        explicit Matcher(const MatcherInterface<T>* impl) : internal::MatcherBase<T>(impl) {}
        Matcher(T value);
    };
    template <> class GTEST_API_ Matcher<const internal::string&> : public internal::MatcherBase<const internal::string&> {
    public:
        Matcher() {}
        explicit Matcher(const MatcherInterface<const internal::string&>* impl) : internal::MatcherBase<const internal::string&>(impl) {}
        Matcher(const internal::string& s);
        Matcher(const char* s);
    };
    template <> class GTEST_API_ Matcher<internal::string> : public internal::MatcherBase<internal::string> {
    public:
        Matcher() {}
        explicit Matcher(const MatcherInterface<internal::string>* impl) : internal::MatcherBase<internal::string>(impl) {}
        Matcher(const internal::string& s);
        Matcher(const char* s);
    };
#if GTEST_HAS_STRING_PIECE_
    template<> class GTEST_API_ Matcher<const StringPiece&> : public internal::MatcherBase<const StringPiece&> {
    public:
        Matcher() {}
        explicit Matcher(const MatcherInterface<const StringPiece&>* impl) : internal::MatcherBase<const StringPiece&>(impl) {}
        Matcher(const internal::string& s);
        Matcher(const char* s);
        Matcher(StringPiece s);
    };
    template<> class GTEST_API_ Matcher<StringPiece> : public internal::MatcherBase<StringPiece> {
    public:
        Matcher() {}
        explicit Matcher(const MatcherInterface<StringPiece>* impl) : internal::MatcherBase<StringPiece>(impl) {}
        Matcher(const internal::string& s);
        Matcher(const char* s);
        Matcher(StringPiece s);
    };
#endif
    template <class Impl> class PolymorphicMatcher {
    public:
        explicit PolymorphicMatcher(const Impl& an_impl) : impl_(an_impl) {}
        Impl& mutable_impl() { return impl_; }
        const Impl& impl() const { return impl_; }
        template <typename T> operator Matcher<T>() const {
            return Matcher<T>(new MonomorphicImpl<T>(impl_));
        }
    private:
        template <typename T> class MonomorphicImpl : public MatcherInterface<T> {
        public:
            explicit MonomorphicImpl(const Impl& impl) : impl_(impl) {}
            virtual void DescribeTo(::std::ostream* os) const {
                impl_.DescribeTo(os);
            }
            virtual void DescribeNegationTo(::std::ostream* os) const {
                impl_.DescribeNegationTo(os);
            }
            virtual bool MatchAndExplain(T x, MatchResultListener* listener) const {
                return impl_.MatchAndExplain(x, listener);
            }
        private:
            const Impl impl_;
            GTEST_DISALLOW_ASSIGN_(MonomorphicImpl);
        };
        Impl impl_;
        GTEST_DISALLOW_ASSIGN_(PolymorphicMatcher);
    };
    template <typename T> inline Matcher<T> MakeMatcher(const MatcherInterface<T>* impl) {
        return Matcher<T>(impl);
    }
    template <class Impl> inline PolymorphicMatcher<Impl> MakePolymorphicMatcher(const Impl& impl) {
        return PolymorphicMatcher<Impl>(impl);
    }
    namespace internal {
        template <typename T, typename M> class MatcherCastImpl {
        public:
            static Matcher<T> Cast(const M& polymorphic_matcher_or_value) {
                return CastImpl(polymorphic_matcher_or_value,BooleanConstant<internal::ImplicitlyConvertible<M, Matcher<T> >::value>());
            }
        private:
            static Matcher<T> CastImpl(const M& value, BooleanConstant<false>) {
                return Matcher<T>(ImplicitCast_<T>(value));
            }
            static Matcher<T> CastImpl(const M& polymorphic_matcher_or_value, BooleanConstant<true>) {
                return polymorphic_matcher_or_value;
            }
        };
        template <typename T, typename U> class MatcherCastImpl<T, Matcher<U> > {
        public:
            static Matcher<T> Cast(const Matcher<U>& source_matcher) {
                return Matcher<T>(new Impl(source_matcher));
            }
        private:
            class Impl : public MatcherInterface<T> {
            public:
                explicit Impl(const Matcher<U>& source_matcher) : source_matcher_(source_matcher) {}
                virtual bool MatchAndExplain(T x, MatchResultListener* listener) const {
                    return source_matcher_.MatchAndExplain(static_cast<U>(x), listener);
                }
                virtual void DescribeTo(::std::ostream* os) const {
                    source_matcher_.DescribeTo(os);
                }
                virtual void DescribeNegationTo(::std::ostream* os) const {
                    source_matcher_.DescribeNegationTo(os);
                }
            private:
                const Matcher<U> source_matcher_;
                GTEST_DISALLOW_ASSIGN_(Impl);
            };
        };
        template <typename T> class MatcherCastImpl<T, Matcher<T> > {
        public:
            static Matcher<T> Cast(const Matcher<T>& matcher) { return matcher; }
        };
    }
    template <typename T, typename M> inline Matcher<T> MatcherCast(const M& matcher) {
        return internal::MatcherCastImpl<T, M>::Cast(matcher);
    }
    template <typename T> class SafeMatcherCastImpl {
    public:
        template <typename M> static inline Matcher<T> Cast(const M& polymorphic_matcher_or_value) {
        return internal::MatcherCastImpl<T, M>::Cast(polymorphic_matcher_or_value);
        }
        template <typename U> static inline Matcher<T> Cast(const Matcher<U>& matcher) {
            GTEST_COMPILE_ASSERT_((internal::ImplicitlyConvertible<T, U>::value), T_must_be_implicitly_convertible_to_U);
            GTEST_COMPILE_ASSERT_(internal::is_reference<T>::value || !internal::is_reference<U>::value, cannot_convert_non_referentce_arg_to_reference);
            typedef GTEST_REMOVE_REFERENCE_AND_CONST_(T) RawT;
            typedef GTEST_REMOVE_REFERENCE_AND_CONST_(U) RawU;
            const bool kTIsOther = GMOCK_KIND_OF_(RawT) == internal::kOther;
            const bool kUIsOther = GMOCK_KIND_OF_(RawU) == internal::kOther;
            GTEST_COMPILE_ASSERT_(kTIsOther || kUIsOther || (internal::LosslessArithmeticConvertible<RawT, RawU>::value),
                                  conversion_of_arithmetic_types_must_be_lossless);
            return MatcherCast<T>(matcher);
        }
    };
    template <typename T, typename M> inline Matcher<T> SafeMatcherCast(const M& polymorphic_matcher) {
        return SafeMatcherCastImpl<T>::Cast(polymorphic_matcher);
    }
    template <typename T> Matcher<T> A();
    namespace internal {
        inline void PrintIfNotEmpty(const internal::string& explanation,::std::ostream* os) {
            if (explanation != "" &&  os != NULL) *os << ", " << explanation;
        }
        inline bool IsReadableTypeName(const string& type_name) {
            return (type_name.length() <= 20 || type_name.find_first_of("<(") == string::npos);
        }
        template <typename Value, typename T> bool MatchPrintAndExplain(Value& value, const Matcher<T>& matcher, MatchResultListener* listener) {
            if (!listener->IsInterested()) return matcher.Matches(value);
            StringMatchResultListener inner_listener;
            const bool match = matcher.MatchAndExplain(value, &inner_listener);
            UniversalPrint(value, listener->stream());
        #if GTEST_HAS_RTTI
            const string& type_name = GetTypeName<Value>();
            if (IsReadableTypeName(type_name)) *listener->stream() << " (of type " << type_name << ")";
        #endif
            PrintIfNotEmpty(inner_listener.str(), listener->stream());
            return match;
        }
        template <size_t N> class TuplePrefix {
        public:
            template <typename MatcherTuple, typename ValueTuple> static bool Matches(const MatcherTuple& matcher_tuple, const ValueTuple& value_tuple) {
                return TuplePrefix<N - 1>::Matches(matcher_tuple, value_tuple) && get<N - 1>(matcher_tuple).Matches(get<N - 1>(value_tuple));
            }
            template <typename MatcherTuple, typename ValueTuple> static void ExplainMatchFailuresTo(const MatcherTuple& matchers, const ValueTuple& values, ::std::ostream* os) {
                TuplePrefix<N - 1>::ExplainMatchFailuresTo(matchers, values, os);
                typename tuple_element<N - 1, MatcherTuple>::type matcher = get<N - 1>(matchers);
                typedef typename tuple_element<N - 1, ValueTuple>::type Value;
                Value value = get<N - 1>(values);
                StringMatchResultListener listener;
                if (!matcher.MatchAndExplain(value, &listener)) {
                    *os << "  Expected arg #" << N - 1 << ": ";
                    get<N - 1>(matchers).DescribeTo(os);
                    *os << "\n           Actual: ";
                    internal::UniversalPrint(value, os);
                    PrintIfNotEmpty(listener.str(), os);
                    *os << "\n";
                }
            }
        };
        template <> class TuplePrefix<0> {
        public:
            template <typename MatcherTuple, typename ValueTuple> static bool Matches(const MatcherTuple&, const ValueTuple&) {
                return true;
            }
            template <typename MatcherTuple, typename ValueTuple> static void ExplainMatchFailuresTo(const MatcherTuple&, const ValueTuple&, ::std::ostream*) {}
        };
        template <typename MatcherTuple, typename ValueTuple> bool TupleMatches(const MatcherTuple& matcher_tuple, const ValueTuple& value_tuple) {
            GTEST_COMPILE_ASSERT_(tuple_size<MatcherTuple>::value == tuple_size<ValueTuple>::value, matcher_and_value_have_different_numbers_of_fields);
            return TuplePrefix<tuple_size<ValueTuple>::value>::Matches(matcher_tuple, value_tuple);
        }
        template <typename MatcherTuple, typename ValueTuple> void ExplainMatchFailureTupleTo(const MatcherTuple& matchers, const ValueTuple& values, ::std::ostream* os) {
            TuplePrefix<tuple_size<MatcherTuple>::value>::ExplainMatchFailuresTo(matchers, values, os);
        }
        template <typename Tuple, typename Func, typename OutIter> class TransformTupleValuesHelper {
        private:
            typedef ::testing::tuple_size<Tuple> TupleSize;
        public:
            static OutIter Run(Func f, const Tuple& t, OutIter out) {
                return IterateOverTuple<Tuple, TupleSize::value>()(f, t, out);
            }
        private:
            template <typename Tup, size_t kRemainingSize> struct IterateOverTuple {
                OutIter operator() (Func f, const Tup& t, OutIter out) const {
                    *out++ = f(::testing::get<TupleSize::value - kRemainingSize>(t));
                    return IterateOverTuple<Tup, kRemainingSize - 1>()(f, t, out);
                }
            };
            template <typename Tup> struct IterateOverTuple<Tup, 0> {
                OutIter operator() (Func /* f */, const Tup& /* t */, OutIter out) const {
                    return out;
                }
            };
        };
        template <typename Tuple, typename Func, typename OutIter> OutIter TransformTupleValues(Func f, const Tuple& t, OutIter out) {
            return TransformTupleValuesHelper<Tuple, Func, OutIter>::Run(f, t, out);
        }
        template <typename T> class AnyMatcherImpl : public MatcherInterface<T> {
        public:
            virtual bool MatchAndExplain(T /* x */, MatchResultListener* /* listener */) const { return true; }
            virtual void DescribeTo(::std::ostream* os) const { *os << "is anything"; }
            virtual void DescribeNegationTo(::std::ostream* os) const {
                *os << "never matches";
            }
        };
        class AnythingMatcher {
        public:
            template <typename T> operator Matcher<T>() const { return A<T>(); }
        };
        template <typename D, typename Rhs, typename Op> class ComparisonBase {
        public:
            explicit ComparisonBase(const Rhs& rhs) : rhs_(rhs) {}
            template <typename Lhs> operator Matcher<Lhs>() const {
                return MakeMatcher(new Impl<Lhs>(rhs_));
            }
        private:
            template <typename Lhs> class Impl : public MatcherInterface<Lhs> {
            public:
                explicit Impl(const Rhs& rhs) : rhs_(rhs) {}
                virtual bool MatchAndExplain(Lhs lhs, MatchResultListener* /* listener */) const {
                    return Op()(lhs, rhs_);
                }
                virtual void DescribeTo(::std::ostream* os) const {
                    *os << D::Desc() << " ";
                    UniversalPrint(rhs_, os);
                }
                virtual void DescribeNegationTo(::std::ostream* os) const {
                    *os << D::NegatedDesc() <<  " ";
                    UniversalPrint(rhs_, os);
                }
            private:
                Rhs rhs_;
                GTEST_DISALLOW_ASSIGN_(Impl);
            };
            Rhs rhs_;
            GTEST_DISALLOW_ASSIGN_(ComparisonBase);
        };
        template <typename Rhs> class EqMatcher : public ComparisonBase<EqMatcher<Rhs>, Rhs, AnyEq> {
        public:
            explicit EqMatcher(const Rhs& rhs) : ComparisonBase<EqMatcher<Rhs>, Rhs, AnyEq>(rhs) { }
            static const char* Desc() { return "is equal to"; }
            static const char* NegatedDesc() { return "isn't equal to"; }
        };
        template <typename Rhs> class NeMatcher : public ComparisonBase<NeMatcher<Rhs>, Rhs, AnyNe> {
        public:
            explicit NeMatcher(const Rhs& rhs) : ComparisonBase<NeMatcher<Rhs>, Rhs, AnyNe>(rhs) { }
            static const char* Desc() { return "isn't equal to"; }
            static const char* NegatedDesc() { return "is equal to"; }
        };
        template <typename Rhs> class LtMatcher : public ComparisonBase<LtMatcher<Rhs>, Rhs, AnyLt> {
        public:
            explicit LtMatcher(const Rhs& rhs) : ComparisonBase<LtMatcher<Rhs>, Rhs, AnyLt>(rhs) { }
            static const char* Desc() { return "is <"; }
            static const char* NegatedDesc() { return "isn't <"; }
        };
        template <typename Rhs> class GtMatcher : public ComparisonBase<GtMatcher<Rhs>, Rhs, AnyGt> {
        public:
            explicit GtMatcher(const Rhs& rhs) : ComparisonBase<GtMatcher<Rhs>, Rhs, AnyGt>(rhs) { }
            static const char* Desc() { return "is >"; }
            static const char* NegatedDesc() { return "isn't >"; }
        };
        template <typename Rhs> class LeMatcher : public ComparisonBase<LeMatcher<Rhs>, Rhs, AnyLe> {
        public:
            explicit LeMatcher(const Rhs& rhs) : ComparisonBase<LeMatcher<Rhs>, Rhs, AnyLe>(rhs) { }
            static const char* Desc() { return "is <="; }
            static const char* NegatedDesc() { return "isn't <="; }
        };
        template <typename Rhs> class GeMatcher : public ComparisonBase<GeMatcher<Rhs>, Rhs, AnyGe> {
        public:
            explicit GeMatcher(const Rhs& rhs) : ComparisonBase<GeMatcher<Rhs>, Rhs, AnyGe>(rhs) { }
            static const char* Desc() { return "is >="; }
            static const char* NegatedDesc() { return "isn't >="; }
        };
        class IsNullMatcher {
        public:
            template <typename Pointer> bool MatchAndExplain(const Pointer& p, MatchResultListener* /* listener */) const {
            #if GTEST_LANG_CXX11
                return p == nullptr;
            #else
                return GetRawPointer(p) == NULL;
            #endif
            }
            void DescribeTo(::std::ostream* os) const { *os << "is NULL"; }
            void DescribeNegationTo(::std::ostream* os) const {
                *os << "isn't NULL";
            }
        };
        class NotNullMatcher {
        public:
            template <typename Pointer> bool MatchAndExplain(const Pointer& p, MatchResultListener* /* listener */) const {
            #if GTEST_LANG_CXX11
                return p != nullptr;
            #else
                return GetRawPointer(p) != NULL;
            #endif
            }
            void DescribeTo(::std::ostream* os) const { *os << "isn't NULL"; }
            void DescribeNegationTo(::std::ostream* os) const {
                *os << "is NULL";
            }
        };
        template <typename T> class RefMatcher;
        template <typename T> class RefMatcher<T&> {
        public:
            explicit RefMatcher(T& x) : object_(x) {}
            template <typename Super> operator Matcher<Super&>() const {
                return MakeMatcher(new Impl<Super>(object_));
            }
        private:
            template <typename Super> class Impl : public MatcherInterface<Super&> {
            public:
                explicit Impl(Super& x) : object_(x) {}
                virtual bool MatchAndExplain(Super& x, MatchResultListener* listener) const {
                    *listener << "which is located @" << static_cast<const void*>(&x);
                    return &x == &object_;
                }
                virtual void DescribeTo(::std::ostream* os) const {
                    *os << "references the variable ";
                    UniversalPrinter<Super&>::Print(object_, os);
                }
                virtual void DescribeNegationTo(::std::ostream* os) const {
                    *os << "does not reference the variable ";
                    UniversalPrinter<Super&>::Print(object_, os);
                }
            private:
                const Super& object_;
                GTEST_DISALLOW_ASSIGN_(Impl);
            };
            T& object_;
            GTEST_DISALLOW_ASSIGN_(RefMatcher);
        };
        inline bool CaseInsensitiveCStringEquals(const char* lhs, const char* rhs) {
            return String::CaseInsensitiveCStringEquals(lhs, rhs);
        }
        inline bool CaseInsensitiveCStringEquals(const wchar_t* lhs, const wchar_t* rhs) {
            return String::CaseInsensitiveWideCStringEquals(lhs, rhs);
        }
        template <typename StringType> bool CaseInsensitiveStringEquals(const StringType& s1, const StringType& s2) {
            if (!CaseInsensitiveCStringEquals(s1.c_str(), s2.c_str())) return false;
            const typename StringType::value_type nul = 0;
            const size_t i1 = s1.find(nul), i2 = s2.find(nul);
            if (i1 == StringType::npos || i2 == StringType::npos) return i1 == i2;
            return CaseInsensitiveStringEquals(s1.substr(i1 + 1), s2.substr(i2 + 1));
        }
        template <typename StringType> class StrEqualityMatcher {
        public:
            StrEqualityMatcher(const StringType& str, bool expect_eq, bool case_sensitive) : string_(str), expect_eq_(expect_eq), case_sensitive_(case_sensitive) {}
            template <typename CharType> bool MatchAndExplain(CharType* s, MatchResultListener* listener) const {
                if (s == NULL) return !expect_eq_;
                return MatchAndExplain(StringType(s), listener);
            }
            template <typename MatcheeStringType> bool MatchAndExplain(const MatcheeStringType& s, MatchResultListener* /* listener */) const {
                const StringType& s2(s);
                const bool eq = case_sensitive_ ? s2 == string_ : CaseInsensitiveStringEquals(s2, string_);
                return expect_eq_ == eq;
            }
            void DescribeTo(::std::ostream* os) const {
                DescribeToHelper(expect_eq_, os);
            }
            void DescribeNegationTo(::std::ostream* os) const {
                DescribeToHelper(!expect_eq_, os);
            }
        private:
            void DescribeToHelper(bool expect_eq, ::std::ostream* os) const {
                *os << (expect_eq ? "is " : "isn't ");
                *os << "equal to ";
                if (!case_sensitive_) *os << "(ignoring case) ";
                UniversalPrint(string_, os);
            }
            const StringType string_;
            const bool expect_eq_;
            const bool case_sensitive_;
            GTEST_DISALLOW_ASSIGN_(StrEqualityMatcher);
        };
        template <typename StringType> class HasSubstrMatcher {
        public:
            explicit HasSubstrMatcher(const StringType& substring) : substring_(substring) {}
            template <typename CharType> bool MatchAndExplain(CharType* s, MatchResultListener* listener) const {
                return s != NULL && MatchAndExplain(StringType(s), listener);
            }
            template <typename MatcheeStringType> bool MatchAndExplain(const MatcheeStringType& s, MatchResultListener* /* listener */) const {
                const StringType& s2(s);
                return s2.find(substring_) != StringType::npos;
            }
            void DescribeTo(::std::ostream* os) const {
                *os << "has substring ";
                UniversalPrint(substring_, os);
            }
            void DescribeNegationTo(::std::ostream* os) const {
                *os << "has no substring ";
                UniversalPrint(substring_, os);
            }
        private:
            const StringType substring_;
            GTEST_DISALLOW_ASSIGN_(HasSubstrMatcher);
        };
        template <typename StringType> class StartsWithMatcher {
        public:
            explicit StartsWithMatcher(const StringType& prefix) : prefix_(prefix) {}
            template <typename CharType> bool MatchAndExplain(CharType* s, MatchResultListener* listener) const {
                return s != NULL && MatchAndExplain(StringType(s), listener);
            }
            template <typename MatcheeStringType> bool MatchAndExplain(const MatcheeStringType& s, MatchResultListener* /* listener */) const {
                const StringType& s2(s);
                return s2.length() >= prefix_.length() && s2.substr(0, prefix_.length()) == prefix_;
            }
            void DescribeTo(::std::ostream* os) const {
                *os << "starts with ";
                UniversalPrint(prefix_, os);
            }
            void DescribeNegationTo(::std::ostream* os) const {
                *os << "doesn't start with ";
                UniversalPrint(prefix_, os);
            }
        private:
            const StringType prefix_;
            GTEST_DISALLOW_ASSIGN_(StartsWithMatcher);
        };
        template <typename StringType> class EndsWithMatcher {
        public:
            explicit EndsWithMatcher(const StringType& suffix) : suffix_(suffix) {}
            template <typename CharType> bool MatchAndExplain(CharType* s, MatchResultListener* listener) const {
                return s != NULL && MatchAndExplain(StringType(s), listener);
            }
            template <typename MatcheeStringType> bool MatchAndExplain(const MatcheeStringType& s, MatchResultListener* /* listener */) const {
                const StringType& s2(s);
                return s2.length() >= suffix_.length() && s2.substr(s2.length() - suffix_.length()) == suffix_;
            }
            void DescribeTo(::std::ostream* os) const {
                *os << "ends with ";
                UniversalPrint(suffix_, os);
            }
            void DescribeNegationTo(::std::ostream* os) const {
                *os << "doesn't end with ";
                UniversalPrint(suffix_, os);
            }
        private:
            const StringType suffix_;
            GTEST_DISALLOW_ASSIGN_(EndsWithMatcher);
        };
        class MatchesRegexMatcher {
        public:
            MatchesRegexMatcher(const RE* regex, bool full_match) : regex_(regex), full_match_(full_match) {}
            template <typename CharType> bool MatchAndExplain(CharType* s, MatchResultListener* listener) const {
                return s != NULL && MatchAndExplain(internal::string(s), listener);
            }
            template <class MatcheeStringType> bool MatchAndExplain(const MatcheeStringType& s, MatchResultListener* /* listener */) const {
                const internal::string& s2(s);
                return full_match_ ? RE::FullMatch(s2, *regex_) : RE::PartialMatch(s2, *regex_);
            }
            void DescribeTo(::std::ostream* os) const {
                *os << (full_match_ ? "matches" : "contains") << " regular expression ";
                UniversalPrinter<internal::string>::Print(regex_->pattern(), os);
            }
            void DescribeNegationTo(::std::ostream* os) const {
                *os << "doesn't " << (full_match_ ? "match" : "contain") << " regular expression ";
                UniversalPrinter<internal::string>::Print(regex_->pattern(), os);
            }
        private:
            const internal::linked_ptr<const RE> regex_;
            const bool full_match_;
            GTEST_DISALLOW_ASSIGN_(MatchesRegexMatcher);
        };
        template <typename D, typename Op> class PairMatchBase {
        public:
            template <typename T1, typename T2> operator Matcher< ::testing::tuple<T1, T2> >() const {
                return MakeMatcher(new Impl< ::testing::tuple<T1, T2> >);
            }
            template <typename T1, typename T2> operator Matcher<const ::testing::tuple<T1, T2>&>() const {
                return MakeMatcher(new Impl<const ::testing::tuple<T1, T2>&>);
            }
        private:
            static ::std::ostream& GetDesc(::std::ostream& os) {  // NOLINT
                return os << D::Desc();
            }
            template <typename Tuple> class Impl : public MatcherInterface<Tuple> {
            public:
                virtual bool MatchAndExplain(Tuple args, MatchResultListener* /* listener */) const {
                    return Op()(::testing::get<0>(args), ::testing::get<1>(args));
                }
                virtual void DescribeTo(::std::ostream* os) const {
                    *os << "are " << GetDesc;
                }
                virtual void DescribeNegationTo(::std::ostream* os) const {
                    *os << "aren't " << GetDesc;
                }
            };
        };
        class Eq2Matcher : public PairMatchBase<Eq2Matcher, AnyEq> {
        public:
            static const char* Desc() { return "an equal pair"; }
        };
        class Ne2Matcher : public PairMatchBase<Ne2Matcher, AnyNe> {
        public:
            static const char* Desc() { return "an unequal pair"; }
        };
        class Lt2Matcher : public PairMatchBase<Lt2Matcher, AnyLt> {
        public:
            static const char* Desc() { return "a pair where the first < the second"; }
        };
        class Gt2Matcher : public PairMatchBase<Gt2Matcher, AnyGt> {
        public:
            static const char* Desc() { return "a pair where the first > the second"; }
        };
        class Le2Matcher : public PairMatchBase<Le2Matcher, AnyLe> {
        public:
            static const char* Desc() { return "a pair where the first <= the second"; }
        };
        class Ge2Matcher : public PairMatchBase<Ge2Matcher, AnyGe> {
        public:
            static const char* Desc() { return "a pair where the first >= the second"; }
        };
        template <typename T> class NotMatcherImpl : public MatcherInterface<T> {
        public:
            explicit NotMatcherImpl(const Matcher<T>& matcher) : matcher_(matcher) {}
            virtual bool MatchAndExplain(T x, MatchResultListener* listener) const {
                return !matcher_.MatchAndExplain(x, listener);
            }
            virtual void DescribeTo(::std::ostream* os) const {
                matcher_.DescribeNegationTo(os);
            }
            virtual void DescribeNegationTo(::std::ostream* os) const {
                matcher_.DescribeTo(os);
            }
        private:
            const Matcher<T> matcher_;
            GTEST_DISALLOW_ASSIGN_(NotMatcherImpl);
        };
        template <typename InnerMatcher> class NotMatcher {
        public:
            explicit NotMatcher(InnerMatcher matcher) : matcher_(matcher) {}
            template <typename T> operator Matcher<T>() const {
                return Matcher<T>(new NotMatcherImpl<T>(SafeMatcherCast<T>(matcher_)));
            }
        private:
            InnerMatcher matcher_;
            GTEST_DISALLOW_ASSIGN_(NotMatcher);
        };
        template <typename T> class BothOfMatcherImpl : public MatcherInterface<T> {
        public:
            BothOfMatcherImpl(const Matcher<T>& matcher1, const Matcher<T>& matcher2) : matcher1_(matcher1), matcher2_(matcher2) {}
            virtual void DescribeTo(::std::ostream* os) const {
                *os << "(";
                matcher1_.DescribeTo(os);
                *os << ") and (";
                matcher2_.DescribeTo(os);
                *os << ")";
            }
            virtual void DescribeNegationTo(::std::ostream* os) const {
                *os << "(";
                matcher1_.DescribeNegationTo(os);
                *os << ") or (";
                matcher2_.DescribeNegationTo(os);
                *os << ")";
            }
            virtual bool MatchAndExplain(T x, MatchResultListener* listener) const {
                StringMatchResultListener listener1;
                if (!matcher1_.MatchAndExplain(x, &listener1)) {
                    *listener << listener1.str();
                    return false;
                }
                StringMatchResultListener listener2;
                if (!matcher2_.MatchAndExplain(x, &listener2)) {
                    *listener << listener2.str();
                    return false;
                }
                const internal::string s1 = listener1.str();
                const internal::string s2 = listener2.str();
                if (s1 == "") *listener << s2;
                else {
                    *listener << s1;
                    if (s2 != "") *listener << ", and " << s2;
                }
                return true;
            }
        private:
            const Matcher<T> matcher1_;
            const Matcher<T> matcher2_;
            GTEST_DISALLOW_ASSIGN_(BothOfMatcherImpl);
        };
    #if GTEST_LANG_CXX11
        template <int kSize, typename Head, typename... Tail> struct MatcherList {
            typedef MatcherList<kSize - 1, Tail...> MatcherListTail;
            typedef ::std::pair<Head, typename MatcherListTail::ListType> ListType;
            static ListType BuildList(const Head& matcher, const Tail&... tail) {
            return ListType(matcher, MatcherListTail::BuildList(tail...));
            }
            template <typename T, template <typename /* T */> class CombiningMatcher> static Matcher<T> CreateMatcher(const ListType& matchers) {
                return Matcher<T>(new CombiningMatcher<T>(SafeMatcherCast<T>(matchers.first),
                                  MatcherListTail::template CreateMatcher<T, CombiningMatcher>(matchers.second)));
            }
        };
        template <typename Matcher1, typename Matcher2> struct MatcherList<2, Matcher1, Matcher2> {
            typedef ::std::pair<Matcher1, Matcher2> ListType;
            static ListType BuildList(const Matcher1& matcher1, const Matcher2& matcher2) {
                return ::std::pair<Matcher1, Matcher2>(matcher1, matcher2);
            }
            template <typename T, template <typename /* T */> class CombiningMatcher> static Matcher<T> CreateMatcher(const ListType& matchers) {
                return Matcher<T>(new CombiningMatcher<T>(SafeMatcherCast<T>(matchers.first), SafeMatcherCast<T>(matchers.second)));
            }
        };
        template <template <typename T> class CombiningMatcher, typename... Args> class VariadicMatcher {
        public:
            VariadicMatcher(const Args&... matchers) : matchers_(MatcherListType::BuildList(matchers...)) {}
            template <typename T> operator Matcher<T>() const {
                return MatcherListType::template CreateMatcher<T, CombiningMatcher>(matchers_);
            }
        private:
            typedef MatcherList<sizeof...(Args), Args...> MatcherListType;
            const typename MatcherListType::ListType matchers_;
            GTEST_DISALLOW_ASSIGN_(VariadicMatcher);
        };
        template <typename... Args> using AllOfMatcher = VariadicMatcher<BothOfMatcherImpl, Args...>;
    #endif
        template <typename Matcher1, typename Matcher2> class BothOfMatcher {
        public:
            BothOfMatcher(Matcher1 matcher1, Matcher2 matcher2) : matcher1_(matcher1), matcher2_(matcher2) {}
            template <typename T> operator Matcher<T>() const {
                return Matcher<T>(new BothOfMatcherImpl<T>(SafeMatcherCast<T>(matcher1_), SafeMatcherCast<T>(matcher2_)));
            }
        private:
            Matcher1 matcher1_;
            Matcher2 matcher2_;
            GTEST_DISALLOW_ASSIGN_(BothOfMatcher);
        };
        template <typename T> class EitherOfMatcherImpl : public MatcherInterface<T> {
        public:
            EitherOfMatcherImpl(const Matcher<T>& matcher1, const Matcher<T>& matcher2) : matcher1_(matcher1), matcher2_(matcher2) {}
            virtual void DescribeTo(::std::ostream* os) const {
                *os << "(";
                matcher1_.DescribeTo(os);
                *os << ") or (";
                matcher2_.DescribeTo(os);
                *os << ")";
            }
            virtual void DescribeNegationTo(::std::ostream* os) const {
                *os << "(";
                matcher1_.DescribeNegationTo(os);
                *os << ") and (";
                matcher2_.DescribeNegationTo(os);
                *os << ")";
            }
            virtual bool MatchAndExplain(T x, MatchResultListener* listener) const {
                StringMatchResultListener listener1;
                if (matcher1_.MatchAndExplain(x, &listener1)) {
                    *listener << listener1.str();
                    return true;
                }
                StringMatchResultListener listener2;
                if (matcher2_.MatchAndExplain(x, &listener2)) {
                  *listener << listener2.str();
                  return true;
                }
                const internal::string s1 = listener1.str();
                const internal::string s2 = listener2.str();
                if (s1 == "") *listener << s2;
                else {
                  *listener << s1;
                  if (s2 != "") *listener << ", and " << s2;
                }
                return false;
            }
        private:
            const Matcher<T> matcher1_;
            const Matcher<T> matcher2_;
            GTEST_DISALLOW_ASSIGN_(EitherOfMatcherImpl);
        };
    #if GTEST_LANG_CXX11
        template <typename... Args> using AnyOfMatcher = VariadicMatcher<EitherOfMatcherImpl, Args...>;
    #endif
        template <typename Matcher1, typename Matcher2> class EitherOfMatcher {
        public:
            EitherOfMatcher(Matcher1 matcher1, Matcher2 matcher2) : matcher1_(matcher1), matcher2_(matcher2) {}
            template <typename T> operator Matcher<T>() const {
                return Matcher<T>(new EitherOfMatcherImpl<T>(SafeMatcherCast<T>(matcher1_), SafeMatcherCast<T>(matcher2_)));
            }
        private:
            Matcher1 matcher1_;
            Matcher2 matcher2_;
            GTEST_DISALLOW_ASSIGN_(EitherOfMatcher);
        };
        template <typename Predicate> class TrulyMatcher {
        public:
            explicit TrulyMatcher(Predicate pred) : predicate_(pred) {}
            template <typename T> bool MatchAndExplain(T& x, MatchResultListener* /* listener */) const {
            if (predicate_(x)) return true;
            return false;
            }
            void DescribeTo(::std::ostream* os) const {
                *os << "satisfies the given predicate";
            }
            void DescribeNegationTo(::std::ostream* os) const {
                *os << "doesn't satisfy the given predicate";
            }
        private:
            Predicate predicate_;
            GTEST_DISALLOW_ASSIGN_(TrulyMatcher);
        };
        template <typename M> class MatcherAsPredicate {
        public:
            explicit MatcherAsPredicate(M matcher) : matcher_(matcher) {}
            template <typename T> bool operator()(const T& x) const {
                return MatcherCast<const T&>(matcher_).Matches(x);
            }
        private:
            M matcher_;
            GTEST_DISALLOW_ASSIGN_(MatcherAsPredicate);
        };
        template <typename M> class PredicateFormatterFromMatcher {
        public:
            explicit PredicateFormatterFromMatcher(M m) : matcher_(internal::move(m)) {}
            template <typename T> AssertionResult operator()(const char* value_text, const T& x) const {
                const Matcher<const T&> matcher = SafeMatcherCast<const T&>(matcher_);
                StringMatchResultListener listener;
                if (MatchPrintAndExplain(x, matcher, &listener)) return AssertionSuccess();
                ::std::stringstream ss;
                ss << "Value of: " << value_text << "\n" << "Expected: ";
                matcher.DescribeTo(&ss);
                ss << "\n  Actual: " << listener.str();
                return AssertionFailure() << ss.str();
            }
        private:
            const M matcher_;
            GTEST_DISALLOW_ASSIGN_(PredicateFormatterFromMatcher);
        };
        template <typename M> inline PredicateFormatterFromMatcher<M> MakePredicateFormatterFromMatcher(M matcher) {
            return PredicateFormatterFromMatcher<M>(internal::move(matcher));
        }
        template <typename FloatType> class FloatingEqMatcher {
        public:
            FloatingEqMatcher(FloatType expected, bool nan_eq_nan) : expected_(expected), nan_eq_nan_(nan_eq_nan), max_abs_error_(-1) {}
            FloatingEqMatcher(FloatType expected, bool nan_eq_nan, FloatType max_abs_error) : expected_(expected), nan_eq_nan_(nan_eq_nan),
                                                                                              max_abs_error_(max_abs_error) {
                GTEST_CHECK_(max_abs_error >= 0) << ", where max_abs_error is" << max_abs_error;
            }
            template <typename T> class Impl : public MatcherInterface<T> {
            public:
                Impl(FloatType expected, bool nan_eq_nan, FloatType max_abs_error) : expected_(expected), nan_eq_nan_(nan_eq_nan), max_abs_error_(max_abs_error) {}
                virtual bool MatchAndExplain(T value, MatchResultListener* listener) const {
                    const FloatingPoint<FloatType> actual(value), expected(expected_);
                    if (actual.is_nan() || expected.is_nan()) {
                        if (actual.is_nan() && expected.is_nan()) return nan_eq_nan_;
                        return false;
                    }
                    if (HasMaxAbsError()) {
                        if (value == expected_) return true;
                        const FloatType diff = value - expected_;
                        if (fabs(diff) <= max_abs_error_) return true;
                        if (listener->IsInterested()) *listener << "which is " << diff << " from " << expected_;
                        return false;
                    } else return actual.AlmostEquals(expected);
                }
                virtual void DescribeTo(::std::ostream* os) const {
                    const ::std::streamsize old_precision = os->precision(::std::numeric_limits<FloatType>::digits10 + 2);
                    if (FloatingPoint<FloatType>(expected_).is_nan()) {
                        if (nan_eq_nan_) *os << "is NaN";
                        else *os << "never matches";
                    } else {
                        *os << "is approximately " << expected_;
                        if (HasMaxAbsError()) *os << " (absolute error <= " << max_abs_error_ << ")";
                    }
                    os->precision(old_precision);
                }
                virtual void DescribeNegationTo(::std::ostream* os) const {
                    const ::std::streamsize old_precision = os->precision(::std::numeric_limits<FloatType>::digits10 + 2);
                    if (FloatingPoint<FloatType>(expected_).is_nan()) {
                        if (nan_eq_nan_) *os << "isn't NaN";
                        else *os << "is anything";
                    } else {
                        *os << "isn't approximately " << expected_;
                        if (HasMaxAbsError()) *os << " (absolute error > " << max_abs_error_ << ")";
                    }
                    os->precision(old_precision);
                }
            private:
                bool HasMaxAbsError() const {
                  return max_abs_error_ >= 0;
                }
                const FloatType expected_;
                const bool nan_eq_nan_;
                const FloatType max_abs_error_;
                GTEST_DISALLOW_ASSIGN_(Impl);
            };
            operator Matcher<FloatType>() const {
            return MakeMatcher(new Impl<FloatType>(expected_, nan_eq_nan_, max_abs_error_));
            }
            operator Matcher<const FloatType&>() const {
            return MakeMatcher(new Impl<const FloatType&>(expected_, nan_eq_nan_, max_abs_error_));
            }
            operator Matcher<FloatType&>() const {
            return MakeMatcher(new Impl<FloatType&>(expected_, nan_eq_nan_, max_abs_error_));
            }
        private:
            const FloatType expected_;
            const bool nan_eq_nan_;
            const FloatType max_abs_error_;
            GTEST_DISALLOW_ASSIGN_(FloatingEqMatcher);
        };
        template <typename InnerMatcher> class PointeeMatcher {
        public:
            explicit PointeeMatcher(const InnerMatcher& matcher) : matcher_(matcher) {}
            template <typename Pointer> operator Matcher<Pointer>() const {
                return MakeMatcher(new Impl<Pointer>(matcher_));
            }
        private:
            template <typename Pointer> class Impl : public MatcherInterface<Pointer> {
            public:
                typedef typename PointeeOf<GTEST_REMOVE_CONST_(GTEST_REMOVE_REFERENCE_(Pointer))>::type Pointee;
                explicit Impl(const InnerMatcher& matcher) : matcher_(MatcherCast<const Pointee&>(matcher)) {}
                virtual void DescribeTo(::std::ostream* os) const {
                    *os << "points to a value that ";
                    matcher_.DescribeTo(os);
                }
                virtual void DescribeNegationTo(::std::ostream* os) const {
                    *os << "does not point to a value that ";
                    matcher_.DescribeTo(os);
                }
                virtual bool MatchAndExplain(Pointer pointer, MatchResultListener* listener) const {
                    if (GetRawPointer(pointer) == NULL) return false;
                    *listener << "which points to ";
                    return MatchPrintAndExplain(*pointer, matcher_, listener);
                }
            private:
                const Matcher<const Pointee&> matcher_;
                GTEST_DISALLOW_ASSIGN_(Impl);
            };
            const InnerMatcher matcher_;
            GTEST_DISALLOW_ASSIGN_(PointeeMatcher);
        };
        template <typename To> class WhenDynamicCastToMatcherBase {
        public:
            explicit WhenDynamicCastToMatcherBase(const Matcher<To>& matcher) : matcher_(matcher) {}
            void DescribeTo(::std::ostream* os) const {
                GetCastTypeDescription(os);
                matcher_.DescribeTo(os);
            }
            void DescribeNegationTo(::std::ostream* os) const {
                GetCastTypeDescription(os);
                matcher_.DescribeNegationTo(os);
            }
        protected:
            const Matcher<To> matcher_;
            static string GetToName() {
            #if GTEST_HAS_RTTI
                return GetTypeName<To>();
            #else
                return "the target type";
            #endif
            }
        private:
            static void GetCastTypeDescription(::std::ostream* os) {
                *os << "when dynamic_cast to " << GetToName() << ", ";
            }
            GTEST_DISALLOW_ASSIGN_(WhenDynamicCastToMatcherBase);
        };
        template <typename To> class WhenDynamicCastToMatcher : public WhenDynamicCastToMatcherBase<To> {
        public:
            explicit WhenDynamicCastToMatcher(const Matcher<To>& matcher) : WhenDynamicCastToMatcherBase<To>(matcher) {}
            template <typename From> bool MatchAndExplain(From from, MatchResultListener* listener) const {
                To to = dynamic_cast<To>(from);
                return MatchPrintAndExplain(to, this->matcher_, listener);
            }
        };
        template <typename To> class WhenDynamicCastToMatcher<To&> : public WhenDynamicCastToMatcherBase<To&> {
        public:
            explicit WhenDynamicCastToMatcher(const Matcher<To&>& matcher) : WhenDynamicCastToMatcherBase<To&>(matcher) {}
            template <typename From> bool MatchAndExplain(From& from, MatchResultListener* listener) const {
                To* to = dynamic_cast<To*>(&from);
                if (to == NULL) {
                    *listener << "which cannot be dynamic_cast to " << this->GetToName();
                    return false;
                }
                return MatchPrintAndExplain(*to, this->matcher_, listener);
            }
        };
        template <typename Class, typename FieldType> class FieldMatcher {
        public:
            FieldMatcher(FieldType Class::*field, const Matcher<const FieldType&>& matcher) : field_(field), matcher_(matcher) {}
            void DescribeTo(::std::ostream* os) const {
                *os << "is an object whose given field ";
                matcher_.DescribeTo(os);
            }
            void DescribeNegationTo(::std::ostream* os) const {
                *os << "is an object whose given field ";
                matcher_.DescribeNegationTo(os);
            }
            template <typename T> bool MatchAndExplain(const T& value, MatchResultListener* listener) const {
                return MatchAndExplainImpl(typename ::testing::internal::is_pointer<GTEST_REMOVE_CONST_(T)>::type(), value, listener);
            }
        private:
            bool MatchAndExplainImpl(false_type /* is_not_pointer */, const Class& obj, MatchResultListener* listener) const {
                *listener << "whose given field is ";
                return MatchPrintAndExplain(obj.*field_, matcher_, listener);
            }
            bool MatchAndExplainImpl(true_type /* is_pointer */, const Class* p, MatchResultListener* listener) const {
                if (p == NULL) return false;
                *listener << "which points to an object ";
                return MatchAndExplainImpl(false_type(), *p, listener);
            }
            const FieldType Class::*field_;
            const Matcher<const FieldType&> matcher_;
            GTEST_DISALLOW_ASSIGN_(FieldMatcher);
        };
        template <typename Class, typename PropertyType> class PropertyMatcher {
        public:
            typedef GTEST_REFERENCE_TO_CONST_(PropertyType) RefToConstProperty;
            PropertyMatcher(PropertyType (Class::*property)() const, const Matcher<RefToConstProperty>& matcher) : property_(property), matcher_(matcher) {}
            void DescribeTo(::std::ostream* os) const {
                *os << "is an object whose given property ";
                matcher_.DescribeTo(os);
            }
            void DescribeNegationTo(::std::ostream* os) const {
                *os << "is an object whose given property ";
                matcher_.DescribeNegationTo(os);
            }
            template <typename T> bool MatchAndExplain(const T&value, MatchResultListener* listener) const {
                return MatchAndExplainImpl(typename ::testing::internal::is_pointer<GTEST_REMOVE_CONST_(T)>::type(), value, listener);
            }
        private:
            bool MatchAndExplainImpl(false_type /* is_not_pointer */, const Class& obj, MatchResultListener* listener) const {
                *listener << "whose given property is ";
            #if defined(_PREFAST_ ) && _MSC_VER == 1800
                posix::Abort();
                return false;
            #else
                RefToConstProperty result = (obj.*property_)();
                return MatchPrintAndExplain(result, matcher_, listener);
        #endif
            }
            bool MatchAndExplainImpl(true_type /* is_pointer */, const Class* p, MatchResultListener* listener) const {
                if (p == NULL) return false;
                *listener << "which points to an object ";
                return MatchAndExplainImpl(false_type(), *p, listener);
            }
            PropertyType (Class::*property_)() const;
            const Matcher<RefToConstProperty> matcher_;
            GTEST_DISALLOW_ASSIGN_(PropertyMatcher);
        };
        template <typename Functor> struct CallableTraits {
            typedef typename Functor::result_type ResultType;
            typedef Functor StorageType;
            static void CheckIsValid(Functor /* functor */) {}
            template <typename T> static ResultType Invoke(Functor f, T arg) { return f(arg); }
        };
        template <typename ArgType, typename ResType> struct CallableTraits<ResType(*)(ArgType)> {
            typedef ResType ResultType;
            typedef ResType(*StorageType)(ArgType);
            static void CheckIsValid(ResType(*f)(ArgType)) {
                GTEST_CHECK_(f != NULL) << "NULL function pointer is passed into ResultOf().";
            }
            template <typename T> static ResType Invoke(ResType(*f)(ArgType), T arg) {
                return (*f)(arg);
            }
        };
        template <typename Callable> class ResultOfMatcher {
        public:
            typedef typename CallableTraits<Callable>::ResultType ResultType;
            ResultOfMatcher(Callable callable, const Matcher<ResultType>& matcher) : callable_(callable), matcher_(matcher) {
            CallableTraits<Callable>::CheckIsValid(callable_);
            }
            template <typename T> operator Matcher<T>() const {
            return Matcher<T>(new Impl<T>(callable_, matcher_));
            }
        private:
            typedef typename CallableTraits<Callable>::StorageType CallableStorageType;
            template <typename T> class Impl : public MatcherInterface<T> {
            public:
                Impl(CallableStorageType callable, const Matcher<ResultType>& matcher) : callable_(callable), matcher_(matcher) {}
                virtual void DescribeTo(::std::ostream* os) const {
                    *os << "is mapped by the given callable to a value that ";
                    matcher_.DescribeTo(os);
                }
                virtual void DescribeNegationTo(::std::ostream* os) const {
                    *os << "is mapped by the given callable to a value that ";
                    matcher_.DescribeNegationTo(os);
                }
                virtual bool MatchAndExplain(T obj, MatchResultListener* listener) const {
                    *listener << "which is mapped by the given callable to ";
                    ResultType result = CallableTraits<Callable>::template Invoke<T>(callable_, obj);
                    return MatchPrintAndExplain(result, matcher_, listener);
                }
            private:
                mutable CallableStorageType callable_;
                const Matcher<ResultType> matcher_;
                GTEST_DISALLOW_ASSIGN_(Impl);
            };
            const CallableStorageType callable_;
            const Matcher<ResultType> matcher_;
            GTEST_DISALLOW_ASSIGN_(ResultOfMatcher);
        };
        template <typename SizeMatcher> class SizeIsMatcher {
        public:
            explicit SizeIsMatcher(const SizeMatcher& size_matcher) : size_matcher_(size_matcher) {}
            template <typename Container> operator Matcher<Container>() const {
            return MakeMatcher(new Impl<Container>(size_matcher_));
            }
            template <typename Container> class Impl : public MatcherInterface<Container> {
            public:
                typedef internal::StlContainerView<GTEST_REMOVE_REFERENCE_AND_CONST_(Container)> ContainerView;
                typedef typename ContainerView::type::size_type SizeType;
                explicit Impl(const SizeMatcher& size_matcher) : size_matcher_(MatcherCast<SizeType>(size_matcher)) {}
                virtual void DescribeTo(::std::ostream* os) const {
                    *os << "size ";
                    size_matcher_.DescribeTo(os);
                }
                virtual void DescribeNegationTo(::std::ostream* os) const {
                    *os << "size ";
                    size_matcher_.DescribeNegationTo(os);
                }
                virtual bool MatchAndExplain(Container container, MatchResultListener* listener) const {
                    SizeType size = container.size();
                    StringMatchResultListener size_listener;
                    const bool result = size_matcher_.MatchAndExplain(size, &size_listener);
                    *listener << "whose size " << size << (result ? " matches" : " doesn't match");
                    PrintIfNotEmpty(size_listener.str(), listener->stream());
                    return result;
                }
            private:
                const Matcher<SizeType> size_matcher_;
                GTEST_DISALLOW_ASSIGN_(Impl);
            };
        private:
            const SizeMatcher size_matcher_;
            GTEST_DISALLOW_ASSIGN_(SizeIsMatcher);
        };
        template <typename DistanceMatcher> class BeginEndDistanceIsMatcher {
        public:
            explicit BeginEndDistanceIsMatcher(const DistanceMatcher& distance_matcher) : distance_matcher_(distance_matcher) {}
            template <typename Container> operator Matcher<Container>() const {
                return MakeMatcher(new Impl<Container>(distance_matcher_));
            }
            template <typename Container> class Impl : public MatcherInterface<Container> {
            public:
                typedef internal::StlContainerView<GTEST_REMOVE_REFERENCE_AND_CONST_(Container)> ContainerView;
                typedef typename std::iterator_traits<typename ContainerView::type::const_iterator>::difference_type DistanceType;
                explicit Impl(const DistanceMatcher& distance_matcher) : distance_matcher_(MatcherCast<DistanceType>(distance_matcher)) {}
                virtual void DescribeTo(::std::ostream* os) const {
                    *os << "distance between begin() and end() ";
                    distance_matcher_.DescribeTo(os);
                }
                virtual void DescribeNegationTo(::std::ostream* os) const {
                    *os << "distance between begin() and end() ";
                    distance_matcher_.DescribeNegationTo(os);
                }
                virtual bool MatchAndExplain(Container container, MatchResultListener* listener) const {
                #if GTEST_HAS_STD_BEGIN_AND_END_
                    using std::begin;
                    using std::end;
                    DistanceType distance = std::distance(begin(container), end(container));
                #else
                    DistanceType distance = std::distance(container.begin(), container.end());
                #endif
                    StringMatchResultListener distance_listener;
                    const bool result = distance_matcher_.MatchAndExplain(distance, &distance_listener);
                    *listener << "whose distance between begin() and end() " << distance << (result ? " matches" : " doesn't match");
                    PrintIfNotEmpty(distance_listener.str(), listener->stream());
                    return result;
                }
            private:
                const Matcher<DistanceType> distance_matcher_;
                GTEST_DISALLOW_ASSIGN_(Impl);
            };
        private:
            const DistanceMatcher distance_matcher_;
            GTEST_DISALLOW_ASSIGN_(BeginEndDistanceIsMatcher);
        };
        template <typename Container> class ContainerEqMatcher {
        public:
            typedef internal::StlContainerView<Container> View;
            typedef typename View::type StlContainer;
            typedef typename View::const_reference StlContainerReference;
            explicit ContainerEqMatcher(const Container& expected) : expected_(View::Copy(expected)) {
                (void)testing::StaticAssertTypeEq<Container, GTEST_REMOVE_REFERENCE_AND_CONST_(Container)>();
            }
            void DescribeTo(::std::ostream* os) const {
                *os << "equals ";
                UniversalPrint(expected_, os);
            }
            void DescribeNegationTo(::std::ostream* os) const {
                *os << "does not equal ";
                UniversalPrint(expected_, os);
            }
            template <typename LhsContainer> bool MatchAndExplain(const LhsContainer& lhs, MatchResultListener* listener) const {
                typedef internal::StlContainerView<GTEST_REMOVE_CONST_(LhsContainer)> LhsView;
                typedef typename LhsView::type LhsStlContainer;
                StlContainerReference lhs_stl_container = LhsView::ConstReference(lhs);
                if (lhs_stl_container == expected_) return true;
                ::std::ostream* const os = listener->stream();
                if (os != NULL) {
                    bool printed_header = false;
                    for (typename LhsStlContainer::const_iterator it = lhs_stl_container.begin(); it != lhs_stl_container.end(); ++it) {
                        if (internal::ArrayAwareFind(expected_.begin(), expected_.end(), *it) == expected_.end()) {
                            if (printed_header) *os << ", ";
                            else {
                                *os << "which has these unexpected elements: ";
                                printed_header = true;
                            }
                            UniversalPrint(*it, os);
                        }
                    }
                    bool printed_header2 = false;
                    for (typename StlContainer::const_iterator it = expected_.begin(); it != expected_.end(); ++it) {
                        if (internal::ArrayAwareFind(lhs_stl_container.begin(), lhs_stl_container.end(), *it) == lhs_stl_container.end()) {
                            if (printed_header2) *os << ", ";
                            else {
                                *os << (printed_header ? ",\nand" : "which") << " doesn't have these expected elements: ";
                                printed_header2 = true;
                            }
                            UniversalPrint(*it, os);
                        }
                    }
                }
                return false;
            }
        private:
            const StlContainer expected_;
            GTEST_DISALLOW_ASSIGN_(ContainerEqMatcher);
        };
        struct LessComparator {
            template <typename T, typename U> bool operator()(const T& lhs, const U& rhs) const { return lhs < rhs; }
        };
        template <typename Comparator, typename ContainerMatcher> class WhenSortedByMatcher {
        public:
            WhenSortedByMatcher(const Comparator& comparator, const ContainerMatcher& matcher) : comparator_(comparator), matcher_(matcher) {}
            template <typename LhsContainer> operator Matcher<LhsContainer>() const {
                return MakeMatcher(new Impl<LhsContainer>(comparator_, matcher_));
            }
            template <typename LhsContainer> class Impl : public MatcherInterface<LhsContainer> {
            public:
                typedef internal::StlContainerView<GTEST_REMOVE_REFERENCE_AND_CONST_(LhsContainer)> LhsView;
                typedef typename LhsView::type LhsStlContainer;
                typedef typename LhsView::const_reference LhsStlContainerReference;
                typedef typename RemoveConstFromKey<typename LhsStlContainer::value_type>::type LhsValue;
                Impl(const Comparator& comparator, const ContainerMatcher& matcher) : comparator_(comparator), matcher_(matcher) {}
                virtual void DescribeTo(::std::ostream* os) const {
                    *os << "(when sorted) ";
                    matcher_.DescribeTo(os);
                }
                virtual void DescribeNegationTo(::std::ostream* os) const {
                    *os << "(when sorted) ";
                    matcher_.DescribeNegationTo(os);
                }
                virtual bool MatchAndExplain(LhsContainer lhs, MatchResultListener* listener) const {
                    LhsStlContainerReference lhs_stl_container = LhsView::ConstReference(lhs);
                    ::std::vector<LhsValue> sorted_container(lhs_stl_container.begin(), lhs_stl_container.end());
                    ::std::sort(sorted_container.begin(), sorted_container.end(), comparator_);
                    if (!listener->IsInterested()) return matcher_.Matches(sorted_container);
                    *listener << "which is ";
                    UniversalPrint(sorted_container, listener->stream());
                    *listener << " when sorted";
                    StringMatchResultListener inner_listener;
                    const bool match = matcher_.MatchAndExplain(sorted_container, &inner_listener);
                    PrintIfNotEmpty(inner_listener.str(), listener->stream());
                    return match;
                }
            private:
                const Comparator comparator_;
                const Matcher<const ::std::vector<LhsValue>&> matcher_;
                GTEST_DISALLOW_COPY_AND_ASSIGN_(Impl);
            };
        private:
            const Comparator comparator_;
            const ContainerMatcher matcher_;
            GTEST_DISALLOW_ASSIGN_(WhenSortedByMatcher);
        };
        template <typename TupleMatcher, typename RhsContainer> class PointwiseMatcher {
        public:
            typedef internal::StlContainerView<RhsContainer> RhsView;
            typedef typename RhsView::type RhsStlContainer;
            typedef typename RhsStlContainer::value_type RhsValue;
            PointwiseMatcher(const TupleMatcher& tuple_matcher, const RhsContainer& rhs) : tuple_matcher_(tuple_matcher), rhs_(RhsView::Copy(rhs)) {
                (void)testing::StaticAssertTypeEq<RhsContainer, GTEST_REMOVE_REFERENCE_AND_CONST_(RhsContainer)>();
            }
            template <typename LhsContainer> operator Matcher<LhsContainer>() const {
                return MakeMatcher(new Impl<LhsContainer>(tuple_matcher_, rhs_));
            }
            template <typename LhsContainer> class Impl : public MatcherInterface<LhsContainer> {
            public:
                typedef internal::StlContainerView<GTEST_REMOVE_REFERENCE_AND_CONST_(LhsContainer)> LhsView;
                typedef typename LhsView::type LhsStlContainer;
                typedef typename LhsView::const_reference LhsStlContainerReference;
                typedef typename LhsStlContainer::value_type LhsValue;
                typedef ::testing::tuple<const LhsValue&, const RhsValue&> InnerMatcherArg;
                Impl(const TupleMatcher& tuple_matcher, const RhsStlContainer& rhs) :
                     mono_tuple_matcher_(SafeMatcherCast<InnerMatcherArg>(tuple_matcher)), rhs_(rhs) {}
                virtual void DescribeTo(::std::ostream* os) const {
                    *os << "contains " << rhs_.size() << " values, where each value and its corresponding value in ";
                    UniversalPrinter<RhsStlContainer>::Print(rhs_, os);
                    *os << " ";
                    mono_tuple_matcher_.DescribeTo(os);
                }
                virtual void DescribeNegationTo(::std::ostream* os) const {
                    *os << "doesn't contain exactly " << rhs_.size() << " values, or contains a value x at some index i" << " where x and the i-th value of ";
                    UniversalPrint(rhs_, os);
                    *os << " ";
                    mono_tuple_matcher_.DescribeNegationTo(os);
                }
                virtual bool MatchAndExplain(LhsContainer lhs, MatchResultListener* listener) const {
                    LhsStlContainerReference lhs_stl_container = LhsView::ConstReference(lhs);
                    const size_t actual_size = lhs_stl_container.size();
                    if (actual_size != rhs_.size()) {
                    *listener << "which contains " << actual_size << " values";
                    return false;
                    }
                    typename LhsStlContainer::const_iterator left = lhs_stl_container.begin();
                    typename RhsStlContainer::const_iterator right = rhs_.begin();
                    for (size_t i = 0; i != actual_size; ++i, ++left, ++right) {
                        const InnerMatcherArg value_pair(*left, *right);
                        if (listener->IsInterested()) {
                            StringMatchResultListener inner_listener;
                            if (!mono_tuple_matcher_.MatchAndExplain(value_pair, &inner_listener)) {
                            *listener << "where the value pair (";
                            UniversalPrint(*left, listener->stream());
                            *listener << ", ";
                            UniversalPrint(*right, listener->stream());
                            *listener << ") at index #" << i << " don't match";
                            PrintIfNotEmpty(inner_listener.str(), listener->stream());
                            return false;
                            }
                        } else {
                            if (!mono_tuple_matcher_.Matches(value_pair)) return false;
                        }
                    }
                    return true;
                }
            private:
                const Matcher<InnerMatcherArg> mono_tuple_matcher_;
                const RhsStlContainer rhs_;
                GTEST_DISALLOW_ASSIGN_(Impl);
            };
        private:
            const TupleMatcher tuple_matcher_;
            const RhsStlContainer rhs_;
            GTEST_DISALLOW_ASSIGN_(PointwiseMatcher);
        };
        template <typename Container> class QuantifierMatcherImpl : public MatcherInterface<Container> {
        public:
            typedef GTEST_REMOVE_REFERENCE_AND_CONST_(Container) RawContainer;
            typedef StlContainerView<RawContainer> View;
            typedef typename View::type StlContainer;
            typedef typename View::const_reference StlContainerReference;
            typedef typename StlContainer::value_type Element;
            template <typename InnerMatcher> explicit QuantifierMatcherImpl(InnerMatcher inner_matcher) : inner_matcher_(testing::SafeMatcherCast<const Element&>(inner_matcher)) {}
            bool MatchAndExplainImpl(bool all_elements_should_match, Container container, MatchResultListener* listener) const {
                StlContainerReference stl_container = View::ConstReference(container);
                size_t i = 0;
                for (typename StlContainer::const_iterator it = stl_container.begin(); it != stl_container.end(); ++it, ++i) {
                    StringMatchResultListener inner_listener;
                    const bool matches = inner_matcher_.MatchAndExplain(*it, &inner_listener);
                    if (matches != all_elements_should_match) {
                    *listener << "whose element #" << i << (matches ? " matches" : " doesn't match");
                    PrintIfNotEmpty(inner_listener.str(), listener->stream());
                    return !all_elements_should_match;
                    }
                }
                return all_elements_should_match;
            }
        protected:
            const Matcher<const Element&> inner_matcher_;
            GTEST_DISALLOW_ASSIGN_(QuantifierMatcherImpl);
        };
        template <typename Container> class ContainsMatcherImpl : public QuantifierMatcherImpl<Container> {
        public:
            template <typename InnerMatcher> explicit ContainsMatcherImpl(InnerMatcher inner_matcher) : QuantifierMatcherImpl<Container>(inner_matcher) {}
            virtual void DescribeTo(::std::ostream* os) const {
                *os << "contains at least one element that ";
                this->inner_matcher_.DescribeTo(os);
            }
            virtual void DescribeNegationTo(::std::ostream* os) const {
                *os << "doesn't contain any element that ";
                this->inner_matcher_.DescribeTo(os);
            }
            virtual bool MatchAndExplain(Container container, MatchResultListener* listener) const {
                return this->MatchAndExplainImpl(false, container, listener);
            }
        private:
            GTEST_DISALLOW_ASSIGN_(ContainsMatcherImpl);
        };
        template <typename Container> class EachMatcherImpl : public QuantifierMatcherImpl<Container> {
        public:
            template <typename InnerMatcher> explicit EachMatcherImpl(InnerMatcher inner_matcher) : QuantifierMatcherImpl<Container>(inner_matcher) {}
            virtual void DescribeTo(::std::ostream* os) const {
                *os << "only contains elements that ";
                this->inner_matcher_.DescribeTo(os);
            }
            virtual void DescribeNegationTo(::std::ostream* os) const {
                *os << "contains some element that ";
                this->inner_matcher_.DescribeNegationTo(os);
            }
            virtual bool MatchAndExplain(Container container, MatchResultListener* listener) const {
                return this->MatchAndExplainImpl(true, container, listener);
            }
        private:
            GTEST_DISALLOW_ASSIGN_(EachMatcherImpl);
        };
        template <typename M> class ContainsMatcher {
        public:
            explicit ContainsMatcher(M m) : inner_matcher_(m) {}
            template <typename Container> operator Matcher<Container>() const {
            return MakeMatcher(new ContainsMatcherImpl<Container>(inner_matcher_));
            }
        private:
            const M inner_matcher_;
            GTEST_DISALLOW_ASSIGN_(ContainsMatcher);
        };
        template <typename M> class EachMatcher {
        public:
            explicit EachMatcher(M m) : inner_matcher_(m) {}
            template <typename Container> operator Matcher<Container>() const {
                return MakeMatcher(new EachMatcherImpl<Container>(inner_matcher_));
            }
        private:
            const M inner_matcher_;
            GTEST_DISALLOW_ASSIGN_(EachMatcher);
        };
        template <typename PairType> class KeyMatcherImpl : public MatcherInterface<PairType> {
        public:
            typedef GTEST_REMOVE_REFERENCE_AND_CONST_(PairType) RawPairType;
            typedef typename RawPairType::first_type KeyType;
            template <typename InnerMatcher> explicit KeyMatcherImpl(InnerMatcher inner_matcher) :
                                                   inner_matcher_(testing::SafeMatcherCast<const KeyType&>(inner_matcher)) {}
            virtual bool MatchAndExplain(PairType key_value, MatchResultListener* listener) const {
                StringMatchResultListener inner_listener;
                const bool match = inner_matcher_.MatchAndExplain(key_value.first, &inner_listener);
                const internal::string explanation = inner_listener.str();
                if (explanation != "") *listener << "whose first field is a value " << explanation;
                return match;
            }
            virtual void DescribeTo(::std::ostream* os) const {
                *os << "has a key that ";
                inner_matcher_.DescribeTo(os);
            }
            virtual void DescribeNegationTo(::std::ostream* os) const {
                *os << "doesn't have a key that ";
                inner_matcher_.DescribeTo(os);
            }
        private:
            const Matcher<const KeyType&> inner_matcher_;
            GTEST_DISALLOW_ASSIGN_(KeyMatcherImpl);
        };
        template <typename M> class KeyMatcher {
        public:
            explicit KeyMatcher(M m) : matcher_for_key_(m) {}
            template <typename PairType> operator Matcher<PairType>() const {
                return MakeMatcher(new KeyMatcherImpl<PairType>(matcher_for_key_));
            }
        private:
            const M matcher_for_key_;
            GTEST_DISALLOW_ASSIGN_(KeyMatcher);
        };
        template <typename PairType> class PairMatcherImpl : public MatcherInterface<PairType> {
        public:
            typedef GTEST_REMOVE_REFERENCE_AND_CONST_(PairType) RawPairType;
            typedef typename RawPairType::first_type FirstType;
            typedef typename RawPairType::second_type SecondType;
            template <typename FirstMatcher, typename SecondMatcher> PairMatcherImpl(FirstMatcher first_matcher, SecondMatcher second_matcher) :
                                                         first_matcher_(testing::SafeMatcherCast<const FirstType&>(first_matcher)),
                                                         second_matcher_(testing::SafeMatcherCast<const SecondType&>(second_matcher)) {}
            virtual void DescribeTo(::std::ostream* os) const {
                *os << "has a first field that ";
                first_matcher_.DescribeTo(os);
                *os << ", and has a second field that ";
                second_matcher_.DescribeTo(os);
            }
            virtual void DescribeNegationTo(::std::ostream* os) const {
                *os << "has a first field that ";
                first_matcher_.DescribeNegationTo(os);
                *os << ", or has a second field that ";
                second_matcher_.DescribeNegationTo(os);
            }
            virtual bool MatchAndExplain(PairType a_pair, MatchResultListener* listener) const {
                if (!listener->IsInterested()) {
                    return first_matcher_.Matches(a_pair.first) && second_matcher_.Matches(a_pair.second);
                }
                StringMatchResultListener first_inner_listener;
                if (!first_matcher_.MatchAndExplain(a_pair.first, &first_inner_listener)) {
                    *listener << "whose first field does not match";
                    PrintIfNotEmpty(first_inner_listener.str(), listener->stream());
                    return false;
                }
                StringMatchResultListener second_inner_listener;
                if (!second_matcher_.MatchAndExplain(a_pair.second, &second_inner_listener)) {
                    *listener << "whose second field does not match";
                    PrintIfNotEmpty(second_inner_listener.str(), listener->stream());
                    return false;
                }
                ExplainSuccess(first_inner_listener.str(), second_inner_listener.str(), listener);
                return true;
            }
        private:
            void ExplainSuccess(const internal::string& first_explanation, const internal::string& second_explanation,
                                MatchResultListener* listener) const {
                *listener << "whose both fields match";
                if (first_explanation != "") {
                    *listener << ", where the first field is a value " << first_explanation;
                }
                if (second_explanation != "") {
                    *listener << ", ";
                    if (first_explanation != "") *listener << "and ";
                    else *listener << "where ";
                    *listener << "the second field is a value " << second_explanation;
                }
            }
            const Matcher<const FirstType&> first_matcher_;
            const Matcher<const SecondType&> second_matcher_;
            GTEST_DISALLOW_ASSIGN_(PairMatcherImpl);
        };
        template <typename FirstMatcher, typename SecondMatcher> class PairMatcher {
        public:
            PairMatcher(FirstMatcher first_matcher, SecondMatcher second_matcher) : first_matcher_(first_matcher),
                        second_matcher_(second_matcher) {}
            template <typename PairType> operator Matcher<PairType> () const {
                return MakeMatcher(new PairMatcherImpl<PairType>(first_matcher_, second_matcher_));
            }
        private:
            const FirstMatcher first_matcher_;
            const SecondMatcher second_matcher_;
            GTEST_DISALLOW_ASSIGN_(PairMatcher);
        };
        template <typename Container> class ElementsAreMatcherImpl : public MatcherInterface<Container> {
        public:
            typedef GTEST_REMOVE_REFERENCE_AND_CONST_(Container) RawContainer;
            typedef internal::StlContainerView<RawContainer> View;
            typedef typename View::type StlContainer;
            typedef typename View::const_reference StlContainerReference;
            typedef typename StlContainer::value_type Element;
            template <typename InputIter> ElementsAreMatcherImpl(InputIter first, InputIter last) {
                while (first != last) {
                    matchers_.push_back(MatcherCast<const Element&>(*first++));
                }
            }
            virtual void DescribeTo(::std::ostream* os) const {
                if (count() == 0) *os << "is empty";
                else if (count() == 1) {
                    *os << "has 1 element that ";
                    matchers_[0].DescribeTo(os);
                } else {
                    *os << "has " << Elements(count()) << " where\n";
                    for (size_t i = 0; i != count(); ++i) {
                    *os << "element #" << i << " ";
                    matchers_[i].DescribeTo(os);
                    if (i + 1 < count()) *os << ",\n";
                    }
                }
            }
            virtual void DescribeNegationTo(::std::ostream* os) const {
                if (count() == 0) {
                    *os << "isn't empty";
                    return;
                }
                *os << "doesn't have " << Elements(count()) << ", or\n";
                for (size_t i = 0; i != count(); ++i) {
                    *os << "element #" << i << " ";
                    matchers_[i].DescribeNegationTo(os);
                    if (i + 1 < count()) *os << ", or\n";
                }
            }
            virtual bool MatchAndExplain(Container container, MatchResultListener* listener) const {
                const bool listener_interested = listener->IsInterested();
                ::std::vector<internal::string> explanations(count());
                StlContainerReference stl_container = View::ConstReference(container);
                typename StlContainer::const_iterator it = stl_container.begin();
                size_t exam_pos = 0;
                bool mismatch_found = false;
                for (; it != stl_container.end() && exam_pos != count(); ++it, ++exam_pos) {
                    bool match;
                    if (listener_interested) {
                        StringMatchResultListener s;
                        match = matchers_[exam_pos].MatchAndExplain(*it, &s);
                        explanations[exam_pos] = s.str();
                    } else match = matchers_[exam_pos].Matches(*it);
                    if (!match) {
                        mismatch_found = true;
                        break;
                    }
                }
                size_t actual_count = exam_pos;
                for (; it != stl_container.end(); ++it) {
                    ++actual_count;
                }
                if (actual_count != count()) {
                    if (listener_interested && (actual_count != 0)) {
                        *listener << "which has " << Elements(actual_count);
                    }
                    return false;
                }
                if (mismatch_found) {
                    if (listener_interested) {
                        *listener << "whose element #" << exam_pos << " doesn't match";
                        PrintIfNotEmpty(explanations[exam_pos], listener->stream());
                    }
                    return false;
                }
                if (listener_interested) {
                    bool reason_printed = false;
                    for (size_t i = 0; i != count(); ++i) {
                        const internal::string& s = explanations[i];
                        if (!s.empty()) {
                            if (reason_printed) *listener << ",\nand ";
                            *listener << "whose element #" << i << " matches, " << s;
                            reason_printed = true;
                        }
                    }
                }
                return true;
            }
        private:
            static Message Elements(size_t count) {
                return Message() << count << (count == 1 ? " element" : " elements");
            }
            size_t count() const { return matchers_.size(); }
            ::std::vector<Matcher<const Element&> > matchers_;
            GTEST_DISALLOW_ASSIGN_(ElementsAreMatcherImpl);
        };
        class GTEST_API_ MatchMatrix {
        public:
            MatchMatrix(size_t num_elements, size_t num_matchers) : num_elements_(num_elements), num_matchers_(num_matchers),
                                                                    matched_(num_elements_* num_matchers_, 0) {}
            size_t LhsSize() const { return num_elements_; }
            size_t RhsSize() const { return num_matchers_; }
            bool HasEdge(size_t ilhs, size_t irhs) const {
                return matched_[SpaceIndex(ilhs, irhs)] == 1;
            }
            void SetEdge(size_t ilhs, size_t irhs, bool b) {
                matched_[SpaceIndex(ilhs, irhs)] = b ? 1 : 0;
            }
            bool NextGraph();
            void Randomize();
            string DebugString() const;
        private:
            size_t SpaceIndex(size_t ilhs, size_t irhs) const {
                return ilhs * num_matchers_ + irhs;
            }
            size_t num_elements_;
            size_t num_matchers_;
            ::std::vector<char> matched_;
        };
        typedef ::std::pair<size_t, size_t> ElementMatcherPair;
        typedef ::std::vector<ElementMatcherPair> ElementMatcherPairs;
        GTEST_API_ ElementMatcherPairs FindMaxBipartiteMatching(const MatchMatrix& g);
        GTEST_API_ bool FindPairing(const MatchMatrix& matrix, MatchResultListener* listener);
        class GTEST_API_ UnorderedElementsAreMatcherImplBase {
        protected:
            typedef ::std::vector<const MatcherDescriberInterface*> MatcherDescriberVec;
            void DescribeToImpl(::std::ostream* os) const;
            void DescribeNegationToImpl(::std::ostream* os) const;
            bool VerifyAllElementsAndMatchersAreMatched(const ::std::vector<string>& element_printouts, const MatchMatrix& matrix, MatchResultListener* listener) const;
            MatcherDescriberVec& matcher_describers() {
                return matcher_describers_;
            }
            static Message Elements(size_t n) {
                return Message() << n << " element" << (n == 1 ? "" : "s");
            }
        private:
            MatcherDescriberVec matcher_describers_;
            GTEST_DISALLOW_ASSIGN_(UnorderedElementsAreMatcherImplBase);
        };
        template <typename Container> class UnorderedElementsAreMatcherImpl : public MatcherInterface<Container>, public UnorderedElementsAreMatcherImplBase {
        public:
            typedef GTEST_REMOVE_REFERENCE_AND_CONST_(Container) RawContainer;
            typedef internal::StlContainerView<RawContainer> View;
            typedef typename View::type StlContainer;
            typedef typename View::const_reference StlContainerReference;
            typedef typename StlContainer::const_iterator StlContainerConstIterator;
            typedef typename StlContainer::value_type Element;
            template <typename InputIter> UnorderedElementsAreMatcherImpl(InputIter first, InputIter last) {
                for (; first != last; ++first) {
                    matchers_.push_back(MatcherCast<const Element&>(*first));
                    matcher_describers().push_back(matchers_.back().GetDescriber());
                }
            }
            virtual void DescribeTo(::std::ostream* os) const {
                return UnorderedElementsAreMatcherImplBase::DescribeToImpl(os);
            }
            virtual void DescribeNegationTo(::std::ostream* os) const {
                return UnorderedElementsAreMatcherImplBase::DescribeNegationToImpl(os);
            }
            virtual bool MatchAndExplain(Container container, MatchResultListener* listener) const {
                StlContainerReference stl_container = View::ConstReference(container);
                ::std::vector<string> element_printouts;
                MatchMatrix matrix = AnalyzeElements(stl_container.begin(), stl_container.end(), &element_printouts, listener);
                const size_t actual_count = matrix.LhsSize();
                if (actual_count == 0 && matchers_.empty()) return true;
                if (actual_count != matchers_.size()) {
                    if (actual_count != 0 && listener->IsInterested()) *listener << "which has " << Elements(actual_count);
                    return false;
                }
                return VerifyAllElementsAndMatchersAreMatched(element_printouts, matrix, listener) && FindPairing(matrix, listener);
            }
        private:
            typedef ::std::vector<Matcher<const Element&> > MatcherVec;
            template <typename ElementIter> MatchMatrix AnalyzeElements(ElementIter elem_first, ElementIter elem_last, ::std::vector<string>* element_printouts,
                                                      MatchResultListener* listener) const {
                element_printouts->clear();
                ::std::vector<char> did_match;
                size_t num_elements = 0;
                for (; elem_first != elem_last; ++num_elements, ++elem_first) {
                    if (listener->IsInterested()) element_printouts->push_back(PrintToString(*elem_first));
                    for (size_t irhs = 0; irhs != matchers_.size(); ++irhs) {
                        did_match.push_back(Matches(matchers_[irhs])(*elem_first));
                    }
                }
                MatchMatrix matrix(num_elements, matchers_.size());
                ::std::vector<char>::const_iterator did_match_iter = did_match.begin();
                for (size_t ilhs = 0; ilhs != num_elements; ++ilhs) {
                    for (size_t irhs = 0; irhs != matchers_.size(); ++irhs) {
                        matrix.SetEdge(ilhs, irhs, *did_match_iter++ != 0);
                    }
                }
                return matrix;
            }
            MatcherVec matchers_;
            GTEST_DISALLOW_ASSIGN_(UnorderedElementsAreMatcherImpl);
        };
        template <typename Target> struct CastAndAppendTransform {
            template <typename Arg> Matcher<Target> operator()(const Arg& a) const {
                return MatcherCast<Target>(a);
            }
        };
        template <typename MatcherTuple> class UnorderedElementsAreMatcher {
        public:
            explicit UnorderedElementsAreMatcher(const MatcherTuple& args) : matchers_(args) {}
            template <typename Container> operator Matcher<Container>() const {
                typedef GTEST_REMOVE_REFERENCE_AND_CONST_(Container) RawContainer;
                typedef typename internal::StlContainerView<RawContainer>::type View;
                typedef typename View::value_type Element;
                typedef ::std::vector<Matcher<const Element&> > MatcherVec;
                MatcherVec matchers;
                matchers.reserve(::testing::tuple_size<MatcherTuple>::value);
                TransformTupleValues(CastAndAppendTransform<const Element&>(), matchers_, ::std::back_inserter(matchers));
                return MakeMatcher(new UnorderedElementsAreMatcherImpl<Container>(matchers.begin(), matchers.end()));
            }
        private:
            const MatcherTuple matchers_;
            GTEST_DISALLOW_ASSIGN_(UnorderedElementsAreMatcher);
        };
        template <typename MatcherTuple> class ElementsAreMatcher {
        public:
            explicit ElementsAreMatcher(const MatcherTuple& args) : matchers_(args) {}
            template <typename Container> operator Matcher<Container>() const {
                typedef GTEST_REMOVE_REFERENCE_AND_CONST_(Container) RawContainer;
                typedef typename internal::StlContainerView<RawContainer>::type View;
                typedef typename View::value_type Element;
                typedef ::std::vector<Matcher<const Element&>> MatcherVec;
                MatcherVec matchers;
                matchers.reserve(::testing::tuple_size<MatcherTuple>::value);
                TransformTupleValues(CastAndAppendTransform<const Element&>(), matchers_, ::std::back_inserter(matchers));
                return MakeMatcher(new ElementsAreMatcherImpl<Container>(matchers.begin(), matchers.end()));
            }
        private:
            const MatcherTuple matchers_;
            GTEST_DISALLOW_ASSIGN_(ElementsAreMatcher);
        };
        template <typename T> class UnorderedElementsAreArrayMatcher {
        public:
            UnorderedElementsAreArrayMatcher() {}
            template <typename Iter> UnorderedElementsAreArrayMatcher(Iter first, Iter last) : matchers_(first, last) {}
            template <typename Container> operator Matcher<Container>() const {
                return MakeMatcher(new UnorderedElementsAreMatcherImpl<Container>(matchers_.begin(), matchers_.end()));
            }
        private:
            ::std::vector<T> matchers_;
            GTEST_DISALLOW_ASSIGN_(UnorderedElementsAreArrayMatcher);
        };
        template <typename T> class ElementsAreArrayMatcher {
        public:
            template <typename Iter> ElementsAreArrayMatcher(Iter first, Iter last) : matchers_(first, last) {}
            template <typename Container> operator Matcher<Container>() const {
                return MakeMatcher(new ElementsAreMatcherImpl<Container>(matchers_.begin(), matchers_.end()));
            }
        private:
            const ::std::vector<T> matchers_;
            GTEST_DISALLOW_ASSIGN_(ElementsAreArrayMatcher);
        };
        template <typename Tuple2Matcher, typename Second> class BoundSecondMatcher {
        public:
            BoundSecondMatcher(const Tuple2Matcher& tm, const Second& second) : tuple2_matcher_(tm), second_value_(second) {}
            template <typename T> operator Matcher<T>() const {
                return MakeMatcher(new Impl<T>(tuple2_matcher_, second_value_));
            }
            void operator=(const BoundSecondMatcher& /*rhs*/) {
                GTEST_LOG_(FATAL) << "BoundSecondMatcher should never be assigned.";
            }
        private:
            template <typename T> class Impl : public MatcherInterface<T> {
            public:
                typedef ::testing::tuple<T, Second> ArgTuple;
                Impl(const Tuple2Matcher& tm, const Second& second) : mono_tuple2_matcher_(SafeMatcherCast<const ArgTuple&>(tm)),
                                                                      second_value_(second) {}
                virtual void DescribeTo(::std::ostream* os) const {
                    *os << "and ";
                    UniversalPrint(second_value_, os);
                    *os << " ";
                    mono_tuple2_matcher_.DescribeTo(os);
                }
                virtual bool MatchAndExplain(T x, MatchResultListener* listener) const {
                    return mono_tuple2_matcher_.MatchAndExplain(ArgTuple(x, second_value_), listener);
                }
            private:
                const Matcher<const ArgTuple&> mono_tuple2_matcher_;
                const Second second_value_;
                GTEST_DISALLOW_ASSIGN_(Impl);
            };
            const Tuple2Matcher tuple2_matcher_;
            const Second second_value_;
        };
        template <typename Tuple2Matcher, typename Second> BoundSecondMatcher<Tuple2Matcher, Second> MatcherBindSecond(const Tuple2Matcher& tm, const Second& second) {
            return BoundSecondMatcher<Tuple2Matcher, Second>(tm, second);
        }
        GTEST_API_ string FormatMatcherDescription(bool negation, const char* matcher_name, const Strings& param_values);
    }
    template <typename Iter> inline internal::ElementsAreArrayMatcher<typename ::std::iterator_traits<Iter>::value_type> ElementsAreArray(Iter first, Iter last) {
        typedef typename ::std::iterator_traits<Iter>::value_type T;
        return internal::ElementsAreArrayMatcher<T>(first, last);
    }
    template <typename T> inline internal::ElementsAreArrayMatcher<T> ElementsAreArray(const T* pointer, size_t count) {
        return ElementsAreArray(pointer, pointer + count);
    }
    template <typename T, size_t N> inline internal::ElementsAreArrayMatcher<T> ElementsAreArray(const T (&array)[N]) {
        return ElementsAreArray(array, N);
    }
    template <typename Container> inline internal::ElementsAreArrayMatcher<typename Container::value_type> ElementsAreArray(const Container& container) {
        return ElementsAreArray(container.begin(), container.end());
    }
#if GTEST_HAS_STD_INITIALIZER_LIST_
    template <typename T> inline internal::ElementsAreArrayMatcher<T> ElementsAreArray(::std::initializer_list<T> xs) {
        return ElementsAreArray(xs.begin(), xs.end());
    }
#endif
    template <typename Iter> inline internal::UnorderedElementsAreArrayMatcher<typename ::std::iterator_traits<Iter>::value_type> UnorderedElementsAreArray(Iter first, Iter last) {
        typedef typename ::std::iterator_traits<Iter>::value_type T;
        return internal::UnorderedElementsAreArrayMatcher<T>(first, last);
    }
    template <typename T> inline internal::UnorderedElementsAreArrayMatcher<T> UnorderedElementsAreArray(const T* pointer, size_t count) {
        return UnorderedElementsAreArray(pointer, pointer + count);
    }
    template <typename T, size_t N> inline internal::UnorderedElementsAreArrayMatcher<T> UnorderedElementsAreArray(const T (&array)[N]) {
        return UnorderedElementsAreArray(array, N);
    }
    template <typename Container> inline internal::UnorderedElementsAreArrayMatcher<typename Container::value_type>
    UnorderedElementsAreArray(const Container& container) {
        return UnorderedElementsAreArray(container.begin(), container.end());
    }
#if GTEST_HAS_STD_INITIALIZER_LIST_
    template <typename T> inline internal::UnorderedElementsAreArrayMatcher<T> UnorderedElementsAreArray(::std::initializer_list<T> xs) {
        return UnorderedElementsAreArray(xs.begin(), xs.end());
    }
#endif
    const internal::AnythingMatcher _ = {};
    template <typename T> inline Matcher<T> A() { return MakeMatcher(new internal::AnyMatcherImpl<T>()); }
    template <typename T> inline Matcher<T> An() { return A<T>(); }
    template <typename T> inline internal::EqMatcher<T> Eq(T x) { return internal::EqMatcher<T>(x); }
    template <typename T> Matcher<T>::Matcher(T value) { *this = Eq(value); }
    template <typename Lhs, typename Rhs> inline Matcher<Lhs> TypedEq(const Rhs& rhs) { return Eq(rhs); }
    template <typename Rhs> inline internal::GeMatcher<Rhs> Ge(Rhs x) {
        return internal::GeMatcher<Rhs>(x);
    }
    template <typename Rhs> inline internal::GtMatcher<Rhs> Gt(Rhs x) {
        return internal::GtMatcher<Rhs>(x);
    }
    template <typename Rhs> inline internal::LeMatcher<Rhs> Le(Rhs x) {
        return internal::LeMatcher<Rhs>(x);
    }
    template <typename Rhs> inline internal::LtMatcher<Rhs> Lt(Rhs x) {
        return internal::LtMatcher<Rhs>(x);
    }
    template <typename Rhs> inline internal::NeMatcher<Rhs> Ne(Rhs x) {
        return internal::NeMatcher<Rhs>(x);
    }
    inline PolymorphicMatcher<internal::IsNullMatcher > IsNull() {
        return MakePolymorphicMatcher(internal::IsNullMatcher());
    }
    inline PolymorphicMatcher<internal::NotNullMatcher > NotNull() {
      return MakePolymorphicMatcher(internal::NotNullMatcher());
    }
    template <typename T> inline internal::RefMatcher<T&> Ref(T& x) {
        return internal::RefMatcher<T&>(x);
    }
    inline internal::FloatingEqMatcher<double> DoubleEq(double rhs) {
        return internal::FloatingEqMatcher<double>(rhs, false);
    }
    inline internal::FloatingEqMatcher<double> NanSensitiveDoubleEq(double rhs) {
        return internal::FloatingEqMatcher<double>(rhs, true);
    }
    inline internal::FloatingEqMatcher<double> DoubleNear(double rhs, double max_abs_error) {
        return internal::FloatingEqMatcher<double>(rhs, false, max_abs_error);
    }
    inline internal::FloatingEqMatcher<double> NanSensitiveDoubleNear(double rhs, double max_abs_error) {
        return internal::FloatingEqMatcher<double>(rhs, true, max_abs_error);
    }
    inline internal::FloatingEqMatcher<float> FloatEq(float rhs) {
        return internal::FloatingEqMatcher<float>(rhs, false);
    }
    inline internal::FloatingEqMatcher<float> NanSensitiveFloatEq(float rhs) {
        return internal::FloatingEqMatcher<float>(rhs, true);
    }
    inline internal::FloatingEqMatcher<float> FloatNear(float rhs, float max_abs_error) {
        return internal::FloatingEqMatcher<float>(rhs, false, max_abs_error);
    }
    inline internal::FloatingEqMatcher<float> NanSensitiveFloatNear(float rhs, float max_abs_error) {
        return internal::FloatingEqMatcher<float>(rhs, true, max_abs_error);
    }
    template <typename InnerMatcher> inline internal::PointeeMatcher<InnerMatcher> Pointee(const InnerMatcher& inner_matcher) {
        return internal::PointeeMatcher<InnerMatcher>(inner_matcher);
    }
    template <typename To> inline PolymorphicMatcher<internal::WhenDynamicCastToMatcher<To>> WhenDynamicCastTo(const Matcher<To>& inner_matcher) {
        return MakePolymorphicMatcher(internal::WhenDynamicCastToMatcher<To>(inner_matcher));
    }
    template <typename Class, typename FieldType, typename FieldMatcher>
    inline PolymorphicMatcher<internal::FieldMatcher<Class, FieldType> > Field(FieldType Class::*field, const FieldMatcher& matcher) {
        return MakePolymorphicMatcher(internal::FieldMatcher<Class, FieldType>(field, MatcherCast<const FieldType&>(matcher)));
    }
    template <typename Class, typename PropertyType, typename PropertyMatcher> inline PolymorphicMatcher<internal::PropertyMatcher<Class, PropertyType>>
    Property(PropertyType (Class::*property)() const, const PropertyMatcher& matcher) {
        return MakePolymorphicMatcher(internal::PropertyMatcher<Class, PropertyType>(property, MatcherCast<GTEST_REFERENCE_TO_CONST_(PropertyType)>(matcher)));
    }
    template <typename Callable, typename ResultOfMatcher> internal::ResultOfMatcher<Callable> ResultOf(Callable callable, const ResultOfMatcher& matcher) {
        return internal::ResultOfMatcher<Callable>(callable, MatcherCast<typename internal::CallableTraits<Callable>::ResultType>(matcher));
    }
    inline PolymorphicMatcher<internal::StrEqualityMatcher<internal::string>> StrEq(const internal::string& str) {
        return MakePolymorphicMatcher(internal::StrEqualityMatcher<internal::string>(str, true, true));
    }
    inline PolymorphicMatcher<internal::StrEqualityMatcher<internal::string>> StrNe(const internal::string& str) {
        return MakePolymorphicMatcher(internal::StrEqualityMatcher<internal::string>(str, false, true));
    }
    inline PolymorphicMatcher<internal::StrEqualityMatcher<internal::string>> StrCaseEq(const internal::string& str) {
        return MakePolymorphicMatcher(internal::StrEqualityMatcher<internal::string>(str, true, false));
    }
    inline PolymorphicMatcher<internal::StrEqualityMatcher<internal::string>> StrCaseNe(const internal::string& str) {
        return MakePolymorphicMatcher(internal::StrEqualityMatcher<internal::string>(str, false, false));
    }
    inline PolymorphicMatcher<internal::HasSubstrMatcher<internal::string>> HasSubstr(const internal::string& substring) {
        return MakePolymorphicMatcher(internal::HasSubstrMatcher<internal::string>(substring));
    }
    inline PolymorphicMatcher<internal::StartsWithMatcher<internal::string>> StartsWith(const internal::string& prefix) {
        return MakePolymorphicMatcher(internal::StartsWithMatcher<internal::string>(prefix));
    }
    inline PolymorphicMatcher<internal::EndsWithMatcher<internal::string>> EndsWith(const internal::string& suffix) {
        return MakePolymorphicMatcher(internal::EndsWithMatcher<internal::string>(suffix));
    }
    inline PolymorphicMatcher<internal::MatchesRegexMatcher> MatchesRegex(const internal::RE* regex) {
        return MakePolymorphicMatcher(internal::MatchesRegexMatcher(regex, true));
    }
    inline PolymorphicMatcher<internal::MatchesRegexMatcher> MatchesRegex(const internal::string& regex) {
        return MatchesRegex(new internal::RE(regex));
    }
    inline PolymorphicMatcher<internal::MatchesRegexMatcher> ContainsRegex(const internal::RE* regex) {
        return MakePolymorphicMatcher(internal::MatchesRegexMatcher(regex, false));
    }
    inline PolymorphicMatcher<internal::MatchesRegexMatcher> ContainsRegex(const internal::string& regex) {
        return ContainsRegex(new internal::RE(regex));
    }
#if GTEST_HAS_GLOBAL_WSTRING || GTEST_HAS_STD_WSTRING
    inline PolymorphicMatcher<internal::StrEqualityMatcher<internal::wstring>> StrEq(const internal::wstring& str) {
        return MakePolymorphicMatcher(internal::StrEqualityMatcher<internal::wstring>(str, true, true));
    }
    inline PolymorphicMatcher<internal::StrEqualityMatcher<internal::wstring>> StrNe(const internal::wstring& str) {
        return MakePolymorphicMatcher(internal::StrEqualityMatcher<internal::wstring>(str, false, true));
    }
    inline PolymorphicMatcher<internal::StrEqualityMatcher<internal::wstring>> StrCaseEq(const internal::wstring& str) {
        return MakePolymorphicMatcher(internal::StrEqualityMatcher<internal::wstring>(str, true, false));
    }
    inline PolymorphicMatcher<internal::StrEqualityMatcher<internal::wstring>> StrCaseNe(const internal::wstring& str) {
        return MakePolymorphicMatcher(internal::StrEqualityMatcher<internal::wstring>(str, false, false));
    }
    inline PolymorphicMatcher<internal::HasSubstrMatcher<internal::wstring>> HasSubstr(const internal::wstring& substring) {
        return MakePolymorphicMatcher(internal::HasSubstrMatcher<internal::wstring>(substring));
    }
    inline PolymorphicMatcher<internal::StartsWithMatcher<internal::wstring>> StartsWith(const internal::wstring& prefix) {
        return MakePolymorphicMatcher(internal::StartsWithMatcher<internal::wstring>(prefix));
    }
    inline PolymorphicMatcher<internal::EndsWithMatcher<internal::wstring>> EndsWith(const internal::wstring& suffix) {
        return MakePolymorphicMatcher(internal::EndsWithMatcher<internal::wstring>(suffix));
    }
#endif
    inline internal::Eq2Matcher Eq() { return internal::Eq2Matcher(); }
    inline internal::Ge2Matcher Ge() { return internal::Ge2Matcher(); }
    inline internal::Gt2Matcher Gt() { return internal::Gt2Matcher(); }
    inline internal::Le2Matcher Le() { return internal::Le2Matcher(); }
    inline internal::Lt2Matcher Lt() { return internal::Lt2Matcher(); }
    inline internal::Ne2Matcher Ne() { return internal::Ne2Matcher(); }
    template <typename InnerMatcher> inline internal::NotMatcher<InnerMatcher> Not(InnerMatcher m) {
        return internal::NotMatcher<InnerMatcher>(m);
    }
    template <typename Predicate> inline PolymorphicMatcher<internal::TrulyMatcher<Predicate>> Truly(Predicate pred) {
        return MakePolymorphicMatcher(internal::TrulyMatcher<Predicate>(pred));
    }
    template <typename SizeMatcher> inline internal::SizeIsMatcher<SizeMatcher> SizeIs(const SizeMatcher& size_matcher) {
        return internal::SizeIsMatcher<SizeMatcher>(size_matcher);
    }
    template <typename DistanceMatcher> inline internal::BeginEndDistanceIsMatcher<DistanceMatcher> BeginEndDistanceIs(const DistanceMatcher& distance_matcher) {
        return internal::BeginEndDistanceIsMatcher<DistanceMatcher>(distance_matcher);
    }
    template <typename Container> inline PolymorphicMatcher<internal::ContainerEqMatcher<GTEST_REMOVE_CONST_(Container)>> ContainerEq(const Container& rhs) {
        typedef GTEST_REMOVE_CONST_(Container) RawContainer;
        return MakePolymorphicMatcher(internal::ContainerEqMatcher<RawContainer>(rhs));
    }
    template <typename Comparator, typename ContainerMatcher> inline internal::WhenSortedByMatcher<Comparator, ContainerMatcher>
    WhenSortedBy(const Comparator& comparator, const ContainerMatcher& container_matcher) {
        return internal::WhenSortedByMatcher<Comparator, ContainerMatcher>(comparator, container_matcher);
    }
    template <typename ContainerMatcher>
    inline internal::WhenSortedByMatcher<internal::LessComparator, ContainerMatcher> WhenSorted(const ContainerMatcher& container_matcher) {
        return internal::WhenSortedByMatcher<internal::LessComparator, ContainerMatcher>(internal::LessComparator(), container_matcher);
    }
    template <typename TupleMatcher, typename Container> inline internal::PointwiseMatcher<TupleMatcher, GTEST_REMOVE_CONST_(Container)>
    Pointwise(const TupleMatcher& tuple_matcher, const Container& rhs) {
        typedef GTEST_REMOVE_CONST_(Container) RawContainer;
        return internal::PointwiseMatcher<TupleMatcher, RawContainer>(tuple_matcher, rhs);
    }
#if GTEST_HAS_STD_INITIALIZER_LIST_
    template <typename TupleMatcher, typename T>
    inline internal::PointwiseMatcher<TupleMatcher, std::vector<T> > Pointwise(const TupleMatcher& tuple_matcher, std::initializer_list<T> rhs) {
        return Pointwise(tuple_matcher, std::vector<T>(rhs));
    }
#endif
    template <typename Tuple2Matcher, typename RhsContainer>
    inline internal::UnorderedElementsAreArrayMatcher<typename internal::BoundSecondMatcher<Tuple2Matcher,
              typename internal::StlContainerView<GTEST_REMOVE_CONST_(RhsContainer)>::type::value_type>>
    UnorderedPointwise(const Tuple2Matcher& tuple2_matcher, const RhsContainer& rhs_container) {
        typedef GTEST_REMOVE_CONST_(RhsContainer) RawRhsContainer;
        typedef typename internal::StlContainerView<RawRhsContainer> RhsView;
        typedef typename RhsView::type RhsStlContainer;
        typedef typename RhsStlContainer::value_type Second;
        const RhsStlContainer& rhs_stl_container = RhsView::ConstReference(rhs_container);
        ::std::vector<internal::BoundSecondMatcher<Tuple2Matcher, Second> > matchers;
        for (typename RhsStlContainer::const_iterator it = rhs_stl_container.begin(); it != rhs_stl_container.end(); ++it) {
            matchers.push_back(internal::MatcherBindSecond(tuple2_matcher, *it));
        }
        return UnorderedElementsAreArray(matchers);
    }
#if GTEST_HAS_STD_INITIALIZER_LIST_
    template <typename Tuple2Matcher, typename T> inline internal::UnorderedElementsAreArrayMatcher<typename internal::BoundSecondMatcher<Tuple2Matcher, T>>
    UnorderedPointwise(const Tuple2Matcher& tuple2_matcher, std::initializer_list<T> rhs) {
        return UnorderedPointwise(tuple2_matcher, std::vector<T>(rhs));
    }
#endif
    template <typename M> inline internal::ContainsMatcher<M> Contains(M matcher) {
        return internal::ContainsMatcher<M>(matcher);
    }
    template <typename M> inline internal::EachMatcher<M> Each(M matcher) {
        return internal::EachMatcher<M>(matcher);
    }
    template <typename M> inline internal::KeyMatcher<M> Key(M inner_matcher) {
        return internal::KeyMatcher<M>(inner_matcher);
    }
    template <typename FirstMatcher, typename SecondMatcher> inline internal::PairMatcher<FirstMatcher, SecondMatcher>
    Pair(FirstMatcher first_matcher, SecondMatcher second_matcher) {
        return internal::PairMatcher<FirstMatcher, SecondMatcher>(first_matcher, second_matcher);
    }
    template <typename M> inline internal::MatcherAsPredicate<M> Matches(M matcher) {
        return internal::MatcherAsPredicate<M>(matcher);
    }
    template <typename T, typename M> inline bool Value(const T& value, M matcher) {
        return testing::Matches(matcher)(value);
    }
    template <typename T, typename M> inline bool ExplainMatchResult(M matcher, const T& value, MatchResultListener* listener) {
        return SafeMatcherCast<const T&>(matcher).MatchAndExplain(value, listener);
    }
#if GTEST_LANG_CXX11
    template <typename... Args> inline internal::AllOfMatcher<Args...> AllOf(const Args&... matchers) {
        return internal::AllOfMatcher<Args...>(matchers...);
    }
    template <typename... Args> inline internal::AnyOfMatcher<Args...> AnyOf(const Args&... matchers) {
        return internal::AnyOfMatcher<Args...>(matchers...);
    }
#endif
    template <typename InnerMatcher> inline InnerMatcher AllArgs(const InnerMatcher& matcher) { return matcher; }
    #define ASSERT_THAT(value, matcher) ASSERT_PRED_FORMAT1(::testing::internal::MakePredicateFormatterFromMatcher(matcher), value)
    #define EXPECT_THAT(value, matcher) EXPECT_PRED_FORMAT1(::testing::internal::MakePredicateFormatterFromMatcher(matcher), value)
}
#include "internal/custom/gmock-matchers.h"
#endif