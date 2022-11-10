llvm_map_components_to_libnames(llvm_libs bitwriter core ipo irreader instcombine instrumentation target linker analysis scalaropts support )
file (GLOB SOURCES
   *.cpp
)
add_executable(ela ${SOURCES})

target_link_libraries(ela ${SVF_LIB} ${LLVMCudd} ${llvm_libs})
target_link_libraries(ela ${Z3_DIR}/bin/libz3.a )
set_target_properties(ela PROPERTIES
                       RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})