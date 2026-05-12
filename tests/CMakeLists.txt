enable_testing()

find_package(GTest REQUIRED)

include(GoogleTest)


set(E2E_DIR    "${CMAKE_CURRENT_SOURCE_DIR}/e2e")
set(CASES_DIR  "${E2E_DIR}/io")
set(RUNNER     "${E2E_DIR}/run.sh")
 
# Find all case directories at configure time.
file(GLOB case_dirs RELATIVE "${CASES_DIR}" "${CASES_DIR}/*")
 
foreach(case_name IN LISTS case_dirs)
    set(case_path "${CASES_DIR}/${case_name}")
    if(IS_DIRECTORY "${case_path}")
        add_test(
            NAME e2e_${case_name}
            COMMAND bash "${RUNNER}" "$<TARGET_FILE:main>" "${case_name}"
        )
    endif()
endforeach()