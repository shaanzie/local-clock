set(source_files
    model/local-clock.cc
    model/lamport-clock.cc
    model/replay-clock.cc
    model/unbounded-skew-clock.cc
    model/vector-clock.cc
    helper/local-clock-helper.cc
)

set(header_files
    model/local-clock.h
    model/lamport-clock.h
    model/replay-clock.h
    model/unbounded-skew-clock.h
    model/vector-clock.h
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
    test/test-lamport-clock.cc
    test/test-replay-clock.cc
    test/test-unbounded-skew-clock.cc
    test/test-vector-clock.cc

)
