# Keep symbols for JIT resolution
set(LLVM_NO_DEAD_STRIP 1)

include_directories(../../../cling/include)

if(BUILD_SHARED_LIBS)
  set(LIBS
    LLVMSupport
    clangFrontendTool
    clingInterpreter
    clingMetaProcessor
    clingUserInterface
    clingUtils
  )
  add_cintent_executable(cintent
    cintent.cpp
  )
else()
  set(LIBS
    LLVMSupport
    clangFrontendTool
    clingUserInterface
  )
  add_cintent_executable(cintent
    cintent.cpp
    $<TARGET_OBJECTS:obj.clingInterpreter>
    $<TARGET_OBJECTS:obj.clingMetaProcessor>
    $<TARGET_OBJECTS:obj.clingUtils>
  )
endif(BUILD_SHARED_LIBS)

set_target_properties(cintent
  PROPERTIES ENABLE_EXPORTS 1)

target_link_libraries(cintent
  ${LIBS}
  )

install(TARGETS cintent
  RUNTIME DESTINATION bin)
