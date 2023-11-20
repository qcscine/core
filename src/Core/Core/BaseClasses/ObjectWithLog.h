/**
 * @file
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Department of Chemistry and Applied Biosciences, Reiher Group.\n
 *            See LICENSE.txt for details.
 */

#ifndef INCLUDE_CORE_OBJECT_WITH_LOG_H
#define INCLUDE_CORE_OBJECT_WITH_LOG_H

#include "Core/ExportControl.h"
#include "Core/Log.h"

namespace Scine {
namespace Core {

/**
 * @brief Base class for objects with a Log member
 *
 * Default log member prints output domain to stdout and the
 * warning and error domains to stderr.
 *
 * @note Privately implemented to avoid include pollution with Log interface if
 * unused.
 */
class CORE_EXPORT ObjectWithLog {
 public:
  //! Fetches a reference to the owned log instance
  Log& getLog() {
    return log_;
  }

  //! Fetches a const reference to the owned log instance
  const Log& getLog() const {
    return log_;
  }

  //! Sets the log instance
  void setLog(Log log) {
    log_ = std::move(log);
  }

 private:
  Log log_;
};

} // namespace Core
} // namespace Scine

#endif
