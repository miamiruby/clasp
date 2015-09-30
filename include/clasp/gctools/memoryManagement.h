/*
    File: memoryManagement.h
*/

/*
Copyright (c) 2014, Christian E. Schafmeister
 
CLASP is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public
License as published by the Free Software Foundation; either
version 2 of the License, or (at your option) any later version.
 
See directory 'clasp/licenses' for full details.
 
The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/
/* -^- */
#ifndef _clasp_memoryManagement_H
#define _clasp_memoryManagement_H






#include <clasp/gctools/config.h>

namespace core {
    class T_O;
    class Symbol_O;
    class Cons_O;
};


namespace gctools {
extern int _global_signalTrap;
extern bool _global_debuggerOnSIGABRT; // If this is false then SIGABRT is processed normally and it will lead to termination of the program. See core_exit!
void lisp_pollSignals();
};
#define SET_SIGNAL(s) { gctools::_global_signalTrap = s; }
#define POLL_SIGNALS() gctools::lisp_pollSignals();

#define ALWAYS_INLINE __attribute__((always_inline))
#define NOINLINE __attribute__((noinline))

//! Macro for attribute that causes symbols to be exposed
#define ATTR_WEAK __attribute__((weak))



#define BF boost::format


#define clasp_disable_interrupts()
#define clasp_enable_interrupts()

#define clasp_unlikely(x) __builtin_expect(!!(x), 0)
#define clasp_likely(x) __builtin_expect(!!(x), 1)
#define UNLIKELY(x) clasp_unlikely(x)
#define LIKELY(x) clasp_likely(x)

#define unlikely_if(x) if (UNLIKELY(x))



typedef std::size_t class_id;

#include <limits>
#include <typeinfo>
#include <boost/operators.hpp>

namespace reg {

    struct null_type {};
    class_id const unknown_class = (std::numeric_limits<class_id>::max)();
    class type_id
        : public boost::less_than_comparable<type_id> {
    public:
        type_id()
            : id(&typeid(null_type)) {}

        type_id(std::type_info const &id)
            : id(&id) {}

        bool operator!=(type_id const &other) const {
            return id != other.id;
        }

        bool operator==(type_id const &other) const {
            return id == other.id;
        }

        bool operator<(type_id const &other) const {
            return id->before(*other.id);
        }

        char const *name() const {
            return id->name();
        }

        std::type_info const *get_type_info() const { return this->id; };

    private:
        std::type_info const *id;
    };

    class_id allocate_class_id(type_id const &cls);
    template <class T>
    struct registered_class {
        static class_id const id;
    };
    template <class T>
    class_id const registered_class<T>::id = allocate_class_id(typeid(T));
    template <class T>
    struct registered_class<T const>
        : registered_class<T> {};
};



#include <clasp/gctools/hardErrors.h>

#define INTRUSIVE_POINTER_REFERENCE_COUNT_ACCESSORS(x)

#define USE_WEAK_POINTER

#ifdef USE_BOEHM
#include <gc/gc.h>
#include <gc/gc_allocator.h>
typedef void *LocationDependencyPtrT;
#endif // USE_BOEHM

#ifdef USE_MPS
extern "C" {
#include <clasp/mps/code/mps.h>
#include <clasp/mps/code/mpsavm.h>
};
typedef mps_ld_t LocationDependencyPtrT;
#endif

typedef int (*MainFunctionType)(int argc, char *argv[], bool &mpiEnabled, int &mpiRank, int &mpiSize);

#define GC_LOG(x)
#define GCPRIVATE public
#define GCPROTECTED public

#include <clasp/gctools/hardErrors.h>

namespace gctools {

template <typename T>
constexpr size_t depreciatedAlignmentT() { return alignof(T); };
template <typename T>
constexpr size_t depreciatedAlignUpT(size_t size) { return (size + depreciatedAlignmentT<T>() - 1) & ~(depreciatedAlignmentT<T>() - 1); };
};

// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
//
// Define what a Header_s is for each garbage collector
// as well as other GC specific stuff
//
// ----------------------------------------------------------------------
// ----------------------------------------------------------------------
namespace gctools {

extern void *_global_stack_marker;

/*! Specialize GcKindSelector so that it returns the appropriate GcKindEnum for OT */
template <class OT>
struct GCKind;
};

namespace gctools {

#ifdef USE_BOEHM
class Header_s;
#endif
#ifdef USE_MPS
class Header_s;
#endif

template <typename T>
struct GCHeader {
#ifdef USE_BOEHM
  typedef Header_s HeaderType;
#endif
#ifdef USE_MPS
#ifdef RUNNING_GC_BUILDER
  typedef Header_s HeaderType;
#else
  typedef Header_s HeaderType;
#endif
#endif
};

template <typename T>
struct GCAllocationPoint;
};

/*!
  Template struct:   DynamicCast

  Specialized in clasp_gc.cc

*/



#include <clasp/gctools/pointer_tagging.h>

#include <clasp/gctools/tagged_cast.h>

#ifdef USE_BOEHM
#include <clasp/gctools/boehmGarbageCollection.h>
#endif
#ifdef USE_MPS
#include <clasp/gctools/mpsGarbageCollection.h>
#endif



namespace gctools {
/*! Specialize GcKindSelector so that it returns the appropriate GcKindEnum for OT */
template <class OT>
struct GCKind {
#ifdef USE_MPS
#ifdef RUNNING_GC_BUILDER
  static GCKindEnum const Kind = KIND_null;
#else
// We need a default Kind when running the gc-builder.lsp static analyzer
// but we don't want a default Kind when compiling the mps version of the code
// to force compiler errors when the Kind for an object hasn't been declared
#endif // RUNNING_GC_BUILDER
#endif // USE_MPS
#ifdef USE_BOEHM
  #ifdef USE_CXX_DYNAMIC_CAST
  static GCKindEnum const Kind = KIND_null; // minimally define KIND_null
  #else
// We don't want a default Kind when compiling the boehm version of the code
// to force compiler errors when the Kind for an object hasn't been declared
// using clasp_gc.cc
  #endif // USE_CXX_DYNAMIC_CAST
#endif
};
};

namespace gctools {

template <class OT>
struct GCInfo {
  static constexpr bool Atomic = false;
  static bool const NeedsInitialization = true; // Currently, by default,  everything needs initialization
  static bool const NeedsFinalization = false;   // By default, nothing needs finalization
  static constexpr bool Moveable = true;
};
};

#include <clasp/gctools/smart_pointers.h>

namespace gctools {
template <typename T>
void *SmartPtrToBasePtr(smart_ptr<T> obj) {
  void *ptr;
  if (obj.objectp()) {
    ptr = reinterpret_cast<void *>(reinterpret_cast<char *>(obj.untag_object()) - sizeof(Header_s));
  } else {
    THROW_HARD_ERROR(BF("Bad pointer for SmartPtrToBasePtr"));
    //            ptr = reinterpret_cast<void*>(obj.px_ref());
  }
  return ptr;
}
};

#define DECLARE_onHeapScanGCRoots()
#define DECLARE_onStackScanGCRoots()

namespace gctools {

/*! Size of containers given the number of elements */
template <typename Cont_impl>
size_t sizeof_container(size_t n) {
  size_t headerSz = sizeof(Cont_impl);
  size_t dataSz = sizeof(typename Cont_impl::value_type) * n;
  size_t totalSz = headerSz + dataSz;
  GC_LOG(("headerSz[%lu] + ( value_size[%lu] * n[%lu] -> dataSz[%lu] ) --> totalSz[%lu]\n",
          headerSz, sizeof(typename Cont_impl::value_type), n, dataSz, totalSz));
  return AlignUp(totalSz);
};

template <class T>
inline size_t sizeof_container_with_header(size_t num) {
  return sizeof_container<T>(num) + sizeof(Header_s);
};
};

namespace gctools {
  class GCStack;
  GCStack* threadLocalStack();
};

#define LCC_MACROS
#include <clasp/gctools/lispCallingConvention.h>
#undef LCC_MACROS

#include <clasp/gctools/gcStack.h>
#include <clasp/gctools/gcalloc.h>

#define GC_ALLOCATE(_class_, _obj_) gctools::smart_ptr<_class_> _obj_ = gctools::GCObjectAllocator<_class_>::allocate()
#define GC_ALLOCATE_VARIADIC(_class_, _obj_, ...) gctools::smart_ptr<_class_> _obj_ = gctools::GCObjectAllocator<_class_>::allocate(__VA_ARGS__)

#define GC_ALLOCATE_UNCOLLECTABLE(_class_, _obj_) gctools::smart_ptr<_class_> _obj_ = gctools::GCObjectAllocator<_class_>::rootAllocate()

#define GC_COPY(_class_, _obj_, _orig_) gctools::smart_ptr<_class_> _obj_ = gctools::GCObjectAllocator<_class_>::copy(_orig_)

/*! These don't do anything at the moment
  but may be used in the future to create unsafe-gc points
*/

#define SUPPRESS_GC() \
  {}
#define ENABLE_GC() \
  {}

namespace gctools {

int handleFatalCondition();

/* Start up the garbage collector and the main function.
       The main function is wrapped within this function */
int startupGarbageCollectorAndSystem(MainFunctionType startupFn, int argc, char *argv[], bool mpiEnabled, int mpiRank, int mpiSize);
};

#include <clasp/gctools/containers.h>

#include <clasp/gctools/multiple_value_pointers.h>

#include <clasp/gctools/multipleValues.h>


namespace core {
    typedef gctools::multiple_values<core::T_O> T_mv;
};





#endif // _clasp_memoryManagement_H
