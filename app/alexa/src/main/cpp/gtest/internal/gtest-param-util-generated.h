#ifndef GTEST_INCLUDE_GTEST_INTERNAL_GTEST_PARAM_UTIL_GENERATED_H_
#define GTEST_INCLUDE_GTEST_INTERNAL_GTEST_PARAM_UTIL_GENERATED_H_

#include <iostream>
#include "gtest-param-util.h"
#include "gtest-port.h"

#if GTEST_HAS_PARAM_TEST
namespace testing {
    template <typename ForwardIterator> internal::ParamGenerator<typename ::testing::internal::IteratorTraits<ForwardIterator>::value_type>
    ValuesIn(ForwardIterator begin, ForwardIterator end);
    template <typename T, size_t N> internal::ParamGenerator<T> ValuesIn(const T (&array)[N]);
    template <class Container> internal::ParamGenerator<typename Container::value_type> ValuesIn(const Container& container);
    namespace internal {
        template <typename T1> class ValueArray1 {
        public:
            explicit ValueArray1(T1 v1) : v1_(v1) {}
            template <typename T> operator ParamGenerator<T>() const {
                const T array[] = {static_cast<T>(v1_)};
                return ValuesIn(array);
            }
        private:
            void operator=(const ValueArray1& other);
            const T1 v1_;
        };
        template <typename T1, typename T2> class ValueArray2 {
        public:
            ValueArray2(T1 v1, T2 v2) : v1_(v1), v2_(v2) {}
            template <typename T> operator ParamGenerator<T>() const {
                const T array[] = {static_cast<T>(v1_), static_cast<T>(v2_)};
                return ValuesIn(array);
            }
        private:
            void operator=(const ValueArray2& other);
            const T1 v1_;
            const T2 v2_;
        };
        template <typename T1, typename T2, typename T3> class ValueArray3 {
        public:
            ValueArray3(T1 v1, T2 v2, T3 v3) : v1_(v1), v2_(v2), v3_(v3) {}
            template <typename T> operator ParamGenerator<T>() const {
                const T array[] = {static_cast<T>(v1_), static_cast<T>(v2_), static_cast<T>(v3_)};
                return ValuesIn(array);
            }
        private:
            void operator=(const ValueArray3& other);
            const T1 v1_;
            const T2 v2_;
            const T3 v3_;
        };
        template <typename T1, typename T2, typename T3, typename T4> class ValueArray4 {
        public:
            ValueArray4(T1 v1, T2 v2, T3 v3, T4 v4) : v1_(v1), v2_(v2), v3_(v3), v4_(v4) {}
            template <typename T> operator ParamGenerator<T>() const {
                const T array[] = {static_cast<T>(v1_), static_cast<T>(v2_), static_cast<T>(v3_), static_cast<T>(v4_)};
                return ValuesIn(array);
            }
        private:
            void operator=(const ValueArray4& other);
            const T1 v1_;
            const T2 v2_;
            const T3 v3_;
            const T4 v4_;
        };
        template <typename T1, typename T2, typename T3, typename T4, typename T5> class ValueArray5 {
        public:
            ValueArray5(T1 v1, T2 v2, T3 v3, T4 v4, T5 v5) : v1_(v1), v2_(v2), v3_(v3), v4_(v4), v5_(v5) {}
            template <typename T> operator ParamGenerator<T>() const {
                const T array[] = {static_cast<T>(v1_), static_cast<T>(v2_), static_cast<T>(v3_), static_cast<T>(v4_), static_cast<T>(v5_)};
                return ValuesIn(array);
            }
        private:
            void operator=(const ValueArray5& other);
            const T1 v1_;
            const T2 v2_;
            const T3 v3_;
            const T4 v4_;
            const T5 v5_;
        };
        template <typename T1, typename T2, typename T3, typename T4, typename T5,
            typename T6> class ValueArray6 {
        public:
            ValueArray6(T1 v1, T2 v2, T3 v3, T4 v4, T5 v5, T6 v6) : v1_(v1), v2_(v2), v3_(v3), v4_(v4), v5_(v5), v6_(v6) {}
            template <typename T> operator ParamGenerator<T>() const {
                const T array[] = {static_cast<T>(v1_), static_cast<T>(v2_), static_cast<T>(v3_), static_cast<T>(v4_), static_cast<T>(v5_),
                                   static_cast<T>(v6_)};
                return ValuesIn(array);
            }
        private:
            void operator=(const ValueArray6& other);
            const T1 v1_;
            const T2 v2_;
            const T3 v3_;
            const T4 v4_;
            const T5 v5_;
            const T6 v6_;
        };
        template <typename T1, typename T2, typename T3, typename T4, typename T5,
            typename T6, typename T7> class ValueArray7 {
        public:
            ValueArray7(T1 v1, T2 v2, T3 v3, T4 v4, T5 v5, T6 v6, T7 v7) : v1_(v1), v2_(v2), v3_(v3), v4_(v4), v5_(v5), v6_(v6), v7_(v7) {}
            template <typename T> operator ParamGenerator<T>() const {
            const T array[] = { static_cast<T>(v1_), static_cast<T>(v2_), static_cast<T>(v3_), static_cast<T>(v4_), static_cast<T>(v5_),
                                static_cast<T>(v6_), static_cast<T>(v7_) };
            return ValuesIn(array);
            }
        private:
            void operator=(const ValueArray7& other);
            const T1 v1_;
            const T2 v2_;
            const T3 v3_;
            const T4 v4_;
            const T5 v5_;
            const T6 v6_;
            const T7 v7_;
        };
        template <typename T1, typename T2, typename T3, typename T4, typename T5,
            typename T6, typename T7, typename T8> class ValueArray8 {
        public:
            ValueArray8(T1 v1, T2 v2, T3 v3, T4 v4, T5 v5, T6 v6, T7 v7, T8 v8) : v1_(v1), v2_(v2), v3_(v3), v4_(v4), v5_(v5), v6_(v6),
                        v7_(v7), v8_(v8) {}
            template <typename T> operator ParamGenerator<T>() const {
                const T array[] = { static_cast<T>(v1_), static_cast<T>(v2_),static_cast<T>(v3_), static_cast<T>(v4_), static_cast<T>(v5_),
                                    static_cast<T>(v6_), static_cast<T>(v7_), static_cast<T>(v8_) };
                return ValuesIn(array);
            }
        private:
            void operator=(const ValueArray8& other);
            const T1 v1_;
            const T2 v2_;
            const T3 v3_;
            const T4 v4_;
            const T5 v5_;
            const T6 v6_;
            const T7 v7_;
            const T8 v8_;
        };
        template <typename T1, typename T2, typename T3, typename T4, typename T5,
            typename T6, typename T7, typename T8, typename T9> class ValueArray9 {
        public:
            ValueArray9(T1 v1, T2 v2, T3 v3, T4 v4, T5 v5, T6 v6, T7 v7, T8 v8, T9 v9) : v1_(v1), v2_(v2), v3_(v3), v4_(v4), v5_(v5), v6_(v6),
                        v7_(v7), v8_(v8), v9_(v9) {}
            template <typename T> operator ParamGenerator<T>() const {
            const T array[] = { static_cast<T>(v1_), static_cast<T>(v2_), static_cast<T>(v3_), static_cast<T>(v4_), static_cast<T>(v5_),
                                static_cast<T>(v6_), static_cast<T>(v7_), static_cast<T>(v8_), static_cast<T>(v9_) };
            return ValuesIn(array);
            }
        private:
            void operator=(const ValueArray9& other);
            const T1 v1_;
            const T2 v2_;
            const T3 v3_;
            const T4 v4_;
            const T5 v5_;
            const T6 v6_;
            const T7 v7_;
            const T8 v8_;
            const T9 v9_;
        };
        template <typename T1, typename T2, typename T3, typename T4, typename T5,
            typename T6, typename T7, typename T8, typename T9, typename T10> class ValueArray10 {
        public:
            ValueArray10(T1 v1, T2 v2, T3 v3, T4 v4, T5 v5, T6 v6, T7 v7, T8 v8, T9 v9, T10 v10) : v1_(v1), v2_(v2), v3_(v3), v4_(v4), v5_(v5),
                         v6_(v6), v7_(v7), v8_(v8), v9_(v9), v10_(v10) {}
            template <typename T> operator ParamGenerator<T>() const {
                const T array[] = { static_cast<T>(v1_), static_cast<T>(v2_), static_cast<T>(v3_), static_cast<T>(v4_), static_cast<T>(v5_),
                                    static_cast<T>(v6_), static_cast<T>(v7_), static_cast<T>(v8_), static_cast<T>(v9_), static_cast<T>(v10_) };
                return ValuesIn(array);
            }
        private:
            void operator=(const ValueArray10& other);
            const T1 v1_;
            const T2 v2_;
            const T3 v3_;
            const T4 v4_;
            const T5 v5_;
            const T6 v6_;
            const T7 v7_;
            const T8 v8_;
            const T9 v9_;
            const T10 v10_;
        };
        template <typename T1, typename T2, typename T3, typename T4, typename T5,
            typename T6, typename T7, typename T8, typename T9, typename T10,
            typename T11> class ValueArray11 {
        public:
            ValueArray11(T1 v1, T2 v2, T3 v3, T4 v4, T5 v5, T6 v6, T7 v7, T8 v8, T9 v9, T10 v10, T11 v11) : v1_(v1), v2_(v2), v3_(v3), v4_(v4),
                         v5_(v5), v6_(v6), v7_(v7), v8_(v8), v9_(v9), v10_(v10), v11_(v11) {}
            template <typename T> operator ParamGenerator<T>() const {
                const T array[] = { static_cast<T>(v1_), static_cast<T>(v2_), static_cast<T>(v3_), static_cast<T>(v4_), static_cast<T>(v5_),
                                    static_cast<T>(v6_), static_cast<T>(v7_), static_cast<T>(v8_), static_cast<T>(v9_), static_cast<T>(v10_),
                                    static_cast<T>(v11_) };
                return ValuesIn(array);
            }
        private:
            void operator=(const ValueArray11& other);
            const T1 v1_;
            const T2 v2_;
            const T3 v3_;
            const T4 v4_;
            const T5 v5_;
            const T6 v6_;
            const T7 v7_;
            const T8 v8_;
            const T9 v9_;
            const T10 v10_;
            const T11 v11_;
        };
        template <typename T1, typename T2, typename T3, typename T4, typename T5,
            typename T6, typename T7, typename T8, typename T9, typename T10,
            typename T11, typename T12> class ValueArray12 {
        public:
            ValueArray12(T1 v1, T2 v2, T3 v3, T4 v4, T5 v5, T6 v6, T7 v7, T8 v8, T9 v9, T10 v10, T11 v11, T12 v12) : v1_(v1), v2_(v2), v3_(v3),
                         v4_(v4), v5_(v5), v6_(v6), v7_(v7), v8_(v8), v9_(v9), v10_(v10), v11_(v11), v12_(v12) {}
            template <typename T> operator ParamGenerator<T>() const {
                const T array[] = {static_cast<T>(v1_), static_cast<T>(v2_), static_cast<T>(v3_), static_cast<T>(v4_), static_cast<T>(v5_),
                                   static_cast<T>(v6_), static_cast<T>(v7_), static_cast<T>(v8_), static_cast<T>(v9_), static_cast<T>(v10_),
                                   static_cast<T>(v11_), static_cast<T>(v12_)};
                return ValuesIn(array);
            }
        private:
            void operator=(const ValueArray12& other);
            const T1 v1_;
            const T2 v2_;
            const T3 v3_;
            const T4 v4_;
            const T5 v5_;
            const T6 v6_;
            const T7 v7_;
            const T8 v8_;
            const T9 v9_;
            const T10 v10_;
            const T11 v11_;
            const T12 v12_;
        };
        template <typename T1, typename T2, typename T3, typename T4, typename T5,
            typename T6, typename T7, typename T8, typename T9, typename T10,
            typename T11, typename T12, typename T13> class ValueArray13 {
        public:
            ValueArray13(T1 v1, T2 v2, T3 v3, T4 v4, T5 v5, T6 v6, T7 v7, T8 v8, T9 v9, T10 v10, T11 v11, T12 v12, T13 v13) : v1_(v1), v2_(v2),
                         v3_(v3), v4_(v4), v5_(v5), v6_(v6), v7_(v7), v8_(v8), v9_(v9), v10_(v10), v11_(v11), v12_(v12), v13_(v13) {}
            template <typename T> operator ParamGenerator<T>() const {
                const T array[] = {static_cast<T>(v1_), static_cast<T>(v2_), static_cast<T>(v3_), static_cast<T>(v4_), static_cast<T>(v5_),
                                   static_cast<T>(v6_), static_cast<T>(v7_), static_cast<T>(v8_), static_cast<T>(v9_), static_cast<T>(v10_),
                                   static_cast<T>(v11_), static_cast<T>(v12_), static_cast<T>(v13_)};
                return ValuesIn(array);
            }
        private:
            void operator=(const ValueArray13& other);
            const T1 v1_;
            const T2 v2_;
            const T3 v3_;
            const T4 v4_;
            const T5 v5_;
            const T6 v6_;
            const T7 v7_;
            const T8 v8_;
            const T9 v9_;
            const T10 v10_;
            const T11 v11_;
            const T12 v12_;
            const T13 v13_;
        };
        template <typename T1, typename T2, typename T3, typename T4, typename T5,
            typename T6, typename T7, typename T8, typename T9, typename T10,
            typename T11, typename T12, typename T13, typename T14> class ValueArray14 {
        public:
            ValueArray14(T1 v1, T2 v2, T3 v3, T4 v4, T5 v5, T6 v6, T7 v7, T8 v8, T9 v9, T10 v10, T11 v11, T12 v12, T13 v13, T14 v14) : v1_(v1),
                         v2_(v2), v3_(v3), v4_(v4), v5_(v5), v6_(v6), v7_(v7), v8_(v8), v9_(v9), v10_(v10), v11_(v11), v12_(v12), v13_(v13),
                         v14_(v14) {}
            template <typename T> operator ParamGenerator<T>() const {
            const T array[] = {static_cast<T>(v1_), static_cast<T>(v2_), static_cast<T>(v3_), static_cast<T>(v4_), static_cast<T>(v5_),
                               static_cast<T>(v6_), static_cast<T>(v7_), static_cast<T>(v8_), static_cast<T>(v9_), static_cast<T>(v10_),
                               static_cast<T>(v11_), static_cast<T>(v12_), static_cast<T>(v13_), static_cast<T>(v14_)};
            return ValuesIn(array);
            }
        private:
            void operator=(const ValueArray14& other);
            const T1 v1_;
            const T2 v2_;
            const T3 v3_;
            const T4 v4_;
            const T5 v5_;
            const T6 v6_;
            const T7 v7_;
            const T8 v8_;
            const T9 v9_;
            const T10 v10_;
            const T11 v11_;
            const T12 v12_;
            const T13 v13_;
            const T14 v14_;
        };
        template <typename T1, typename T2, typename T3, typename T4, typename T5,
            typename T6, typename T7, typename T8, typename T9, typename T10,
            typename T11, typename T12, typename T13, typename T14, typename T15> class ValueArray15 {
        public:
            ValueArray15(T1 v1, T2 v2, T3 v3, T4 v4, T5 v5, T6 v6, T7 v7, T8 v8, T9 v9, T10 v10, T11 v11, T12 v12, T13 v13, T14 v14, T15 v15) :
                         v1_(v1), v2_(v2), v3_(v3), v4_(v4), v5_(v5), v6_(v6), v7_(v7), v8_(v8), v9_(v9), v10_(v10), v11_(v11), v12_(v12),
                         v13_(v13), v14_(v14), v15_(v15) {}
            template <typename T> operator ParamGenerator<T>() const {
                const T array[] = { static_cast<T>(v1_), static_cast<T>(v2_), static_cast<T>(v3_), static_cast<T>(v4_), static_cast<T>(v5_),
                                    static_cast<T>(v6_), static_cast<T>(v7_), static_cast<T>(v8_), static_cast<T>(v9_), static_cast<T>(v10_),
                                    static_cast<T>(v11_), static_cast<T>(v12_), static_cast<T>(v13_), static_cast<T>(v14_),
                                    static_cast<T>(v15_) };
                return ValuesIn(array);
            }
        private:
            void operator=(const ValueArray15& other);
            const T1 v1_;
            const T2 v2_;
            const T3 v3_;
            const T4 v4_;
            const T5 v5_;
            const T6 v6_;
            const T7 v7_;
            const T8 v8_;
            const T9 v9_;
            const T10 v10_;
            const T11 v11_;
            const T12 v12_;
            const T13 v13_;
            const T14 v14_;
            const T15 v15_;
        };
        template <typename T1, typename T2, typename T3, typename T4, typename T5,
            typename T6, typename T7, typename T8, typename T9, typename T10,
            typename T11, typename T12, typename T13, typename T14, typename T15,
            typename T16>
        class ValueArray16 {
        public:
            ValueArray16(T1 v1, T2 v2, T3 v3, T4 v4, T5 v5, T6 v6, T7 v7, T8 v8, T9 v9, T10 v10, T11 v11, T12 v12, T13 v13, T14 v14, T15 v15,
                         T16 v16) : v1_(v1), v2_(v2), v3_(v3), v4_(v4), v5_(v5), v6_(v6), v7_(v7), v8_(v8), v9_(v9), v10_(v10), v11_(v11),
                         v12_(v12), v13_(v13), v14_(v14), v15_(v15), v16_(v16) {}
            template <typename T> operator ParamGenerator<T>() const {
                const T array[] = {static_cast<T>(v1_), static_cast<T>(v2_), static_cast<T>(v3_), static_cast<T>(v4_), static_cast<T>(v5_),
                                   static_cast<T>(v6_), static_cast<T>(v7_), static_cast<T>(v8_), static_cast<T>(v9_), static_cast<T>(v10_),
                                   static_cast<T>(v11_), static_cast<T>(v12_), static_cast<T>(v13_), static_cast<T>(v14_),
                                   static_cast<T>(v15_), static_cast<T>(v16_)};
                return ValuesIn(array);
            }
        private:
            void operator=(const ValueArray16& other);
            const T1 v1_;
            const T2 v2_;
            const T3 v3_;
            const T4 v4_;
            const T5 v5_;
            const T6 v6_;
            const T7 v7_;
            const T8 v8_;
            const T9 v9_;
            const T10 v10_;
            const T11 v11_;
            const T12 v12_;
            const T13 v13_;
            const T14 v14_;
            const T15 v15_;
            const T16 v16_;
        };
        template <typename T1, typename T2, typename T3, typename T4, typename T5,
            typename T6, typename T7, typename T8, typename T9, typename T10,
            typename T11, typename T12, typename T13, typename T14, typename T15,
            typename T16, typename T17> class ValueArray17 {
        public:
            ValueArray17(T1 v1, T2 v2, T3 v3, T4 v4, T5 v5, T6 v6, T7 v7, T8 v8, T9 v9, T10 v10, T11 v11, T12 v12, T13 v13, T14 v14, T15 v15,
                         T16 v16, T17 v17) : v1_(v1), v2_(v2), v3_(v3), v4_(v4), v5_(v5), v6_(v6), v7_(v7), v8_(v8), v9_(v9), v10_(v10),
                         v11_(v11), v12_(v12), v13_(v13), v14_(v14), v15_(v15), v16_(v16), v17_(v17) {}
            template <typename T> operator ParamGenerator<T>() const {
                const T array[] = { static_cast<T>(v1_), static_cast<T>(v2_), static_cast<T>(v3_), static_cast<T>(v4_), static_cast<T>(v5_),
                                    static_cast<T>(v6_), static_cast<T>(v7_), static_cast<T>(v8_), static_cast<T>(v9_), static_cast<T>(v10_),
                                    static_cast<T>(v11_), static_cast<T>(v12_), static_cast<T>(v13_), static_cast<T>(v14_),
                                    static_cast<T>(v15_), static_cast<T>(v16_), static_cast<T>(v17_) };
                return ValuesIn(array);
            }
        private:
            void operator=(const ValueArray17& other);
            const T1 v1_;
            const T2 v2_;
            const T3 v3_;
            const T4 v4_;
            const T5 v5_;
            const T6 v6_;
            const T7 v7_;
            const T8 v8_;
            const T9 v9_;
            const T10 v10_;
            const T11 v11_;
            const T12 v12_;
            const T13 v13_;
            const T14 v14_;
            const T15 v15_;
            const T16 v16_;
            const T17 v17_;
        };
        template <typename T1, typename T2, typename T3, typename T4, typename T5,
            typename T6, typename T7, typename T8, typename T9, typename T10,
            typename T11, typename T12, typename T13, typename T14, typename T15,
            typename T16, typename T17, typename T18> class ValueArray18 {
        public:
            ValueArray18(T1 v1, T2 v2, T3 v3, T4 v4, T5 v5, T6 v6, T7 v7, T8 v8, T9 v9, T10 v10, T11 v11, T12 v12, T13 v13, T14 v14, T15 v15,
                         T16 v16, T17 v17, T18 v18) : v1_(v1), v2_(v2), v3_(v3), v4_(v4), v5_(v5), v6_(v6), v7_(v7), v8_(v8), v9_(v9),
                         v10_(v10), v11_(v11), v12_(v12), v13_(v13), v14_(v14), v15_(v15), v16_(v16), v17_(v17), v18_(v18) {}
            template <typename T> operator ParamGenerator<T>() const {
                const T array[] = { static_cast<T>(v1_), static_cast<T>(v2_), static_cast<T>(v3_), static_cast<T>(v4_), static_cast<T>(v5_),
                                    static_cast<T>(v6_), static_cast<T>(v7_), static_cast<T>(v8_), static_cast<T>(v9_), static_cast<T>(v10_),
                                    static_cast<T>(v11_), static_cast<T>(v12_), static_cast<T>(v13_), static_cast<T>(v14_),
                                    static_cast<T>(v15_), static_cast<T>(v16_), static_cast<T>(v17_), static_cast<T>(v18_) };
                return ValuesIn(array);
            }
        private:
            void operator=(const ValueArray18& other);
            const T1 v1_;
            const T2 v2_;
            const T3 v3_;
            const T4 v4_;
            const T5 v5_;
            const T6 v6_;
            const T7 v7_;
            const T8 v8_;
            const T9 v9_;
            const T10 v10_;
            const T11 v11_;
            const T12 v12_;
            const T13 v13_;
            const T14 v14_;
            const T15 v15_;
            const T16 v16_;
            const T17 v17_;
            const T18 v18_;
        };
        template <typename T1, typename T2, typename T3, typename T4, typename T5,
            typename T6, typename T7, typename T8, typename T9, typename T10,
            typename T11, typename T12, typename T13, typename T14, typename T15,
            typename T16, typename T17, typename T18, typename T19> class ValueArray19 {
        public:
            ValueArray19(T1 v1, T2 v2, T3 v3, T4 v4, T5 v5, T6 v6, T7 v7, T8 v8, T9 v9, T10 v10, T11 v11, T12 v12, T13 v13, T14 v14, T15 v15,
                         T16 v16, T17 v17, T18 v18, T19 v19) : v1_(v1), v2_(v2), v3_(v3), v4_(v4), v5_(v5), v6_(v6), v7_(v7), v8_(v8), v9_(v9),
                         v10_(v10), v11_(v11), v12_(v12), v13_(v13), v14_(v14), v15_(v15), v16_(v16), v17_(v17), v18_(v18), v19_(v19) {}
            template <typename T> operator ParamGenerator<T>() const {
                const T array[] = { static_cast<T>(v1_), static_cast<T>(v2_), static_cast<T>(v3_), static_cast<T>(v4_), static_cast<T>(v5_),
                                    static_cast<T>(v6_), static_cast<T>(v7_), static_cast<T>(v8_), static_cast<T>(v9_), static_cast<T>(v10_),
                                    static_cast<T>(v11_), static_cast<T>(v12_), static_cast<T>(v13_), static_cast<T>(v14_),
                                    static_cast<T>(v15_), static_cast<T>(v16_), static_cast<T>(v17_), static_cast<T>(v18_),
                                    static_cast<T>(v19_) };
                return ValuesIn(array);
            }
        private:
            void operator=(const ValueArray19& other);
            const T1 v1_;
            const T2 v2_;
            const T3 v3_;
            const T4 v4_;
            const T5 v5_;
            const T6 v6_;
            const T7 v7_;
            const T8 v8_;
            const T9 v9_;
            const T10 v10_;
            const T11 v11_;
            const T12 v12_;
            const T13 v13_;
            const T14 v14_;
            const T15 v15_;
            const T16 v16_;
            const T17 v17_;
            const T18 v18_;
            const T19 v19_;
        };
        template <typename T1, typename T2, typename T3, typename T4, typename T5,
            typename T6, typename T7, typename T8, typename T9, typename T10,
            typename T11, typename T12, typename T13, typename T14, typename T15,
            typename T16, typename T17, typename T18, typename T19, typename T20> class ValueArray20 {
        public:
            ValueArray20(T1 v1, T2 v2, T3 v3, T4 v4, T5 v5, T6 v6, T7 v7, T8 v8, T9 v9, T10 v10, T11 v11, T12 v12, T13 v13, T14 v14, T15 v15,
                         T16 v16, T17 v17, T18 v18, T19 v19, T20 v20) : v1_(v1), v2_(v2), v3_(v3), v4_(v4), v5_(v5), v6_(v6), v7_(v7), v8_(v8),
                         v9_(v9), v10_(v10), v11_(v11), v12_(v12), v13_(v13), v14_(v14), v15_(v15), v16_(v16), v17_(v17), v18_(v18), v19_(v19),
                         v20_(v20) {}
            template <typename T> operator ParamGenerator<T>() const {
                const T array[] = { static_cast<T>(v1_), static_cast<T>(v2_), static_cast<T>(v3_), static_cast<T>(v4_), static_cast<T>(v5_),
                                    static_cast<T>(v6_), static_cast<T>(v7_), static_cast<T>(v8_), static_cast<T>(v9_), static_cast<T>(v10_),
                                    static_cast<T>(v11_), static_cast<T>(v12_), static_cast<T>(v13_), static_cast<T>(v14_),
                                    static_cast<T>(v15_), static_cast<T>(v16_), static_cast<T>(v17_), static_cast<T>(v18_),
                                    static_cast<T>(v19_), static_cast<T>(v20_) };
                return ValuesIn(array);
            }
        private:
            void operator=(const ValueArray20& other);
            const T1 v1_;
            const T2 v2_;
            const T3 v3_;
            const T4 v4_;
            const T5 v5_;
            const T6 v6_;
            const T7 v7_;
            const T8 v8_;
            const T9 v9_;
            const T10 v10_;
            const T11 v11_;
            const T12 v12_;
            const T13 v13_;
            const T14 v14_;
            const T15 v15_;
            const T16 v16_;
            const T17 v17_;
            const T18 v18_;
            const T19 v19_;
            const T20 v20_;
        };
        template <typename T1, typename T2, typename T3, typename T4, typename T5,
            typename T6, typename T7, typename T8, typename T9, typename T10,
            typename T11, typename T12, typename T13, typename T14, typename T15,
            typename T16, typename T17, typename T18, typename T19, typename T20,
            typename T21> class ValueArray21 {
        public:
            ValueArray21(T1 v1, T2 v2, T3 v3, T4 v4, T5 v5, T6 v6, T7 v7, T8 v8, T9 v9, T10 v10, T11 v11, T12 v12, T13 v13, T14 v14, T15 v15,
                         T16 v16, T17 v17, T18 v18, T19 v19, T20 v20, T21 v21) : v1_(v1), v2_(v2), v3_(v3), v4_(v4), v5_(v5), v6_(v6), v7_(v7),
                         v8_(v8), v9_(v9), v10_(v10), v11_(v11), v12_(v12), v13_(v13), v14_(v14), v15_(v15), v16_(v16), v17_(v17), v18_(v18),
                         v19_(v19), v20_(v20), v21_(v21) {}
            template <typename T> operator ParamGenerator<T>() const {
                const T array[] = { static_cast<T>(v1_), static_cast<T>(v2_), static_cast<T>(v3_), static_cast<T>(v4_), static_cast<T>(v5_),
                                    static_cast<T>(v6_), static_cast<T>(v7_), static_cast<T>(v8_), static_cast<T>(v9_), static_cast<T>(v10_),
                                    static_cast<T>(v11_), static_cast<T>(v12_), static_cast<T>(v13_), static_cast<T>(v14_),
                                    static_cast<T>(v15_), static_cast<T>(v16_), static_cast<T>(v17_), static_cast<T>(v18_),
                                    static_cast<T>(v19_), static_cast<T>(v20_), static_cast<T>(v21_) };
                return ValuesIn(array);
            }
        private:
            void operator=(const ValueArray21& other);
            const T1 v1_;
            const T2 v2_;
            const T3 v3_;
            const T4 v4_;
            const T5 v5_;
            const T6 v6_;
            const T7 v7_;
            const T8 v8_;
            const T9 v9_;
            const T10 v10_;
            const T11 v11_;
            const T12 v12_;
            const T13 v13_;
            const T14 v14_;
            const T15 v15_;
            const T16 v16_;
            const T17 v17_;
            const T18 v18_;
            const T19 v19_;
            const T20 v20_;
            const T21 v21_;
        };
        template <typename T1, typename T2, typename T3, typename T4, typename T5,
            typename T6, typename T7, typename T8, typename T9, typename T10,
            typename T11, typename T12, typename T13, typename T14, typename T15,
            typename T16, typename T17, typename T18, typename T19, typename T20,
            typename T21, typename T22> class ValueArray22 {
        public:
            ValueArray22(T1 v1, T2 v2, T3 v3, T4 v4, T5 v5, T6 v6, T7 v7, T8 v8, T9 v9, T10 v10, T11 v11, T12 v12, T13 v13, T14 v14, T15 v15,
                         T16 v16, T17 v17, T18 v18, T19 v19, T20 v20, T21 v21, T22 v22) : v1_(v1), v2_(v2), v3_(v3), v4_(v4), v5_(v5), v6_(v6),
                         v7_(v7), v8_(v8), v9_(v9), v10_(v10), v11_(v11), v12_(v12), v13_(v13), v14_(v14), v15_(v15), v16_(v16), v17_(v17),
                         v18_(v18), v19_(v19), v20_(v20), v21_(v21), v22_(v22) {}
            template <typename T> operator ParamGenerator<T>() const {
                const T array[] = { static_cast<T>(v1_), static_cast<T>(v2_), static_cast<T>(v3_), static_cast<T>(v4_), static_cast<T>(v5_),
                                    static_cast<T>(v6_), static_cast<T>(v7_), static_cast<T>(v8_), static_cast<T>(v9_), static_cast<T>(v10_),
                                    static_cast<T>(v11_), static_cast<T>(v12_), static_cast<T>(v13_), static_cast<T>(v14_),
                                    static_cast<T>(v15_), static_cast<T>(v16_), static_cast<T>(v17_), static_cast<T>(v18_),
                                    static_cast<T>(v19_), static_cast<T>(v20_), static_cast<T>(v21_), static_cast<T>(v22_) };
                return ValuesIn(array);
            }
        private:
            void operator=(const ValueArray22& other);
            const T1 v1_;
            const T2 v2_;
            const T3 v3_;
            const T4 v4_;
            const T5 v5_;
            const T6 v6_;
            const T7 v7_;
            const T8 v8_;
            const T9 v9_;
            const T10 v10_;
            const T11 v11_;
            const T12 v12_;
            const T13 v13_;
            const T14 v14_;
            const T15 v15_;
            const T16 v16_;
            const T17 v17_;
            const T18 v18_;
            const T19 v19_;
            const T20 v20_;
            const T21 v21_;
            const T22 v22_;
        };
        template <typename T1, typename T2, typename T3, typename T4, typename T5,
            typename T6, typename T7, typename T8, typename T9, typename T10,
            typename T11, typename T12, typename T13, typename T14, typename T15,
            typename T16, typename T17, typename T18, typename T19, typename T20,
            typename T21, typename T22, typename T23> class ValueArray23 {
        public:
            ValueArray23(T1 v1, T2 v2, T3 v3, T4 v4, T5 v5, T6 v6, T7 v7, T8 v8, T9 v9, T10 v10, T11 v11, T12 v12, T13 v13, T14 v14, T15 v15,
                         T16 v16, T17 v17, T18 v18, T19 v19, T20 v20, T21 v21, T22 v22, T23 v23) : v1_(v1), v2_(v2), v3_(v3), v4_(v4), v5_(v5),
                         v6_(v6), v7_(v7), v8_(v8), v9_(v9), v10_(v10), v11_(v11), v12_(v12), v13_(v13), v14_(v14), v15_(v15), v16_(v16),
                         v17_(v17), v18_(v18), v19_(v19), v20_(v20), v21_(v21), v22_(v22), v23_(v23) {}
            template <typename T> operator ParamGenerator<T>() const {
                const T array[] = { static_cast<T>(v1_), static_cast<T>(v2_), static_cast<T>(v3_), static_cast<T>(v4_), static_cast<T>(v5_),
                                    static_cast<T>(v6_), static_cast<T>(v7_), static_cast<T>(v8_), static_cast<T>(v9_), static_cast<T>(v10_),
                                    static_cast<T>(v11_), static_cast<T>(v12_), static_cast<T>(v13_), static_cast<T>(v14_),
                                    static_cast<T>(v15_), static_cast<T>(v16_), static_cast<T>(v17_), static_cast<T>(v18_),
                                    static_cast<T>(v19_), static_cast<T>(v20_), static_cast<T>(v21_), static_cast<T>(v22_),
                                    static_cast<T>(v23_) };
                return ValuesIn(array);
            }
        private:
            void operator=(const ValueArray23& other);
            const T1 v1_;
            const T2 v2_;
            const T3 v3_;
            const T4 v4_;
            const T5 v5_;
            const T6 v6_;
            const T7 v7_;
            const T8 v8_;
            const T9 v9_;
            const T10 v10_;
            const T11 v11_;
            const T12 v12_;
            const T13 v13_;
            const T14 v14_;
            const T15 v15_;
            const T16 v16_;
            const T17 v17_;
            const T18 v18_;
            const T19 v19_;
            const T20 v20_;
            const T21 v21_;
            const T22 v22_;
            const T23 v23_;
        };
        template <typename T1, typename T2, typename T3, typename T4, typename T5,
            typename T6, typename T7, typename T8, typename T9, typename T10,
            typename T11, typename T12, typename T13, typename T14, typename T15,
            typename T16, typename T17, typename T18, typename T19, typename T20,
            typename T21, typename T22, typename T23, typename T24> class ValueArray24 {
        public:
            ValueArray24(T1 v1, T2 v2, T3 v3, T4 v4, T5 v5, T6 v6, T7 v7, T8 v8, T9 v9, T10 v10, T11 v11, T12 v12, T13 v13, T14 v14, T15 v15,
                         T16 v16, T17 v17, T18 v18, T19 v19, T20 v20, T21 v21, T22 v22, T23 v23, T24 v24) : v1_(v1), v2_(v2), v3_(v3), v4_(v4),
                         v5_(v5), v6_(v6), v7_(v7), v8_(v8), v9_(v9), v10_(v10), v11_(v11), v12_(v12), v13_(v13), v14_(v14), v15_(v15),
                         v16_(v16), v17_(v17), v18_(v18), v19_(v19), v20_(v20), v21_(v21), v22_(v22), v23_(v23), v24_(v24) {}
            template <typename T> operator ParamGenerator<T>() const {
                const T array[] = { static_cast<T>(v1_), static_cast<T>(v2_), static_cast<T>(v3_), static_cast<T>(v4_), static_cast<T>(v5_),
                                    static_cast<T>(v6_), static_cast<T>(v7_), static_cast<T>(v8_), static_cast<T>(v9_), static_cast<T>(v10_),
                                    static_cast<T>(v11_), static_cast<T>(v12_), static_cast<T>(v13_), static_cast<T>(v14_),
                                    static_cast<T>(v15_), static_cast<T>(v16_), static_cast<T>(v17_), static_cast<T>(v18_),
                                    static_cast<T>(v19_), static_cast<T>(v20_), static_cast<T>(v21_), static_cast<T>(v22_),
                                    static_cast<T>(v23_), static_cast<T>(v24_) };
                return ValuesIn(array);
            }
        private:
            void operator=(const ValueArray24& other);
            const T1 v1_;
            const T2 v2_;
            const T3 v3_;
            const T4 v4_;
            const T5 v5_;
            const T6 v6_;
            const T7 v7_;
            const T8 v8_;
            const T9 v9_;
            const T10 v10_;
            const T11 v11_;
            const T12 v12_;
            const T13 v13_;
            const T14 v14_;
            const T15 v15_;
            const T16 v16_;
            const T17 v17_;
            const T18 v18_;
            const T19 v19_;
            const T20 v20_;
            const T21 v21_;
            const T22 v22_;
            const T23 v23_;
            const T24 v24_;
        };
        template <typename T1, typename T2, typename T3, typename T4, typename T5,
            typename T6, typename T7, typename T8, typename T9, typename T10,
            typename T11, typename T12, typename T13, typename T14, typename T15,
            typename T16, typename T17, typename T18, typename T19, typename T20,
            typename T21, typename T22, typename T23, typename T24, typename T25> class ValueArray25 {
        public:
            ValueArray25(T1 v1, T2 v2, T3 v3, T4 v4, T5 v5, T6 v6, T7 v7, T8 v8, T9 v9, T10 v10, T11 v11, T12 v12, T13 v13, T14 v14, T15 v15,
                         T16 v16, T17 v17, T18 v18, T19 v19, T20 v20, T21 v21, T22 v22, T23 v23, T24 v24, T25 v25) : v1_(v1), v2_(v2), v3_(v3),
                         v4_(v4), v5_(v5), v6_(v6), v7_(v7), v8_(v8), v9_(v9), v10_(v10), v11_(v11), v12_(v12), v13_(v13), v14_(v14),
                         v15_(v15), v16_(v16), v17_(v17), v18_(v18), v19_(v19), v20_(v20), v21_(v21), v22_(v22), v23_(v23), v24_(v24),
                         v25_(v25) {}
            template <typename T> operator ParamGenerator<T>() const {
                const T array[] = { static_cast<T>(v1_), static_cast<T>(v2_), static_cast<T>(v3_), static_cast<T>(v4_), static_cast<T>(v5_),
                                    static_cast<T>(v6_), static_cast<T>(v7_), static_cast<T>(v8_), static_cast<T>(v9_), static_cast<T>(v10_),
                                    static_cast<T>(v11_), static_cast<T>(v12_), static_cast<T>(v13_), static_cast<T>(v14_),
                                    static_cast<T>(v15_), static_cast<T>(v16_), static_cast<T>(v17_), static_cast<T>(v18_),
                                    static_cast<T>(v19_), static_cast<T>(v20_), static_cast<T>(v21_), static_cast<T>(v22_),
                                    static_cast<T>(v23_), static_cast<T>(v24_), static_cast<T>(v25_) };
                return ValuesIn(array);
            }
        private:
            void operator=(const ValueArray25& other);
            const T1 v1_;
            const T2 v2_;
            const T3 v3_;
            const T4 v4_;
            const T5 v5_;
            const T6 v6_;
            const T7 v7_;
            const T8 v8_;
            const T9 v9_;
            const T10 v10_;
            const T11 v11_;
            const T12 v12_;
            const T13 v13_;
            const T14 v14_;
            const T15 v15_;
            const T16 v16_;
            const T17 v17_;
            const T18 v18_;
            const T19 v19_;
            const T20 v20_;
            const T21 v21_;
            const T22 v22_;
            const T23 v23_;
            const T24 v24_;
            const T25 v25_;
        };
        template <typename T1, typename T2, typename T3, typename T4, typename T5,
            typename T6, typename T7, typename T8, typename T9, typename T10,
            typename T11, typename T12, typename T13, typename T14, typename T15,
            typename T16, typename T17, typename T18, typename T19, typename T20,
            typename T21, typename T22, typename T23, typename T24, typename T25,
            typename T26> class ValueArray26 {
        public:
            ValueArray26(T1 v1, T2 v2, T3 v3, T4 v4, T5 v5, T6 v6, T7 v7, T8 v8, T9 v9, T10 v10, T11 v11, T12 v12, T13 v13, T14 v14, T15 v15,
                         T16 v16, T17 v17, T18 v18, T19 v19, T20 v20, T21 v21, T22 v22, T23 v23, T24 v24, T25 v25, T26 v26) : v1_(v1), v2_(v2),
                         v3_(v3), v4_(v4), v5_(v5), v6_(v6), v7_(v7), v8_(v8), v9_(v9), v10_(v10), v11_(v11), v12_(v12), v13_(v13), v14_(v14),
                         v15_(v15), v16_(v16), v17_(v17), v18_(v18), v19_(v19), v20_(v20), v21_(v21), v22_(v22), v23_(v23), v24_(v24),
                         v25_(v25), v26_(v26) {}
            template <typename T> operator ParamGenerator<T>() const {
                const T array[] = { static_cast<T>(v1_), static_cast<T>(v2_), static_cast<T>(v3_), static_cast<T>(v4_), static_cast<T>(v5_),
                                    static_cast<T>(v6_), static_cast<T>(v7_), static_cast<T>(v8_), static_cast<T>(v9_), static_cast<T>(v10_),
                                    static_cast<T>(v11_), static_cast<T>(v12_), static_cast<T>(v13_), static_cast<T>(v14_),
                                    static_cast<T>(v15_), static_cast<T>(v16_), static_cast<T>(v17_), static_cast<T>(v18_),
                                    static_cast<T>(v19_), static_cast<T>(v20_), static_cast<T>(v21_), static_cast<T>(v22_),
                                    static_cast<T>(v23_), static_cast<T>(v24_), static_cast<T>(v25_), static_cast<T>(v26_) };
                return ValuesIn(array);
            }
        private:
            void operator=(const ValueArray26& other);
            const T1 v1_;
            const T2 v2_;
            const T3 v3_;
            const T4 v4_;
            const T5 v5_;
            const T6 v6_;
            const T7 v7_;
            const T8 v8_;
            const T9 v9_;
            const T10 v10_;
            const T11 v11_;
            const T12 v12_;
            const T13 v13_;
            const T14 v14_;
            const T15 v15_;
            const T16 v16_;
            const T17 v17_;
            const T18 v18_;
            const T19 v19_;
            const T20 v20_;
            const T21 v21_;
            const T22 v22_;
            const T23 v23_;
            const T24 v24_;
            const T25 v25_;
            const T26 v26_;
        };
        template <typename T1, typename T2, typename T3, typename T4, typename T5,
            typename T6, typename T7, typename T8, typename T9, typename T10,
            typename T11, typename T12, typename T13, typename T14, typename T15,
            typename T16, typename T17, typename T18, typename T19, typename T20,
            typename T21, typename T22, typename T23, typename T24, typename T25,
            typename T26, typename T27> class ValueArray27 {
        public:
            ValueArray27(T1 v1, T2 v2, T3 v3, T4 v4, T5 v5, T6 v6, T7 v7, T8 v8, T9 v9, T10 v10, T11 v11, T12 v12, T13 v13, T14 v14, T15 v15,
                         T16 v16, T17 v17, T18 v18, T19 v19, T20 v20, T21 v21, T22 v22, T23 v23, T24 v24, T25 v25, T26 v26, T27 v27) : v1_(v1),
                         v2_(v2), v3_(v3), v4_(v4), v5_(v5), v6_(v6), v7_(v7), v8_(v8), v9_(v9), v10_(v10), v11_(v11), v12_(v12), v13_(v13),
                         v14_(v14), v15_(v15), v16_(v16), v17_(v17), v18_(v18), v19_(v19), v20_(v20), v21_(v21), v22_(v22), v23_(v23),
                         v24_(v24), v25_(v25), v26_(v26), v27_(v27) {}
            template <typename T> operator ParamGenerator<T>() const {
                const T array[] = { static_cast<T>(v1_), static_cast<T>(v2_), static_cast<T>(v3_), static_cast<T>(v4_), static_cast<T>(v5_),
                                    static_cast<T>(v6_), static_cast<T>(v7_), static_cast<T>(v8_), static_cast<T>(v9_), static_cast<T>(v10_),
                                    static_cast<T>(v11_), static_cast<T>(v12_), static_cast<T>(v13_), static_cast<T>(v14_),
                                    static_cast<T>(v15_), static_cast<T>(v16_), static_cast<T>(v17_), static_cast<T>(v18_),
                                    static_cast<T>(v19_), static_cast<T>(v20_), static_cast<T>(v21_), static_cast<T>(v22_),
                                    static_cast<T>(v23_), static_cast<T>(v24_), static_cast<T>(v25_), static_cast<T>(v26_),
                                    static_cast<T>(v27_) };
                return ValuesIn(array);
            }
        private:
            void operator=(const ValueArray27& other);
            const T1 v1_;
            const T2 v2_;
            const T3 v3_;
            const T4 v4_;
            const T5 v5_;
            const T6 v6_;
            const T7 v7_;
            const T8 v8_;
            const T9 v9_;
            const T10 v10_;
            const T11 v11_;
            const T12 v12_;
            const T13 v13_;
            const T14 v14_;
            const T15 v15_;
            const T16 v16_;
            const T17 v17_;
            const T18 v18_;
            const T19 v19_;
            const T20 v20_;
            const T21 v21_;
            const T22 v22_;
            const T23 v23_;
            const T24 v24_;
            const T25 v25_;
            const T26 v26_;
            const T27 v27_;
        };
        template <typename T1, typename T2, typename T3, typename T4, typename T5,
            typename T6, typename T7, typename T8, typename T9, typename T10,
            typename T11, typename T12, typename T13, typename T14, typename T15,
            typename T16, typename T17, typename T18, typename T19, typename T20,
            typename T21, typename T22, typename T23, typename T24, typename T25,
            typename T26, typename T27, typename T28> class ValueArray28 {
        public:
            ValueArray28(T1 v1, T2 v2, T3 v3, T4 v4, T5 v5, T6 v6, T7 v7, T8 v8, T9 v9, T10 v10, T11 v11, T12 v12, T13 v13, T14 v14, T15 v15,
                         T16 v16, T17 v17, T18 v18, T19 v19, T20 v20, T21 v21, T22 v22, T23 v23, T24 v24, T25 v25, T26 v26, T27 v27, T28 v28) :
                         v1_(v1), v2_(v2), v3_(v3), v4_(v4), v5_(v5), v6_(v6), v7_(v7), v8_(v8), v9_(v9), v10_(v10), v11_(v11), v12_(v12),
                         v13_(v13), v14_(v14), v15_(v15), v16_(v16), v17_(v17), v18_(v18), v19_(v19), v20_(v20), v21_(v21), v22_(v22),
                         v23_(v23), v24_(v24), v25_(v25), v26_(v26), v27_(v27), v28_(v28) {}
            template <typename T> operator ParamGenerator<T>() const {
                const T array[] = { static_cast<T>(v1_), static_cast<T>(v2_), static_cast<T>(v3_), static_cast<T>(v4_), static_cast<T>(v5_),
                                    static_cast<T>(v6_), static_cast<T>(v7_), static_cast<T>(v8_), static_cast<T>(v9_), static_cast<T>(v10_),
                                    static_cast<T>(v11_), static_cast<T>(v12_), static_cast<T>(v13_), static_cast<T>(v14_),
                                    static_cast<T>(v15_), static_cast<T>(v16_), static_cast<T>(v17_), static_cast<T>(v18_),
                                    static_cast<T>(v19_), static_cast<T>(v20_), static_cast<T>(v21_), static_cast<T>(v22_),
                                    static_cast<T>(v23_), static_cast<T>(v24_), static_cast<T>(v25_), static_cast<T>(v26_),
                                    static_cast<T>(v27_), static_cast<T>(v28_) };
                return ValuesIn(array);
            }
        private:
            void operator=(const ValueArray28& other);
            const T1 v1_;
            const T2 v2_;
            const T3 v3_;
            const T4 v4_;
            const T5 v5_;
            const T6 v6_;
            const T7 v7_;
            const T8 v8_;
            const T9 v9_;
            const T10 v10_;
            const T11 v11_;
            const T12 v12_;
            const T13 v13_;
            const T14 v14_;
            const T15 v15_;
            const T16 v16_;
            const T17 v17_;
            const T18 v18_;
            const T19 v19_;
            const T20 v20_;
            const T21 v21_;
            const T22 v22_;
            const T23 v23_;
            const T24 v24_;
            const T25 v25_;
            const T26 v26_;
            const T27 v27_;
            const T28 v28_;
        };
        template <typename T1, typename T2, typename T3, typename T4, typename T5,
            typename T6, typename T7, typename T8, typename T9, typename T10,
            typename T11, typename T12, typename T13, typename T14, typename T15,
            typename T16, typename T17, typename T18, typename T19, typename T20,
            typename T21, typename T22, typename T23, typename T24, typename T25,
            typename T26, typename T27, typename T28, typename T29> class ValueArray29 {
        public:
            ValueArray29(T1 v1, T2 v2, T3 v3, T4 v4, T5 v5, T6 v6, T7 v7, T8 v8, T9 v9, T10 v10, T11 v11, T12 v12, T13 v13, T14 v14, T15 v15,
                         T16 v16, T17 v17, T18 v18, T19 v19, T20 v20, T21 v21, T22 v22, T23 v23, T24 v24, T25 v25, T26 v26, T27 v27, T28 v28,
                         T29 v29) : v1_(v1), v2_(v2), v3_(v3), v4_(v4), v5_(v5), v6_(v6), v7_(v7), v8_(v8), v9_(v9), v10_(v10), v11_(v11),
                         v12_(v12), v13_(v13), v14_(v14), v15_(v15), v16_(v16), v17_(v17), v18_(v18), v19_(v19), v20_(v20), v21_(v21),
                         v22_(v22), v23_(v23), v24_(v24), v25_(v25), v26_(v26), v27_(v27), v28_(v28), v29_(v29) {}
            template <typename T> operator ParamGenerator<T>() const {
                const T array[] = { static_cast<T>(v1_), static_cast<T>(v2_), static_cast<T>(v3_), static_cast<T>(v4_), static_cast<T>(v5_),
                                    static_cast<T>(v6_), static_cast<T>(v7_), static_cast<T>(v8_), static_cast<T>(v9_), static_cast<T>(v10_),
                                    static_cast<T>(v11_), static_cast<T>(v12_), static_cast<T>(v13_), static_cast<T>(v14_),
                                    static_cast<T>(v15_), static_cast<T>(v16_), static_cast<T>(v17_), static_cast<T>(v18_),
                                    static_cast<T>(v19_), static_cast<T>(v20_), static_cast<T>(v21_), static_cast<T>(v22_),
                                    static_cast<T>(v23_), static_cast<T>(v24_), static_cast<T>(v25_), static_cast<T>(v26_),
                                    static_cast<T>(v27_), static_cast<T>(v28_), static_cast<T>(v29_) };
                return ValuesIn(array);
            }
        private:
            void operator=(const ValueArray29& other);
            const T1 v1_;
            const T2 v2_;
            const T3 v3_;
            const T4 v4_;
            const T5 v5_;
            const T6 v6_;
            const T7 v7_;
            const T8 v8_;
            const T9 v9_;
            const T10 v10_;
            const T11 v11_;
            const T12 v12_;
            const T13 v13_;
            const T14 v14_;
            const T15 v15_;
            const T16 v16_;
            const T17 v17_;
            const T18 v18_;
            const T19 v19_;
            const T20 v20_;
            const T21 v21_;
            const T22 v22_;
            const T23 v23_;
            const T24 v24_;
            const T25 v25_;
            const T26 v26_;
            const T27 v27_;
            const T28 v28_;
            const T29 v29_;
        };
        template <typename T1, typename T2, typename T3, typename T4, typename T5,
            typename T6, typename T7, typename T8, typename T9, typename T10,
            typename T11, typename T12, typename T13, typename T14, typename T15,
            typename T16, typename T17, typename T18, typename T19, typename T20,
            typename T21, typename T22, typename T23, typename T24, typename T25,
            typename T26, typename T27, typename T28, typename T29, typename T30> class ValueArray30 {
        public:
            ValueArray30(T1 v1, T2 v2, T3 v3, T4 v4, T5 v5, T6 v6, T7 v7, T8 v8, T9 v9, T10 v10, T11 v11, T12 v12, T13 v13, T14 v14, T15 v15,
                         T16 v16, T17 v17, T18 v18, T19 v19, T20 v20, T21 v21, T22 v22, T23 v23, T24 v24, T25 v25, T26 v26, T27 v27, T28 v28,
                         T29 v29, T30 v30) : v1_(v1), v2_(v2), v3_(v3), v4_(v4), v5_(v5), v6_(v6), v7_(v7), v8_(v8), v9_(v9), v10_(v10),
                         v11_(v11), v12_(v12), v13_(v13), v14_(v14), v15_(v15), v16_(v16), v17_(v17), v18_(v18), v19_(v19), v20_(v20),
                         v21_(v21), v22_(v22), v23_(v23), v24_(v24), v25_(v25), v26_(v26), v27_(v27), v28_(v28), v29_(v29), v30_(v30) {}
            template <typename T> operator ParamGenerator<T>() const {
                const T array[] = { static_cast<T>(v1_), static_cast<T>(v2_), static_cast<T>(v3_), static_cast<T>(v4_), static_cast<T>(v5_),
                                    static_cast<T>(v6_), static_cast<T>(v7_), static_cast<T>(v8_), static_cast<T>(v9_), static_cast<T>(v10_),
                                    static_cast<T>(v11_), static_cast<T>(v12_), static_cast<T>(v13_), static_cast<T>(v14_),
                                    static_cast<T>(v15_), static_cast<T>(v16_), static_cast<T>(v17_), static_cast<T>(v18_),
                                    static_cast<T>(v19_), static_cast<T>(v20_), static_cast<T>(v21_), static_cast<T>(v22_),
                                    static_cast<T>(v23_), static_cast<T>(v24_), static_cast<T>(v25_), static_cast<T>(v26_),
                                    static_cast<T>(v27_), static_cast<T>(v28_), static_cast<T>(v29_), static_cast<T>(v30_) };
                return ValuesIn(array);
            }
        private:
            void operator=(const ValueArray30& other);
            const T1 v1_;
            const T2 v2_;
            const T3 v3_;
            const T4 v4_;
            const T5 v5_;
            const T6 v6_;
            const T7 v7_;
            const T8 v8_;
            const T9 v9_;
            const T10 v10_;
            const T11 v11_;
            const T12 v12_;
            const T13 v13_;
            const T14 v14_;
            const T15 v15_;
            const T16 v16_;
            const T17 v17_;
            const T18 v18_;
            const T19 v19_;
            const T20 v20_;
            const T21 v21_;
            const T22 v22_;
            const T23 v23_;
            const T24 v24_;
            const T25 v25_;
            const T26 v26_;
            const T27 v27_;
            const T28 v28_;
            const T29 v29_;
            const T30 v30_;
        };
        template <typename T1, typename T2, typename T3, typename T4, typename T5,
            typename T6, typename T7, typename T8, typename T9, typename T10,
            typename T11, typename T12, typename T13, typename T14, typename T15,
            typename T16, typename T17, typename T18, typename T19, typename T20,
            typename T21, typename T22, typename T23, typename T24, typename T25,
            typename T26, typename T27, typename T28, typename T29, typename T30,
            typename T31>
        class ValueArray31 {
        public:
            ValueArray31(T1 v1, T2 v2, T3 v3, T4 v4, T5 v5, T6 v6, T7 v7, T8 v8, T9 v9, T10 v10, T11 v11, T12 v12, T13 v13, T14 v14, T15 v15,
                         T16 v16, T17 v17, T18 v18, T19 v19, T20 v20, T21 v21, T22 v22, T23 v23, T24 v24, T25 v25, T26 v26, T27 v27, T28 v28,
                         T29 v29, T30 v30, T31 v31) : v1_(v1), v2_(v2), v3_(v3), v4_(v4), v5_(v5), v6_(v6), v7_(v7), v8_(v8), v9_(v9),
                         v10_(v10), v11_(v11), v12_(v12), v13_(v13), v14_(v14), v15_(v15), v16_(v16), v17_(v17), v18_(v18), v19_(v19),
                         v20_(v20), v21_(v21), v22_(v22), v23_(v23), v24_(v24), v25_(v25), v26_(v26), v27_(v27), v28_(v28), v29_(v29),
                         v30_(v30), v31_(v31) {}
            template <typename T> operator ParamGenerator<T>() const {
                const T array[] = { static_cast<T>(v1_), static_cast<T>(v2_), static_cast<T>(v3_), static_cast<T>(v4_), static_cast<T>(v5_),
                                    static_cast<T>(v6_), static_cast<T>(v7_), static_cast<T>(v8_), static_cast<T>(v9_), static_cast<T>(v10_),
                                    static_cast<T>(v11_), static_cast<T>(v12_), static_cast<T>(v13_), static_cast<T>(v14_),
                                    static_cast<T>(v15_), static_cast<T>(v16_), static_cast<T>(v17_), static_cast<T>(v18_),
                                    static_cast<T>(v19_), static_cast<T>(v20_), static_cast<T>(v21_), static_cast<T>(v22_),
                                    static_cast<T>(v23_), static_cast<T>(v24_), static_cast<T>(v25_), static_cast<T>(v26_),
                                    static_cast<T>(v27_), static_cast<T>(v28_), static_cast<T>(v29_), static_cast<T>(v30_),
                                    static_cast<T>(v31_) };
                return ValuesIn(array);
            }
        private:
            void operator=(const ValueArray31& other);
            const T1 v1_;
            const T2 v2_;
            const T3 v3_;
            const T4 v4_;
            const T5 v5_;
            const T6 v6_;
            const T7 v7_;
            const T8 v8_;
            const T9 v9_;
            const T10 v10_;
            const T11 v11_;
            const T12 v12_;
            const T13 v13_;
            const T14 v14_;
            const T15 v15_;
            const T16 v16_;
            const T17 v17_;
            const T18 v18_;
            const T19 v19_;
            const T20 v20_;
            const T21 v21_;
            const T22 v22_;
            const T23 v23_;
            const T24 v24_;
            const T25 v25_;
            const T26 v26_;
            const T27 v27_;
            const T28 v28_;
            const T29 v29_;
            const T30 v30_;
            const T31 v31_;
        };
        template <typename T1, typename T2, typename T3, typename T4, typename T5,
            typename T6, typename T7, typename T8, typename T9, typename T10,
            typename T11, typename T12, typename T13, typename T14, typename T15,
            typename T16, typename T17, typename T18, typename T19, typename T20,
            typename T21, typename T22, typename T23, typename T24, typename T25,
            typename T26, typename T27, typename T28, typename T29, typename T30,
            typename T31, typename T32>
        class ValueArray32 {
        public:
            ValueArray32(T1 v1, T2 v2, T3 v3, T4 v4, T5 v5, T6 v6, T7 v7, T8 v8, T9 v9, T10 v10, T11 v11, T12 v12, T13 v13, T14 v14, T15 v15,
                         T16 v16, T17 v17, T18 v18, T19 v19, T20 v20, T21 v21, T22 v22, T23 v23, T24 v24, T25 v25, T26 v26, T27 v27, T28 v28,
                         T29 v29, T30 v30, T31 v31, T32 v32) : v1_(v1), v2_(v2), v3_(v3), v4_(v4), v5_(v5), v6_(v6), v7_(v7), v8_(v8),
                         v9_(v9), v10_(v10), v11_(v11), v12_(v12), v13_(v13), v14_(v14), v15_(v15), v16_(v16), v17_(v17), v18_(v18),
                         v19_(v19), v20_(v20), v21_(v21), v22_(v22), v23_(v23), v24_(v24), v25_(v25), v26_(v26), v27_(v27), v28_(v28),
                         v29_(v29), v30_(v30), v31_(v31), v32_(v32) {}
            template <typename T> operator ParamGenerator<T>() const {
                const T array[] = { static_cast<T>(v1_), static_cast<T>(v2_), static_cast<T>(v3_), static_cast<T>(v4_), static_cast<T>(v5_),
                                    static_cast<T>(v6_), static_cast<T>(v7_), static_cast<T>(v8_), static_cast<T>(v9_), static_cast<T>(v10_),
                                    static_cast<T>(v11_), static_cast<T>(v12_), static_cast<T>(v13_), static_cast<T>(v14_),
                                    static_cast<T>(v15_), static_cast<T>(v16_), static_cast<T>(v17_), static_cast<T>(v18_),
                                    static_cast<T>(v19_), static_cast<T>(v20_), static_cast<T>(v21_), static_cast<T>(v22_),
                                    static_cast<T>(v23_), static_cast<T>(v24_), static_cast<T>(v25_), static_cast<T>(v26_),
                                    static_cast<T>(v27_), static_cast<T>(v28_), static_cast<T>(v29_), static_cast<T>(v30_),
                                    static_cast<T>(v31_), static_cast<T>(v32_) };
                return ValuesIn(array);
            }
        private:
            void operator=(const ValueArray32& other);
            const T1 v1_;
            const T2 v2_;
            const T3 v3_;
            const T4 v4_;
            const T5 v5_;
            const T6 v6_;
            const T7 v7_;
            const T8 v8_;
            const T9 v9_;
            const T10 v10_;
            const T11 v11_;
            const T12 v12_;
            const T13 v13_;
            const T14 v14_;
            const T15 v15_;
            const T16 v16_;
            const T17 v17_;
            const T18 v18_;
            const T19 v19_;
            const T20 v20_;
            const T21 v21_;
            const T22 v22_;
            const T23 v23_;
            const T24 v24_;
            const T25 v25_;
            const T26 v26_;
            const T27 v27_;
            const T28 v28_;
            const T29 v29_;
            const T30 v30_;
            const T31 v31_;
            const T32 v32_;
        };
        template <typename T1, typename T2, typename T3, typename T4, typename T5,
            typename T6, typename T7, typename T8, typename T9, typename T10,
            typename T11, typename T12, typename T13, typename T14, typename T15,
            typename T16, typename T17, typename T18, typename T19, typename T20,
            typename T21, typename T22, typename T23, typename T24, typename T25,
            typename T26, typename T27, typename T28, typename T29, typename T30,
            typename T31, typename T32, typename T33>
        class ValueArray33 {
        public:
            ValueArray33(T1 v1, T2 v2, T3 v3, T4 v4, T5 v5, T6 v6, T7 v7, T8 v8, T9 v9, T10 v10, T11 v11, T12 v12, T13 v13, T14 v14, T15 v15,
                         T16 v16, T17 v17, T18 v18, T19 v19, T20 v20, T21 v21, T22 v22, T23 v23, T24 v24, T25 v25, T26 v26, T27 v27, T28 v28,
                         T29 v29, T30 v30, T31 v31, T32 v32, T33 v33) : v1_(v1), v2_(v2), v3_(v3), v4_(v4), v5_(v5), v6_(v6), v7_(v7), v8_(v8),
                         v9_(v9), v10_(v10), v11_(v11), v12_(v12), v13_(v13), v14_(v14), v15_(v15), v16_(v16), v17_(v17), v18_(v18), v19_(v19),
                         v20_(v20), v21_(v21), v22_(v22), v23_(v23), v24_(v24), v25_(v25), v26_(v26), v27_(v27), v28_(v28), v29_(v29),
                         v30_(v30), v31_(v31), v32_(v32), v33_(v33) {}
            template <typename T> operator ParamGenerator<T>() const {
                const T array[] = { static_cast<T>(v1_), static_cast<T>(v2_), static_cast<T>(v3_), static_cast<T>(v4_), static_cast<T>(v5_),
                                    static_cast<T>(v6_), static_cast<T>(v7_), static_cast<T>(v8_), static_cast<T>(v9_), static_cast<T>(v10_),
                                    static_cast<T>(v11_), static_cast<T>(v12_), static_cast<T>(v13_), static_cast<T>(v14_),
                                    static_cast<T>(v15_), static_cast<T>(v16_), static_cast<T>(v17_), static_cast<T>(v18_),
                                    static_cast<T>(v19_), static_cast<T>(v20_), static_cast<T>(v21_), static_cast<T>(v22_),
                                    static_cast<T>(v23_), static_cast<T>(v24_), static_cast<T>(v25_), static_cast<T>(v26_),
                                    static_cast<T>(v27_), static_cast<T>(v28_), static_cast<T>(v29_), static_cast<T>(v30_),
                                    static_cast<T>(v31_), static_cast<T>(v32_), static_cast<T>(v33_) };
                return ValuesIn(array);
            }
        private:
            void operator=(const ValueArray33& other);
            const T1 v1_;
            const T2 v2_;
            const T3 v3_;
            const T4 v4_;
            const T5 v5_;
            const T6 v6_;
            const T7 v7_;
            const T8 v8_;
            const T9 v9_;
            const T10 v10_;
            const T11 v11_;
            const T12 v12_;
            const T13 v13_;
            const T14 v14_;
            const T15 v15_;
            const T16 v16_;
            const T17 v17_;
            const T18 v18_;
            const T19 v19_;
            const T20 v20_;
            const T21 v21_;
            const T22 v22_;
            const T23 v23_;
            const T24 v24_;
            const T25 v25_;
            const T26 v26_;
            const T27 v27_;
            const T28 v28_;
            const T29 v29_;
            const T30 v30_;
            const T31 v31_;
            const T32 v32_;
            const T33 v33_;
        };
        template <typename T1, typename T2, typename T3, typename T4, typename T5,
            typename T6, typename T7, typename T8, typename T9, typename T10,
            typename T11, typename T12, typename T13, typename T14, typename T15,
            typename T16, typename T17, typename T18, typename T19, typename T20,
            typename T21, typename T22, typename T23, typename T24, typename T25,
            typename T26, typename T27, typename T28, typename T29, typename T30,
            typename T31, typename T32, typename T33, typename T34>
        class ValueArray34 {
        public:
            ValueArray34(T1 v1, T2 v2, T3 v3, T4 v4, T5 v5, T6 v6, T7 v7, T8 v8, T9 v9, T10 v10, T11 v11, T12 v12, T13 v13, T14 v14, T15 v15,
                         T16 v16, T17 v17, T18 v18, T19 v19, T20 v20, T21 v21, T22 v22, T23 v23, T24 v24, T25 v25, T26 v26, T27 v27, T28 v28,
                         T29 v29, T30 v30, T31 v31, T32 v32, T33 v33, T34 v34) : v1_(v1), v2_(v2), v3_(v3), v4_(v4), v5_(v5), v6_(v6), v7_(v7),
                         v8_(v8), v9_(v9), v10_(v10), v11_(v11), v12_(v12), v13_(v13), v14_(v14), v15_(v15), v16_(v16), v17_(v17), v18_(v18),
                         v19_(v19), v20_(v20), v21_(v21), v22_(v22), v23_(v23), v24_(v24), v25_(v25), v26_(v26), v27_(v27), v28_(v28),
                         v29_(v29), v30_(v30), v31_(v31), v32_(v32), v33_(v33), v34_(v34) {}
            template <typename T> operator ParamGenerator<T>() const {
                const T array[] = { static_cast<T>(v1_), static_cast<T>(v2_), static_cast<T>(v3_), static_cast<T>(v4_), static_cast<T>(v5_),
                                    static_cast<T>(v6_), static_cast<T>(v7_), static_cast<T>(v8_), static_cast<T>(v9_), static_cast<T>(v10_),
                                    static_cast<T>(v11_), static_cast<T>(v12_), static_cast<T>(v13_), static_cast<T>(v14_),
                                    static_cast<T>(v15_), static_cast<T>(v16_), static_cast<T>(v17_), static_cast<T>(v18_),
                                    static_cast<T>(v19_), static_cast<T>(v20_), static_cast<T>(v21_), static_cast<T>(v22_),
                                    static_cast<T>(v23_), static_cast<T>(v24_), static_cast<T>(v25_), static_cast<T>(v26_),
                                    static_cast<T>(v27_), static_cast<T>(v28_), static_cast<T>(v29_), static_cast<T>(v30_),
                                    static_cast<T>(v31_), static_cast<T>(v32_), static_cast<T>(v33_), static_cast<T>(v34_) };
                return ValuesIn(array);
            }
        private:
            void operator=(const ValueArray34& other);
            const T1 v1_;
            const T2 v2_;
            const T3 v3_;
            const T4 v4_;
            const T5 v5_;
            const T6 v6_;
            const T7 v7_;
            const T8 v8_;
            const T9 v9_;
            const T10 v10_;
            const T11 v11_;
            const T12 v12_;
            const T13 v13_;
            const T14 v14_;
            const T15 v15_;
            const T16 v16_;
            const T17 v17_;
            const T18 v18_;
            const T19 v19_;
            const T20 v20_;
            const T21 v21_;
            const T22 v22_;
            const T23 v23_;
            const T24 v24_;
            const T25 v25_;
            const T26 v26_;
            const T27 v27_;
            const T28 v28_;
            const T29 v29_;
            const T30 v30_;
            const T31 v31_;
            const T32 v32_;
            const T33 v33_;
            const T34 v34_;
        };
        template <typename T1, typename T2, typename T3, typename T4, typename T5,
            typename T6, typename T7, typename T8, typename T9, typename T10,
            typename T11, typename T12, typename T13, typename T14, typename T15,
            typename T16, typename T17, typename T18, typename T19, typename T20,
            typename T21, typename T22, typename T23, typename T24, typename T25,
            typename T26, typename T27, typename T28, typename T29, typename T30,
            typename T31, typename T32, typename T33, typename T34, typename T35>
        class ValueArray35 {
        public:
            ValueArray35(T1 v1, T2 v2, T3 v3, T4 v4, T5 v5, T6 v6, T7 v7, T8 v8, T9 v9, T10 v10, T11 v11, T12 v12, T13 v13, T14 v14, T15 v15,
                         T16 v16, T17 v17, T18 v18, T19 v19, T20 v20, T21 v21, T22 v22, T23 v23, T24 v24, T25 v25, T26 v26, T27 v27, T28 v28,
                         T29 v29, T30 v30, T31 v31, T32 v32, T33 v33, T34 v34, T35 v35) : v1_(v1), v2_(v2), v3_(v3), v4_(v4), v5_(v5), v6_(v6),
                         v7_(v7), v8_(v8), v9_(v9), v10_(v10), v11_(v11), v12_(v12), v13_(v13), v14_(v14), v15_(v15), v16_(v16), v17_(v17),
                         v18_(v18), v19_(v19), v20_(v20), v21_(v21), v22_(v22), v23_(v23), v24_(v24), v25_(v25), v26_(v26), v27_(v27),
                         v28_(v28), v29_(v29), v30_(v30), v31_(v31), v32_(v32), v33_(v33), v34_(v34), v35_(v35) {}
            template <typename T> operator ParamGenerator<T>() const {
                const T array[] = { static_cast<T>(v1_), static_cast<T>(v2_), static_cast<T>(v3_), static_cast<T>(v4_), static_cast<T>(v5_),
                                    static_cast<T>(v6_), static_cast<T>(v7_), static_cast<T>(v8_), static_cast<T>(v9_), static_cast<T>(v10_),
                                    static_cast<T>(v11_), static_cast<T>(v12_), static_cast<T>(v13_), static_cast<T>(v14_),
                                    static_cast<T>(v15_), static_cast<T>(v16_), static_cast<T>(v17_), static_cast<T>(v18_),
                                    static_cast<T>(v19_), static_cast<T>(v20_), static_cast<T>(v21_), static_cast<T>(v22_),
                                    static_cast<T>(v23_), static_cast<T>(v24_), static_cast<T>(v25_), static_cast<T>(v26_),
                                    static_cast<T>(v27_), static_cast<T>(v28_), static_cast<T>(v29_), static_cast<T>(v30_),
                                    static_cast<T>(v31_), static_cast<T>(v32_), static_cast<T>(v33_), static_cast<T>(v34_),
                                    static_cast<T>(v35_) };
                return ValuesIn(array);
            }
        private:
            void operator=(const ValueArray35& other);
            const T1 v1_;
            const T2 v2_;
            const T3 v3_;
            const T4 v4_;
            const T5 v5_;
            const T6 v6_;
            const T7 v7_;
            const T8 v8_;
            const T9 v9_;
            const T10 v10_;
            const T11 v11_;
            const T12 v12_;
            const T13 v13_;
            const T14 v14_;
            const T15 v15_;
            const T16 v16_;
            const T17 v17_;
            const T18 v18_;
            const T19 v19_;
            const T20 v20_;
            const T21 v21_;
            const T22 v22_;
            const T23 v23_;
            const T24 v24_;
            const T25 v25_;
            const T26 v26_;
            const T27 v27_;
            const T28 v28_;
            const T29 v29_;
            const T30 v30_;
            const T31 v31_;
            const T32 v32_;
            const T33 v33_;
            const T34 v34_;
            const T35 v35_;
        };
        template <typename T1, typename T2, typename T3, typename T4, typename T5,
            typename T6, typename T7, typename T8, typename T9, typename T10,
            typename T11, typename T12, typename T13, typename T14, typename T15,
            typename T16, typename T17, typename T18, typename T19, typename T20,
            typename T21, typename T22, typename T23, typename T24, typename T25,
            typename T26, typename T27, typename T28, typename T29, typename T30,
            typename T31, typename T32, typename T33, typename T34, typename T35,
            typename T36>
        class ValueArray36 {
        public:
            ValueArray36(T1 v1, T2 v2, T3 v3, T4 v4, T5 v5, T6 v6, T7 v7, T8 v8, T9 v9, T10 v10, T11 v11, T12 v12, T13 v13, T14 v14, T15 v15,
                         T16 v16, T17 v17, T18 v18, T19 v19, T20 v20, T21 v21, T22 v22, T23 v23, T24 v24, T25 v25, T26 v26, T27 v27, T28 v28,
                         T29 v29, T30 v30, T31 v31, T32 v32, T33 v33, T34 v34, T35 v35, T36 v36) : v1_(v1), v2_(v2), v3_(v3), v4_(v4), v5_(v5),
                         v6_(v6), v7_(v7), v8_(v8), v9_(v9), v10_(v10), v11_(v11), v12_(v12), v13_(v13), v14_(v14), v15_(v15), v16_(v16),
                         v17_(v17), v18_(v18), v19_(v19), v20_(v20), v21_(v21), v22_(v22), v23_(v23), v24_(v24), v25_(v25), v26_(v26),
                         v27_(v27), v28_(v28), v29_(v29), v30_(v30), v31_(v31), v32_(v32), v33_(v33), v34_(v34), v35_(v35), v36_(v36) {}
            template <typename T> operator ParamGenerator<T>() const {
                const T array[] = { static_cast<T>(v1_), static_cast<T>(v2_), static_cast<T>(v3_), static_cast<T>(v4_), static_cast<T>(v5_),
                                    static_cast<T>(v6_), static_cast<T>(v7_), static_cast<T>(v8_), static_cast<T>(v9_), static_cast<T>(v10_),
                                    static_cast<T>(v11_), static_cast<T>(v12_), static_cast<T>(v13_), static_cast<T>(v14_),
                                    static_cast<T>(v15_), static_cast<T>(v16_), static_cast<T>(v17_), static_cast<T>(v18_),
                                    static_cast<T>(v19_), static_cast<T>(v20_), static_cast<T>(v21_), static_cast<T>(v22_),
                                    static_cast<T>(v23_), static_cast<T>(v24_), static_cast<T>(v25_), static_cast<T>(v26_),
                                    static_cast<T>(v27_), static_cast<T>(v28_), static_cast<T>(v29_), static_cast<T>(v30_),
                                    static_cast<T>(v31_), static_cast<T>(v32_), static_cast<T>(v33_), static_cast<T>(v34_),
                                    static_cast<T>(v35_), static_cast<T>(v36_) };
                return ValuesIn(array);
            }
        private:
            void operator=(const ValueArray36& other);
            const T1 v1_;
            const T2 v2_;
            const T3 v3_;
            const T4 v4_;
            const T5 v5_;
            const T6 v6_;
            const T7 v7_;
            const T8 v8_;
            const T9 v9_;
            const T10 v10_;
            const T11 v11_;
            const T12 v12_;
            const T13 v13_;
            const T14 v14_;
            const T15 v15_;
            const T16 v16_;
            const T17 v17_;
            const T18 v18_;
            const T19 v19_;
            const T20 v20_;
            const T21 v21_;
            const T22 v22_;
            const T23 v23_;
            const T24 v24_;
            const T25 v25_;
            const T26 v26_;
            const T27 v27_;
            const T28 v28_;
            const T29 v29_;
            const T30 v30_;
            const T31 v31_;
            const T32 v32_;
            const T33 v33_;
            const T34 v34_;
            const T35 v35_;
            const T36 v36_;
        };
        template <typename T1, typename T2, typename T3, typename T4, typename T5,
            typename T6, typename T7, typename T8, typename T9, typename T10,
            typename T11, typename T12, typename T13, typename T14, typename T15,
            typename T16, typename T17, typename T18, typename T19, typename T20,
            typename T21, typename T22, typename T23, typename T24, typename T25,
            typename T26, typename T27, typename T28, typename T29, typename T30,
            typename T31, typename T32, typename T33, typename T34, typename T35,
            typename T36, typename T37>
        class ValueArray37 {
        public:
            ValueArray37(T1 v1, T2 v2, T3 v3, T4 v4, T5 v5, T6 v6, T7 v7, T8 v8, T9 v9, T10 v10, T11 v11, T12 v12, T13 v13, T14 v14, T15 v15,
                         T16 v16, T17 v17, T18 v18, T19 v19, T20 v20, T21 v21, T22 v22, T23 v23, T24 v24, T25 v25, T26 v26, T27 v27, T28 v28,
                         T29 v29, T30 v30, T31 v31, T32 v32, T33 v33, T34 v34, T35 v35, T36 v36, T37 v37) : v1_(v1), v2_(v2), v3_(v3), v4_(v4),
                         v5_(v5), v6_(v6), v7_(v7), v8_(v8), v9_(v9), v10_(v10), v11_(v11), v12_(v12), v13_(v13), v14_(v14), v15_(v15),
                         v16_(v16), v17_(v17), v18_(v18), v19_(v19), v20_(v20), v21_(v21), v22_(v22), v23_(v23), v24_(v24), v25_(v25),
                         v26_(v26), v27_(v27), v28_(v28), v29_(v29), v30_(v30), v31_(v31), v32_(v32), v33_(v33), v34_(v34), v35_(v35),
                         v36_(v36), v37_(v37) {}
            template <typename T> operator ParamGenerator<T>() const {
                const T array[] = { static_cast<T>(v1_), static_cast<T>(v2_), static_cast<T>(v3_), static_cast<T>(v4_), static_cast<T>(v5_),
                                    static_cast<T>(v6_), static_cast<T>(v7_), static_cast<T>(v8_), static_cast<T>(v9_), static_cast<T>(v10_),
                                    static_cast<T>(v11_), static_cast<T>(v12_), static_cast<T>(v13_), static_cast<T>(v14_),
                                    static_cast<T>(v15_), static_cast<T>(v16_), static_cast<T>(v17_), static_cast<T>(v18_),
                                    static_cast<T>(v19_), static_cast<T>(v20_), static_cast<T>(v21_), static_cast<T>(v22_),
                                    static_cast<T>(v23_), static_cast<T>(v24_), static_cast<T>(v25_), static_cast<T>(v26_),
                                    static_cast<T>(v27_), static_cast<T>(v28_), static_cast<T>(v29_), static_cast<T>(v30_),
                                    static_cast<T>(v31_), static_cast<T>(v32_), static_cast<T>(v33_), static_cast<T>(v34_),
                                    static_cast<T>(v35_), static_cast<T>(v36_), static_cast<T>(v37_) };
                return ValuesIn(array);
            }
        private:
            void operator=(const ValueArray37& other);
            const T1 v1_;
            const T2 v2_;
            const T3 v3_;
            const T4 v4_;
            const T5 v5_;
            const T6 v6_;
            const T7 v7_;
            const T8 v8_;
            const T9 v9_;
            const T10 v10_;
            const T11 v11_;
            const T12 v12_;
            const T13 v13_;
            const T14 v14_;
            const T15 v15_;
            const T16 v16_;
            const T17 v17_;
            const T18 v18_;
            const T19 v19_;
            const T20 v20_;
            const T21 v21_;
            const T22 v22_;
            const T23 v23_;
            const T24 v24_;
            const T25 v25_;
            const T26 v26_;
            const T27 v27_;
            const T28 v28_;
            const T29 v29_;
            const T30 v30_;
            const T31 v31_;
            const T32 v32_;
            const T33 v33_;
            const T34 v34_;
            const T35 v35_;
            const T36 v36_;
            const T37 v37_;
        };
        template <typename T1, typename T2, typename T3, typename T4, typename T5,
            typename T6, typename T7, typename T8, typename T9, typename T10,
            typename T11, typename T12, typename T13, typename T14, typename T15,
            typename T16, typename T17, typename T18, typename T19, typename T20,
            typename T21, typename T22, typename T23, typename T24, typename T25,
            typename T26, typename T27, typename T28, typename T29, typename T30,
            typename T31, typename T32, typename T33, typename T34, typename T35,
            typename T36, typename T37, typename T38>
        class ValueArray38 {
        public:
            ValueArray38(T1 v1, T2 v2, T3 v3, T4 v4, T5 v5, T6 v6, T7 v7, T8 v8, T9 v9, T10 v10, T11 v11, T12 v12, T13 v13, T14 v14, T15 v15,
                         T16 v16, T17 v17, T18 v18, T19 v19, T20 v20, T21 v21, T22 v22, T23 v23, T24 v24, T25 v25, T26 v26, T27 v27, T28 v28,
                         T29 v29, T30 v30, T31 v31, T32 v32, T33 v33, T34 v34, T35 v35, T36 v36, T37 v37, T38 v38) : v1_(v1), v2_(v2), v3_(v3),
                         v4_(v4), v5_(v5), v6_(v6), v7_(v7), v8_(v8), v9_(v9), v10_(v10), v11_(v11), v12_(v12), v13_(v13), v14_(v14),
                         v15_(v15), v16_(v16), v17_(v17), v18_(v18), v19_(v19), v20_(v20), v21_(v21), v22_(v22), v23_(v23), v24_(v24),
                         v25_(v25), v26_(v26), v27_(v27), v28_(v28), v29_(v29), v30_(v30), v31_(v31), v32_(v32), v33_(v33), v34_(v34),
                         v35_(v35), v36_(v36), v37_(v37), v38_(v38) {}
            template <typename T> operator ParamGenerator<T>() const {
                const T array[] = { static_cast<T>(v1_), static_cast<T>(v2_), static_cast<T>(v3_), static_cast<T>(v4_), static_cast<T>(v5_),
                                    static_cast<T>(v6_), static_cast<T>(v7_), static_cast<T>(v8_), static_cast<T>(v9_), static_cast<T>(v10_),
                                    static_cast<T>(v11_), static_cast<T>(v12_), static_cast<T>(v13_), static_cast<T>(v14_),
                                    static_cast<T>(v15_), static_cast<T>(v16_), static_cast<T>(v17_), static_cast<T>(v18_),
                                    static_cast<T>(v19_), static_cast<T>(v20_), static_cast<T>(v21_), static_cast<T>(v22_),
                                    static_cast<T>(v23_), static_cast<T>(v24_), static_cast<T>(v25_), static_cast<T>(v26_),
                                    static_cast<T>(v27_), static_cast<T>(v28_), static_cast<T>(v29_), static_cast<T>(v30_),
                                    static_cast<T>(v31_), static_cast<T>(v32_), static_cast<T>(v33_), static_cast<T>(v34_),
                                    static_cast<T>(v35_), static_cast<T>(v36_), static_cast<T>(v37_), static_cast<T>(v38_) };
                return ValuesIn(array);
            }
        private:
            void operator=(const ValueArray38& other);
            const T1 v1_;
            const T2 v2_;
            const T3 v3_;
            const T4 v4_;
            const T5 v5_;
            const T6 v6_;
            const T7 v7_;
            const T8 v8_;
            const T9 v9_;
            const T10 v10_;
            const T11 v11_;
            const T12 v12_;
            const T13 v13_;
            const T14 v14_;
            const T15 v15_;
            const T16 v16_;
            const T17 v17_;
            const T18 v18_;
            const T19 v19_;
            const T20 v20_;
            const T21 v21_;
            const T22 v22_;
            const T23 v23_;
            const T24 v24_;
            const T25 v25_;
            const T26 v26_;
            const T27 v27_;
            const T28 v28_;
            const T29 v29_;
            const T30 v30_;
            const T31 v31_;
            const T32 v32_;
            const T33 v33_;
            const T34 v34_;
            const T35 v35_;
            const T36 v36_;
            const T37 v37_;
            const T38 v38_;
        };
        template <typename T1, typename T2, typename T3, typename T4, typename T5,
            typename T6, typename T7, typename T8, typename T9, typename T10,
            typename T11, typename T12, typename T13, typename T14, typename T15,
            typename T16, typename T17, typename T18, typename T19, typename T20,
            typename T21, typename T22, typename T23, typename T24, typename T25,
            typename T26, typename T27, typename T28, typename T29, typename T30,
            typename T31, typename T32, typename T33, typename T34, typename T35,
            typename T36, typename T37, typename T38, typename T39>
        class ValueArray39 {
        public:
            ValueArray39(T1 v1, T2 v2, T3 v3, T4 v4, T5 v5, T6 v6, T7 v7, T8 v8, T9 v9, T10 v10, T11 v11, T12 v12, T13 v13, T14 v14, T15 v15,
                         T16 v16, T17 v17, T18 v18, T19 v19, T20 v20, T21 v21, T22 v22, T23 v23, T24 v24, T25 v25, T26 v26, T27 v27, T28 v28,
                         T29 v29, T30 v30, T31 v31, T32 v32, T33 v33, T34 v34, T35 v35, T36 v36, T37 v37, T38 v38, T39 v39) : v1_(v1), v2_(v2),
                         v3_(v3), v4_(v4), v5_(v5), v6_(v6), v7_(v7), v8_(v8), v9_(v9), v10_(v10), v11_(v11), v12_(v12), v13_(v13), v14_(v14),
                         v15_(v15), v16_(v16), v17_(v17), v18_(v18), v19_(v19), v20_(v20), v21_(v21), v22_(v22), v23_(v23), v24_(v24),
                         v25_(v25), v26_(v26), v27_(v27), v28_(v28), v29_(v29), v30_(v30), v31_(v31), v32_(v32), v33_(v33), v34_(v34),
                         v35_(v35), v36_(v36), v37_(v37), v38_(v38), v39_(v39) {}
            template <typename T> operator ParamGenerator<T>() const {
                const T array[] = { static_cast<T>(v1_), static_cast<T>(v2_), static_cast<T>(v3_), static_cast<T>(v4_), static_cast<T>(v5_),
                                    static_cast<T>(v6_), static_cast<T>(v7_), static_cast<T>(v8_), static_cast<T>(v9_), static_cast<T>(v10_),
                                    static_cast<T>(v11_), static_cast<T>(v12_), static_cast<T>(v13_), static_cast<T>(v14_),
                                    static_cast<T>(v15_), static_cast<T>(v16_), static_cast<T>(v17_), static_cast<T>(v18_),
                                    static_cast<T>(v19_), static_cast<T>(v20_), static_cast<T>(v21_), static_cast<T>(v22_),
                                    static_cast<T>(v23_), static_cast<T>(v24_), static_cast<T>(v25_), static_cast<T>(v26_),
                                    static_cast<T>(v27_), static_cast<T>(v28_), static_cast<T>(v29_), static_cast<T>(v30_),
                                    static_cast<T>(v31_), static_cast<T>(v32_), static_cast<T>(v33_), static_cast<T>(v34_),
                                    static_cast<T>(v35_), static_cast<T>(v36_), static_cast<T>(v37_), static_cast<T>(v38_),
                                    static_cast<T>(v39_) };
                return ValuesIn(array);
            }
        private:
            void operator=(const ValueArray39& other);
            const T1 v1_;
            const T2 v2_;
            const T3 v3_;
            const T4 v4_;
            const T5 v5_;
            const T6 v6_;
            const T7 v7_;
            const T8 v8_;
            const T9 v9_;
            const T10 v10_;
            const T11 v11_;
            const T12 v12_;
            const T13 v13_;
            const T14 v14_;
            const T15 v15_;
            const T16 v16_;
            const T17 v17_;
            const T18 v18_;
            const T19 v19_;
            const T20 v20_;
            const T21 v21_;
            const T22 v22_;
            const T23 v23_;
            const T24 v24_;
            const T25 v25_;
            const T26 v26_;
            const T27 v27_;
            const T28 v28_;
            const T29 v29_;
            const T30 v30_;
            const T31 v31_;
            const T32 v32_;
            const T33 v33_;
            const T34 v34_;
            const T35 v35_;
            const T36 v36_;
            const T37 v37_;
            const T38 v38_;
            const T39 v39_;
        };
        template <typename T1, typename T2, typename T3, typename T4, typename T5,
            typename T6, typename T7, typename T8, typename T9, typename T10,
            typename T11, typename T12, typename T13, typename T14, typename T15,
            typename T16, typename T17, typename T18, typename T19, typename T20,
            typename T21, typename T22, typename T23, typename T24, typename T25,
            typename T26, typename T27, typename T28, typename T29, typename T30,
            typename T31, typename T32, typename T33, typename T34, typename T35,
            typename T36, typename T37, typename T38, typename T39, typename T40>
        class ValueArray40 {
        public:
            ValueArray40(T1 v1, T2 v2, T3 v3, T4 v4, T5 v5, T6 v6, T7 v7, T8 v8, T9 v9, T10 v10, T11 v11, T12 v12, T13 v13, T14 v14, T15 v15,
                         T16 v16, T17 v17, T18 v18, T19 v19, T20 v20, T21 v21, T22 v22, T23 v23, T24 v24, T25 v25, T26 v26, T27 v27, T28 v28,
                         T29 v29, T30 v30, T31 v31, T32 v32, T33 v33, T34 v34, T35 v35, T36 v36, T37 v37, T38 v38, T39 v39, T40 v40) : v1_(v1),
                         v2_(v2), v3_(v3), v4_(v4), v5_(v5), v6_(v6), v7_(v7), v8_(v8), v9_(v9), v10_(v10), v11_(v11), v12_(v12), v13_(v13),
                         v14_(v14), v15_(v15), v16_(v16), v17_(v17), v18_(v18), v19_(v19), v20_(v20), v21_(v21), v22_(v22), v23_(v23),
                         v24_(v24), v25_(v25), v26_(v26), v27_(v27), v28_(v28), v29_(v29), v30_(v30), v31_(v31), v32_(v32), v33_(v33),
                         v34_(v34), v35_(v35), v36_(v36), v37_(v37), v38_(v38), v39_(v39), v40_(v40) {}
            template <typename T> operator ParamGenerator<T>() const {
                const T array[] = { static_cast<T>(v1_), static_cast<T>(v2_), static_cast<T>(v3_), static_cast<T>(v4_), static_cast<T>(v5_),
                                    static_cast<T>(v6_), static_cast<T>(v7_), static_cast<T>(v8_), static_cast<T>(v9_), static_cast<T>(v10_),
                                    static_cast<T>(v11_), static_cast<T>(v12_), static_cast<T>(v13_), static_cast<T>(v14_),
                                    static_cast<T>(v15_), static_cast<T>(v16_), static_cast<T>(v17_), static_cast<T>(v18_),
                                    static_cast<T>(v19_), static_cast<T>(v20_), static_cast<T>(v21_), static_cast<T>(v22_),
                                    static_cast<T>(v23_), static_cast<T>(v24_), static_cast<T>(v25_), static_cast<T>(v26_),
                                    static_cast<T>(v27_), static_cast<T>(v28_), static_cast<T>(v29_), static_cast<T>(v30_),
                                    static_cast<T>(v31_), static_cast<T>(v32_), static_cast<T>(v33_), static_cast<T>(v34_),
                                    static_cast<T>(v35_), static_cast<T>(v36_), static_cast<T>(v37_), static_cast<T>(v38_),
                                    static_cast<T>(v39_), static_cast<T>(v40_) };
                return ValuesIn(array);
            }
        private:
            void operator=(const ValueArray40& other);
            const T1 v1_;
            const T2 v2_;
            const T3 v3_;
            const T4 v4_;
            const T5 v5_;
            const T6 v6_;
            const T7 v7_;
            const T8 v8_;
            const T9 v9_;
            const T10 v10_;
            const T11 v11_;
            const T12 v12_;
            const T13 v13_;
            const T14 v14_;
            const T15 v15_;
            const T16 v16_;
            const T17 v17_;
            const T18 v18_;
            const T19 v19_;
            const T20 v20_;
            const T21 v21_;
            const T22 v22_;
            const T23 v23_;
            const T24 v24_;
            const T25 v25_;
            const T26 v26_;
            const T27 v27_;
            const T28 v28_;
            const T29 v29_;
            const T30 v30_;
            const T31 v31_;
            const T32 v32_;
            const T33 v33_;
            const T34 v34_;
            const T35 v35_;
            const T36 v36_;
            const T37 v37_;
            const T38 v38_;
            const T39 v39_;
            const T40 v40_;
        };
        template <typename T1, typename T2, typename T3, typename T4, typename T5,
            typename T6, typename T7, typename T8, typename T9, typename T10,
            typename T11, typename T12, typename T13, typename T14, typename T15,
            typename T16, typename T17, typename T18, typename T19, typename T20,
            typename T21, typename T22, typename T23, typename T24, typename T25,
            typename T26, typename T27, typename T28, typename T29, typename T30,
            typename T31, typename T32, typename T33, typename T34, typename T35,
            typename T36, typename T37, typename T38, typename T39, typename T40,
            typename T41>
        class ValueArray41 {
        public:
            ValueArray41(T1 v1, T2 v2, T3 v3, T4 v4, T5 v5, T6 v6, T7 v7, T8 v8, T9 v9, T10 v10, T11 v11, T12 v12, T13 v13, T14 v14, T15 v15,
                         T16 v16, T17 v17, T18 v18, T19 v19, T20 v20, T21 v21, T22 v22, T23 v23, T24 v24, T25 v25, T26 v26, T27 v27, T28 v28,
                         T29 v29, T30 v30, T31 v31, T32 v32, T33 v33, T34 v34, T35 v35, T36 v36, T37 v37, T38 v38, T39 v39, T40 v40, T41 v41) :
                         v1_(v1), v2_(v2), v3_(v3), v4_(v4), v5_(v5), v6_(v6), v7_(v7), v8_(v8), v9_(v9), v10_(v10), v11_(v11), v12_(v12),
                         v13_(v13), v14_(v14), v15_(v15), v16_(v16), v17_(v17), v18_(v18), v19_(v19), v20_(v20), v21_(v21), v22_(v22),
                         v23_(v23), v24_(v24), v25_(v25), v26_(v26), v27_(v27), v28_(v28), v29_(v29), v30_(v30), v31_(v31), v32_(v32),
                         v33_(v33), v34_(v34), v35_(v35), v36_(v36), v37_(v37), v38_(v38), v39_(v39), v40_(v40), v41_(v41) {}
            template <typename T> operator ParamGenerator<T>() const {
                const T array[] = { static_cast<T>(v1_), static_cast<T>(v2_), static_cast<T>(v3_), static_cast<T>(v4_), static_cast<T>(v5_),
                                    static_cast<T>(v6_), static_cast<T>(v7_), static_cast<T>(v8_), static_cast<T>(v9_), static_cast<T>(v10_),
                                    static_cast<T>(v11_), static_cast<T>(v12_), static_cast<T>(v13_), static_cast<T>(v14_),
                                    static_cast<T>(v15_), static_cast<T>(v16_), static_cast<T>(v17_), static_cast<T>(v18_),
                                    static_cast<T>(v19_), static_cast<T>(v20_), static_cast<T>(v21_), static_cast<T>(v22_),
                                    static_cast<T>(v23_), static_cast<T>(v24_), static_cast<T>(v25_), static_cast<T>(v26_),
                                    static_cast<T>(v27_), static_cast<T>(v28_), static_cast<T>(v29_), static_cast<T>(v30_),
                                    static_cast<T>(v31_), static_cast<T>(v32_), static_cast<T>(v33_), static_cast<T>(v34_),
                                    static_cast<T>(v35_), static_cast<T>(v36_), static_cast<T>(v37_), static_cast<T>(v38_),
                                    static_cast<T>(v39_), static_cast<T>(v40_), static_cast<T>(v41_) };
                return ValuesIn(array);
            }
        private:
            void operator=(const ValueArray41& other);
            const T1 v1_;
            const T2 v2_;
            const T3 v3_;
            const T4 v4_;
            const T5 v5_;
            const T6 v6_;
            const T7 v7_;
            const T8 v8_;
            const T9 v9_;
            const T10 v10_;
            const T11 v11_;
            const T12 v12_;
            const T13 v13_;
            const T14 v14_;
            const T15 v15_;
            const T16 v16_;
            const T17 v17_;
            const T18 v18_;
            const T19 v19_;
            const T20 v20_;
            const T21 v21_;
            const T22 v22_;
            const T23 v23_;
            const T24 v24_;
            const T25 v25_;
            const T26 v26_;
            const T27 v27_;
            const T28 v28_;
            const T29 v29_;
            const T30 v30_;
            const T31 v31_;
            const T32 v32_;
            const T33 v33_;
            const T34 v34_;
            const T35 v35_;
            const T36 v36_;
            const T37 v37_;
            const T38 v38_;
            const T39 v39_;
            const T40 v40_;
            const T41 v41_;
        };
        template <typename T1, typename T2, typename T3, typename T4, typename T5,
            typename T6, typename T7, typename T8, typename T9, typename T10,
            typename T11, typename T12, typename T13, typename T14, typename T15,
            typename T16, typename T17, typename T18, typename T19, typename T20,
            typename T21, typename T22, typename T23, typename T24, typename T25,
            typename T26, typename T27, typename T28, typename T29, typename T30,
            typename T31, typename T32, typename T33, typename T34, typename T35,
            typename T36, typename T37, typename T38, typename T39, typename T40,
            typename T41, typename T42>
        class ValueArray42 {
        public:
            ValueArray42(T1 v1, T2 v2, T3 v3, T4 v4, T5 v5, T6 v6, T7 v7, T8 v8, T9 v9, T10 v10, T11 v11, T12 v12, T13 v13, T14 v14, T15 v15,
                         T16 v16, T17 v17, T18 v18, T19 v19, T20 v20, T21 v21, T22 v22, T23 v23, T24 v24, T25 v25, T26 v26, T27 v27, T28 v28,
                         T29 v29, T30 v30, T31 v31, T32 v32, T33 v33, T34 v34, T35 v35, T36 v36, T37 v37, T38 v38, T39 v39, T40 v40, T41 v41,
                         T42 v42) : v1_(v1), v2_(v2), v3_(v3), v4_(v4), v5_(v5), v6_(v6), v7_(v7), v8_(v8), v9_(v9), v10_(v10), v11_(v11),
                         v12_(v12), v13_(v13), v14_(v14), v15_(v15), v16_(v16), v17_(v17), v18_(v18), v19_(v19), v20_(v20), v21_(v21),
                         v22_(v22), v23_(v23), v24_(v24), v25_(v25), v26_(v26), v27_(v27), v28_(v28), v29_(v29), v30_(v30), v31_(v31),
                         v32_(v32), v33_(v33), v34_(v34), v35_(v35), v36_(v36), v37_(v37), v38_(v38), v39_(v39), v40_(v40), v41_(v41),
                         v42_(v42) {}
            template <typename T> operator ParamGenerator<T>() const {
                const T array[] = { static_cast<T>(v1_), static_cast<T>(v2_), static_cast<T>(v3_), static_cast<T>(v4_), static_cast<T>(v5_),
                                    static_cast<T>(v6_), static_cast<T>(v7_), static_cast<T>(v8_), static_cast<T>(v9_), static_cast<T>(v10_),
                                    static_cast<T>(v11_), static_cast<T>(v12_), static_cast<T>(v13_), static_cast<T>(v14_),
                                    static_cast<T>(v15_), static_cast<T>(v16_), static_cast<T>(v17_), static_cast<T>(v18_),
                                    static_cast<T>(v19_), static_cast<T>(v20_), static_cast<T>(v21_), static_cast<T>(v22_),
                                    static_cast<T>(v23_), static_cast<T>(v24_), static_cast<T>(v25_), static_cast<T>(v26_),
                                    static_cast<T>(v27_), static_cast<T>(v28_), static_cast<T>(v29_), static_cast<T>(v30_),
                                    static_cast<T>(v31_), static_cast<T>(v32_), static_cast<T>(v33_), static_cast<T>(v34_),
                                    static_cast<T>(v35_), static_cast<T>(v36_), static_cast<T>(v37_), static_cast<T>(v38_),
                                    static_cast<T>(v39_), static_cast<T>(v40_), static_cast<T>(v41_), static_cast<T>(v42_) };
                return ValuesIn(array);
            }
        private:
            void operator=(const ValueArray42& other);
            const T1 v1_;
            const T2 v2_;
            const T3 v3_;
            const T4 v4_;
            const T5 v5_;
            const T6 v6_;
            const T7 v7_;
            const T8 v8_;
            const T9 v9_;
            const T10 v10_;
            const T11 v11_;
            const T12 v12_;
            const T13 v13_;
            const T14 v14_;
            const T15 v15_;
            const T16 v16_;
            const T17 v17_;
            const T18 v18_;
            const T19 v19_;
            const T20 v20_;
            const T21 v21_;
            const T22 v22_;
            const T23 v23_;
            const T24 v24_;
            const T25 v25_;
            const T26 v26_;
            const T27 v27_;
            const T28 v28_;
            const T29 v29_;
            const T30 v30_;
            const T31 v31_;
            const T32 v32_;
            const T33 v33_;
            const T34 v34_;
            const T35 v35_;
            const T36 v36_;
            const T37 v37_;
            const T38 v38_;
            const T39 v39_;
            const T40 v40_;
            const T41 v41_;
            const T42 v42_;
        };
        template <typename T1, typename T2, typename T3, typename T4, typename T5,
            typename T6, typename T7, typename T8, typename T9, typename T10,
            typename T11, typename T12, typename T13, typename T14, typename T15,
            typename T16, typename T17, typename T18, typename T19, typename T20,
            typename T21, typename T22, typename T23, typename T24, typename T25,
            typename T26, typename T27, typename T28, typename T29, typename T30,
            typename T31, typename T32, typename T33, typename T34, typename T35,
            typename T36, typename T37, typename T38, typename T39, typename T40,
            typename T41, typename T42, typename T43>
        class ValueArray43 {
        public:
            ValueArray43(T1 v1, T2 v2, T3 v3, T4 v4, T5 v5, T6 v6, T7 v7, T8 v8, T9 v9, T10 v10, T11 v11, T12 v12, T13 v13, T14 v14, T15 v15,
                         T16 v16, T17 v17, T18 v18, T19 v19, T20 v20, T21 v21, T22 v22, T23 v23, T24 v24, T25 v25, T26 v26, T27 v27, T28 v28,
                         T29 v29, T30 v30, T31 v31, T32 v32, T33 v33, T34 v34, T35 v35, T36 v36, T37 v37, T38 v38, T39 v39, T40 v40, T41 v41,
                         T42 v42, T43 v43) : v1_(v1), v2_(v2), v3_(v3), v4_(v4), v5_(v5), v6_(v6), v7_(v7), v8_(v8), v9_(v9), v10_(v10),
                         v11_(v11), v12_(v12), v13_(v13), v14_(v14), v15_(v15), v16_(v16), v17_(v17), v18_(v18), v19_(v19), v20_(v20),
                         v21_(v21), v22_(v22), v23_(v23), v24_(v24), v25_(v25), v26_(v26), v27_(v27), v28_(v28), v29_(v29), v30_(v30),
                         v31_(v31), v32_(v32), v33_(v33), v34_(v34), v35_(v35), v36_(v36), v37_(v37), v38_(v38), v39_(v39), v40_(v40),
                         v41_(v41), v42_(v42), v43_(v43) {}
            template <typename T> operator ParamGenerator<T>() const {
                const T array[] = { static_cast<T>(v1_), static_cast<T>(v2_), static_cast<T>(v3_), static_cast<T>(v4_), static_cast<T>(v5_),
                                    static_cast<T>(v6_), static_cast<T>(v7_), static_cast<T>(v8_), static_cast<T>(v9_), static_cast<T>(v10_),
                                    static_cast<T>(v11_), static_cast<T>(v12_), static_cast<T>(v13_), static_cast<T>(v14_),
                                    static_cast<T>(v15_), static_cast<T>(v16_), static_cast<T>(v17_), static_cast<T>(v18_),
                                    static_cast<T>(v19_), static_cast<T>(v20_), static_cast<T>(v21_), static_cast<T>(v22_),
                                    static_cast<T>(v23_), static_cast<T>(v24_), static_cast<T>(v25_), static_cast<T>(v26_),
                                    static_cast<T>(v27_), static_cast<T>(v28_), static_cast<T>(v29_), static_cast<T>(v30_),
                                    static_cast<T>(v31_), static_cast<T>(v32_), static_cast<T>(v33_), static_cast<T>(v34_),
                                    static_cast<T>(v35_), static_cast<T>(v36_), static_cast<T>(v37_), static_cast<T>(v38_),
                                    static_cast<T>(v39_), static_cast<T>(v40_), static_cast<T>(v41_), static_cast<T>(v42_),
                                    static_cast<T>(v43_) };
                return ValuesIn(array);
            }
        private:
            void operator=(const ValueArray43& other);
            const T1 v1_;
            const T2 v2_;
            const T3 v3_;
            const T4 v4_;
            const T5 v5_;
            const T6 v6_;
            const T7 v7_;
            const T8 v8_;
            const T9 v9_;
            const T10 v10_;
            const T11 v11_;
            const T12 v12_;
            const T13 v13_;
            const T14 v14_;
            const T15 v15_;
            const T16 v16_;
            const T17 v17_;
            const T18 v18_;
            const T19 v19_;
            const T20 v20_;
            const T21 v21_;
            const T22 v22_;
            const T23 v23_;
            const T24 v24_;
            const T25 v25_;
            const T26 v26_;
            const T27 v27_;
            const T28 v28_;
            const T29 v29_;
            const T30 v30_;
            const T31 v31_;
            const T32 v32_;
            const T33 v33_;
            const T34 v34_;
            const T35 v35_;
            const T36 v36_;
            const T37 v37_;
            const T38 v38_;
            const T39 v39_;
            const T40 v40_;
            const T41 v41_;
            const T42 v42_;
            const T43 v43_;
        };
        template <typename T1, typename T2, typename T3, typename T4, typename T5,
            typename T6, typename T7, typename T8, typename T9, typename T10,
            typename T11, typename T12, typename T13, typename T14, typename T15,
            typename T16, typename T17, typename T18, typename T19, typename T20,
            typename T21, typename T22, typename T23, typename T24, typename T25,
            typename T26, typename T27, typename T28, typename T29, typename T30,
            typename T31, typename T32, typename T33, typename T34, typename T35,
            typename T36, typename T37, typename T38, typename T39, typename T40,
            typename T41, typename T42, typename T43, typename T44>
        class ValueArray44 {
        public:
            ValueArray44(T1 v1, T2 v2, T3 v3, T4 v4, T5 v5, T6 v6, T7 v7, T8 v8, T9 v9, T10 v10, T11 v11, T12 v12, T13 v13, T14 v14, T15 v15,
                         T16 v16, T17 v17, T18 v18, T19 v19, T20 v20, T21 v21, T22 v22, T23 v23, T24 v24, T25 v25, T26 v26, T27 v27, T28 v28,
                         T29 v29, T30 v30, T31 v31, T32 v32, T33 v33, T34 v34, T35 v35, T36 v36, T37 v37, T38 v38, T39 v39, T40 v40, T41 v41,
                         T42 v42, T43 v43, T44 v44) : v1_(v1), v2_(v2), v3_(v3), v4_(v4), v5_(v5), v6_(v6), v7_(v7), v8_(v8), v9_(v9),
                         v10_(v10), v11_(v11), v12_(v12), v13_(v13), v14_(v14), v15_(v15), v16_(v16), v17_(v17), v18_(v18), v19_(v19),
                         v20_(v20), v21_(v21), v22_(v22), v23_(v23), v24_(v24), v25_(v25), v26_(v26), v27_(v27), v28_(v28), v29_(v29),
                         v30_(v30), v31_(v31), v32_(v32), v33_(v33), v34_(v34), v35_(v35), v36_(v36), v37_(v37), v38_(v38), v39_(v39),
                         v40_(v40), v41_(v41), v42_(v42), v43_(v43), v44_(v44) {}
            template <typename T> operator ParamGenerator<T>() const {
                const T array[] = { static_cast<T>(v1_), static_cast<T>(v2_), static_cast<T>(v3_), static_cast<T>(v4_), static_cast<T>(v5_),
                                    static_cast<T>(v6_), static_cast<T>(v7_), static_cast<T>(v8_), static_cast<T>(v9_), static_cast<T>(v10_),
                                    static_cast<T>(v11_), static_cast<T>(v12_), static_cast<T>(v13_), static_cast<T>(v14_),
                                    static_cast<T>(v15_), static_cast<T>(v16_), static_cast<T>(v17_), static_cast<T>(v18_),
                                    static_cast<T>(v19_), static_cast<T>(v20_), static_cast<T>(v21_), static_cast<T>(v22_),
                                    static_cast<T>(v23_), static_cast<T>(v24_), static_cast<T>(v25_), static_cast<T>(v26_),
                                    static_cast<T>(v27_), static_cast<T>(v28_), static_cast<T>(v29_), static_cast<T>(v30_),
                                    static_cast<T>(v31_), static_cast<T>(v32_), static_cast<T>(v33_), static_cast<T>(v34_),
                                    static_cast<T>(v35_), static_cast<T>(v36_), static_cast<T>(v37_), static_cast<T>(v38_),
                                    static_cast<T>(v39_), static_cast<T>(v40_), static_cast<T>(v41_), static_cast<T>(v42_),
                                    static_cast<T>(v43_), static_cast<T>(v44_) };
                return ValuesIn(array);
            }
        private:
            void operator=(const ValueArray44& other);
            const T1 v1_;
            const T2 v2_;
            const T3 v3_;
            const T4 v4_;
            const T5 v5_;
            const T6 v6_;
            const T7 v7_;
            const T8 v8_;
            const T9 v9_;
            const T10 v10_;
            const T11 v11_;
            const T12 v12_;
            const T13 v13_;
            const T14 v14_;
            const T15 v15_;
            const T16 v16_;
            const T17 v17_;
            const T18 v18_;
            const T19 v19_;
            const T20 v20_;
            const T21 v21_;
            const T22 v22_;
            const T23 v23_;
            const T24 v24_;
            const T25 v25_;
            const T26 v26_;
            const T27 v27_;
            const T28 v28_;
            const T29 v29_;
            const T30 v30_;
            const T31 v31_;
            const T32 v32_;
            const T33 v33_;
            const T34 v34_;
            const T35 v35_;
            const T36 v36_;
            const T37 v37_;
            const T38 v38_;
            const T39 v39_;
            const T40 v40_;
            const T41 v41_;
            const T42 v42_;
            const T43 v43_;
            const T44 v44_;
        };
        template <typename T1, typename T2, typename T3, typename T4, typename T5,
            typename T6, typename T7, typename T8, typename T9, typename T10,
            typename T11, typename T12, typename T13, typename T14, typename T15,
            typename T16, typename T17, typename T18, typename T19, typename T20,
            typename T21, typename T22, typename T23, typename T24, typename T25,
            typename T26, typename T27, typename T28, typename T29, typename T30,
            typename T31, typename T32, typename T33, typename T34, typename T35,
            typename T36, typename T37, typename T38, typename T39, typename T40,
            typename T41, typename T42, typename T43, typename T44, typename T45>
        class ValueArray45 {
        public:
            ValueArray45(T1 v1, T2 v2, T3 v3, T4 v4, T5 v5, T6 v6, T7 v7, T8 v8, T9 v9, T10 v10, T11 v11, T12 v12, T13 v13, T14 v14, T15 v15,
                         T16 v16, T17 v17, T18 v18, T19 v19, T20 v20, T21 v21, T22 v22, T23 v23, T24 v24, T25 v25, T26 v26, T27 v27, T28 v28,
                         T29 v29, T30 v30, T31 v31, T32 v32, T33 v33, T34 v34, T35 v35, T36 v36, T37 v37, T38 v38, T39 v39, T40 v40, T41 v41,
                         T42 v42, T43 v43, T44 v44, T45 v45) : v1_(v1), v2_(v2), v3_(v3), v4_(v4), v5_(v5), v6_(v6), v7_(v7), v8_(v8), v9_(v9),
                         v10_(v10), v11_(v11), v12_(v12), v13_(v13), v14_(v14), v15_(v15), v16_(v16), v17_(v17), v18_(v18), v19_(v19),
                         v20_(v20), v21_(v21), v22_(v22), v23_(v23), v24_(v24), v25_(v25), v26_(v26), v27_(v27), v28_(v28), v29_(v29),
                         v30_(v30), v31_(v31), v32_(v32), v33_(v33), v34_(v34), v35_(v35), v36_(v36), v37_(v37), v38_(v38), v39_(v39),
                         v40_(v40), v41_(v41), v42_(v42), v43_(v43), v44_(v44), v45_(v45) {}
            template <typename T> operator ParamGenerator<T>() const {
                const T array[] = { static_cast<T>(v1_), static_cast<T>(v2_), static_cast<T>(v3_), static_cast<T>(v4_), static_cast<T>(v5_),
                                    static_cast<T>(v6_), static_cast<T>(v7_), static_cast<T>(v8_), static_cast<T>(v9_), static_cast<T>(v10_),
                                    static_cast<T>(v11_), static_cast<T>(v12_), static_cast<T>(v13_), static_cast<T>(v14_),
                                    static_cast<T>(v15_), static_cast<T>(v16_), static_cast<T>(v17_), static_cast<T>(v18_),
                                    static_cast<T>(v19_), static_cast<T>(v20_), static_cast<T>(v21_), static_cast<T>(v22_),
                                    static_cast<T>(v23_), static_cast<T>(v24_), static_cast<T>(v25_), static_cast<T>(v26_),
                                    static_cast<T>(v27_), static_cast<T>(v28_), static_cast<T>(v29_), static_cast<T>(v30_),
                                    static_cast<T>(v31_), static_cast<T>(v32_), static_cast<T>(v33_), static_cast<T>(v34_),
                                    static_cast<T>(v35_), static_cast<T>(v36_), static_cast<T>(v37_), static_cast<T>(v38_),
                                    static_cast<T>(v39_), static_cast<T>(v40_), static_cast<T>(v41_), static_cast<T>(v42_),
                                    static_cast<T>(v43_), static_cast<T>(v44_), static_cast<T>(v45_) };
                return ValuesIn(array);
            }
        private:
            void operator=(const ValueArray45& other);
            const T1 v1_;
            const T2 v2_;
            const T3 v3_;
            const T4 v4_;
            const T5 v5_;
            const T6 v6_;
            const T7 v7_;
            const T8 v8_;
            const T9 v9_;
            const T10 v10_;
            const T11 v11_;
            const T12 v12_;
            const T13 v13_;
            const T14 v14_;
            const T15 v15_;
            const T16 v16_;
            const T17 v17_;
            const T18 v18_;
            const T19 v19_;
            const T20 v20_;
            const T21 v21_;
            const T22 v22_;
            const T23 v23_;
            const T24 v24_;
            const T25 v25_;
            const T26 v26_;
            const T27 v27_;
            const T28 v28_;
            const T29 v29_;
            const T30 v30_;
            const T31 v31_;
            const T32 v32_;
            const T33 v33_;
            const T34 v34_;
            const T35 v35_;
            const T36 v36_;
            const T37 v37_;
            const T38 v38_;
            const T39 v39_;
            const T40 v40_;
            const T41 v41_;
            const T42 v42_;
            const T43 v43_;
            const T44 v44_;
            const T45 v45_;
        };
        template <typename T1, typename T2, typename T3, typename T4, typename T5,
            typename T6, typename T7, typename T8, typename T9, typename T10,
            typename T11, typename T12, typename T13, typename T14, typename T15,
            typename T16, typename T17, typename T18, typename T19, typename T20,
            typename T21, typename T22, typename T23, typename T24, typename T25,
            typename T26, typename T27, typename T28, typename T29, typename T30,
            typename T31, typename T32, typename T33, typename T34, typename T35,
            typename T36, typename T37, typename T38, typename T39, typename T40,
            typename T41, typename T42, typename T43, typename T44, typename T45,
            typename T46>
        class ValueArray46 {
        public:
            ValueArray46(T1 v1, T2 v2, T3 v3, T4 v4, T5 v5, T6 v6, T7 v7, T8 v8, T9 v9, T10 v10, T11 v11, T12 v12, T13 v13, T14 v14, T15 v15,
                         T16 v16, T17 v17, T18 v18, T19 v19, T20 v20, T21 v21, T22 v22, T23 v23, T24 v24, T25 v25, T26 v26, T27 v27, T28 v28,
                         T29 v29, T30 v30, T31 v31, T32 v32, T33 v33, T34 v34, T35 v35, T36 v36, T37 v37, T38 v38, T39 v39, T40 v40, T41 v41,
                         T42 v42, T43 v43, T44 v44, T45 v45, T46 v46) : v1_(v1), v2_(v2), v3_(v3), v4_(v4), v5_(v5), v6_(v6), v7_(v7),
                         v8_(v8), v9_(v9), v10_(v10), v11_(v11), v12_(v12), v13_(v13), v14_(v14), v15_(v15), v16_(v16), v17_(v17), v18_(v18),
                         v19_(v19), v20_(v20), v21_(v21), v22_(v22), v23_(v23), v24_(v24), v25_(v25), v26_(v26), v27_(v27), v28_(v28),
                         v29_(v29), v30_(v30), v31_(v31), v32_(v32), v33_(v33), v34_(v34), v35_(v35), v36_(v36), v37_(v37), v38_(v38),
                         v39_(v39), v40_(v40), v41_(v41), v42_(v42), v43_(v43), v44_(v44), v45_(v45), v46_(v46) {}
            template <typename T> operator ParamGenerator<T>() const {
                const T array[] = { static_cast<T>(v1_), static_cast<T>(v2_), static_cast<T>(v3_), static_cast<T>(v4_), static_cast<T>(v5_),
                                    static_cast<T>(v6_), static_cast<T>(v7_), static_cast<T>(v8_), static_cast<T>(v9_), static_cast<T>(v10_),
                                    static_cast<T>(v11_), static_cast<T>(v12_), static_cast<T>(v13_), static_cast<T>(v14_),
                                    static_cast<T>(v15_), static_cast<T>(v16_), static_cast<T>(v17_), static_cast<T>(v18_),
                                    static_cast<T>(v19_), static_cast<T>(v20_), static_cast<T>(v21_), static_cast<T>(v22_),
                                    static_cast<T>(v23_), static_cast<T>(v24_), static_cast<T>(v25_), static_cast<T>(v26_),
                                    static_cast<T>(v27_), static_cast<T>(v28_), static_cast<T>(v29_), static_cast<T>(v30_),
                                    static_cast<T>(v31_), static_cast<T>(v32_), static_cast<T>(v33_), static_cast<T>(v34_),
                                    static_cast<T>(v35_), static_cast<T>(v36_), static_cast<T>(v37_), static_cast<T>(v38_),
                                    static_cast<T>(v39_), static_cast<T>(v40_), static_cast<T>(v41_), static_cast<T>(v42_),
                                    static_cast<T>(v43_), static_cast<T>(v44_), static_cast<T>(v45_), static_cast<T>(v46_) };
                return ValuesIn(array);
            }
        private:
            void operator=(const ValueArray46& other);
            const T1 v1_;
            const T2 v2_;
            const T3 v3_;
            const T4 v4_;
            const T5 v5_;
            const T6 v6_;
            const T7 v7_;
            const T8 v8_;
            const T9 v9_;
            const T10 v10_;
            const T11 v11_;
            const T12 v12_;
            const T13 v13_;
            const T14 v14_;
            const T15 v15_;
            const T16 v16_;
            const T17 v17_;
            const T18 v18_;
            const T19 v19_;
            const T20 v20_;
            const T21 v21_;
            const T22 v22_;
            const T23 v23_;
            const T24 v24_;
            const T25 v25_;
            const T26 v26_;
            const T27 v27_;
            const T28 v28_;
            const T29 v29_;
            const T30 v30_;
            const T31 v31_;
            const T32 v32_;
            const T33 v33_;
            const T34 v34_;
            const T35 v35_;
            const T36 v36_;
            const T37 v37_;
            const T38 v38_;
            const T39 v39_;
            const T40 v40_;
            const T41 v41_;
            const T42 v42_;
            const T43 v43_;
            const T44 v44_;
            const T45 v45_;
            const T46 v46_;
        };
        template <typename T1, typename T2, typename T3, typename T4, typename T5,
            typename T6, typename T7, typename T8, typename T9, typename T10,
            typename T11, typename T12, typename T13, typename T14, typename T15,
            typename T16, typename T17, typename T18, typename T19, typename T20,
            typename T21, typename T22, typename T23, typename T24, typename T25,
            typename T26, typename T27, typename T28, typename T29, typename T30,
            typename T31, typename T32, typename T33, typename T34, typename T35,
            typename T36, typename T37, typename T38, typename T39, typename T40,
            typename T41, typename T42, typename T43, typename T44, typename T45,
            typename T46, typename T47>
        class ValueArray47 {
        public:
            ValueArray47(T1 v1, T2 v2, T3 v3, T4 v4, T5 v5, T6 v6, T7 v7, T8 v8, T9 v9, T10 v10, T11 v11, T12 v12, T13 v13, T14 v14, T15 v15,
                         T16 v16, T17 v17, T18 v18, T19 v19, T20 v20, T21 v21, T22 v22, T23 v23, T24 v24, T25 v25, T26 v26, T27 v27, T28 v28,
                         T29 v29, T30 v30, T31 v31, T32 v32, T33 v33, T34 v34, T35 v35, T36 v36, T37 v37, T38 v38, T39 v39, T40 v40, T41 v41,
                         T42 v42, T43 v43, T44 v44, T45 v45, T46 v46, T47 v47) : v1_(v1), v2_(v2), v3_(v3), v4_(v4), v5_(v5), v6_(v6), v7_(v7),
                         v8_(v8), v9_(v9), v10_(v10), v11_(v11), v12_(v12), v13_(v13), v14_(v14), v15_(v15), v16_(v16), v17_(v17), v18_(v18),
                         v19_(v19), v20_(v20), v21_(v21), v22_(v22), v23_(v23), v24_(v24), v25_(v25), v26_(v26), v27_(v27), v28_(v28),
                         v29_(v29), v30_(v30), v31_(v31), v32_(v32), v33_(v33), v34_(v34), v35_(v35), v36_(v36), v37_(v37), v38_(v38),
                         v39_(v39), v40_(v40), v41_(v41), v42_(v42), v43_(v43), v44_(v44), v45_(v45), v46_(v46), v47_(v47) {}
            template <typename T> operator ParamGenerator<T>() const {
                const T array[] = { static_cast<T>(v1_), static_cast<T>(v2_), static_cast<T>(v3_), static_cast<T>(v4_), static_cast<T>(v5_),
                                    static_cast<T>(v6_), static_cast<T>(v7_), static_cast<T>(v8_), static_cast<T>(v9_), static_cast<T>(v10_),
                                    static_cast<T>(v11_), static_cast<T>(v12_), static_cast<T>(v13_), static_cast<T>(v14_),
                                    static_cast<T>(v15_), static_cast<T>(v16_), static_cast<T>(v17_), static_cast<T>(v18_),
                                    static_cast<T>(v19_), static_cast<T>(v20_), static_cast<T>(v21_), static_cast<T>(v22_),
                                    static_cast<T>(v23_), static_cast<T>(v24_), static_cast<T>(v25_), static_cast<T>(v26_),
                                    static_cast<T>(v27_), static_cast<T>(v28_), static_cast<T>(v29_), static_cast<T>(v30_),
                                    static_cast<T>(v31_), static_cast<T>(v32_), static_cast<T>(v33_), static_cast<T>(v34_),
                                    static_cast<T>(v35_), static_cast<T>(v36_), static_cast<T>(v37_), static_cast<T>(v38_),
                                    static_cast<T>(v39_), static_cast<T>(v40_), static_cast<T>(v41_), static_cast<T>(v42_),
                                    static_cast<T>(v43_), static_cast<T>(v44_), static_cast<T>(v45_), static_cast<T>(v46_),
                                    static_cast<T>(v47_) };
                return ValuesIn(array);
            }
        private:
            void operator=(const ValueArray47& other);
            const T1 v1_;
            const T2 v2_;
            const T3 v3_;
            const T4 v4_;
            const T5 v5_;
            const T6 v6_;
            const T7 v7_;
            const T8 v8_;
            const T9 v9_;
            const T10 v10_;
            const T11 v11_;
            const T12 v12_;
            const T13 v13_;
            const T14 v14_;
            const T15 v15_;
            const T16 v16_;
            const T17 v17_;
            const T18 v18_;
            const T19 v19_;
            const T20 v20_;
            const T21 v21_;
            const T22 v22_;
            const T23 v23_;
            const T24 v24_;
            const T25 v25_;
            const T26 v26_;
            const T27 v27_;
            const T28 v28_;
            const T29 v29_;
            const T30 v30_;
            const T31 v31_;
            const T32 v32_;
            const T33 v33_;
            const T34 v34_;
            const T35 v35_;
            const T36 v36_;
            const T37 v37_;
            const T38 v38_;
            const T39 v39_;
            const T40 v40_;
            const T41 v41_;
            const T42 v42_;
            const T43 v43_;
            const T44 v44_;
            const T45 v45_;
            const T46 v46_;
            const T47 v47_;
        };
        template <typename T1, typename T2, typename T3, typename T4, typename T5,
            typename T6, typename T7, typename T8, typename T9, typename T10,
            typename T11, typename T12, typename T13, typename T14, typename T15,
            typename T16, typename T17, typename T18, typename T19, typename T20,
            typename T21, typename T22, typename T23, typename T24, typename T25,
            typename T26, typename T27, typename T28, typename T29, typename T30,
            typename T31, typename T32, typename T33, typename T34, typename T35,
            typename T36, typename T37, typename T38, typename T39, typename T40,
            typename T41, typename T42, typename T43, typename T44, typename T45,
            typename T46, typename T47, typename T48>
        class ValueArray48 {
        public:
            ValueArray48(T1 v1, T2 v2, T3 v3, T4 v4, T5 v5, T6 v6, T7 v7, T8 v8, T9 v9, T10 v10, T11 v11, T12 v12, T13 v13, T14 v14, T15 v15,
                         T16 v16, T17 v17, T18 v18, T19 v19, T20 v20, T21 v21, T22 v22, T23 v23, T24 v24, T25 v25, T26 v26, T27 v27, T28 v28,
                         T29 v29, T30 v30, T31 v31, T32 v32, T33 v33, T34 v34, T35 v35, T36 v36, T37 v37, T38 v38, T39 v39, T40 v40, T41 v41,
                         T42 v42, T43 v43, T44 v44, T45 v45, T46 v46, T47 v47, T48 v48) : v1_(v1), v2_(v2), v3_(v3), v4_(v4), v5_(v5),
                         v6_(v6), v7_(v7), v8_(v8), v9_(v9), v10_(v10), v11_(v11), v12_(v12), v13_(v13), v14_(v14), v15_(v15), v16_(v16),
                         v17_(v17), v18_(v18), v19_(v19), v20_(v20), v21_(v21), v22_(v22), v23_(v23), v24_(v24), v25_(v25), v26_(v26),
                         v27_(v27), v28_(v28), v29_(v29), v30_(v30), v31_(v31), v32_(v32), v33_(v33), v34_(v34), v35_(v35), v36_(v36),
                         v37_(v37), v38_(v38), v39_(v39), v40_(v40), v41_(v41), v42_(v42), v43_(v43), v44_(v44), v45_(v45), v46_(v46),
                         v47_(v47), v48_(v48) {}
            template <typename T> operator ParamGenerator<T>() const {
                const T array[] = { static_cast<T>(v1_), static_cast<T>(v2_), static_cast<T>(v3_), static_cast<T>(v4_), static_cast<T>(v5_),
                                    static_cast<T>(v6_), static_cast<T>(v7_), static_cast<T>(v8_), static_cast<T>(v9_), static_cast<T>(v10_),
                                    static_cast<T>(v11_), static_cast<T>(v12_), static_cast<T>(v13_), static_cast<T>(v14_),
                                    static_cast<T>(v15_), static_cast<T>(v16_), static_cast<T>(v17_), static_cast<T>(v18_),
                                    static_cast<T>(v19_), static_cast<T>(v20_), static_cast<T>(v21_), static_cast<T>(v22_),
                                    static_cast<T>(v23_), static_cast<T>(v24_), static_cast<T>(v25_), static_cast<T>(v26_),
                                    static_cast<T>(v27_), static_cast<T>(v28_), static_cast<T>(v29_), static_cast<T>(v30_),
                                    static_cast<T>(v31_), static_cast<T>(v32_), static_cast<T>(v33_), static_cast<T>(v34_),
                                    static_cast<T>(v35_), static_cast<T>(v36_), static_cast<T>(v37_), static_cast<T>(v38_),
                                    static_cast<T>(v39_), static_cast<T>(v40_), static_cast<T>(v41_), static_cast<T>(v42_),
                                    static_cast<T>(v43_), static_cast<T>(v44_), static_cast<T>(v45_), static_cast<T>(v46_),
                                    static_cast<T>(v47_), static_cast<T>(v48_) };
                return ValuesIn(array);
            }
        private:
            void operator=(const ValueArray48& other);
            const T1 v1_;
            const T2 v2_;
            const T3 v3_;
            const T4 v4_;
            const T5 v5_;
            const T6 v6_;
            const T7 v7_;
            const T8 v8_;
            const T9 v9_;
            const T10 v10_;
            const T11 v11_;
            const T12 v12_;
            const T13 v13_;
            const T14 v14_;
            const T15 v15_;
            const T16 v16_;
            const T17 v17_;
            const T18 v18_;
            const T19 v19_;
            const T20 v20_;
            const T21 v21_;
            const T22 v22_;
            const T23 v23_;
            const T24 v24_;
            const T25 v25_;
            const T26 v26_;
            const T27 v27_;
            const T28 v28_;
            const T29 v29_;
            const T30 v30_;
            const T31 v31_;
            const T32 v32_;
            const T33 v33_;
            const T34 v34_;
            const T35 v35_;
            const T36 v36_;
            const T37 v37_;
            const T38 v38_;
            const T39 v39_;
            const T40 v40_;
            const T41 v41_;
            const T42 v42_;
            const T43 v43_;
            const T44 v44_;
            const T45 v45_;
            const T46 v46_;
            const T47 v47_;
            const T48 v48_;
        };
        template <typename T1, typename T2, typename T3, typename T4, typename T5,
            typename T6, typename T7, typename T8, typename T9, typename T10,
            typename T11, typename T12, typename T13, typename T14, typename T15,
            typename T16, typename T17, typename T18, typename T19, typename T20,
            typename T21, typename T22, typename T23, typename T24, typename T25,
            typename T26, typename T27, typename T28, typename T29, typename T30,
            typename T31, typename T32, typename T33, typename T34, typename T35,
            typename T36, typename T37, typename T38, typename T39, typename T40,
            typename T41, typename T42, typename T43, typename T44, typename T45,
            typename T46, typename T47, typename T48, typename T49>
        class ValueArray49 {
        public:
            ValueArray49(T1 v1, T2 v2, T3 v3, T4 v4, T5 v5, T6 v6, T7 v7, T8 v8, T9 v9, T10 v10, T11 v11, T12 v12, T13 v13, T14 v14, T15 v15,
                         T16 v16, T17 v17, T18 v18, T19 v19, T20 v20, T21 v21, T22 v22, T23 v23, T24 v24, T25 v25, T26 v26, T27 v27, T28 v28,
                         T29 v29, T30 v30, T31 v31, T32 v32, T33 v33, T34 v34, T35 v35, T36 v36, T37 v37, T38 v38, T39 v39, T40 v40, T41 v41,
                         T42 v42, T43 v43, T44 v44, T45 v45, T46 v46, T47 v47, T48 v48, T49 v49) : v1_(v1), v2_(v2), v3_(v3), v4_(v4),
                         v5_(v5), v6_(v6), v7_(v7), v8_(v8), v9_(v9), v10_(v10), v11_(v11), v12_(v12), v13_(v13), v14_(v14), v15_(v15),
                         v16_(v16), v17_(v17), v18_(v18), v19_(v19), v20_(v20), v21_(v21), v22_(v22), v23_(v23), v24_(v24), v25_(v25),
                         v26_(v26), v27_(v27), v28_(v28), v29_(v29), v30_(v30), v31_(v31), v32_(v32), v33_(v33), v34_(v34), v35_(v35),
                         v36_(v36), v37_(v37), v38_(v38), v39_(v39), v40_(v40), v41_(v41), v42_(v42), v43_(v43), v44_(v44), v45_(v45),
                         v46_(v46), v47_(v47), v48_(v48), v49_(v49) {}
            template <typename T> operator ParamGenerator<T>() const {
                const T array[] = { static_cast<T>(v1_), static_cast<T>(v2_), static_cast<T>(v3_), static_cast<T>(v4_), static_cast<T>(v5_),
                                    static_cast<T>(v6_), static_cast<T>(v7_), static_cast<T>(v8_), static_cast<T>(v9_), static_cast<T>(v10_),
                                    static_cast<T>(v11_), static_cast<T>(v12_), static_cast<T>(v13_), static_cast<T>(v14_),
                                    static_cast<T>(v15_), static_cast<T>(v16_), static_cast<T>(v17_), static_cast<T>(v18_),
                                    static_cast<T>(v19_), static_cast<T>(v20_), static_cast<T>(v21_), static_cast<T>(v22_),
                                    static_cast<T>(v23_), static_cast<T>(v24_), static_cast<T>(v25_), static_cast<T>(v26_),
                                    static_cast<T>(v27_), static_cast<T>(v28_), static_cast<T>(v29_), static_cast<T>(v30_),
                                    static_cast<T>(v31_), static_cast<T>(v32_), static_cast<T>(v33_), static_cast<T>(v34_),
                                    static_cast<T>(v35_), static_cast<T>(v36_), static_cast<T>(v37_), static_cast<T>(v38_),
                                    static_cast<T>(v39_), static_cast<T>(v40_), static_cast<T>(v41_), static_cast<T>(v42_),
                                    static_cast<T>(v43_), static_cast<T>(v44_), static_cast<T>(v45_), static_cast<T>(v46_),
                                    static_cast<T>(v47_), static_cast<T>(v48_), static_cast<T>(v49_) };
                return ValuesIn(array);
            }
        private:
            void operator=(const ValueArray49& other);
            const T1 v1_;
            const T2 v2_;
            const T3 v3_;
            const T4 v4_;
            const T5 v5_;
            const T6 v6_;
            const T7 v7_;
            const T8 v8_;
            const T9 v9_;
            const T10 v10_;
            const T11 v11_;
            const T12 v12_;
            const T13 v13_;
            const T14 v14_;
            const T15 v15_;
            const T16 v16_;
            const T17 v17_;
            const T18 v18_;
            const T19 v19_;
            const T20 v20_;
            const T21 v21_;
            const T22 v22_;
            const T23 v23_;
            const T24 v24_;
            const T25 v25_;
            const T26 v26_;
            const T27 v27_;
            const T28 v28_;
            const T29 v29_;
            const T30 v30_;
            const T31 v31_;
            const T32 v32_;
            const T33 v33_;
            const T34 v34_;
            const T35 v35_;
            const T36 v36_;
            const T37 v37_;
            const T38 v38_;
            const T39 v39_;
            const T40 v40_;
            const T41 v41_;
            const T42 v42_;
            const T43 v43_;
            const T44 v44_;
            const T45 v45_;
            const T46 v46_;
            const T47 v47_;
            const T48 v48_;
            const T49 v49_;
        };
        template <typename T1, typename T2, typename T3, typename T4, typename T5,
            typename T6, typename T7, typename T8, typename T9, typename T10,
            typename T11, typename T12, typename T13, typename T14, typename T15,
            typename T16, typename T17, typename T18, typename T19, typename T20,
            typename T21, typename T22, typename T23, typename T24, typename T25,
            typename T26, typename T27, typename T28, typename T29, typename T30,
            typename T31, typename T32, typename T33, typename T34, typename T35,
            typename T36, typename T37, typename T38, typename T39, typename T40,
            typename T41, typename T42, typename T43, typename T44, typename T45,
            typename T46, typename T47, typename T48, typename T49, typename T50>
        class ValueArray50 {
        public:
            ValueArray50(T1 v1, T2 v2, T3 v3, T4 v4, T5 v5, T6 v6, T7 v7, T8 v8, T9 v9, T10 v10, T11 v11, T12 v12, T13 v13, T14 v14, T15 v15,
                         T16 v16, T17 v17, T18 v18, T19 v19, T20 v20, T21 v21, T22 v22, T23 v23, T24 v24, T25 v25, T26 v26, T27 v27, T28 v28,
                         T29 v29, T30 v30, T31 v31, T32 v32, T33 v33, T34 v34, T35 v35, T36 v36, T37 v37, T38 v38, T39 v39, T40 v40, T41 v41,
                         T42 v42, T43 v43, T44 v44, T45 v45, T46 v46, T47 v47, T48 v48, T49 v49, T50 v50) : v1_(v1), v2_(v2), v3_(v3),
                         v4_(v4), v5_(v5), v6_(v6), v7_(v7), v8_(v8), v9_(v9), v10_(v10), v11_(v11), v12_(v12), v13_(v13), v14_(v14),
                         v15_(v15), v16_(v16), v17_(v17), v18_(v18), v19_(v19), v20_(v20), v21_(v21), v22_(v22), v23_(v23), v24_(v24),
                         v25_(v25), v26_(v26), v27_(v27), v28_(v28), v29_(v29), v30_(v30), v31_(v31), v32_(v32), v33_(v33), v34_(v34),
                         v35_(v35), v36_(v36), v37_(v37), v38_(v38), v39_(v39), v40_(v40), v41_(v41), v42_(v42), v43_(v43), v44_(v44),
                         v45_(v45), v46_(v46), v47_(v47), v48_(v48), v49_(v49), v50_(v50) {}
            template <typename T> operator ParamGenerator<T>() const {
                const T array[] = { static_cast<T>(v1_), static_cast<T>(v2_), static_cast<T>(v3_), static_cast<T>(v4_), static_cast<T>(v5_),
                                    static_cast<T>(v6_), static_cast<T>(v7_), static_cast<T>(v8_), static_cast<T>(v9_), static_cast<T>(v10_),
                                    static_cast<T>(v11_), static_cast<T>(v12_), static_cast<T>(v13_), static_cast<T>(v14_),
                                    static_cast<T>(v15_), static_cast<T>(v16_), static_cast<T>(v17_), static_cast<T>(v18_),
                                    static_cast<T>(v19_), static_cast<T>(v20_), static_cast<T>(v21_), static_cast<T>(v22_),
                                    static_cast<T>(v23_), static_cast<T>(v24_), static_cast<T>(v25_), static_cast<T>(v26_),
                                    static_cast<T>(v27_), static_cast<T>(v28_), static_cast<T>(v29_), static_cast<T>(v30_),
                                    static_cast<T>(v31_), static_cast<T>(v32_), static_cast<T>(v33_), static_cast<T>(v34_),
                                    static_cast<T>(v35_), static_cast<T>(v36_), static_cast<T>(v37_), static_cast<T>(v38_),
                                    static_cast<T>(v39_), static_cast<T>(v40_), static_cast<T>(v41_), static_cast<T>(v42_),
                                    static_cast<T>(v43_), static_cast<T>(v44_), static_cast<T>(v45_), static_cast<T>(v46_),
                                    static_cast<T>(v47_), static_cast<T>(v48_), static_cast<T>(v49_), static_cast<T>(v50_) };
                return ValuesIn(array);
            }
        private:
            void operator=(const ValueArray50& other);
            const T1 v1_;
            const T2 v2_;
            const T3 v3_;
            const T4 v4_;
            const T5 v5_;
            const T6 v6_;
            const T7 v7_;
            const T8 v8_;
            const T9 v9_;
            const T10 v10_;
            const T11 v11_;
            const T12 v12_;
            const T13 v13_;
            const T14 v14_;
            const T15 v15_;
            const T16 v16_;
            const T17 v17_;
            const T18 v18_;
            const T19 v19_;
            const T20 v20_;
            const T21 v21_;
            const T22 v22_;
            const T23 v23_;
            const T24 v24_;
            const T25 v25_;
            const T26 v26_;
            const T27 v27_;
            const T28 v28_;
            const T29 v29_;
            const T30 v30_;
            const T31 v31_;
            const T32 v32_;
            const T33 v33_;
            const T34 v34_;
            const T35 v35_;
            const T36 v36_;
            const T37 v37_;
            const T38 v38_;
            const T39 v39_;
            const T40 v40_;
            const T41 v41_;
            const T42 v42_;
            const T43 v43_;
            const T44 v44_;
            const T45 v45_;
            const T46 v46_;
            const T47 v47_;
            const T48 v48_;
            const T49 v49_;
            const T50 v50_;
        };
    #if GTEST_HAS_COMBINE
        template <typename T1, typename T2> class CartesianProductGenerator2 : public ParamGeneratorInterface<::testing::tuple<T1, T2>> {
        public:
            typedef ::testing::tuple<T1, T2> ParamType;
            CartesianProductGenerator2(const ParamGenerator<T1>& g1, const ParamGenerator<T2>& g2) : g1_(g1), g2_(g2) {}
            virtual ~CartesianProductGenerator2() {}
            virtual ParamIteratorInterface<ParamType>* Begin() const {
                return new Iterator(this, g1_, g1_.begin(), g2_, g2_.begin());
            }
            virtual ParamIteratorInterface<ParamType>* End() const {
                return new Iterator(this, g1_, g1_.end(), g2_, g2_.end());
            }
        private:
            class Iterator : public ParamIteratorInterface<ParamType> {
            public:
                Iterator(const ParamGeneratorInterface<ParamType>* base, const ParamGenerator<T1>& g1,
                         const typename ParamGenerator<T1>::iterator& current1,
                         const ParamGenerator<T2>& g2, const typename ParamGenerator<T2>::iterator& current2) : base_(base),
                         begin1_(g1.begin()), end1_(g1.end()), current1_(current1), begin2_(g2.begin()), end2_(g2.end()), current2_(current2) {
                    ComputeCurrentValue();
                }
                virtual ~Iterator() {}
                virtual const ParamGeneratorInterface<ParamType>* BaseGenerator() const {
                    return base_;
                }
                virtual void Advance() {
                    assert(!AtEnd());
                    ++current2_;
                    if (current2_ == end2_) {
                        current2_ = begin2_;
                        ++current1_;
                    }
                    ComputeCurrentValue();
                }
                virtual ParamIteratorInterface<ParamType>* Clone() const {
                    return new Iterator(*this);
                }
                virtual const ParamType* Current() const { return &current_value_; }
                virtual bool Equals(const ParamIteratorInterface<ParamType>& other) const {
                    GTEST_CHECK_(BaseGenerator() == other.BaseGenerator()) << "The program attempted to compare iterators "
                                 << "from different generators." << std::endl;
                    const Iterator* typed_other = CheckedDowncastToActualType<const Iterator>(&other);
                    return (AtEnd() && typed_other->AtEnd()) || (current1_ == typed_other->current1_ && current2_ == typed_other->current2_);
                }
                private:
                Iterator(const Iterator& other) : base_(other.base_), begin1_(other.begin1_), end1_(other.end1_), current1_(other.current1_),
                         begin2_(other.begin2_), end2_(other.end2_), current2_(other.current2_) {
                    ComputeCurrentValue();
                }
                void ComputeCurrentValue() {
                    if (!AtEnd()) current_value_ = ParamType(*current1_, *current2_);
                }
                bool AtEnd() const {
                    return current1_ == end1_ || current2_ == end2_;
                }
                void operator=(const Iterator& other);
                const ParamGeneratorInterface<ParamType>* const base_;
                const typename ParamGenerator<T1>::iterator begin1_;
                const typename ParamGenerator<T1>::iterator end1_;
                typename ParamGenerator<T1>::iterator current1_;
                const typename ParamGenerator<T2>::iterator begin2_;
                const typename ParamGenerator<T2>::iterator end2_;
                typename ParamGenerator<T2>::iterator current2_;
                ParamType current_value_;
            };
            void operator=(const CartesianProductGenerator2& other);
            const ParamGenerator<T1> g1_;
            const ParamGenerator<T2> g2_;
        };
        template <typename T1, typename T2, typename T3> class CartesianProductGenerator3 : public ParamGeneratorInterface<::testing::tuple<T1, T2, T3>> {
        public:
            typedef ::testing::tuple<T1, T2, T3> ParamType;
            CartesianProductGenerator3(const ParamGenerator<T1>& g1, const ParamGenerator<T2>& g2, const ParamGenerator<T3>& g3) : g1_(g1),
                                       g2_(g2), g3_(g3) {}
            virtual ~CartesianProductGenerator3() {}
            virtual ParamIteratorInterface<ParamType>* Begin() const {
                return new Iterator(this, g1_, g1_.begin(), g2_, g2_.begin(), g3_, g3_.begin());
            }
            virtual ParamIteratorInterface<ParamType>* End() const {
                return new Iterator(this, g1_, g1_.end(), g2_, g2_.end(), g3_, g3_.end());
            }
        private:
            class Iterator : public ParamIteratorInterface<ParamType> {
            public:
                Iterator(const ParamGeneratorInterface<ParamType>* base, const ParamGenerator<T1>& g1,
                         const typename ParamGenerator<T1>::iterator& current1,
                         const ParamGenerator<T2>& g2, const typename ParamGenerator<T2>::iterator& current2, const ParamGenerator<T3>& g3,
                         const typename ParamGenerator<T3>::iterator& current3) : base_(base), begin1_(g1.begin()), end1_(g1.end()),
                         current1_(current1), begin2_(g2.begin()), end2_(g2.end()), current2_(current2), begin3_(g3.begin()), end3_(g3.end()),
                         current3_(current3) {
                    ComputeCurrentValue();
                }
                virtual ~Iterator() {}
                virtual const ParamGeneratorInterface<ParamType>* BaseGenerator() const {
                    return base_;
                }
                virtual void Advance() {
                    assert(!AtEnd());
                    ++current3_;
                    if (current3_ == end3_) {
                        current3_ = begin3_;
                        ++current2_;
                    }
                    if (current2_ == end2_) {
                        current2_ = begin2_;
                        ++current1_;
                    }
                    ComputeCurrentValue();
                }
                virtual ParamIteratorInterface<ParamType>* Clone() const {
                    return new Iterator(*this);
                }
                virtual const ParamType* Current() const { return &current_value_; }
                virtual bool Equals(const ParamIteratorInterface<ParamType>& other) const {
                    GTEST_CHECK_(BaseGenerator() == other.BaseGenerator()) << "The program attempted to compare iterators "
                                 << "from different generators." << std::endl;
                    const Iterator* typed_other = CheckedDowncastToActualType<const Iterator>(&other);
                    return (AtEnd() && typed_other->AtEnd()) || (current1_ == typed_other->current1_ && current2_ == typed_other->current2_ &&
                            current3_ == typed_other->current3_);
                }
            private:
                Iterator(const Iterator& other) : base_(other.base_), begin1_(other.begin1_), end1_(other.end1_), current1_(other.current1_),
                         begin2_(other.begin2_), end2_(other.end2_), current2_(other.current2_), begin3_(other.begin3_), end3_(other.end3_),
                         current3_(other.current3_) {
                    ComputeCurrentValue();
                }
                void ComputeCurrentValue() {
                    if (!AtEnd()) current_value_ = ParamType(*current1_, *current2_, *current3_);
                }
                bool AtEnd() const {
                    return current1_ == end1_ || current2_ == end2_ || current3_ == end3_;
                }
                void operator=(const Iterator& other);
                const ParamGeneratorInterface<ParamType>* const base_;
                const typename ParamGenerator<T1>::iterator begin1_;
                const typename ParamGenerator<T1>::iterator end1_;
                typename ParamGenerator<T1>::iterator current1_;
                const typename ParamGenerator<T2>::iterator begin2_;
                const typename ParamGenerator<T2>::iterator end2_;
                typename ParamGenerator<T2>::iterator current2_;
                const typename ParamGenerator<T3>::iterator begin3_;
                const typename ParamGenerator<T3>::iterator end3_;
                typename ParamGenerator<T3>::iterator current3_;
                ParamType current_value_;
            };
            void operator=(const CartesianProductGenerator3& other);
            const ParamGenerator<T1> g1_;
            const ParamGenerator<T2> g2_;
            const ParamGenerator<T3> g3_;
        };
        template <typename T1, typename T2, typename T3, typename T4> class CartesianProductGenerator4 : public ParamGeneratorInterface< ::testing::tuple<T1, T2, T3, T4> > {
        public:
            typedef ::testing::tuple<T1, T2, T3, T4> ParamType;
            CartesianProductGenerator4(const ParamGenerator<T1>& g1, const ParamGenerator<T2>& g2, const ParamGenerator<T3>& g3,
                                       const ParamGenerator<T4>& g4) : g1_(g1), g2_(g2), g3_(g3), g4_(g4) {}
            virtual ~CartesianProductGenerator4() {}
            virtual ParamIteratorInterface<ParamType>* Begin() const {
                return new Iterator(this, g1_, g1_.begin(), g2_, g2_.begin(), g3_, g3_.begin(), g4_, g4_.begin());
            }
            virtual ParamIteratorInterface<ParamType>* End() const {
                return new Iterator(this, g1_, g1_.end(), g2_, g2_.end(), g3_, g3_.end(), g4_, g4_.end());
            }
        private:
            class Iterator : public ParamIteratorInterface<ParamType> {
            public:
                Iterator(const ParamGeneratorInterface<ParamType>* base, const ParamGenerator<T1>& g1,
                         const typename ParamGenerator<T1>::iterator& current1,
                         const ParamGenerator<T2>& g2, const typename ParamGenerator<T2>::iterator& current2, const ParamGenerator<T3>& g3,
                         const typename ParamGenerator<T3>::iterator& current3, const ParamGenerator<T4>& g4,
                         const typename ParamGenerator<T4>::iterator& current4) : base_(base), begin1_(g1.begin()), end1_(g1.end()),
                         current1_(current1), begin2_(g2.begin()), end2_(g2.end()), current2_(current2), begin3_(g3.begin()), end3_(g3.end()),
                         current3_(current3), begin4_(g4.begin()), end4_(g4.end()), current4_(current4) {
                    ComputeCurrentValue();
                }
                virtual ~Iterator() {}
                virtual const ParamGeneratorInterface<ParamType>* BaseGenerator() const {
                    return base_;
                }
                virtual void Advance() {
                    assert(!AtEnd());
                    ++current4_;
                    if (current4_ == end4_) {
                        current4_ = begin4_;
                        ++current3_;
                    }
                    if (current3_ == end3_) {
                        current3_ = begin3_;
                        ++current2_;
                    }
                    if (current2_ == end2_) {
                        current2_ = begin2_;
                        ++current1_;
                    }
                    ComputeCurrentValue();
                }
                virtual ParamIteratorInterface<ParamType>* Clone() const {
                    return new Iterator(*this);
                }
                virtual const ParamType* Current() const { return &current_value_; }
                virtual bool Equals(const ParamIteratorInterface<ParamType>& other) const {
                    GTEST_CHECK_(BaseGenerator() == other.BaseGenerator()) << "The program attempted to compare iterators " << "from different generators."
                                                                                    << std::endl;
                    const Iterator* typed_other = CheckedDowncastToActualType<const Iterator>(&other);
                    return (AtEnd() && typed_other->AtEnd()) || (current1_ == typed_other->current1_ && current2_ == typed_other->current2_ &&
                            current3_ == typed_other->current3_ && current4_ == typed_other->current4_);
                }
            private:
                Iterator(const Iterator& other) : base_(other.base_), begin1_(other.begin1_), end1_(other.end1_), current1_(other.current1_), begin2_(other.begin2_),
                                                  end2_(other.end2_), current2_(other.current2_), begin3_(other.begin3_), end3_(other.end3_),
                                                  current3_(other.current3_), begin4_(other.begin4_), end4_(other.end4_), current4_(other.current4_) {
                    ComputeCurrentValue();
                }
                void ComputeCurrentValue() {
                    if (!AtEnd()) current_value_ = ParamType(*current1_, *current2_, *current3_, *current4_);
                }
                bool AtEnd() const {
                    return current1_ == end1_ || current2_ == end2_ || current3_ == end3_ || current4_ == end4_;
                }
                void operator=(const Iterator& other);
                const ParamGeneratorInterface<ParamType>* const base_;
                const typename ParamGenerator<T1>::iterator begin1_;
                const typename ParamGenerator<T1>::iterator end1_;
                typename ParamGenerator<T1>::iterator current1_;
                const typename ParamGenerator<T2>::iterator begin2_;
                const typename ParamGenerator<T2>::iterator end2_;
                typename ParamGenerator<T2>::iterator current2_;
                const typename ParamGenerator<T3>::iterator begin3_;
                const typename ParamGenerator<T3>::iterator end3_;
                typename ParamGenerator<T3>::iterator current3_;
                const typename ParamGenerator<T4>::iterator begin4_;
                const typename ParamGenerator<T4>::iterator end4_;
                typename ParamGenerator<T4>::iterator current4_;
                ParamType current_value_;
            };
            void operator=(const CartesianProductGenerator4& other);
            const ParamGenerator<T1> g1_;
            const ParamGenerator<T2> g2_;
            const ParamGenerator<T3> g3_;
            const ParamGenerator<T4> g4_;
        };
        template <typename T1, typename T2, typename T3, typename T4, typename T5> class CartesianProductGenerator5 : public ParamGeneratorInterface< ::testing::tuple<T1, T2, T3, T4, T5> > {
        public:
            typedef ::testing::tuple<T1, T2, T3, T4, T5> ParamType;
            CartesianProductGenerator5(const ParamGenerator<T1>& g1, const ParamGenerator<T2>& g2, const ParamGenerator<T3>& g3,
                                       const ParamGenerator<T4>& g4, const ParamGenerator<T5>& g5) : g1_(g1), g2_(g2), g3_(g3), g4_(g4),
                                       g5_(g5) {}
            virtual ~CartesianProductGenerator5() {}
            virtual ParamIteratorInterface<ParamType>* Begin() const {
                return new Iterator(this, g1_, g1_.begin(), g2_, g2_.begin(), g3_, g3_.begin(), g4_, g4_.begin(), g5_, g5_.begin());
            }
            virtual ParamIteratorInterface<ParamType>* End() const {
                return new Iterator(this, g1_, g1_.end(), g2_, g2_.end(), g3_, g3_.end(), g4_, g4_.end(), g5_, g5_.end());
            }
        private:
            class Iterator : public ParamIteratorInterface<ParamType> {
            public:
                Iterator(const ParamGeneratorInterface<ParamType>* base, const ParamGenerator<T1>& g1,
                         const typename ParamGenerator<T1>::iterator& current1,
                         const ParamGenerator<T2>& g2, const typename ParamGenerator<T2>::iterator& current2, const ParamGenerator<T3>& g3,
                         const typename ParamGenerator<T3>::iterator& current3, const ParamGenerator<T4>& g4,
                         const typename ParamGenerator<T4>::iterator& current4, const ParamGenerator<T5>& g5,
                         const typename ParamGenerator<T5>::iterator& current5) : base_(base), begin1_(g1.begin()), end1_(g1.end()),
                         current1_(current1), begin2_(g2.begin()), end2_(g2.end()), current2_(current2), begin3_(g3.begin()), end3_(g3.end()),
                         current3_(current3), begin4_(g4.begin()), end4_(g4.end()), current4_(current4), begin5_(g5.begin()), end5_(g5.end()),
                         current5_(current5)    {
                    ComputeCurrentValue();
                }
                virtual ~Iterator() {}
                virtual const ParamGeneratorInterface<ParamType>* BaseGenerator() const {
                    return base_;
                }
                virtual void Advance() {
                    assert(!AtEnd());
                    ++current5_;
                    if (current5_ == end5_) {
                        current5_ = begin5_;
                        ++current4_;
                    }
                    if (current4_ == end4_) {
                        current4_ = begin4_;
                        ++current3_;
                    }
                    if (current3_ == end3_) {
                        current3_ = begin3_;
                        ++current2_;
                    }
                    if (current2_ == end2_) {
                        current2_ = begin2_;
                        ++current1_;
                    }
                    ComputeCurrentValue();
                }
                virtual ParamIteratorInterface<ParamType>* Clone() const {
                    return new Iterator(*this);
                }
                virtual const ParamType* Current() const { return &current_value_; }
                virtual bool Equals(const ParamIteratorInterface<ParamType>& other) const {
                    GTEST_CHECK_(BaseGenerator() == other.BaseGenerator()) << "The program attempted to compare iterators "
                                 << "from different generators." << std::endl;
                    const Iterator* typed_other = CheckedDowncastToActualType<const Iterator>(&other);
                    return (AtEnd() && typed_other->AtEnd()) || (current1_ == typed_other->current1_ && current2_ == typed_other->current2_ &&
                            current3_ == typed_other->current3_ && current4_ == typed_other->current4_ &&current5_ == typed_other->current5_);
                }
            private:
                Iterator(const Iterator& other) : base_(other.base_), begin1_(other.begin1_), end1_(other.end1_), current1_(other.current1_),
                         begin2_(other.begin2_), end2_(other.end2_), current2_(other.current2_), begin3_(other.begin3_), end3_(other.end3_),
                         current3_(other.current3_), begin4_(other.begin4_), end4_(other.end4_), current4_(other.current4_),
                         begin5_(other.begin5_), end5_(other.end5_), current5_(other.current5_) {
                    ComputeCurrentValue();
                }
                void ComputeCurrentValue() {
                    if (!AtEnd()) current_value_ = ParamType(*current1_, *current2_, *current3_, *current4_, *current5_);
                }
                bool AtEnd() const {
                    return current1_ == end1_ || current2_ == end2_ || current3_ == end3_ || current4_ == end4_ || current5_ == end5_;
                }
                void operator=(const Iterator& other);
                const ParamGeneratorInterface<ParamType>* const base_;
                const typename ParamGenerator<T1>::iterator begin1_;
                const typename ParamGenerator<T1>::iterator end1_;
                typename ParamGenerator<T1>::iterator current1_;
                const typename ParamGenerator<T2>::iterator begin2_;
                const typename ParamGenerator<T2>::iterator end2_;
                typename ParamGenerator<T2>::iterator current2_;
                const typename ParamGenerator<T3>::iterator begin3_;
                const typename ParamGenerator<T3>::iterator end3_;
                typename ParamGenerator<T3>::iterator current3_;
                const typename ParamGenerator<T4>::iterator begin4_;
                const typename ParamGenerator<T4>::iterator end4_;
                typename ParamGenerator<T4>::iterator current4_;
                const typename ParamGenerator<T5>::iterator begin5_;
                const typename ParamGenerator<T5>::iterator end5_;
                typename ParamGenerator<T5>::iterator current5_;
                ParamType current_value_;
            };
            void operator=(const CartesianProductGenerator5& other);
            const ParamGenerator<T1> g1_;
            const ParamGenerator<T2> g2_;
            const ParamGenerator<T3> g3_;
            const ParamGenerator<T4> g4_;
            const ParamGenerator<T5> g5_;
        };
        template <typename T1, typename T2, typename T3, typename T4, typename T5,
            typename T6> class CartesianProductGenerator6 : public ParamGeneratorInterface< ::testing::tuple<T1, T2, T3, T4, T5, T6>> {
        public:
            typedef ::testing::tuple<T1, T2, T3, T4, T5, T6> ParamType;
            CartesianProductGenerator6(const ParamGenerator<T1>& g1, const ParamGenerator<T2>& g2, const ParamGenerator<T3>& g3,
                                       const ParamGenerator<T4>& g4, const ParamGenerator<T5>& g5, const ParamGenerator<T6>& g6) : g1_(g1),
                                       g2_(g2), g3_(g3), g4_(g4), g5_(g5), g6_(g6) {}
            virtual ~CartesianProductGenerator6() {}
            virtual ParamIteratorInterface<ParamType>* Begin() const {
                return new Iterator(this, g1_, g1_.begin(), g2_, g2_.begin(), g3_, g3_.begin(), g4_, g4_.begin(), g5_, g5_.begin(), g6_,
                                    g6_.begin());
            }
            virtual ParamIteratorInterface<ParamType>* End() const {
                return new Iterator(this, g1_, g1_.end(), g2_, g2_.end(), g3_, g3_.end(), g4_, g4_.end(), g5_, g5_.end(), g6_, g6_.end());
            }
        private:
            class Iterator : public ParamIteratorInterface<ParamType> {
            public:
                Iterator(const ParamGeneratorInterface<ParamType>* base, const ParamGenerator<T1>& g1,
                         const typename ParamGenerator<T1>::iterator& current1, const ParamGenerator<T2>& g2,
                         const typename ParamGenerator<T2>::iterator& current2, const ParamGenerator<T3>& g3,
                         const typename ParamGenerator<T3>::iterator& current3, const ParamGenerator<T4>& g4,
                         const typename ParamGenerator<T4>::iterator& current4, const ParamGenerator<T5>& g5,
                         const typename ParamGenerator<T5>::iterator& current5, const ParamGenerator<T6>& g6,
                         const typename ParamGenerator<T6>::iterator& current6) : base_(base), begin1_(g1.begin()), end1_(g1.end()),
                         current1_(current1), begin2_(g2.begin()), end2_(g2.end()), current2_(current2), begin3_(g3.begin()), end3_(g3.end()),
                         current3_(current3), begin4_(g4.begin()), end4_(g4.end()), current4_(current4), begin5_(g5.begin()), end5_(g5.end()),
                         current5_(current5), begin6_(g6.begin()), end6_(g6.end()), current6_(current6) {
                    ComputeCurrentValue();
                }
                virtual ~Iterator() {}
                virtual const ParamGeneratorInterface<ParamType>* BaseGenerator() const {
                    return base_;
                }
                virtual void Advance() {
                    assert(!AtEnd());
                    ++current6_;
                    if (current6_ == end6_) {
                        current6_ = begin6_;
                        ++current5_;
                    }
                    if (current5_ == end5_) {
                        current5_ = begin5_;
                        ++current4_;
                    }
                    if (current4_ == end4_) {
                        current4_ = begin4_;
                        ++current3_;
                    }
                    if (current3_ == end3_) {
                        current3_ = begin3_;
                        ++current2_;
                    }
                    if (current2_ == end2_) {
                        current2_ = begin2_;
                        ++current1_;
                    }
                    ComputeCurrentValue();
                }
                virtual ParamIteratorInterface<ParamType>* Clone() const {
                    return new Iterator(*this);
                }
                virtual const ParamType* Current() const { return &current_value_; }
                virtual bool Equals(const ParamIteratorInterface<ParamType>& other) const {
                    GTEST_CHECK_(BaseGenerator() == other.BaseGenerator()) << "The program attempted to compare iterators "
                                 << "from different generators." << std::endl;
                    const Iterator* typed_other = CheckedDowncastToActualType<const Iterator>(&other);
                    return (AtEnd() && typed_other->AtEnd()) || (current1_ == typed_other->current1_ && current2_ == typed_other->current2_ &&
                            current3_ == typed_other->current3_ && current4_ == typed_other->current4_ && current5_ == typed_other->current5_ &&
                            current6_ == typed_other->current6_);
                }
            private:
                Iterator(const Iterator& other) : base_(other.base_), begin1_(other.begin1_), end1_(other.end1_), current1_(other.current1_),
                         begin2_(other.begin2_), end2_(other.end2_), current2_(other.current2_), begin3_(other.begin3_), end3_(other.end3_),
                         current3_(other.current3_), begin4_(other.begin4_), end4_(other.end4_), current4_(other.current4_),
                         begin5_(other.begin5_), end5_(other.end5_), current5_(other.current5_), begin6_(other.begin6_), end6_(other.end6_),
                         current6_(other.current6_) {
                    ComputeCurrentValue();
                }
                void ComputeCurrentValue() {
                    if (!AtEnd()) current_value_ = ParamType(*current1_, *current2_, *current3_, *current4_, *current5_, *current6_);
                }
                bool AtEnd() const {
                    return current1_ == end1_ || current2_ == end2_ || current3_ == end3_ || current4_ == end4_ || current5_ == end5_ || current6_ == end6_;
                }
                void operator=(const Iterator& other);
                const ParamGeneratorInterface<ParamType>* const base_;
                const typename ParamGenerator<T1>::iterator begin1_;
                const typename ParamGenerator<T1>::iterator end1_;
                typename ParamGenerator<T1>::iterator current1_;
                const typename ParamGenerator<T2>::iterator begin2_;
                const typename ParamGenerator<T2>::iterator end2_;
                typename ParamGenerator<T2>::iterator current2_;
                const typename ParamGenerator<T3>::iterator begin3_;
                const typename ParamGenerator<T3>::iterator end3_;
                typename ParamGenerator<T3>::iterator current3_;
                const typename ParamGenerator<T4>::iterator begin4_;
                const typename ParamGenerator<T4>::iterator end4_;
                typename ParamGenerator<T4>::iterator current4_;
                const typename ParamGenerator<T5>::iterator begin5_;
                const typename ParamGenerator<T5>::iterator end5_;
                typename ParamGenerator<T5>::iterator current5_;
                const typename ParamGenerator<T6>::iterator begin6_;
                const typename ParamGenerator<T6>::iterator end6_;
                typename ParamGenerator<T6>::iterator current6_;
                ParamType current_value_;
            };
            void operator=(const CartesianProductGenerator6& other);
            const ParamGenerator<T1> g1_;
            const ParamGenerator<T2> g2_;
            const ParamGenerator<T3> g3_;
            const ParamGenerator<T4> g4_;
            const ParamGenerator<T5> g5_;
            const ParamGenerator<T6> g6_;
        };
        template <typename T1, typename T2, typename T3, typename T4, typename T5,
            typename T6, typename T7>
        class CartesianProductGenerator7 : public ParamGeneratorInterface<::testing::tuple<T1, T2, T3, T4, T5, T6, T7>> {
        public:
            typedef ::testing::tuple<T1, T2, T3, T4, T5, T6, T7> ParamType;
            CartesianProductGenerator7(const ParamGenerator<T1>& g1, const ParamGenerator<T2>& g2, const ParamGenerator<T3>& g3,
                                       const ParamGenerator<T4>& g4, const ParamGenerator<T5>& g5, const ParamGenerator<T6>& g6,
                                       const ParamGenerator<T7>& g7) : g1_(g1), g2_(g2), g3_(g3), g4_(g4), g5_(g5), g6_(g6), g7_(g7) {}
            virtual ~CartesianProductGenerator7() {}
            virtual ParamIteratorInterface<ParamType>* Begin() const {
                return new Iterator(this, g1_, g1_.begin(), g2_, g2_.begin(), g3_, g3_.begin(), g4_, g4_.begin(), g5_, g5_.begin(), g6_,
                                    g6_.begin(), g7_, g7_.begin());
            }
            virtual ParamIteratorInterface<ParamType>* End() const {
                return new Iterator(this, g1_, g1_.end(), g2_, g2_.end(), g3_, g3_.end(), g4_, g4_.end(), g5_, g5_.end(), g6_, g6_.end(),
                                    g7_, g7_.end());
            }
        private:
            class Iterator : public ParamIteratorInterface<ParamType> {
            public:
                Iterator(const ParamGeneratorInterface<ParamType>* base, const ParamGenerator<T1>& g1,
                         const typename ParamGenerator<T1>::iterator& current1, const ParamGenerator<T2>& g2,
                         const typename ParamGenerator<T2>::iterator& current2, const ParamGenerator<T3>& g3,
                         const typename ParamGenerator<T3>::iterator& current3, const ParamGenerator<T4>& g4,
                         const typename ParamGenerator<T4>::iterator& current4, const ParamGenerator<T5>& g5,
                         const typename ParamGenerator<T5>::iterator& current5, const ParamGenerator<T6>& g6,
                         const typename ParamGenerator<T6>::iterator& current6, const ParamGenerator<T7>& g7,
                         const typename ParamGenerator<T7>::iterator& current7) : base_(base), begin1_(g1.begin()), end1_(g1.end()),
                         current1_(current1), begin2_(g2.begin()), end2_(g2.end()), current2_(current2), begin3_(g3.begin()), end3_(g3.end()),
                         current3_(current3), begin4_(g4.begin()), end4_(g4.end()), current4_(current4), begin5_(g5.begin()),
                         end5_(g5.end()), current5_(current5), begin6_(g6.begin()), end6_(g6.end()), current6_(current6), begin7_(g7.begin()),
                         end7_(g7.end()), current7_(current7) {
                    ComputeCurrentValue();
                }
                virtual ~Iterator() {}
                virtual const ParamGeneratorInterface<ParamType>* BaseGenerator() const {
                    return base_;
                }
                virtual void Advance() {
                    assert(!AtEnd());
                    ++current7_;
                    if (current7_ == end7_) {
                        current7_ = begin7_;
                        ++current6_;
                    }
                    if (current6_ == end6_) {
                        current6_ = begin6_;
                        ++current5_;
                    }
                    if (current5_ == end5_) {
                        current5_ = begin5_;
                        ++current4_;
                    }
                    if (current4_ == end4_) {
                        current4_ = begin4_;
                        ++current3_;
                    }
                    if (current3_ == end3_) {
                        current3_ = begin3_;
                        ++current2_;
                    }
                    if (current2_ == end2_) {
                        current2_ = begin2_;
                        ++current1_;
                    }
                    ComputeCurrentValue();
                }
                virtual ParamIteratorInterface<ParamType>* Clone() const {
                    return new Iterator(*this);
                }
                virtual const ParamType* Current() const { return &current_value_; }
                virtual bool Equals(const ParamIteratorInterface<ParamType>& other) const {
                    GTEST_CHECK_(BaseGenerator() == other.BaseGenerator()) << "The program attempted to compare iterators "
                                 << "from different generators." << std::endl;
                    const Iterator* typed_other = CheckedDowncastToActualType<const Iterator>(&other);
                    return (AtEnd() && typed_other->AtEnd()) || (current1_ == typed_other->current1_ && current2_ == typed_other->current2_ &&
                            current3_ == typed_other->current3_ && current4_ == typed_other->current4_ && current5_ == typed_other->current5_ &&
                            current6_ == typed_other->current6_ && current7_ == typed_other->current7_);
                }
            private:
                Iterator(const Iterator& other) : base_(other.base_), begin1_(other.begin1_), end1_(other.end1_), current1_(other.current1_),
                         begin2_(other.begin2_), end2_(other.end2_), current2_(other.current2_), begin3_(other.begin3_), end3_(other.end3_),
                         current3_(other.current3_), begin4_(other.begin4_), end4_(other.end4_), current4_(other.current4_),
                         begin5_(other.begin5_), end5_(other.end5_), current5_(other.current5_), begin6_(other.begin6_), end6_(other.end6_),
                         current6_(other.current6_), begin7_(other.begin7_), end7_(other.end7_), current7_(other.current7_) {
                    ComputeCurrentValue();
                }
                void ComputeCurrentValue() {
                    if (!AtEnd()) current_value_ = ParamType(*current1_, *current2_, *current3_, *current4_, *current5_, *current6_, *current7_);
                }
                bool AtEnd() const {
                    return current1_ == end1_ || current2_ == end2_ || current3_ == end3_ || current4_ == end4_ || current5_ == end5_ ||
                           current6_ == end6_ || current7_ == end7_;
                }
                void operator=(const Iterator& other);
                const ParamGeneratorInterface<ParamType>* const base_;
                const typename ParamGenerator<T1>::iterator begin1_;
                const typename ParamGenerator<T1>::iterator end1_;
                typename ParamGenerator<T1>::iterator current1_;
                const typename ParamGenerator<T2>::iterator begin2_;
                const typename ParamGenerator<T2>::iterator end2_;
                typename ParamGenerator<T2>::iterator current2_;
                const typename ParamGenerator<T3>::iterator begin3_;
                const typename ParamGenerator<T3>::iterator end3_;
                typename ParamGenerator<T3>::iterator current3_;
                const typename ParamGenerator<T4>::iterator begin4_;
                const typename ParamGenerator<T4>::iterator end4_;
                typename ParamGenerator<T4>::iterator current4_;
                const typename ParamGenerator<T5>::iterator begin5_;
                const typename ParamGenerator<T5>::iterator end5_;
                typename ParamGenerator<T5>::iterator current5_;
                const typename ParamGenerator<T6>::iterator begin6_;
                const typename ParamGenerator<T6>::iterator end6_;
                typename ParamGenerator<T6>::iterator current6_;
                const typename ParamGenerator<T7>::iterator begin7_;
                const typename ParamGenerator<T7>::iterator end7_;
                typename ParamGenerator<T7>::iterator current7_;
                ParamType current_value_;
            };
            void operator=(const CartesianProductGenerator7& other);
            const ParamGenerator<T1> g1_;
            const ParamGenerator<T2> g2_;
            const ParamGenerator<T3> g3_;
            const ParamGenerator<T4> g4_;
            const ParamGenerator<T5> g5_;
            const ParamGenerator<T6> g6_;
            const ParamGenerator<T7> g7_;
        };
        template <typename T1, typename T2, typename T3, typename T4, typename T5,
            typename T6, typename T7, typename T8>
        class CartesianProductGenerator8 : public ParamGeneratorInterface<::testing::tuple<T1, T2, T3, T4, T5, T6, T7, T8>> {
        public:
            typedef ::testing::tuple<T1, T2, T3, T4, T5, T6, T7, T8> ParamType;
            CartesianProductGenerator8(const ParamGenerator<T1>& g1, const ParamGenerator<T2>& g2, const ParamGenerator<T3>& g3,
                                       const ParamGenerator<T4>& g4, const ParamGenerator<T5>& g5, const ParamGenerator<T6>& g6,
                                       const ParamGenerator<T7>& g7, const ParamGenerator<T8>& g8) : g1_(g1), g2_(g2), g3_(g3), g4_(g4),
                                       g5_(g5), g6_(g6), g7_(g7), g8_(g8) {}
            virtual ~CartesianProductGenerator8() {}
            virtual ParamIteratorInterface<ParamType>* Begin() const {
                return new Iterator(this, g1_, g1_.begin(), g2_, g2_.begin(), g3_, g3_.begin(), g4_, g4_.begin(), g5_, g5_.begin(), g6_,
                                    g6_.begin(), g7_, g7_.begin(), g8_, g8_.begin());
            }
            virtual ParamIteratorInterface<ParamType>* End() const {
                return new Iterator(this, g1_, g1_.end(), g2_, g2_.end(), g3_, g3_.end(), g4_, g4_.end(), g5_, g5_.end(), g6_, g6_.end(), g7_,
                                    g7_.end(), g8_, g8_.end());
            }
        private:
            class Iterator : public ParamIteratorInterface<ParamType> {
            public:
                Iterator(const ParamGeneratorInterface<ParamType>* base, const ParamGenerator<T1>& g1,
                         const typename ParamGenerator<T1>::iterator& current1, const ParamGenerator<T2>& g2,
                         const typename ParamGenerator<T2>::iterator& current2, const ParamGenerator<T3>& g3,
                         const typename ParamGenerator<T3>::iterator& current3, const ParamGenerator<T4>& g4,
                         const typename ParamGenerator<T4>::iterator& current4, const ParamGenerator<T5>& g5,
                         const typename ParamGenerator<T5>::iterator& current5, const ParamGenerator<T6>& g6,
                         const typename ParamGenerator<T6>::iterator& current6, const ParamGenerator<T7>& g7,
                         const typename ParamGenerator<T7>::iterator& current7, const ParamGenerator<T8>& g8,
                         const typename ParamGenerator<T8>::iterator& current8) : base_(base), begin1_(g1.begin()), end1_(g1.end()),
                         current1_(current1), begin2_(g2.begin()), end2_(g2.end()), current2_(current2), begin3_(g3.begin()), end3_(g3.end()),
                         current3_(current3), begin4_(g4.begin()), end4_(g4.end()), current4_(current4), begin5_(g5.begin()), end5_(g5.end()),
                         current5_(current5), begin6_(g6.begin()), end6_(g6.end()), current6_(current6), begin7_(g7.begin()), end7_(g7.end()),
                         current7_(current7), begin8_(g8.begin()), end8_(g8.end()), current8_(current8) {
                    ComputeCurrentValue();
                }
                virtual ~Iterator() {}
                virtual const ParamGeneratorInterface<ParamType>* BaseGenerator() const {
                    return base_;
                }
                virtual void Advance() {
                    assert(!AtEnd());
                    ++current8_;
                    if (current8_ == end8_) {
                        current8_ = begin8_;
                        ++current7_;
                    }
                    if (current7_ == end7_) {
                        current7_ = begin7_;
                        ++current6_;
                    }
                    if (current6_ == end6_) {
                        current6_ = begin6_;
                        ++current5_;
                    }
                    if (current5_ == end5_) {
                        current5_ = begin5_;
                        ++current4_;
                    }
                    if (current4_ == end4_) {
                        current4_ = begin4_;
                        ++current3_;
                    }
                    if (current3_ == end3_) {
                        current3_ = begin3_;
                        ++current2_;
                    }
                    if (current2_ == end2_) {
                        current2_ = begin2_;
                        ++current1_;
                    }
                    ComputeCurrentValue();
                }
                virtual ParamIteratorInterface<ParamType>* Clone() const {
                    return new Iterator(*this);
                }
                virtual const ParamType* Current() const { return &current_value_; }
                virtual bool Equals(const ParamIteratorInterface<ParamType>& other) const {
                    GTEST_CHECK_(BaseGenerator() == other.BaseGenerator()) << "The program attempted to compare iterators "
                                 << "from different generators." << std::endl;
                    const Iterator* typed_other = CheckedDowncastToActualType<const Iterator>(&other);
                    return (AtEnd() && typed_other->AtEnd()) || (current1_ == typed_other->current1_ && current2_ == typed_other->current2_ &&
                            current3_ == typed_other->current3_ && current4_ == typed_other->current4_ && current5_ == typed_other->current5_ &&
                            current6_ == typed_other->current6_ && current7_ == typed_other->current7_ && current8_ == typed_other->current8_);
                }
            private:
                Iterator(const Iterator& other) : base_(other.base_), begin1_(other.begin1_), end1_(other.end1_), current1_(other.current1_),
                         begin2_(other.begin2_), end2_(other.end2_), current2_(other.current2_), begin3_(other.begin3_), end3_(other.end3_),
                         current3_(other.current3_), begin4_(other.begin4_), end4_(other.end4_), current4_(other.current4_),
                         begin5_(other.begin5_), end5_(other.end5_), current5_(other.current5_), begin6_(other.begin6_), end6_(other.end6_),
                         current6_(other.current6_), begin7_(other.begin7_), end7_(other.end7_), current7_(other.current7_),
                         begin8_(other.begin8_), end8_(other.end8_), current8_(other.current8_) {
                    ComputeCurrentValue();
                }
                void ComputeCurrentValue() {
                    if (!AtEnd()) {
                        current_value_ = ParamType(*current1_, *current2_, *current3_, *current4_, *current5_, *current6_, *current7_,
                                                   *current8_);
                    }
                }
                bool AtEnd() const {
                    return current1_ == end1_ || current2_ == end2_ || current3_ == end3_ || current4_ == end4_ || current5_ == end5_ ||
                           current6_ == end6_ || current7_ == end7_ || current8_ == end8_;
                }
                void operator=(const Iterator& other);
                const ParamGeneratorInterface<ParamType>* const base_;
                const typename ParamGenerator<T1>::iterator begin1_;
                const typename ParamGenerator<T1>::iterator end1_;
                typename ParamGenerator<T1>::iterator current1_;
                const typename ParamGenerator<T2>::iterator begin2_;
                const typename ParamGenerator<T2>::iterator end2_;
                typename ParamGenerator<T2>::iterator current2_;
                const typename ParamGenerator<T3>::iterator begin3_;
                const typename ParamGenerator<T3>::iterator end3_;
                typename ParamGenerator<T3>::iterator current3_;
                const typename ParamGenerator<T4>::iterator begin4_;
                const typename ParamGenerator<T4>::iterator end4_;
                typename ParamGenerator<T4>::iterator current4_;
                const typename ParamGenerator<T5>::iterator begin5_;
                const typename ParamGenerator<T5>::iterator end5_;
                typename ParamGenerator<T5>::iterator current5_;
                const typename ParamGenerator<T6>::iterator begin6_;
                const typename ParamGenerator<T6>::iterator end6_;
                typename ParamGenerator<T6>::iterator current6_;
                const typename ParamGenerator<T7>::iterator begin7_;
                const typename ParamGenerator<T7>::iterator end7_;
                typename ParamGenerator<T7>::iterator current7_;
                const typename ParamGenerator<T8>::iterator begin8_;
                const typename ParamGenerator<T8>::iterator end8_;
                typename ParamGenerator<T8>::iterator current8_;
                ParamType current_value_;
            };
            void operator=(const CartesianProductGenerator8& other);
            const ParamGenerator<T1> g1_;
            const ParamGenerator<T2> g2_;
            const ParamGenerator<T3> g3_;
            const ParamGenerator<T4> g4_;
            const ParamGenerator<T5> g5_;
            const ParamGenerator<T6> g6_;
            const ParamGenerator<T7> g7_;
            const ParamGenerator<T8> g8_;
        };
        template <typename T1, typename T2, typename T3, typename T4, typename T5,
            typename T6, typename T7, typename T8, typename T9>
        class CartesianProductGenerator9 : public ParamGeneratorInterface<::testing::tuple<T1, T2, T3, T4, T5, T6, T7, T8, T9>> {
        public:
            typedef ::testing::tuple<T1, T2, T3, T4, T5, T6, T7, T8, T9> ParamType;
            CartesianProductGenerator9(const ParamGenerator<T1>& g1, const ParamGenerator<T2>& g2, const ParamGenerator<T3>& g3,
                                       const ParamGenerator<T4>& g4, const ParamGenerator<T5>& g5, const ParamGenerator<T6>& g6,
                                       const ParamGenerator<T7>& g7, const ParamGenerator<T8>& g8, const ParamGenerator<T9>& g9) : g1_(g1),
                                       g2_(g2), g3_(g3), g4_(g4), g5_(g5), g6_(g6), g7_(g7), g8_(g8), g9_(g9) {}
            virtual ~CartesianProductGenerator9() {}
            virtual ParamIteratorInterface<ParamType>* Begin() const {
                return new Iterator(this, g1_, g1_.begin(), g2_, g2_.begin(), g3_, g3_.begin(), g4_, g4_.begin(), g5_, g5_.begin(), g6_,
                                    g6_.begin(), g7_, g7_.begin(), g8_, g8_.begin(), g9_, g9_.begin());
            }
            virtual ParamIteratorInterface<ParamType>* End() const {
                return new Iterator(this, g1_, g1_.end(), g2_, g2_.end(), g3_, g3_.end(), g4_, g4_.end(), g5_, g5_.end(), g6_, g6_.end(),
                                    g7_, g7_.end(), g8_, g8_.end(), g9_, g9_.end());
            }
        private:
            class Iterator : public ParamIteratorInterface<ParamType> {
            public:
                Iterator(const ParamGeneratorInterface<ParamType>* base, const ParamGenerator<T1>& g1,
                         const typename ParamGenerator<T1>::iterator& current1, const ParamGenerator<T2>& g2,
                         const typename ParamGenerator<T2>::iterator& current2, const ParamGenerator<T3>& g3,
                         const typename ParamGenerator<T3>::iterator& current3, const ParamGenerator<T4>& g4,
                         const typename ParamGenerator<T4>::iterator& current4, const ParamGenerator<T5>& g5,
                         const typename ParamGenerator<T5>::iterator& current5, const ParamGenerator<T6>& g6,
                         const typename ParamGenerator<T6>::iterator& current6, const ParamGenerator<T7>& g7,
                         const typename ParamGenerator<T7>::iterator& current7, const ParamGenerator<T8>& g8,
                         const typename ParamGenerator<T8>::iterator& current8, const ParamGenerator<T9>& g9,
                         const typename ParamGenerator<T9>::iterator& current9) : base_(base), begin1_(g1.begin()), end1_(g1.end()),
                         current1_(current1), begin2_(g2.begin()), end2_(g2.end()), current2_(current2), begin3_(g3.begin()), end3_(g3.end()),
                         current3_(current3), begin4_(g4.begin()), end4_(g4.end()), current4_(current4), begin5_(g5.begin()), end5_(g5.end()),
                         current5_(current5), begin6_(g6.begin()), end6_(g6.end()), current6_(current6), begin7_(g7.begin()), end7_(g7.end()),
                         current7_(current7), begin8_(g8.begin()), end8_(g8.end()), current8_(current8), begin9_(g9.begin()), end9_(g9.end()),
                         current9_(current9) {
                    ComputeCurrentValue();
                }
                virtual ~Iterator() {}
                virtual const ParamGeneratorInterface<ParamType>* BaseGenerator() const {
                    return base_;
                }
                virtual void Advance() {
                    assert(!AtEnd());
                    ++current9_;
                    if (current9_ == end9_) {
                        current9_ = begin9_;
                        ++current8_;
                    }
                    if (current8_ == end8_) {
                        current8_ = begin8_;
                        ++current7_;
                    }
                    if (current7_ == end7_) {
                        current7_ = begin7_;
                        ++current6_;
                    }
                    if (current6_ == end6_) {
                        current6_ = begin6_;
                        ++current5_;
                    }
                    if (current5_ == end5_) {
                        current5_ = begin5_;
                        ++current4_;
                    }
                    if (current4_ == end4_) {
                        current4_ = begin4_;
                        ++current3_;
                    }
                    if (current3_ == end3_) {
                        current3_ = begin3_;
                        ++current2_;
                    }
                    if (current2_ == end2_) {
                        current2_ = begin2_;
                        ++current1_;
                    }
                    ComputeCurrentValue();
                }
                virtual ParamIteratorInterface<ParamType>* Clone() const {
                    return new Iterator(*this);
                }
                virtual const ParamType* Current() const { return &current_value_; }
                virtual bool Equals(const ParamIteratorInterface<ParamType>& other) const {
                    GTEST_CHECK_(BaseGenerator() == other.BaseGenerator()) << "The program attempted to compare iterators "
                                 << "from different generators." << std::endl;
                    const Iterator* typed_other = CheckedDowncastToActualType<const Iterator>(&other);
                    return (AtEnd() && typed_other->AtEnd()) || ( current1_ == typed_other->current1_ && current2_ == typed_other->current2_ &&
                            current3_ == typed_other->current3_ && current4_ == typed_other->current4_ && current5_ == typed_other->current5_ &&
                            current6_ == typed_other->current6_ && current7_ == typed_other->current7_ && current8_ == typed_other->current8_ &&
                            current9_ == typed_other->current9_);
                }
            private:
                Iterator(const Iterator& other) : base_(other.base_), begin1_(other.begin1_), end1_(other.end1_), current1_(other.current1_),
                         begin2_(other.begin2_), end2_(other.end2_), current2_(other.current2_), begin3_(other.begin3_), end3_(other.end3_),
                         current3_(other.current3_), begin4_(other.begin4_), end4_(other.end4_), current4_(other.current4_),
                         begin5_(other.begin5_), end5_(other.end5_), current5_(other.current5_), begin6_(other.begin6_), end6_(other.end6_),
                         current6_(other.current6_), begin7_(other.begin7_), end7_(other.end7_), current7_(other.current7_),
                         begin8_(other.begin8_), end8_(other.end8_), current8_(other.current8_), begin9_(other.begin9_), end9_(other.end9_),
                         current9_(other.current9_) {
                    ComputeCurrentValue();
                }
                void ComputeCurrentValue() {
                    if (!AtEnd()) {
                        current_value_ = ParamType(*current1_, *current2_, *current3_, *current4_, *current5_, *current6_, *current7_,
                                                   *current8_, *current9_);
                    }
                }
                bool AtEnd() const {
                    return current1_ == end1_ || current2_ == end2_ || current3_ == end3_ || current4_ == end4_ || current5_ == end5_ ||
                           current6_ == end6_ || current7_ == end7_ || current8_ == end8_ || current9_ == end9_;
                }
                void operator=(const Iterator& other);
                const ParamGeneratorInterface<ParamType>* const base_;
                const typename ParamGenerator<T1>::iterator begin1_;
                const typename ParamGenerator<T1>::iterator end1_;
                typename ParamGenerator<T1>::iterator current1_;
                const typename ParamGenerator<T2>::iterator begin2_;
                const typename ParamGenerator<T2>::iterator end2_;
                typename ParamGenerator<T2>::iterator current2_;
                const typename ParamGenerator<T3>::iterator begin3_;
                const typename ParamGenerator<T3>::iterator end3_;
                typename ParamGenerator<T3>::iterator current3_;
                const typename ParamGenerator<T4>::iterator begin4_;
                const typename ParamGenerator<T4>::iterator end4_;
                typename ParamGenerator<T4>::iterator current4_;
                const typename ParamGenerator<T5>::iterator begin5_;
                const typename ParamGenerator<T5>::iterator end5_;
                typename ParamGenerator<T5>::iterator current5_;
                const typename ParamGenerator<T6>::iterator begin6_;
                const typename ParamGenerator<T6>::iterator end6_;
                typename ParamGenerator<T6>::iterator current6_;
                const typename ParamGenerator<T7>::iterator begin7_;
                const typename ParamGenerator<T7>::iterator end7_;
                typename ParamGenerator<T7>::iterator current7_;
                const typename ParamGenerator<T8>::iterator begin8_;
                const typename ParamGenerator<T8>::iterator end8_;
                typename ParamGenerator<T8>::iterator current8_;
                const typename ParamGenerator<T9>::iterator begin9_;
                const typename ParamGenerator<T9>::iterator end9_;
                typename ParamGenerator<T9>::iterator current9_;
                ParamType current_value_;
            };
            void operator=(const CartesianProductGenerator9& other);
            const ParamGenerator<T1> g1_;
            const ParamGenerator<T2> g2_;
            const ParamGenerator<T3> g3_;
            const ParamGenerator<T4> g4_;
            const ParamGenerator<T5> g5_;
            const ParamGenerator<T6> g6_;
            const ParamGenerator<T7> g7_;
            const ParamGenerator<T8> g8_;
            const ParamGenerator<T9> g9_;
        };
        template <typename T1, typename T2, typename T3, typename T4, typename T5,
            typename T6, typename T7, typename T8, typename T9, typename T10>
        class CartesianProductGenerator10 : public ParamGeneratorInterface<::testing::tuple<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10>> {
        public:
            typedef ::testing::tuple<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10> ParamType;
            CartesianProductGenerator10(const ParamGenerator<T1>& g1, const ParamGenerator<T2>& g2, const ParamGenerator<T3>& g3,
                                        const ParamGenerator<T4>& g4, const ParamGenerator<T5>& g5, const ParamGenerator<T6>& g6,
                                        const ParamGenerator<T7>& g7, const ParamGenerator<T8>& g8, const ParamGenerator<T9>& g9,
                                        const ParamGenerator<T10>& g10) : g1_(g1), g2_(g2), g3_(g3), g4_(g4), g5_(g5), g6_(g6), g7_(g7),
                                        g8_(g8), g9_(g9), g10_(g10) {}
            virtual ~CartesianProductGenerator10() {}
            virtual ParamIteratorInterface<ParamType>* Begin() const {
                return new Iterator(this, g1_, g1_.begin(), g2_, g2_.begin(), g3_, g3_.begin(), g4_, g4_.begin(), g5_, g5_.begin(), g6_,
                                    g6_.begin(), g7_, g7_.begin(), g8_, g8_.begin(), g9_, g9_.begin(), g10_, g10_.begin());
            }
            virtual ParamIteratorInterface<ParamType>* End() const {
                return new Iterator(this, g1_, g1_.end(), g2_, g2_.end(), g3_, g3_.end(), g4_, g4_.end(), g5_, g5_.end(), g6_, g6_.end(),
                                    g7_, g7_.end(), g8_, g8_.end(), g9_, g9_.end(), g10_, g10_.end());
            }
        private:
            class Iterator : public ParamIteratorInterface<ParamType> {
            public:
                Iterator(const ParamGeneratorInterface<ParamType>* base, const ParamGenerator<T1>& g1,
                         const typename ParamGenerator<T1>::iterator& current1, const ParamGenerator<T2>& g2,
                         const typename ParamGenerator<T2>::iterator& current2, const ParamGenerator<T3>& g3,
                         const typename ParamGenerator<T3>::iterator& current3, const ParamGenerator<T4>& g4,
                         const typename ParamGenerator<T4>::iterator& current4, const ParamGenerator<T5>& g5,
                         const typename ParamGenerator<T5>::iterator& current5, const ParamGenerator<T6>& g6,
                         const typename ParamGenerator<T6>::iterator& current6, const ParamGenerator<T7>& g7,
                         const typename ParamGenerator<T7>::iterator& current7, const ParamGenerator<T8>& g8,
                         const typename ParamGenerator<T8>::iterator& current8, const ParamGenerator<T9>& g9,
                         const typename ParamGenerator<T9>::iterator& current9, const ParamGenerator<T10>& g10,
                         const typename ParamGenerator<T10>::iterator& current10) : base_(base), begin1_(g1.begin()), end1_(g1.end()),
                         current1_(current1), begin2_(g2.begin()), end2_(g2.end()), current2_(current2), begin3_(g3.begin()), end3_(g3.end()),
                         current3_(current3), begin4_(g4.begin()), end4_(g4.end()), current4_(current4), begin5_(g5.begin()), end5_(g5.end()),
                         current5_(current5), begin6_(g6.begin()), end6_(g6.end()), current6_(current6), begin7_(g7.begin()), end7_(g7.end()),
                         current7_(current7), begin8_(g8.begin()), end8_(g8.end()), current8_(current8), begin9_(g9.begin()), end9_(g9.end()),
                         current9_(current9), begin10_(g10.begin()), end10_(g10.end()), current10_(current10) {
                    ComputeCurrentValue();
                }
                virtual ~Iterator() {}
                virtual const ParamGeneratorInterface<ParamType>* BaseGenerator() const {
                    return base_;
                }
                virtual void Advance() {
                    assert(!AtEnd());
                    ++current10_;
                    if (current10_ == end10_) {
                        current10_ = begin10_;
                        ++current9_;
                    }
                    if (current9_ == end9_) {
                        current9_ = begin9_;
                        ++current8_;
                    }
                    if (current8_ == end8_) {
                        current8_ = begin8_;
                        ++current7_;
                    }
                    if (current7_ == end7_) {
                        current7_ = begin7_;
                        ++current6_;
                    }
                    if (current6_ == end6_) {
                        current6_ = begin6_;
                        ++current5_;
                    }
                    if (current5_ == end5_) {
                        current5_ = begin5_;
                        ++current4_;
                    }
                    if (current4_ == end4_) {
                        current4_ = begin4_;
                        ++current3_;
                    }
                    if (current3_ == end3_) {
                        current3_ = begin3_;
                        ++current2_;
                    }
                    if (current2_ == end2_) {
                        current2_ = begin2_;
                        ++current1_;
                    }
                    ComputeCurrentValue();
                }
                virtual ParamIteratorInterface<ParamType>* Clone() const {
                    return new Iterator(*this);
                }
                virtual const ParamType* Current() const { return &current_value_; }
                virtual bool Equals(const ParamIteratorInterface<ParamType>& other) const {
                    GTEST_CHECK_(BaseGenerator() == other.BaseGenerator()) << "The program attempted to compare iterators "
                                 << "from different generators." << std::endl;
                    const Iterator* typed_other = CheckedDowncastToActualType<const Iterator>(&other);
                    return (AtEnd() && typed_other->AtEnd()) || (current1_ == typed_other->current1_ && current2_ == typed_other->current2_ &&
                            current3_ == typed_other->current3_ && current4_ == typed_other->current4_ && current5_ == typed_other->current5_ &&
                            current6_ == typed_other->current6_ && current7_ == typed_other->current7_ && current8_ == typed_other->current8_ &&
                            current9_ == typed_other->current9_ && current10_ == typed_other->current10_);
                }
            private:
                Iterator(const Iterator& other) : base_(other.base_), begin1_(other.begin1_), end1_(other.end1_), current1_(other.current1_),
                         begin2_(other.begin2_), end2_(other.end2_), current2_(other.current2_), begin3_(other.begin3_), end3_(other.end3_),
                         current3_(other.current3_), begin4_(other.begin4_), end4_(other.end4_), current4_(other.current4_),
                         begin5_(other.begin5_), end5_(other.end5_), current5_(other.current5_), begin6_(other.begin6_), end6_(other.end6_),
                         current6_(other.current6_), begin7_(other.begin7_), end7_(other.end7_), current7_(other.current7_),
                         begin8_(other.begin8_), end8_(other.end8_), current8_(other.current8_), begin9_(other.begin9_), end9_(other.end9_),
                         current9_(other.current9_), begin10_(other.begin10_), end10_(other.end10_), current10_(other.current10_) {
                    ComputeCurrentValue();
                }
                void ComputeCurrentValue() {
                    if (!AtEnd()) {
                        current_value_ = ParamType(*current1_, *current2_, *current3_, *current4_, *current5_, *current6_, *current7_,
                                                   *current8_, *current9_, *current10_);
                    }
                }
                bool AtEnd() const {
                    return current1_ == end1_ || current2_ == end2_ || current3_ == end3_ || current4_ == end4_ || current5_ == end5_ ||
                           current6_ == end6_ || current7_ == end7_ || current8_ == end8_ || current9_ == end9_ || current10_ == end10_;
                }
                void operator=(const Iterator& other);
                const ParamGeneratorInterface<ParamType>* const base_;
                const typename ParamGenerator<T1>::iterator begin1_;
                const typename ParamGenerator<T1>::iterator end1_;
                typename ParamGenerator<T1>::iterator current1_;
                const typename ParamGenerator<T2>::iterator begin2_;
                const typename ParamGenerator<T2>::iterator end2_;
                typename ParamGenerator<T2>::iterator current2_;
                const typename ParamGenerator<T3>::iterator begin3_;
                const typename ParamGenerator<T3>::iterator end3_;
                typename ParamGenerator<T3>::iterator current3_;
                const typename ParamGenerator<T4>::iterator begin4_;
                const typename ParamGenerator<T4>::iterator end4_;
                typename ParamGenerator<T4>::iterator current4_;
                const typename ParamGenerator<T5>::iterator begin5_;
                const typename ParamGenerator<T5>::iterator end5_;
                typename ParamGenerator<T5>::iterator current5_;
                const typename ParamGenerator<T6>::iterator begin6_;
                const typename ParamGenerator<T6>::iterator end6_;
                typename ParamGenerator<T6>::iterator current6_;
                const typename ParamGenerator<T7>::iterator begin7_;
                const typename ParamGenerator<T7>::iterator end7_;
                typename ParamGenerator<T7>::iterator current7_;
                const typename ParamGenerator<T8>::iterator begin8_;
                const typename ParamGenerator<T8>::iterator end8_;
                typename ParamGenerator<T8>::iterator current8_;
                const typename ParamGenerator<T9>::iterator begin9_;
                const typename ParamGenerator<T9>::iterator end9_;
                typename ParamGenerator<T9>::iterator current9_;
                const typename ParamGenerator<T10>::iterator begin10_;
                const typename ParamGenerator<T10>::iterator end10_;
                typename ParamGenerator<T10>::iterator current10_;
                ParamType current_value_;
            };
            void operator=(const CartesianProductGenerator10& other);
            const ParamGenerator<T1> g1_;
            const ParamGenerator<T2> g2_;
            const ParamGenerator<T3> g3_;
            const ParamGenerator<T4> g4_;
            const ParamGenerator<T5> g5_;
            const ParamGenerator<T6> g6_;
            const ParamGenerator<T7> g7_;
            const ParamGenerator<T8> g8_;
            const ParamGenerator<T9> g9_;
            const ParamGenerator<T10> g10_;
        };
        template <class Generator1, class Generator2> class CartesianProductHolder2 {
        public:
            CartesianProductHolder2(const Generator1& g1, const Generator2& g2) : g1_(g1), g2_(g2) {}
            template <typename T1, typename T2> operator ParamGenerator< ::testing::tuple<T1, T2>>() const {
                return ParamGenerator<::testing::tuple<T1, T2>>(new CartesianProductGenerator2<T1, T2>(static_cast<ParamGenerator<T1>>(g1_),
                                                                 static_cast<ParamGenerator<T2>>(g2_)));
            }
        private:
            void operator=(const CartesianProductHolder2& other);
            const Generator1 g1_;
            const Generator2 g2_;
        };
        template <class Generator1, class Generator2, class Generator3> class CartesianProductHolder3 {
        public:
            CartesianProductHolder3(const Generator1& g1, const Generator2& g2, const Generator3& g3) : g1_(g1), g2_(g2), g3_(g3) {}
            template <typename T1, typename T2, typename T3> operator ParamGenerator< ::testing::tuple<T1, T2, T3>>() const {
                return ParamGenerator<::testing::tuple<T1, T2, T3>>(new CartesianProductGenerator3<T1, T2, T3>(static_cast<ParamGenerator<T1>>(g1_),
                                                                    static_cast<ParamGenerator<T2>>(g2_), static_cast<ParamGenerator<T3> >(g3_)));
            }
        private:
            void operator=(const CartesianProductHolder3& other);
            const Generator1 g1_;
            const Generator2 g2_;
            const Generator3 g3_;
        };
        template <class Generator1, class Generator2, class Generator3,
            class Generator4> class CartesianProductHolder4 {
        public:
            CartesianProductHolder4(const Generator1& g1, const Generator2& g2, const Generator3& g3, const Generator4& g4) : g1_(g1), g2_(g2),
                                    g3_(g3), g4_(g4) {}
            template <typename T1, typename T2, typename T3, typename T4> operator ParamGenerator< ::testing::tuple<T1, T2, T3, T4>>() const {
                return ParamGenerator<::testing::tuple<T1, T2, T3, T4>>(new CartesianProductGenerator4<T1, T2, T3, T4>(
                                      static_cast<ParamGenerator<T1>>(g1_), static_cast<ParamGenerator<T2>>(g2_),
                                      static_cast<ParamGenerator<T3>>(g3_), static_cast<ParamGenerator<T4>>(g4_)));
            }
        private:
            void operator=(const CartesianProductHolder4& other);
            const Generator1 g1_;
            const Generator2 g2_;
            const Generator3 g3_;
            const Generator4 g4_;
        };
        template <class Generator1, class Generator2, class Generator3,
            class Generator4, class Generator5> class CartesianProductHolder5 {
        public:
            CartesianProductHolder5(const Generator1& g1, const Generator2& g2, const Generator3& g3, const Generator4& g4,
                                    const Generator5& g5) : g1_(g1), g2_(g2), g3_(g3), g4_(g4), g5_(g5) {}
            template <typename T1, typename T2, typename T3, typename T4, typename T5> operator ParamGenerator< ::testing::tuple<T1, T2, T3, T4, T5> >() const {
            return ParamGenerator< ::testing::tuple<T1, T2, T3, T4, T5> >(
                new CartesianProductGenerator5<T1, T2, T3, T4, T5>(
                static_cast<ParamGenerator<T1> >(g1_),
                static_cast<ParamGenerator<T2> >(g2_),
                static_cast<ParamGenerator<T3> >(g3_),
                static_cast<ParamGenerator<T4> >(g4_),
                static_cast<ParamGenerator<T5> >(g5_)));
            }
        private:
            void operator=(const CartesianProductHolder5& other);
            const Generator1 g1_;
            const Generator2 g2_;
            const Generator3 g3_;
            const Generator4 g4_;
            const Generator5 g5_;
        };
        template <class Generator1, class Generator2, class Generator3,
            class Generator4, class Generator5, class Generator6> class CartesianProductHolder6 {
        public:
            CartesianProductHolder6(const Generator1& g1, const Generator2& g2, const Generator3& g3, const Generator4& g4,
                                    const Generator5& g5, const Generator6& g6) : g1_(g1), g2_(g2), g3_(g3), g4_(g4), g5_(g5), g6_(g6) {}
            template <typename T1, typename T2, typename T3, typename T4, typename T5,
              typename T6> operator ParamGenerator< ::testing::tuple<T1, T2, T3, T4, T5, T6> >() const {
                return ParamGenerator<::testing::tuple<T1, T2, T3, T4, T5, T6>>(new CartesianProductGenerator6<T1, T2, T3, T4, T5, T6>(
                                      static_cast<ParamGenerator<T1>>(g1_), static_cast<ParamGenerator<T2>>(g2_),
                                      static_cast<ParamGenerator<T3>>(g3_), static_cast<ParamGenerator<T4>>(g4_),
                                      static_cast<ParamGenerator<T5>>(g5_), static_cast<ParamGenerator<T6>>(g6_)));
            }
        private:
            void operator=(const CartesianProductHolder6& other);
            const Generator1 g1_;
            const Generator2 g2_;
            const Generator3 g3_;
            const Generator4 g4_;
            const Generator5 g5_;
            const Generator6 g6_;
        };
        template <class Generator1, class Generator2, class Generator3,
            class Generator4, class Generator5, class Generator6, class Generator7> class CartesianProductHolder7 {
        public:
            CartesianProductHolder7(const Generator1& g1, const Generator2& g2, const Generator3& g3, const Generator4& g4,
                                    const Generator5& g5, const Generator6& g6, const Generator7& g7) : g1_(g1), g2_(g2), g3_(g3), g4_(g4),
                                    g5_(g5), g6_(g6), g7_(g7) {}
            template <typename T1, typename T2, typename T3, typename T4, typename T5,
              typename T6, typename T7> operator ParamGenerator< ::testing::tuple<T1, T2, T3, T4, T5, T6, T7> >() const {
                return ParamGenerator<::testing::tuple<T1, T2, T3, T4, T5, T6, T7>>(new CartesianProductGenerator7<T1, T2, T3, T4, T5, T6, T7>(
                                      static_cast<ParamGenerator<T1>>(g1_), static_cast<ParamGenerator<T2>>(g2_),
                                      static_cast<ParamGenerator<T3>>(g3_), static_cast<ParamGenerator<T4>>(g4_),
                                      static_cast<ParamGenerator<T5>>(g5_), static_cast<ParamGenerator<T6>>(g6_),
                                      static_cast<ParamGenerator<T7>>(g7_)));
            }
        private:
            void operator=(const CartesianProductHolder7& other);
            const Generator1 g1_;
            const Generator2 g2_;
            const Generator3 g3_;
            const Generator4 g4_;
            const Generator5 g5_;
            const Generator6 g6_;
            const Generator7 g7_;
        };
        template <class Generator1, class Generator2, class Generator3,
            class Generator4, class Generator5, class Generator6, class Generator7,
            class Generator8> class CartesianProductHolder8 {
        public:
            CartesianProductHolder8(const Generator1& g1, const Generator2& g2, const Generator3& g3, const Generator4& g4, const Generator5& g5,
                                    const Generator6& g6, const Generator7& g7, const Generator8& g8) : g1_(g1), g2_(g2), g3_(g3), g4_(g4),
                                    g5_(g5), g6_(g6), g7_(g7), g8_(g8) {}
            template <typename T1, typename T2, typename T3, typename T4, typename T5,
              typename T6, typename T7, typename T8> operator ParamGenerator< ::testing::tuple<T1, T2, T3, T4, T5, T6, T7, T8> >() const {
                return ParamGenerator<::testing::tuple<T1, T2, T3, T4, T5, T6, T7, T8>>(
                                      new CartesianProductGenerator8<T1, T2, T3, T4, T5, T6, T7, T8>(static_cast<ParamGenerator<T1>>(g1_),
                                      static_cast<ParamGenerator<T2>>(g2_), static_cast<ParamGenerator<T3>>(g3_),
                                      static_cast<ParamGenerator<T4>>(g4_), static_cast<ParamGenerator<T5>>(g5_),
                                      static_cast<ParamGenerator<T6>>(g6_), static_cast<ParamGenerator<T7>>(g7_),
                                      static_cast<ParamGenerator<T8>>(g8_)));
            }
        private:
            void operator=(const CartesianProductHolder8& other);
            const Generator1 g1_;
            const Generator2 g2_;
            const Generator3 g3_;
            const Generator4 g4_;
            const Generator5 g5_;
            const Generator6 g6_;
            const Generator7 g7_;
            const Generator8 g8_;
        };
        template <class Generator1, class Generator2, class Generator3,
            class Generator4, class Generator5, class Generator6, class Generator7,
            class Generator8, class Generator9> class CartesianProductHolder9 {
        public:
            CartesianProductHolder9(const Generator1& g1, const Generator2& g2, const Generator3& g3, const Generator4& g4, const Generator5& g5,
                                    const Generator6& g6, const Generator7& g7, const Generator8& g8, const Generator9& g9) : g1_(g1), g2_(g2),
                                    g3_(g3), g4_(g4),g5_(g5), g6_(g6), g7_(g7), g8_(g8), g9_(g9) {}
            template <typename T1, typename T2, typename T3, typename T4, typename T5,
              typename T6, typename T7, typename T8, typename T9> operator ParamGenerator< ::testing::tuple<T1, T2, T3, T4, T5, T6, T7, T8, T9> >() const {
                return ParamGenerator<::testing::tuple<T1, T2, T3, T4, T5, T6, T7, T8, T9>>(
                                      new CartesianProductGenerator9<T1, T2, T3, T4, T5, T6, T7, T8, T9>(static_cast<ParamGenerator<T1>>(g1_),
                                      static_cast<ParamGenerator<T2>>(g2_), static_cast<ParamGenerator<T3>>(g3_),
                                      static_cast<ParamGenerator<T4>>(g4_), static_cast<ParamGenerator<T5>>(g5_),
                                      static_cast<ParamGenerator<T6>>(g6_), static_cast<ParamGenerator<T7>>(g7_),
                                      static_cast<ParamGenerator<T8>>(g8_), static_cast<ParamGenerator<T9>>(g9_)));
            }
        private:
            void operator=(const CartesianProductHolder9& other);
            const Generator1 g1_;
            const Generator2 g2_;
            const Generator3 g3_;
            const Generator4 g4_;
            const Generator5 g5_;
            const Generator6 g6_;
            const Generator7 g7_;
            const Generator8 g8_;
            const Generator9 g9_;
        };
        template <class Generator1, class Generator2, class Generator3,
            class Generator4, class Generator5, class Generator6, class Generator7,
            class Generator8, class Generator9, class Generator10>
        class CartesianProductHolder10 {
        public:
            CartesianProductHolder10(const Generator1& g1, const Generator2& g2, const Generator3& g3, const Generator4& g4,
                                     const Generator5& g5, const Generator6& g6, const Generator7& g7, const Generator8& g8,
                                     const Generator9& g9, const Generator10& g10) : g1_(g1), g2_(g2), g3_(g3), g4_(g4), g5_(g5), g6_(g6),
                                     g7_(g7), g8_(g8), g9_(g9), g10_(g10) {}
            template <typename T1, typename T2, typename T3, typename T4, typename T5,
              typename T6, typename T7, typename T8, typename T9, typename T10> operator ParamGenerator< ::testing::tuple<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10> >() const {
                return ParamGenerator<::testing::tuple<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10>>(
                                      new CartesianProductGenerator10<T1, T2, T3, T4, T5, T6, T7, T8, T9, T10>(
                                      static_cast<ParamGenerator<T1>>(g1_), static_cast<ParamGenerator<T2>>(g2_),
                                      static_cast<ParamGenerator<T3>>(g3_), static_cast<ParamGenerator<T4>>(g4_),
                                      static_cast<ParamGenerator<T5>>(g5_), static_cast<ParamGenerator<T6>>(g6_),
                                      static_cast<ParamGenerator<T7>>(g7_), static_cast<ParamGenerator<T8>>(g8_),
                                      static_cast<ParamGenerator<T9>>(g9_), static_cast<ParamGenerator<T10>>(g10_)));
            }
        private:
            void operator=(const CartesianProductHolder10& other);
            const Generator1 g1_;
            const Generator2 g2_;
            const Generator3 g3_;
            const Generator4 g4_;
            const Generator5 g5_;
            const Generator6 g6_;
            const Generator7 g7_;
            const Generator8 g8_;
            const Generator9 g9_;
            const Generator10 g10_;
        };
    #endif
    }
}
#endif
#endif