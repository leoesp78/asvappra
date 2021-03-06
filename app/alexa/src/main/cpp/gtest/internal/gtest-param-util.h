#ifndef GTEST_INCLUDE_GTEST_INTERNAL_GTEST_PARAM_UTIL_H_
#define GTEST_INCLUDE_GTEST_INTERNAL_GTEST_PARAM_UTIL_H_

#include <ctype.h>
#include <iterator>
#include <set>
#include <utility>
#include <vector>
#include "../gtest-printers.h"
#include "gtest-internal.h"
#include "gtest-linked_ptr.h"
#include "gtest-port.h"

#if GTEST_HAS_PARAM_TEST
namespace testing {
    template <class ParamType> struct TestParamInfo {
        TestParamInfo(const ParamType& a_param, size_t an_index) : param(a_param), index(an_index) {}
        ParamType param;
        size_t index;
    };
    struct PrintToStringParamName {
        template <class ParamType> std::string operator()(const TestParamInfo<ParamType>& info) const {
            return PrintToString(info.param);
        }
    };
    namespace internal {
        GTEST_API_ void ReportInvalidTestCaseType(const char* test_case_name, CodeLocation code_location);
        template <typename> class ParamGeneratorInterface;
        template <typename> class ParamGenerator;
        template <typename T> class ParamIteratorInterface {
        public:
            virtual ~ParamIteratorInterface() {}
            virtual const ParamGeneratorInterface<T>* BaseGenerator() const;
            virtual void Advance();
            virtual ParamIteratorInterface* Clone() const;
            virtual const T* Current() const;
            virtual bool Equals(const ParamIteratorInterface& other) const;
        };
        template <typename T> class ParamIterator {
        public:
            typedef T value_type;
            typedef const T& reference;
            typedef ptrdiff_t difference_type;
            ParamIterator(const ParamIterator& other) : impl_(other.impl_->Clone()) {}
            ParamIterator& operator=(const ParamIterator& other) {
                if (this != &other) impl_.reset(other.impl_->Clone());
                return *this;
            }
            const T& operator*() const { return *impl_->Current(); }
            const T* operator->() const { return impl_->Current(); }
            ParamIterator& operator++() {
                impl_->Advance();
                return *this;
            }
            ParamIterator operator++(int) {
                ParamIteratorInterface<T>* clone = impl_->Clone();
                impl_->Advance();
                return ParamIterator(clone);
            }
            bool operator==(const ParamIterator& other) const {
                return impl_.get() == other.impl_.get() || impl_->Equals(*other.impl_);
            }
            bool operator!=(const ParamIterator& other) const {
                return !(*this == other);
            }
        private:
            friend class ParamGenerator<T>;
            explicit ParamIterator(ParamIteratorInterface<T>* impl) : impl_(impl) {}
            scoped_ptr<ParamIteratorInterface<T>> impl_;
        };
        template <typename T> class ParamGeneratorInterface {
        public:
            typedef T ParamType;
            virtual ~ParamGeneratorInterface() {}
            virtual ParamIteratorInterface<T>* Begin() const;
            virtual ParamIteratorInterface<T>* End() const;
        };
        template<typename T> class ParamGenerator {
        public:
            typedef ParamIterator<T> iterator;
            explicit ParamGenerator(ParamGeneratorInterface<T>* impl) : impl_(impl) {}
            ParamGenerator(const ParamGenerator& other) : impl_(other.impl_) {}
            ParamGenerator& operator=(const ParamGenerator& other) {
                impl_ = other.impl_;
                return *this;
            }
            iterator begin() const { return iterator(impl_->Begin()); }
            iterator end() const { return iterator(impl_->End()); }
        private:
            linked_ptr<const ParamGeneratorInterface<T> > impl_;
        };
        template <typename T, typename IncrementT> class RangeGenerator : public ParamGeneratorInterface<T> {
        public:
            RangeGenerator(T begin, T end, IncrementT step) : begin_(begin), end_(end), step_(step),
                           end_index_(CalculateEndIndex(begin, end, step)) {}
            virtual ~RangeGenerator() {}
            virtual ParamIteratorInterface<T>* Begin() const {
                return new Iterator(this, begin_, 0, step_);
            }
            virtual ParamIteratorInterface<T>* End() const {
                return new Iterator(this, end_, end_index_, step_);
            }
        private:
            class Iterator : public ParamIteratorInterface<T> {
            public:
                Iterator(const ParamGeneratorInterface<T>* base, T value, int index, IncrementT step) : base_(base), value_(value),
                         index_(index), step_(step) {}
                virtual ~Iterator() {}
                virtual const ParamGeneratorInterface<T>* BaseGenerator() const {
                    return base_;
                }
                virtual void Advance() {
                    value_ = static_cast<T>(value_ + step_);
                    index_++;
                }
                virtual ParamIteratorInterface<T>* Clone() const {
                    return new Iterator(*this);
                }
                virtual const T* Current() const { return &value_; }
                virtual bool Equals(const ParamIteratorInterface<T>& other) const {
                    GTEST_CHECK_(BaseGenerator() == other.BaseGenerator()) << "The program attempted to compare iterators "
                                 << "from different generators." << std::endl;
                    const int other_index = CheckedDowncastToActualType<const Iterator>(&other)->index_;
                    return index_ == other_index;
                }
            private:
                Iterator(const Iterator& other) : ParamIteratorInterface<T>(), base_(other.base_), value_(other.value_), index_(other.index_),
                         step_(other.step_) {}
                void operator=(const Iterator& other);
                const ParamGeneratorInterface<T>* const base_;
                T value_;
                int index_;
                const IncrementT step_;
            };
            static int CalculateEndIndex(const T& begin, const T& end, const IncrementT& step) {
                int end_index = 0;
                for (T i = begin; i < end; i = static_cast<T>(i + step)) end_index++;
                return end_index;
            }
            void operator=(const RangeGenerator& other);
            const T begin_;
            const T end_;
            const IncrementT step_;
            const int end_index_;
        };
        template <typename T> class ValuesInIteratorRangeGenerator : public ParamGeneratorInterface<T> {
        public:
            template <typename ForwardIterator> ValuesInIteratorRangeGenerator(ForwardIterator begin, ForwardIterator end) : container_(begin, end) {}
            virtual ~ValuesInIteratorRangeGenerator() {}
            virtual ParamIteratorInterface<T>* Begin() const {
                return new Iterator(this, container_.begin());
            }
            virtual ParamIteratorInterface<T>* End() const {
                return new Iterator(this, container_.end());
            }
        private:
            typedef typename ::std::vector<T> ContainerType;
            class Iterator : public ParamIteratorInterface<T> {
            public:
                Iterator(const ParamGeneratorInterface<T>* base, typename ContainerType::const_iterator iterator) : base_(base),
                         iterator_(iterator) {}
                virtual ~Iterator() {}
                virtual const ParamGeneratorInterface<T>* BaseGenerator() const {
                    return base_;
                }
                virtual void Advance() {
                    ++iterator_;
                    value_.reset();
                }
                virtual ParamIteratorInterface<T>* Clone() const {
                    return new Iterator(*this);
                }
                virtual const T* Current() const {
                    if (value_.get() == NULL) value_.reset(new T(*iterator_));
                    return value_.get();
                }
                virtual bool Equals(const ParamIteratorInterface<T>& other) const {
                    GTEST_CHECK_(BaseGenerator() == other.BaseGenerator()) << "The program attempted to compare iterators "
                                 << "from different generators." << std::endl;
                    return iterator_ == CheckedDowncastToActualType<const Iterator>(&other)->iterator_;
                }
            private:
                Iterator(const Iterator& other) : ParamIteratorInterface<T>(), base_(other.base_), iterator_(other.iterator_) {}
                const ParamGeneratorInterface<T>* const base_;
                typename ContainerType::const_iterator iterator_;
                mutable scoped_ptr<const T> value_;
            };
            void operator=(const ValuesInIteratorRangeGenerator& other);
            const ContainerType container_;
        };
        template <class ParamType> std::string DefaultParamName(const TestParamInfo<ParamType>& info) {
            Message name_stream;
            name_stream << info.index;
            return name_stream.GetString();
        }
        template <class ParamType, class ParamNameGenFunctor> ParamNameGenFunctor GetParamNameGen(ParamNameGenFunctor func) {
            return func;
        }
        template <class ParamType> struct ParamNameGenFunc {
            typedef std::string Type(const TestParamInfo<ParamType>&);
        };
        template <class ParamType> typename ParamNameGenFunc<ParamType>::Type *GetParamNameGen() {
            return DefaultParamName;
        }
        template <class TestClass> class ParameterizedTestFactory : public TestFactoryBase {
        public:
            typedef typename TestClass::ParamType ParamType;
            explicit ParameterizedTestFactory(ParamType parameter) : parameter_(parameter) {}
            virtual Test* CreateTest() {
                TestClass::SetParam(&parameter_);
                return new TestClass();
            }
        private:
            const ParamType parameter_;
            GTEST_DISALLOW_COPY_AND_ASSIGN_(ParameterizedTestFactory);
        };
        template <class ParamType> class TestMetaFactoryBase {
        public:
            virtual ~TestMetaFactoryBase() {}
            virtual TestFactoryBase* CreateTestFactory(ParamType parameter);
        };
        template <class TestCase> class TestMetaFactory : public TestMetaFactoryBase<typename TestCase::ParamType> {
        public:
            typedef typename TestCase::ParamType ParamType;
            TestMetaFactory() {}
            virtual TestFactoryBase* CreateTestFactory(ParamType parameter) {
                return new ParameterizedTestFactory<TestCase>(parameter);
            }
        private:
            GTEST_DISALLOW_COPY_AND_ASSIGN_(TestMetaFactory);
        };
        class ParameterizedTestCaseInfoBase {
        public:
            virtual ~ParameterizedTestCaseInfoBase() {}
            virtual const string& GetTestCaseName() const;
            virtual TypeId GetTestCaseTypeId() const;
            virtual void RegisterTests();
        protected:
            ParameterizedTestCaseInfoBase() {}
        private:
            GTEST_DISALLOW_COPY_AND_ASSIGN_(ParameterizedTestCaseInfoBase);
        };
        template <class TestCase> class ParameterizedTestCaseInfo : public ParameterizedTestCaseInfoBase {
        public:
            typedef typename TestCase::ParamType ParamType;
            typedef ParamGenerator<ParamType>(GeneratorCreationFunc)();
            typedef typename ParamNameGenFunc<ParamType>::Type ParamNameGeneratorFunc;
            explicit ParameterizedTestCaseInfo(const char* name, CodeLocation code_location) : test_case_name_(name),
                                               code_location_(code_location) {}
            virtual const string& GetTestCaseName() const { return test_case_name_; }
            virtual TypeId GetTestCaseTypeId() const { return GetTypeId<TestCase>(); }
            void AddTestPattern(const char* test_case_name, const char* test_base_name, TestMetaFactoryBase<ParamType>* meta_factory) {
                tests_.push_back(linked_ptr<TestInfo>(new TestInfo(test_case_name, test_base_name, meta_factory)));
            }
            int AddTestCaseInstantiation(const string& instantiation_name, GeneratorCreationFunc* func, ParamNameGeneratorFunc* name_func,
                                         const char* file, int line) {
                instantiations_.push_back(InstantiationInfo(instantiation_name, func, name_func, file, line));
                return 0;
            }
            virtual void RegisterTests() {
                for (typename TestInfoContainer::iterator test_it = tests_.begin(); test_it != tests_.end(); ++test_it) {
                    linked_ptr<TestInfo> test_info = *test_it;
                    for (typename InstantiationContainer::iterator gen_it = instantiations_.begin(); gen_it != instantiations_.end(); ++gen_it) {
                        const string& instantiation_name = gen_it->name;
                        ParamGenerator<ParamType> generator((*gen_it->generator)());
                        ParamNameGeneratorFunc* name_func = gen_it->name_func;
                        const char* file = gen_it->file;
                        int line = gen_it->line;
                        string test_case_name;
                        if (!instantiation_name.empty()) test_case_name = instantiation_name + "/";
                        test_case_name += test_info->test_case_base_name;
                        size_t i = 0;
                        std::set<std::string> test_param_names;
                        for (typename ParamGenerator<ParamType>::iterator param_it = generator.begin(); param_it != generator.end();
                             ++param_it, ++i) {
                            Message test_name_stream;
                            std::string param_name = name_func(TestParamInfo<ParamType>(*param_it, i));
                            GTEST_CHECK_(IsValidParamName(param_name)) << "Parameterized test name '" << param_name
                                         << "' is invalid, in " << file << " line " << line << std::endl;
                            GTEST_CHECK_(test_param_names.count(param_name) == 0) << "Duplicate parameterized test name '"
                                         << param_name << "', in " << file << " line " << line << std::endl;
                            test_param_names.insert(param_name);
                            test_name_stream << test_info->test_base_name << "/" << param_name;
                            MakeAndRegisterTestInfo(test_case_name.c_str(), test_name_stream.GetString().c_str(), NULL,
                                                    PrintToString(*param_it).c_str(), code_location_, GetTestCaseTypeId(),
                                                    TestCase::SetUpTestCase, TestCase::TearDownTestCase,
                                                    test_info->test_meta_factory->CreateTestFactory(*param_it));
                        }
                    }
                }
            }
        private:
            struct TestInfo {
                TestInfo(const char* a_test_case_base_name, const char* a_test_base_name, TestMetaFactoryBase<ParamType>* a_test_meta_factory) :
                         test_case_base_name(a_test_case_base_name), test_base_name(a_test_base_name), test_meta_factory(a_test_meta_factory) {}
                const string test_case_base_name;
                const string test_base_name;
                const scoped_ptr<TestMetaFactoryBase<ParamType>> test_meta_factory;
            };
            typedef ::std::vector<linked_ptr<TestInfo>> TestInfoContainer;
            struct InstantiationInfo {
                InstantiationInfo(const std::string &name_in, GeneratorCreationFunc* generator_in, ParamNameGeneratorFunc* name_func_in,
                                  const char* file_in, int line_in) : name(name_in), generator(generator_in), name_func(name_func_in),
                                  file(file_in), line(line_in) {}
                std::string name;
                GeneratorCreationFunc* generator;
                ParamNameGeneratorFunc* name_func;
                const char* file;
                int line;
            };
            typedef ::std::vector<InstantiationInfo> InstantiationContainer;
            static bool IsValidParamName(const std::string& name) {
                if (name.empty()) return false;
                for (std::string::size_type index = 0; index < name.size(); ++index) {
                    if (!isalnum(name[index]) && name[index] != '_') return false;
                }
                return true;
            }
            const string test_case_name_;
            CodeLocation code_location_;
            TestInfoContainer tests_;
            InstantiationContainer instantiations_;
            GTEST_DISALLOW_COPY_AND_ASSIGN_(ParameterizedTestCaseInfo);
        };
        class ParameterizedTestCaseRegistry {
        public:
            ParameterizedTestCaseRegistry() {}
            ~ParameterizedTestCaseRegistry() {
                for (TestCaseInfoContainer::iterator it = test_case_infos_.begin(); it != test_case_infos_.end(); ++it) {
                    delete *it;
                }
            }
            template <class TestCase> ParameterizedTestCaseInfo<TestCase>* GetTestCasePatternHolder(const char* test_case_name, CodeLocation code_location) {
                ParameterizedTestCaseInfo<TestCase>* typed_test_info = NULL;
                for (TestCaseInfoContainer::iterator it = test_case_infos_.begin(); it != test_case_infos_.end(); ++it) {
                    if ((*it)->GetTestCaseName() == test_case_name) {
                        if ((*it)->GetTestCaseTypeId() != GetTypeId<TestCase>()) {
                            ReportInvalidTestCaseType(test_case_name, code_location);
                            posix::Abort();
                        } else typed_test_info = CheckedDowncastToActualType<ParameterizedTestCaseInfo<TestCase>> (*it);
                        break;
                    }
                }
                if (typed_test_info == NULL) {
                    typed_test_info = new ParameterizedTestCaseInfo<TestCase>(test_case_name, code_location);
                    test_case_infos_.push_back(typed_test_info);
                }
                return typed_test_info;
            }
            void RegisterTests() {
                for (TestCaseInfoContainer::iterator it = test_case_infos_.begin(); it != test_case_infos_.end(); ++it) {
                    (*it)->RegisterTests();
                }
            }
        private:
            typedef ::std::vector<ParameterizedTestCaseInfoBase*> TestCaseInfoContainer;
            TestCaseInfoContainer test_case_infos_;
            GTEST_DISALLOW_COPY_AND_ASSIGN_(ParameterizedTestCaseRegistry);
        };
    }
}
#endif
#endif