/*
  ==============================================================================

   This file is part of the JUCE library.
   Copyright (c) 2017 - ROLI Ltd.

   JUCE is an open source library subject to commercial or open-source
   licensing.

   The code included in this file is provided under the terms of the ISC license
   http://www.isc.org/downloads/software-support-policy/isc-license. Permission
   To use, copy, modify, and/or distribute this software for any purpose with or
   without fee is hereby granted provided that the above copyright notice and
   this permission notice appear in all copies.

   JUCE IS PROVIDED "AS IS" WITHOUT ANY WARRANTY, AND ALL WARRANTIES, WHETHER
   EXPRESSED OR IMPLIED, INCLUDING MERCHANTABILITY AND FITNESS FOR PURPOSE, ARE
   DISCLAIMED.

  ==============================================================================
*/


// This hack is a workaround for a bug (?) in Apple's 10.11 SDK headers
// which cause some configurations of Clang to throw out a spurious error..
#if JUCE_PROJUCER_LIVE_BUILD && (defined (__APPLE_CPP__) || defined(__APPLE_CC__))
 #include <CoreFoundation/CFAvailability.h>
 #undef CF_OPTIONS
 #define CF_OPTIONS(_type, _name) _type _name; enum

 // This is a workaround for the Xcode 9 version of NSUUID.h causing some errors
 // in the live-build engine.
 #define _Nullable
 #define _Nonnull

 // In later versions of libc++ these methods are defined in the functional header,
 // which we don't compile in the live-build engine, so we'll define them here
 #if defined (_LIBCPP_VERSION) && _LIBCPP_VERSION >= 7000
  #include <memory>

  namespace std { inline namespace __1 {
      template <class _BinaryPredicate, class _ForwardIterator1, class _ForwardIterator2>
      pair<_ForwardIterator1, _ForwardIterator1> _LIBCPP_CONSTEXPR_AFTER_CXX11
      __search(_ForwardIterator1 __first1, _ForwardIterator1 __last1,
               _ForwardIterator2 __first2, _ForwardIterator2 __last2, _BinaryPredicate __pred,
               forward_iterator_tag, forward_iterator_tag)
      {
          if (__first2 == __last2)
              return make_pair(__first1, __first1);  // Everything matches an empty sequence
          while (true)
          {
              // Find first element in sequence 1 that matchs *__first2, with a mininum of loop checks
              while (true)
              {
                  if (__first1 == __last1)  // return __last1 if no element matches *__first2
                      return make_pair(__last1, __last1);
                  if (__pred(*__first1, *__first2))
                      break;
                  ++__first1;
              }
              // *__first1 matches *__first2, now match elements after here
              _ForwardIterator1 __m1 = __first1;
              _ForwardIterator2 __m2 = __first2;
              while (true)
              {
                  if (++__m2 == __last2)  // If pattern exhausted, __first1 is the answer (works for 1 element pattern)
                      return make_pair(__first1, __m1);
                  if (++__m1 == __last1)  // Otherwise if source exhaused, pattern not found
                      return make_pair(__last1, __last1);
                  if (!__pred(*__m1, *__m2))  // if there is a mismatch, restart with a new __first1
                  {
                      ++__first1;
                      break;
                  }  // else there is a match, check next elements
              }
          }
      }

      template <class _BinaryPredicate, class _RandomAccessIterator1, class _RandomAccessIterator2>
      _LIBCPP_CONSTEXPR_AFTER_CXX11
      pair<_RandomAccessIterator1, _RandomAccessIterator1>
      __search(_RandomAccessIterator1 __first1, _RandomAccessIterator1 __last1,
               _RandomAccessIterator2 __first2, _RandomAccessIterator2 __last2, _BinaryPredicate __pred,
               random_access_iterator_tag, random_access_iterator_tag)
      {
          typedef typename iterator_traits<_RandomAccessIterator1>::difference_type _D1;
          typedef typename iterator_traits<_RandomAccessIterator2>::difference_type _D2;
          // Take advantage of knowing source and pattern lengths.  Stop short when source is smaller than pattern
          const _D2 __len2 = __last2 - __first2;
          if (__len2 == 0)
              return make_pair(__first1, __first1);
          const _D1 __len1 = __last1 - __first1;
          if (__len1 < __len2)
              return make_pair(__last1, __last1);
          const _RandomAccessIterator1 __s = __last1 - (__len2 - 1);  // Start of pattern match can't go beyond here

          while (true)
          {
              while (true)
              {
                  if (__first1 == __s)
                      return make_pair(__last1, __last1);
                  if (__pred(*__first1, *__first2))
                      break;
                  ++__first1;
              }

              _RandomAccessIterator1 __m1 = __first1;
              _RandomAccessIterator2 __m2 = __first2;
              while (true)
              {
                  if (++__m2 == __last2)
                      return make_pair(__first1, __first1 + __len2);
                  ++__m1;          // no need to check range on __m1 because __s guarantees we have enough source
                  if (!__pred(*__m1, *__m2))
                  {
                      ++__first1;
                      break;
                  }
              }
          }
      }
  } }
 #endif
#endif
