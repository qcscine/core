Changelog
=========

Release 6.0.0
-------------

- Improve support for compilation on Windows (MSVC)
- Update address in license

Release 5.0.0
-------------

- Allow for direct loading of modules

Release 4.1.0
-------------

- Add new interface ``EmbeddingCalculator``. This class is derived from
  the calculator class and allows for embedding calculations with different
  underlying calculators.

Release 4.0.2
-------------

- Various bugfixes and improvements

Release 4.0.1
-------------

- Various bugfixes and improvements

Release 4.0.0
-------------

- Add new interface ``ObjectWithOrbitals``
- Fix bug affecting loading of modules when ``SCINE_MODULE_PATH`` contains
  multiple paths
- Refactor ``ModuleManager`` to private implementation

Release 3.0.1
-------------

- Update development-utils

Release 3.0.0
-------------

- Refactor logging
- Various bugfixes and improvements

Release 2.0.0
-------------

- Replace Boost HANA with Boost MPL. Note that this changes all the interfaces;
  Core 2.0.0 is therefore not backwards compatible.
- Various bugfixes and improvements

Release 1.0.0
-------------

Initial release with all necessary functionality to support Sparrow and ReaDuct.
