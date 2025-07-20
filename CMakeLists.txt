cmake_minimum_required(VERSION 3.28...3.30)

include("${CMAKE_CURRENT_SOURCE_DIR}/cmake/common/bootstrap.cmake" NO_POLICY_SCOPE)
project(playfame VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(ENABLE_FRONTEND_API "Use obs-frontend-api for UI functionality" ON)
option(ENABLE_QT          "Use Qt functionality"           ON)

include(compilerconfig)
include(defaults)
include(helpers)

add_library(${PROJECT_NAME} MODULE)
set_target_properties_plugin(${PROJECT_NAME})

# OBS core
find_package(libobs REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE OBS::libobs)

# Dependencies
target_include_directories(${PROJECT_NAME} PRIVATE
  ./dep/nlohmann-json
 # .deps/obs-studio-31.0.0/libobs
)

# frontend API
if(ENABLE_FRONTEND_API)
  find_package(obs-frontend-api REQUIRED)
  target_link_libraries(${PROJECT_NAME} PRIVATE OBS::obs-frontend-api)
endif()

# Qt
if(ENABLE_QT)
  find_package(Qt6 COMPONENTS Widgets REQUIRED)
  target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt6::Core
    Qt6::Widgets
  )
  target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<C_COMPILER_ID:Clang,AppleClang>:-Wno-quoted-include-in-framework-header -Wno-comma>
  )
  set_target_properties(${PROJECT_NAME} PROPERTIES
    AUTOMOC ON
    AUTOUIC ON
    AUTORCC ON
  )
endif()

# plugin sources
target_sources(${PROJECT_NAME} PRIVATE
  src/plugin-main.cpp
  src/plugin-dock.cpp
  src/obs-config-helper.cpp
  src/config-dialog.cpp
  src/plugin-main.h
  src/plugin-dock.h
  src/obs-config-helper.h
  src/config-dialog.h
  src/toast-helper.h
)


# Firebase SDK
set(FIREBASE_CPP_SDK_DIR ${CMAKE_CURRENT_SOURCE_DIR}/external/firebase_cpp_sdk)
target_include_directories(${PROJECT_NAME} PRIVATE
  ${FIREBASE_CPP_SDK_DIR}/include
)

# 2. Statically link the exact .a files
set(FIREBASE_STATIC_LIB_DIR ${FIREBASE_CPP_SDK_DIR}/libs/darwin/universal)
set(FIREBASE_STATIC_LIBS
  ${FIREBASE_STATIC_LIB_DIR}/libfirebase_auth.a
  ${FIREBASE_STATIC_LIB_DIR}/libfirebase_app.a
)

target_link_libraries(${PROJECT_NAME} PRIVATE
  ${FIREBASE_STATIC_LIBS}

  # macOS frameworks — each as a single string:
  "-framework CoreFoundation"
  "-framework Foundation"
  "-framework Security"
  "-framework SystemConfiguration"
  "-framework GSS"
  "-framework Kerberos"

  pthread
)

# For local testing, inject an Xcode-only Run Script phase to copy the built plugin into the obs app's plugin folder & codesign
if(CMAKE_GENERATOR STREQUAL "Xcode")
  add_custom_command(TARGET ${PROJECT_NAME}
    POST_BUILD

      COMMAND ${CMAKE_COMMAND} -E echo "→ Installing ${PROJECT_NAME}.plugin to OBS…"

      # remove old install
      COMMAND ${CMAKE_COMMAND} -E remove_directory
        "$ENV{HOME}/Library/Application Support/obs-studio/plugins/${PROJECT_NAME}.plugin"

      # make target dir
      COMMAND ${CMAKE_COMMAND} -E make_directory
        "$ENV{HOME}/Library/Application Support/obs-studio/plugins/${PROJECT_NAME}.plugin"

# copy the built .plugin bundle from the actual target dir
COMMAND ${CMAKE_COMMAND} -E copy_directory
        "$<TARGET_BUNDLE_DIR:${PROJECT_NAME}>"
        "$ENV{HOME}/Library/Application Support/obs-studio/plugins/${PROJECT_NAME}.plugin"

      # codesign
      COMMAND codesign --force --deep --sign -
        "$ENV{HOME}/Library/Application Support/obs-studio/plugins/${PROJECT_NAME}.plugin"

    COMMENT "Install & codesign OBS plugin"
  )
endif()
