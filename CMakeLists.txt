set(source_files
    model/local-clock.cc
    model/unbounded-skew-clock.cc
    helper/local-clock-helper.cc
)

set(header_files
    model/local-clock.h
    model/unbounded-skew-clock.h
    helper/local-clock-helper.h
)

build_lib(
  LIBNAME local-clock
  SOURCE_FILES ${source_files}
  HEADER_FILES ${header_files}
  LIBRARIES_TO_LINK
    ${libcore}
    ${libnetwork}
  TEST_SOURCES
    test/test-unbounded-skew-clock.cc
)