build_lib_example(
  NAME unbounded-skew-clock-example
  SOURCE_FILES unbounded-skew-clock-example.cc
  LIBRARIES_TO_LINK ${liblocal-clock}
)

build_lib_example(
  NAME lamport-clock-example
  SOURCE_FILES lamport-clock-example.cc
  LIBRARIES_TO_LINK ${liblocal-clock}
)

build_lib_example(
  NAME vector-clock-example
  SOURCE_FILES vector-clock-example.cc
  LIBRARIES_TO_LINK ${liblocal-clock}
)

build_lib_example(
  NAME replay-clock-example
  SOURCE_FILES replay-clock-example.cc
  LIBRARIES_TO_LINK ${liblocal-clock}
)
