/*
    File: hardErrors.h
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

#ifndef gc_hardErrors_H
#define gc_hardErrors_H

#include <boost/format.hpp>
#include <string>

#define BF boost::format

void dbg_hook(const char *errorString);

namespace core {
void errorFormatted(boost::format fmt);
void errorFormatted(const char *errorString);
    void errorFormatted(const std::string &msg);

class HardError  {
private:
    std::string _Message;

public:
  HardError(const char *sourceFile, const char *functionName, uint lineNumber, const boost::format &fmt);
  HardError(const char *sourceFile, const char *functionName, uint lineNumber, const std::string &msg);
  std::string message();
};
};

#define THROW_HARD_ERROR(fmt)                                      \
  {                                                                \
    dbg_hook((fmt).str().c_str());                                 \
    core::errorFormatted(fmt);                                     \
    throw(core::HardError(__FILE__, __FUNCTION__, __LINE__, fmt)); \
  }
#define HARD_UNREACHABLE() THROW_HARD_ERROR(BF("Unreachable"));
#define HARD_SUBCLASS_MUST_IMPLEMENT() THROW_HARD_ERROR(boost::format("Subclass must implement"));
#ifdef DEBUG_ASSERTS
#define GCTOOLS_ASSERT(x)                                       \
  {                                                             \
    if (!(x))                                                   \
      THROW_HARD_ERROR(BF("Failed gctools assertion %s") % #x); \
  };
#define GCTOOLS_ASSERTF(x, fmt) \
  {                             \
    if (!(x))                   \
      THROW_HARD_ERROR(fmt);    \
  };
#else
#define GCTOOLS_ASSERT(x)
#define GCTOOLS_ASSERTF(x, f)
#endif



extern void lisp_errorDereferencedNonPointer(core::T_O *objP);
extern void lisp_errorBadCast(class_id toType, class_id fromType, core::T_O *objP);
extern void lisp_errorBadCastFromT_O(class_id toType, core::T_O *objP);
extern void lisp_errorBadCastToFixnum(class_id fromType, core::T_O *objP);
extern void lisp_errorBadCastFromT_OToCons_O(core::T_O *objP);
extern void lisp_errorBadCastFromSymbol_O(class_id toType, core::Symbol_O *objP);
extern void lisp_errorUnexpectedType(class_id expectedTyp, class_id givenTyp, core::T_O *objP);
extern void lisp_errorUnexpectedNil(class_id expectedTyp);
extern void lisp_errorDereferencedNil();
extern void lisp_errorDereferencedUnbound();
extern void lisp_errorIllegalDereference(void *v);


template <typename To, typename From, typename ObjPtrType>
inline void __attribute__((noreturn)) lisp_errorCast(ObjPtrType objP) {
  class_id to_typ = reg::registered_class<To>::id;
  class_id from_typ = reg::registered_class<From>::id;
  lisp_errorBadCast(to_typ, from_typ, reinterpret_cast<core::T_O *>(objP));
  __builtin_unreachable();
}

#endif // gc_hardErrors_H
