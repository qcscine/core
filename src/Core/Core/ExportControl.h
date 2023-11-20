/**
 * @file ExportControl.h
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Department of Chemistry and Applied Biosciences, Reiher Group.\n
 *            See LICENSE.txt for details.
 */
#ifndef CORE_EXPORTCONTROL_H_
#define CORE_EXPORTCONTROL_H_

// This logic was borrowed (then namespaced) from the examples on the gcc wiki:
//     https://gcc.gnu.org/wiki/Visibility

#if defined _WIN32 || defined __CYGWIN__
#  ifdef __GNUC__
#    define CORE_EXPORT __attribute__((dllexport))
#  else
#    define CORE_EXPORT __declspec(dllexport)
#  endif
#else
#  define CORE_EXPORT __attribute__((visibility("default")))
#endif

#endif /* CORE_EXPORTCONTROL_H_ */
