/**
 * @file
 * @copyright This code is licensed under the 3-clause BSD license.\n
 *            Copyright ETH Zurich, Department of Chemistry and Applied Biosciences, Reiher Group.\n
 *            See LICENSE.txt for details.
 */

#include <Core/Log.h>
#include <gmock/gmock.h>
#include <functional>
#include <iomanip>

using namespace testing;
namespace Scine {
namespace Core {

std::ostream& writeInt(std::ostream& os, int f) {
  os << f;
  return os;
}

TEST(Log, Interface) {
  Log log = Log::silent();

  /* Iterator member order tests, these are mostly here to ensure nobody messes
   * with the member order without fixing the begin-end fns
   */
  ASSERT_THAT(std::begin(log), Eq(std::addressof(log.debug)));
  ASSERT_THAT(std::end(log), Eq(std::addressof(log.output) + 1));

  // A silent log should bool-cast to false
  ASSERT_THAT(static_cast<bool>(log.debug), Eq(false));

  // Domain interface checks
  log.debug << "Hi" << Log::nl;
  log.debug << std::setw(4) << "Check that iomanip works here" << Log::endl;
  log.debug << std::boolalpha << true << Log::flush;
  log.debug.line("Hello");
  log.debug.printf("%4d \n", 9);

  log.debug.add("sample", Log::coutSink());
  ASSERT_THAT(static_cast<bool>(log.debug), Eq(true));
  ASSERT_NO_THROW(log.debug.remove("sample"));

  log.debug.add("sample", Log::cerrSink());
  Log::SinkPtr sinkPtr;
  ASSERT_NO_THROW(sinkPtr = log.debug.extract("sample"));

  log.debug.add("sample", Log::fileSink("log.txt"));
  log.debug.clear();
  ASSERT_THAT(static_cast<bool>(log.debug), Eq(false));

  auto throws = []() -> int {
    throw std::runtime_error("I throw");
    return 0;
  };

  ASSERT_NO_THROW(log.debug.lazy(throws));

  // For-iteration of all domains
  for (Log::Domain& domain : log) {
    domain << "Hello there" << Log::nl;
  }

  // Actual output
  log.debug.add("cout", Log::coutSink());
  log.debug.line("Barbarenbarbier");
  log.debug << "Something something" << Log::nl << Log::endl << Log::flush;
  log.debug.lazy([]() { return "Hello\n"; });
  log.debug << std::bind(writeInt, std::placeholders::_1, 4) << Log::nl;
  log.debug << [](std::ostream& os) { writeInt(os, 4); } << Log::nl;

  ASSERT_THROW(log.debug.remove("nonexistent sink"), std::out_of_range);

  log.sinkAllToFile("log.txt");

  log.debug.printf("%s %d", "asdf", 0.4);
}

} // namespace Core
} // namespace Scine
