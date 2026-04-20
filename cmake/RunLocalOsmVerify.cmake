if(NOT DEFINED WORK_DIR OR NOT DEFINED VERIFY_EXE OR NOT DEFINED SELF_TEST_EXE)
    message(FATAL_ERROR "WORK_DIR, VERIFY_EXE and SELF_TEST_EXE must be provided.")
endif()

set(OSM_FILE "${WORK_DIR}/maps/map.osm")
if(NOT EXISTS "${OSM_FILE}")
    message(STATUS "SKIP: map.osm missing")
    return()
endif()

set(ENV{MINIMAP_VECTOR_SOURCE} "local_osm")
set(ENV{MINIMAP_OSM_FILE} "maps/map.osm")

execute_process(
    COMMAND "${VERIFY_EXE}"
    WORKING_DIRECTORY "${WORK_DIR}"
    RESULT_VARIABLE VERIFY_RESULT
    OUTPUT_VARIABLE VERIFY_OUT
    ERROR_VARIABLE VERIFY_ERR
)

if(NOT VERIFY_RESULT EQUAL 0)
    message(FATAL_ERROR "Local OSM verify failed.\n${VERIFY_OUT}\n${VERIFY_ERR}")
endif()

execute_process(
    COMMAND "${SELF_TEST_EXE}" --self-test self_test_report_local_osm.csv
    WORKING_DIRECTORY "${WORK_DIR}"
    RESULT_VARIABLE SELF_TEST_RESULT
    OUTPUT_VARIABLE SELF_TEST_OUT
    ERROR_VARIABLE SELF_TEST_ERR
)

if(NOT SELF_TEST_RESULT EQUAL 0)
    message(FATAL_ERROR "Local OSM self-test failed.\n${SELF_TEST_OUT}\n${SELF_TEST_ERR}")
endif()

message(STATUS "${VERIFY_OUT}")
message(STATUS "${SELF_TEST_OUT}")
