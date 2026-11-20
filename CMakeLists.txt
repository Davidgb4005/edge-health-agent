cmake_minimum_required(VERSION 3.16)
project(edge-health-agent VERSION 1.0.0 LANGUAGES C)

include(GNUInstallDirs)
include(CTest)

add_library(edge_health_metrics src/metrics.c)
target_include_directories(edge_health_metrics
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)
target_compile_features(edge_health_metrics PUBLIC c_std_11)

add_executable(edge-health-agent src/main.c)
target_link_libraries(edge-health-agent PRIVATE edge_health_metrics)
target_compile_definitions(edge-health-agent
    PRIVATE EDGE_HEALTH_VERSION="${PROJECT_VERSION}"
)

foreach(target edge_health_metrics edge-health-agent)
    target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Werror)
endforeach()

install(TARGETS edge-health-agent RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(FILES packaging/edge-health-agent.service
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/systemd/system)

if(BUILD_TESTING)
    add_executable(test_metrics tests/test_metrics.c)
    target_link_libraries(test_metrics PRIVATE edge_health_metrics)
    target_compile_options(test_metrics PRIVATE -Wall -Wextra -Wpedantic -Werror)
    target_compile_definitions(test_metrics
        PRIVATE FIXTURE_ROOT="${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures"
    )
    add_test(NAME metrics COMMAND test_metrics)
    add_test(NAME cli_once COMMAND edge-health-agent --once)
endif()
