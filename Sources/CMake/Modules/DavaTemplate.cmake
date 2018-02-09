#
#set( APP_DATA       )

#set( IOS_PLISTT     )
#set( IOS_XIB        )
#
#set( MACOS_PLIST    )
#set( MACOS_ICO      )
#set( MACOS_DYLIB    )
#set( MACOS_XIB      )
#
#set( WIN32_RESOURCES )
#
#set( ANDROID_USE_STANDART_TEMLATE )
#set( ANDROID_PACKAGE              )
#set( ANDROID_APP_NAME             )
#set( ANDROID_ACTIVITY_APP_NAME    )
#set( ANDROID_JAVA_SRC             )
#set( ANDROID_JAVA_LIBS            )
#set( ANDROID_JAVA_RES             )
#set( ANDROID_JAVA_ASSET           )
#set( ANDROID_ICO                  )
#
#set( ADDED_SRC                  )
#set( LIBRARIES                  )
#set( LIBRARIES_RELEASE          )
#set( LIBRARIES_DEBUG            )
#set( ADDED_BINARY_DIR           )
#set( EXECUTABLE_FLAG            )
#set( DEFINITIONS                )
#set( DEFINITIONS_${DAVA_PLATFORM_CURRENT} )
#set( EXTERNAL_TEST_FOLDERS      )
#

# Only interpret ``if()`` arguments as variables or keywords when unquoted.
#if(NOT (CMAKE_VERSION VERSION_LESS 3.1))
    cmake_policy(SET CMP0054 NEW)
#endif()

# You should call this macros before setup_main_executable to add external unittests into executable target
macro( exctact_external_unittests )
    load_property(PROPERTY_LIST EXTERNAL_TEST_FOLDERS)
    foreach( TEST_FOLDER ${EXTERNAL_TEST_FOLDERS} )
    file( GLOB_RECURSE TEST_FILES "${TEST_FOLDER}/*.unittest.cpp"  )

    if( ANDROID OR LINUX )#Fucking android
        foreach( FILE ${TEST_FILES} )
            get_filename_component( FILE_NAME ${FILE} NAME )
            set( OUT_FILE ${CMAKE_CURRENT_BINARY_DIR}/EXTERNAL_TEST/${FILE_NAME}.cpp )
            configure_file( ${FILE}  ${OUT_FILE} COPYONLY)
            list( APPEND ADDED_SRC ${OUT_FILE} )
        endforeach()
    else()
        list( APPEND ADDED_SRC ${TEST_FILES} )
        source_group( "EXTERNAL_TEST" FILES ${TEST_FILES} )

        set_source_files_properties(${TEST_FILES} PROPERTIES
          HEADER_FILE_ONLY FALSE
          KEEP_EXTENSION TRUE
          LANGUAGE CXX
        )
    endif()
endforeach()
endmacro()

macro( generate_modules_initialization )

    # generate cpp for modules initialization
    get_property( MODULE_MANAGER GLOBAL PROPERTY MODULE_MANAGER )
    if( MODULE_MANAGER )
        get_property( DAVA_LOADED_INITIALIZATION_MODULES GLOBAL PROPERTY DAVA_LOADED_INITIALIZATION_MODULES )
        set_property(GLOBAL PROPERTY DAVA_LOADED_INITIALIZATION_MODULES  )
        set_property(GLOBAL PROPERTY MODULE_MANAGER  )

        if( DAVA_LOADED_INITIALIZATION_MODULES )
            message(STATUS "--\n Target [ ${PROJECT_NAME} ] requested modules are: ${DAVA_LOADED_INITIALIZATION_MODULES} \n")
        endif()

        set( MODULE_GEN_CPP ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/modules_generated.cpp)
        list( APPEND PROJECT_SOURCE_FILES ${MODULE_GEN_CPP} )

        execute_process( COMMAND python ${DAVA_SCRIPTS_FILES_PATH}/generate_modules_init.py -r ${DAVA_ROOT_DIR} -o ${MODULE_GEN_CPP} ${DAVA_LOADED_INITIALIZATION_MODULES}
                         RESULT_VARIABLE GENERATION_ERROR)

        if( GENERATION_ERROR )
            message( FATAL_ERROR "Cannot find all modules")
        endif()
    endif()

endmacro()

macro( setup_main_executable )

include      ( PlatformSettings )

save_property( PROPERTY_LIST
               DEFINITIONS
               DEFINITIONS_${DAVA_PLATFORM_CURRENT}
             )

load_property( PROPERTY_LIST
        DEFINITIONS
        DEFINITIONS_${DAVA_PLATFORM_CURRENT}
        PLATFORM_DEFINITIONS_${DAVA_PLATFORM_CURRENT}
        CPP_FILES_EXECUTE
        GLOBAL_DEFINITIONS
        TARGET_MODULES_LIST
        BINARY_WIN32_DIR_RELEASE
        BINARY_WIN32_DIR_DEBUG
        BINARY_WIN32_DIR_RELWITHDEB
        BINARY_WIN64_DIR_RELEASE
        BINARY_WIN64_DIR_DEBUG
        BINARY_WIN64_DIR_RELWITHDEB
        STATIC_LIBRARIES_${DAVA_PLATFORM_CURRENT}
        STATIC_LIBRARIES_${DAVA_PLATFORM_CURRENT}_RELEASE
        STATIC_LIBRARIES_${DAVA_PLATFORM_CURRENT}_DEBUG
        DEPLOY_TO_BIN
        DEPLOY_TO_BIN_${DAVA_PLATFORM_CURRENT}
        DYNAMIC_LIBRARIES_${DAVA_PLATFORM_CURRENT}
        DYNAMIC_LIBRARIES_${DAVA_PLATFORM_CURRENT}_RELEASE
        DYNAMIC_LIBRARIES_${DAVA_PLATFORM_CURRENT}_DEBUG
        MODULE_DYNAMIC_LIBRARIES_DIR
        MODULE_DYNAMIC_LIBRARIES_DIR_RELEASE
        MODULE_DYNAMIC_LIBRARIES_DIR_DEBUG
        INCLUDES
        INCLUDES_${DAVA_PLATFORM_CURRENT}
        JAR_FOLDERS_ANDROID
        JAVA_FOLDERS_ANDROID
        PLUGIN_LIST
    )

list( APPEND DEFINITIONS ${GLOBAL_DEFINITIONS} ${PLATFORM_DEFINITIONS_${DAVA_PLATFORM_CURRENT}})

foreach( ITEM ${PLUGIN_LIST} ${TARGET_MODULES_LIST} )
    load_property( PROPERTY_LIST EXECUTE_DEFINITIONS_${ITEM} )
    list( APPEND DEFINITIONS ${EXECUTE_DEFINITIONS_${ITEM}} )
endforeach()

if( INCLUDES )
    include_directories( ${INCLUDES})
endif()

if( INCLUDES_${DAVA_PLATFORM_CURRENT} )
    include_directories( ${INCLUDES_${DAVA_PLATFORM_CURRENT}} )
endif()

if( DEFINITIONS )
   add_definitions( ${DEFINITIONS} )
endif()

if( DEFINITIONS_${DAVA_PLATFORM_CURRENT} )
    add_definitions( ${DEFINITIONS_${DAVA_PLATFORM_CURRENT}} )
endif()

add_definitions( -DDAVA_ENGINE_EXPORTS )

if( WIN32 )
    add_definitions ( -D_CRT_SECURE_NO_DEPRECATE )
endif()

if( DAVA_DISABLE_AUTOTESTS )
    add_definitions ( -DDISABLE_AUTOTESTS )
endif()

if( WIN32 )
    GET_PROPERTY(DAVA_ADDITIONAL_DYNAMIC_LIBRARIES_WIN GLOBAL PROPERTY DAVA_ADDITIONAL_DYNAMIC_LIBRARIES_WIN)
    list ( APPEND ADDITIONAL_DLL_FILES ${DAVA_ADDITIONAL_DYNAMIC_LIBRARIES_WIN} )
    list ( APPEND ADDITIONAL_DLL_FILES ${DYNAMIC_LIBRARIES_${DAVA_PLATFORM_CURRENT}} ${DYNAMIC_LIBRARIES_${DAVA_PLATFORM_CURRENT}_RELEASE} )
elseif( LINUX )
    GET_PROPERTY(DAVA_ADDITIONAL_DYNAMIC_LIBRARIES_LINUX GLOBAL PROPERTY DAVA_ADDITIONAL_DYNAMIC_LIBRARIES_LINUX)
    list ( APPEND ADDITIONAL_DLL_FILES ${DAVA_ADDITIONAL_DYNAMIC_LIBRARIES_LINUX} )
    list ( APPEND ADDITIONAL_DLL_FILES ${DYNAMIC_LIBRARIES_${DAVA_PLATFORM_CURRENT}} ${DYNAMIC_LIBRARIES_${DAVA_PLATFORM_CURRENT}_RELEASE} )
elseif( MACOS )
    GET_PROPERTY(DAVA_ADDITIONAL_DYNAMIC_LIBRARIES_MAC GLOBAL PROPERTY DAVA_ADDITIONAL_DYNAMIC_LIBRARIES_MAC)
    list ( APPEND MACOS_DYLIB  ${DAVA_ADDITIONAL_DYNAMIC_LIBRARIES_MAC} )
    list ( APPEND MACOS_DYLIB  ${DYNAMIC_LIBRARIES_${DAVA_PLATFORM_CURRENT}} )
endif()

if( STEAM_SDK_FOUND )
    add_definitions ( -D__DAVAENGINE_STEAM__ )
    include_directories( ${STEAM_SDK_HEADERS} )
    list ( APPEND LIBRARIES ${STEAM_SDK_STATIC_LIBRARIES} )

    if ( WIN32 )
        list ( APPEND ADDITIONAL_DLL_FILES ${STEAM_SDK_DYNAMIC_LIBRARIES} )
        list ( APPEND DAVA_BINARY_WIN_DIR ${STEAM_SDK_DYNAMIC_LIBRARIES_PATH} )
    endif ()

    if ( MACOS )
       list ( APPEND MACOS_DYLIB  ${STEAM_SDK_DYNAMIC_LIBRARIES} )
    endif ()

    ASSERT( STEAM_APPID "Please set the correct path to steam_appid.txt in value STEAM_APPID" )

    configure_file( ${STEAM_APPID}
                    ${CMAKE_CURRENT_BINARY_DIR}/steam_appid.txt  )


endif ()

generate_modules_initialization()


if( DAVA_TOOLS_FOUND )
    include_directories( ${DAVA_TOOLS_DIR} )
    list ( APPEND LIBRARIES ${DAVA_TOOLS_LIBRARY} )

endif()

if( DAVA_FOUND )
    include_directories   ( ${DAVA_INCLUDE_DIR} )
    include_directories   ( ${DAVA_ENGINE_DIR} )
    include_directories   ( ${DAVA_THIRD_PARTY_INCLUDES_PATH} )

    list( APPEND ANDROID_JAVA_LIBS  ${DAVA_THIRD_PARTY_ROOT_PATH}/lib_CMake/android/jar )
    list( APPEND ANDROID_JAVA_SRC   ${DAVA_ENGINE_DIR}/Platform/TemplateAndroid/Java )
    list( APPEND ANDROID_JAVA_SRC   ${DAVA_ENGINE_DIR}/Engine/Private/Android/Java )
    list( APPEND ANDROID_JAVA_SRC   ${DAVA_ENGINE_DIR}/Notification/Private/Android/Java )

endif()

###

if( MIX_APP_DATA )
    append_property( MIX_APP_DATA "${MIX_APP_DATA}" )
endif()

if( POSTPONED_MIX_DATA )
    processing_mix_data( NOT_DATA_COPY )
else()
    processing_mix_data()
endif()


###

if( IOS )
    list( APPEND RESOURCES_LIST ${APP_DATA} )
    list( APPEND RESOURCES_LIST ${IOS_XIB} )
    list( APPEND RESOURCES_LIST ${IOS_PLIST} )
    list( APPEND RESOURCES_LIST ${IOS_ICO} )

elseif( MACOS )
    set_source_files_properties( ${DYLIB_FILES} PROPERTIES MACOSX_PACKAGE_LOCATION Resources )

    list ( APPEND DYLIB_FILES     "${DYLIB_FILES}" "${MACOS_DYLIB}" )

    list( APPEND RESOURCES_LIST  ${APP_DATA}  )
    list( APPEND RESOURCES_LIST  ${MACOS_XIB} )
    list( APPEND RESOURCES_LIST  ${MACOS_ICO} )

    if( NOT DEPLOY_DIR_LIBS OR NOT DEPLOY )
        list( APPEND RESOURCES_LIST  ${DYLIB_FILES} )
    endif()

    list( APPEND LIBRARIES      ${DYLIB_FILES} )

elseif ( WINDOWS_UAP )

    if ( NOT "${CMAKE_SYSTEM_NAME}" STREQUAL "WindowsStore" )
        message(FATAL_ERROR "This app supports Store / Phone only. Please check the target platform.")
    endif ()

    set ( WINDOWS_UAP_CONFIG_DIR "${DAVA_ROOT_DIR}/Sources/CMake/ConfigureFiles" )

    #loading config file
    if ( NOT WINDOWS_UAP_CONFIG_FILE )
        set ( WINDOWS_UAP_CONFIG_FILE "${WINDOWS_UAP_CONFIG_DIR}/UWPConfigTemplate.in" )
    endif ()
    configure_file( ${WINDOWS_UAP_CONFIG_FILE} ${CMAKE_CURRENT_BINARY_DIR}/UWPConfig.in )
    load_config ( ${CMAKE_CURRENT_BINARY_DIR}/UWPConfig.in )

    #package languages settings
    #TODO: remove default value setting for this variable after supporting of this feature by client
    if ( NOT WINDOWS_UAP_PACKAGE_LANGUAGES )
        set ( WINDOWS_UAP_PACKAGE_LANGUAGES "x-generate" )
    else ()
        string ( REPLACE "|" ";" WINDOWS_UAP_PACKAGE_LANGUAGES ${WINDOWS_UAP_PACKAGE_LANGUAGES} )
    endif ()

    #build xml tags list
    set ( WINDOWS_UAP_PACKAGE_LANGUAGES_XML_TAGS "\n" )
    foreach ( LANGUAGE ${WINDOWS_UAP_PACKAGE_LANGUAGES} )
        set ( WINDOWS_UAP_PACKAGE_LANGUAGES_XML_TAGS
              "${WINDOWS_UAP_PACKAGE_LANGUAGES_XML_TAGS}  <qualifier name=\"Language\" value=\"${LANGUAGE}\" />\n" )
    endforeach ()

    set ( APP_MANIFEST_NAME "Package.appxmanifest" )
    set ( APP_CERT_NAME "${PROJECT_NAME}_Key.pfx" )
    set ( SHORT_NAME ${PROJECT_NAME} )
    set_property ( GLOBAL PROPERTY USE_FOLDERS ON )

    #search assets
    file( GLOB ASSET_FILES "${WINDOWS_UAP_ASSETS_DIR}/*.png" )
    source_group ( "Content\\Assets" FILES ${ASSET_FILES} )

    #copy manifest
    configure_file ( ${WINDOWS_UAP_MANIFEST_FILE} ${CMAKE_CURRENT_BINARY_DIR}/${APP_MANIFEST_NAME} @ONLY )

    #copy key file
    get_filename_component ( CERT_NAME ${WINDOWS_UAP_CERTIFICATE_FILE} NAME )
    file ( COPY ${WINDOWS_UAP_CERTIFICATE_FILE} DESTINATION ${CMAKE_CURRENT_BINARY_DIR} )
    file ( RENAME ${CMAKE_CURRENT_BINARY_DIR}/${CERT_NAME} ${CMAKE_CURRENT_BINARY_DIR}/${APP_CERT_NAME} )

    #copy priconfig files
    configure_file ( "${WINDOWS_UAP_CONFIG_DIR}/UWPPriConfigDefault.xml" "${CMAKE_CURRENT_BINARY_DIR}/UWPPriConfigDefault.xml" )
    file ( COPY "${WINDOWS_UAP_CONFIG_DIR}/UWPPriConfigPackaging.xml" DESTINATION ${CMAKE_CURRENT_BINARY_DIR} )

    set(CONTENT_FILES ${CONTENT_FILES}
        ${CMAKE_CURRENT_BINARY_DIR}/${APP_MANIFEST_NAME} )

    set(RESOURCE_FILES ${CONTENT_FILES} ${DEBUG_CONTENT_FILES} ${RELEASE_CONTENT_FILES}
        ${ASSET_FILES} ${STRING_FILES} ${CMAKE_CURRENT_BINARY_DIR}/${APP_CERT_NAME} )
    list( APPEND RESOURCES_LIST ${RESOURCE_FILES} )

    #add dll's to project and package
    add_dynamic_libs_win_uap ( ${DAVA_WIN_UAP_LIBRARIES_PATH_COMMON} DAVA_DLL_LIST )


    list( APPEND DAVA_DLL_LIST_RELEASE  ${DYNAMIC_LIBRARIES_${DAVA_PLATFORM_CURRENT}_RELEASE} )
    list( APPEND DAVA_DLL_LIST_DEBUG    ${DYNAMIC_LIBRARIES_${DAVA_PLATFORM_CURRENT}_DEBUG} )

    #add found dll's to project and mark them as deployment content
    if ( DAVA_DLL_LIST_DEBUG )
        set ( NEED_DLL_FIX true )
        source_group ("Binaries\\Debug"   FILES ${DAVA_DLL_LIST_DEBUG})
        set_property(SOURCE ${DAVA_DLL_LIST_DEBUG} PROPERTY VS_DEPLOYMENT_CONTENT $<CONFIG:Debug>)
    endif ()

    if ( DAVA_DLL_LIST_RELEASE )
        set ( NEED_DLL_FIX true )
        source_group ("Binaries\\Release" FILES ${DAVA_DLL_LIST_RELEASE})
        set_property(SOURCE ${DAVA_DLL_LIST_RELEASE} PROPERTY
            VS_DEPLOYMENT_CONTENT $<OR:$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>,$<CONFIG:MinSizeRel>>)
    endif ()

    list( APPEND ADDED_SRC "${DAVA_DLL_LIST_DEBUG}"
                           "${DAVA_DLL_LIST_RELEASE}" )

	set_property(SOURCE ${CONTENT_FILES} PROPERTY VS_DEPLOYMENT_CONTENT 1)
	set_property(SOURCE ${ASSET_FILES} PROPERTY VS_DEPLOYMENT_CONTENT 1)
	set_property(SOURCE ${ASSET_FILES} PROPERTY VS_DEPLOYMENT_LOCATION "Assets")
	set_property(SOURCE ${STRING_FILES} PROPERTY VS_TOOL_OVERRIDE "PRIResource")
	set_property(SOURCE ${DEBUG_CONTENT_FILES} PROPERTY VS_DEPLOYMENT_CONTENT $<CONFIG:Debug>)
	set_property(SOURCE ${RELEASE_CONTENT_FILES} PROPERTY
		VS_DEPLOYMENT_CONTENT $<OR:$<CONFIG:Release>,$<CONFIG:RelWithDebInfo>,$<CONFIG:MinSizeRel>>)

	set ( UAP_DEPLOYMENT_CONTENT ${APP_DATA} )
	list ( APPEND UAP_DEPLOYMENT_CONTENT ${ADDITIONAL_CONTENT} )
	add_content_win_uap ( "${UAP_DEPLOYMENT_CONTENT}" )
	list( APPEND ADDED_SRC ${ADDED_CONTENT_SRC} )

elseif( WIN32 )
    list( APPEND RESOURCES_LIST  ${WIN32_RESOURCES} )

elseif( ANDROID )

    # to avoid of unwind's symbol overriding by .so, need to link libunwind first before any .so
    # this should be done only for ARM
    if(ANDROID_ABI MATCHES "^armeabi")
        list( APPEND LIBRARIES "-lunwind" )
    endif()

    foreach( SOLIB ${DYNAMIC_LIBRARIES_${DAVA_PLATFORM_CURRENT}} )
        get_filename_component(SONAME ${SOLIB} NAME)
        add_library(${SONAME} SHARED IMPORTED)
        set_property(TARGET ${SONAME} PROPERTY IMPORTED_LOCATION ${SOLIB})
        list( APPEND LIBRARIES ${SONAME} )
    endforeach( SOLIB )
endif()
###

if( DAVA_FOUND )
    if( ANDROID )
        include_directories   ( ${DAVA_ENGINE_DIR}/Platform/TemplateAndroid )
        list( APPEND PATTERNS_CPP    ${DAVA_ENGINE_DIR}/Platform/TemplateAndroid/*.cpp )
        list( APPEND PATTERNS_H      ${DAVA_ENGINE_DIR}/Platform/TemplateAndroid/*.h   )

        list( APPEND PATTERNS_CPP    ${ANDROID_NDK}/sources/android/cpufeatures/*.c )
        list( APPEND PATTERNS_H      ${ANDROID_NDK}/sources/android/cpufeatures/*.h )
    endif()

    file( GLOB_RECURSE CPP_FILES ${PATTERNS_CPP} )
    file( GLOB_RECURSE H_FILES   ${PATTERNS_H} )
    set ( PLATFORM_ADDED_SRC ${H_FILES} ${CPP_FILES} )
endif()

###

if( CPP_FILES_EXECUTE )
    source_group( "INIT_MODULE" FILES ${CPP_FILES_EXECUTE} )
endif()

list( APPEND PROJECT_SOURCE_FILES ${ADDED_SRC} ${PLATFORM_ADDED_SRC} ${CPP_FILES_EXECUTE} )
generated_unity_sources( PROJECT_SOURCE_FILES   IGNORE_LIST ${UNIFIED_IGNORE_LIST}
                                                IGNORE_LIST_WIN32 ${UNIFIED_IGNORE_LIST_WIN32}
                                                IGNORE_LIST_APPLE ${UNIFIED_IGNORE_LIST_APPLE}
                                                IGNORE_LIST_MACOS ${UNIFIED_IGNORE_LIST_MACOS} ${MACOS_PLATFORM_SRC}
                                                IGNORE_LIST_IOS   ${UNIFIED_IGNORE_LIST_IOS}
                                                CUSTOM_PACK_1     ${UNIFIED_CUSTOM_PACK_1}
                                                CUSTOM_PACK_2     ${UNIFIED_CUSTOM_PACK_2}
                                                CUSTOM_PACK_3     ${UNIFIED_CUSTOM_PACK_3}
                                                CUSTOM_PACK_4     ${UNIFIED_CUSTOM_PACK_4}
                                                CUSTOM_PACK_5     ${UNIFIED_CUSTOM_PACK_5}
                                                CUSTOM_PACK_6     ${UNIFIED_CUSTOM_PACK_6}
                                                CUSTOM_PACK_7     ${UNIFIED_CUSTOM_PACK_7}
                                                UNITYIGNORE_FILES ${PROJECT_UNITYIGNORE_FILES}
                                               )

###
if( ANDROID )
    set( POSTFIX 0  )
    set( COUNTER 0 )
    set( SRC_LIST  )
    set( REMAINING_LIST  )

    foreach( ITEM ${PROJECT_SOURCE_FILES} )
        get_filename_component( ITEM_EXT ${ITEM} EXT )

        if( ${ITEM_EXT} STREQUAL ".cpp" )
            list( APPEND SRC_LIST  ${ITEM} )
            math( EXPR COUNTER "${COUNTER} + 1" )

            if( ${COUNTER} GREATER ${DAVA_ANDROID_MAX_LIB_SRC} )
                math( EXPR POSTFIX "${POSTFIX} + 1" )

                set( LIB_NAME "${PROJECT_NAME}_${POSTFIX}"  )
                add_library( ${LIB_NAME} STATIC ${SRC_LIST} )
                list( APPEND TARGET_LIBRARIES ${LIB_NAME} )

                set( COUNTER 0 )
                set( SRC_LIST )

            endif()

        else()
            list( APPEND REMAINING_LIST  ${ITEM} )

        endif()

    endforeach()

    if( ${COUNTER} GREATER 0 )
        math( EXPR POSTFIX "${POSTFIX} + 1" )

        set( LIB_NAME "${PROJECT_NAME}_${POSTFIX}"  )
        add_library( ${LIB_NAME} STATIC ${SRC_LIST} )
        list( APPEND TARGET_LIBRARIES ${LIB_NAME} )

        set( COUNTER 0 )
        set( SRC_LIST )

    endif()

    add_library( ${PROJECT_NAME} SHARED ${PLATFORM_ADDED_SRC} ${REMAINING_LIST} )

else()
    if( NOT MAC_DISABLE_BUNDLE )
        set( BUNDLE_FLAG  MACOSX_BUNDLE )
    endif()

    add_executable( ${PROJECT_NAME} ${BUNDLE_FLAG} ${EXECUTABLE_FLAG}
        ${PROJECT_SOURCE_FILES}
        ${PROJECT_HEADER_FILE_ONLY}
        ${RESOURCES_LIST}
    )

   append_property( ALL_TARGET_PROJECTS_LIST ${PROJECT_NAME} )

endif()

if ( STEAM_SDK_FOUND AND WIN32 )
    if(DEPLOY)
        set( STEAM_APPID_DIR ${DEPLOY_DIR} )
    else()
        set( STEAM_APPID_DIR ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR} )
    endif()

    ADD_CUSTOM_COMMAND( TARGET ${PROJECT_NAME}  POST_BUILD
       COMMAND ${CMAKE_COMMAND} -E copy
       ${CMAKE_CURRENT_BINARY_DIR}/steam_appid.txt
       ${STEAM_APPID_DIR}/steam_appid.txt
    )
endif ()

processing_mix_data_dependencies( ${PROJECT_NAME} )

if (QT5_FOUND)
    link_with_qt5(${PROJECT_NAME})
endif()

if ( QT5_FOUND )
    set (QTCONF_TARGET_DIR "${CMAKE_BINARY_DIR}/${CMAKE_CFG_INTDIR}")
    if (DEPLOY_DIR AND DEPLOY)
        set (QTCONF_TARGET_DIR ${DEPLOY_DIR})
    endif()

    if (MACOS)
        set (QTCONF_TARGET_DIR "${QTCONF_TARGET_DIR}/${PROJECT_NAME}.app/Contents/Resources")
    endif()
    set ( QTCONF_DEPLOY_PATH "${QTCONF_TARGET_DIR}/qt.conf" )

    get_filename_component (ABS_QT_PATH "${QT5_LIB_PATH}/../" ABSOLUTE)
    set ( PLUGINS_PATH  ${ABS_QT_PATH}/plugins )
    set ( QML_IMPORT_PATH ${ABS_QT_PATH}/qml)
    set ( QML2_IMPORT_PATH ${ABS_QT_PATH}/qml)

    if (DEPLOY)
        if ( MACOS )
            set ( PLUGINS_PATH  "PlugIns" )
            set ( QML_IMPORT_PATH "Resources/qml" )
            set ( QML2_IMPORT_PATH "Resources/qml" )
        elseif( WIN32 )
            set ( PLUGINS_PATH  "." )
            set ( QML_IMPORT_PATH "." )
            set ( QML2_IMPORT_PATH "." )
        else()
            ASSERT(false, "Unsuported platform for Qt based application")
        endif()
    endif()

    configure_file( ${DAVA_CONFIGURE_FILES_PATH}/QtConfTemplate.in
                    ${CMAKE_CURRENT_BINARY_DIR}/QtConfTemplate.in )

    ADD_CUSTOM_COMMAND( TARGET ${PROJECT_NAME}  POST_BUILD
       COMMAND ${CMAKE_COMMAND} -E copy
       ${CMAKE_CURRENT_BINARY_DIR}/QtConfTemplate.in
       ${QTCONF_DEPLOY_PATH}
    )

endif()

if( IOS )
    set_target_properties( ${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE_INFO_PLIST "${IOS_PLISTT}"
        RESOURCE                 "${RESOURCES_LIST}"
        XCODE_ATTRIBUTE_INFOPLIST_PREPROCESS YES
        XCODE_ATTRIBUTE_INSTALL_PATH "$(LOCAL_APPS_DIR)"
        XCODE_ATTRIBUTE_SKIP_INSTALL "NO"
        XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH[variant=Debug] "YES"
        XCODE_ATTRIBUTE_ONLY_ACTIVE_ARCH[variant=Release] "NO"
    )

    foreach ( TARGET ${PROJECT_NAME} ${DAVA_LIBRARY}  )
        set_xcode_property( ${TARGET} GCC_GENERATE_DEBUGGING_SYMBOLS[variant=Debug] YES )
    endforeach ()

    # Universal (iPad + iPhone)
    set_target_properties( ${PROJECT_NAME} PROPERTIES XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "1,2" )

elseif( MACOS )
    set_target_properties ( ${PROJECT_NAME} PROPERTIES
                            MACOSX_BUNDLE_INFO_PLIST "${MACOS_PLIST}"
                            XCODE_ATTRIBUTE_INFOPLIST_PREPROCESS YES
                            RESOURCE "${RESOURCES_LIST}"
                          )

    set_property(TARGET ${PROJECT_NAME} APPEND_STRING PROPERTY LINK_FLAGS " -Wl,-dead_strip")

    if( COVERAGE AND MACOS )
        set_target_properties(${PROJECT_NAME} PROPERTIES XCODE_ATTRIBUTE_GCC_GENERATE_TEST_COVERAGE_FILES YES )
        set_target_properties(${PROJECT_NAME} PROPERTIES XCODE_ATTRIBUTE_GCC_INSTRUMENT_PROGRAM_FLOW_ARCS YES )
    endif()

    if( DAVA_FOUND )
        set(LD_RUNPATHES "${ADDED_LD_RUNPATHES} @executable_path/ @executable_path/../Resources @executable_path/../Libs @executable_path/../Frameworks @executable_path/Libs")
        if( NOT DEPLOY )
            foreach( DLIB_PATH ${DYNAMIC_LIBRARIES_${DAVA_PLATFORM_CURRENT}} )
                get_filename_component( DLIB_PATH ${DLIB_PATH} ABSOLUTE )
                get_filename_component( DLIB_PATH_DIR ${DLIB_PATH}  DIRECTORY )

                set( LD_RUNPATHES "${LD_RUNPATHES} ${DLIB_PATH_DIR}/" )
            endforeach()
            set( LD_RUNPATHES "${LD_RUNPATHES} ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/" )
        endif()
        set_target_properties(${PROJECT_NAME} PROPERTIES XCODE_ATTRIBUTE_LD_RUNPATH_SEARCH_PATHS "${LD_RUNPATHES}")
    endif()

    if (QT5_FOUND)
        set(LD_RUNPATHES "${LD_RUNPATHES} ${QT5_LIB_PATH}")
        set_target_properties(${PROJECT_NAME} PROPERTIES XCODE_ATTRIBUTE_LD_RUNPATH_SEARCH_PATHS "${LD_RUNPATHES}")
    endif()

    if( NOT DAVA_MEGASOLUTION )
        set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY XCODE_STARTUP_PROJECT ${PROJECT_NAME})
    endif()

elseif ( WIN32 )

    if( "${EXECUTABLE_FLAG}" STREQUAL "WIN32" )
        set_target_properties ( ${PROJECT_NAME} PROPERTIES LINK_FLAGS "/ENTRY:wWinMainCRTStartup" )
    endif()

    # Generate debug info also in release builds
    set_target_properties ( ${PROJECT_NAME} PROPERTIES LINK_FLAGS_RELEASE "/DEBUG /SUBSYSTEM:WINDOWS" )
    set_target_properties ( ${PROJECT_NAME} PROPERTIES LINK_FLAGS_RELWITHDEBINFO "/DEBUG /SUBSYSTEM:WINDOWS" )

    if( NOT DAVA_DEBUGGER_WORKING_DIRECTORY )
        set( DAVA_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR} )
    endif()

    list( APPEND DAVA_BINARY_WIN_DIR "${ADDED_BINARY_DIR}" )

    if ( WINDOWS_UAP )
        set ( DAVA_VCPROJ_USER_TEMPLATE "DavaWinUAPVcxprojUserTemplate.in" )
    else ()
        set ( DAVA_VCPROJ_USER_TEMPLATE "DavaVcxprojUserTemplate.in" )
    endif ()

    set( DAVA_BINARY_WIN32_DIR_RELEASE    ${DAVA_BINARY_WIN_DIR} ${DAVA_BINARY_WIN_DIR_RELEASE}  ${BINARY_WIN32_DIR_RELEASE}      ${MODULE_DYNAMIC_LIBRARIES_DIR} ${MODULE_DYNAMIC_LIBRARIES_DIR_RELEASE} )
    set( DAVA_BINARY_WIN32_DIR_DEBUG      ${DAVA_BINARY_WIN_DIR} ${DAVA_BINARY_WIN_DIR_DEBUG}    ${BINARY_WIN32_DIR_DEBUG}        ${MODULE_DYNAMIC_LIBRARIES_DIR} ${MODULE_DYNAMIC_LIBRARIES_DIR_DEBUG} )
    set( DAVA_BINARY_WIN32_DIR_RELWITHDEB ${DAVA_BINARY_WIN_DIR} ${DAVA_BINARY_WIN_DIR_RELEASE}  ${BINARY_WIN32_DIR_RELWITHDEB}   ${MODULE_DYNAMIC_LIBRARIES_DIR} ${MODULE_DYNAMIC_LIBRARIES_DIR_RELEASE} )
    set( DAVA_BINARY_WIN64_DIR_RELEASE    ${DAVA_BINARY_WIN_DIR} ${DAVA_BINARY_WIN_DIR_RELEASE}  ${BINARY_WIN64_DIR_RELEASE}      ${MODULE_DYNAMIC_LIBRARIES_DIR} ${MODULE_DYNAMIC_LIBRARIES_DIR_RELEASE} )
    set( DAVA_BINARY_WIN64_DIR_DEBUG      ${DAVA_BINARY_WIN_DIR} ${DAVA_BINARY_WIN_DIR_DEBUG}    ${BINARY_WIN64_DIR_DEBUG}        ${MODULE_DYNAMIC_LIBRARIES_DIR} ${MODULE_DYNAMIC_LIBRARIES_DIR_DEBUG} )
    set( DAVA_BINARY_WIN64_DIR_RELWITHDEB ${DAVA_BINARY_WIN_DIR} ${DAVA_BINARY_WIN_DIR_RELEASE}  ${BINARY_WIN64_DIR_RELWITHDEB}   ${MODULE_DYNAMIC_LIBRARIES_DIR} ${MODULE_DYNAMIC_LIBRARIES_DIR_RELEASE} )

    list( REMOVE_DUPLICATES DAVA_BINARY_WIN32_DIR_RELEASE )
    list( REMOVE_DUPLICATES DAVA_BINARY_WIN32_DIR_DEBUG )
    list( REMOVE_DUPLICATES DAVA_BINARY_WIN32_DIR_RELWITHDEB )
    list( REMOVE_DUPLICATES DAVA_BINARY_WIN64_DIR_RELEASE )
    list( REMOVE_DUPLICATES DAVA_BINARY_WIN64_DIR_DEBUG )
    list( REMOVE_DUPLICATES DAVA_BINARY_WIN64_DIR_RELWITHDEB )

    configure_file( ${DAVA_CONFIGURE_FILES_PATH}/${DAVA_VCPROJ_USER_TEMPLATE}
                    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.vcxproj.user @ONLY )

    if( NOT DAVA_MEGASOLUTION )
        set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
    endif()

    if( OUTPUT_TO_BUILD_DIR )
        set( OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR} )
        foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
            string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
            set_target_properties ( ${PROJECT_NAME} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG}  ${OUTPUT_DIR} )
        endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )
     endif()

    if ( WINDOWS_UAP )
        set_property(TARGET ${PROJECT_NAME} PROPERTY VS_WINRT_COMPONENT TRUE)

        #add a build step for dll deploy fix.
        if ( NEED_DLL_FIX )
            set ( DLL_FIX_TARGET_NAME "DLL_FIX_${PROJECT_NAME}" )
                        
            set ( VS_PROJECT_PATH "${CMAKE_CURRENT_BINARY_DIR}" )
           
            add_custom_target ( ${DLL_FIX_TARGET_NAME} ALL
                    COMMAND python.exe ${DAVA_SCRIPTS_FILES_PATH}/vs_prj_modifications.py uwpDeployDll
                                       --pathVcxProj ${VS_PROJECT_PATH}/${PROJECT_NAME}.vcxproj
            )

            add_dependencies( ${PROJECT_NAME} ${DLL_FIX_TARGET_NAME} )
            set_property( TARGET ${DLL_FIX_TARGET_NAME} PROPERTY FOLDER "CMAKE" )
        endif ()
    endif()
endif()

if( WIN32 AND NOT WINDOWS_UAP )

    set( COMMAND_PY dpiAwarness --pathVcxProj ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.vcxproj --typeAwerness PerMonitorHighDPIAware )

    add_custom_target( VS_MODIFIED_${PROJECT_NAME}  ALL
         COMMAND python.exe ${DAVA_SCRIPTS_FILES_PATH}/vs_prj_modifications.py ${COMMAND_PY}  )

    set_property( TARGET VS_MODIFIED_${PROJECT_NAME} PROPERTY FOLDER "CMAKE" )
    add_dependencies( ${PROJECT_NAME}  VS_MODIFIED_${PROJECT_NAME}  )

    if( DAVA_FOUND )
        add_dependencies(  ${DAVA_LIBRARY} VS_MODIFIED_${PROJECT_NAME} )
    endif()

endif()

if( ANDROID )
    # link libraries
    set( LINK_WHOLE_ARCHIVE_FLAG -Wl,--whole-archive -Wl,--allow-multiple-definition )
    set( NO_LINK_WHOLE_ARCHIVE_FLAG -Wl,--no-whole-archive )

    foreach( LIB_1 ${TARGET_LIBRARIES} )
        foreach( LIB_2 ${TARGET_LIBRARIES} )
            if( ${LIB_1} STREQUAL ${LIB_2} )
            else()
                target_link_libraries( ${LIB_1} ${LINK_WHOLE_ARCHIVE_FLAG} ${LIB_2} ${NO_LINK_WHOLE_ARCHIVE_FLAG} ${LIBRARIES} )
            endif()
        endforeach()
    endforeach()
endif()

set_property( GLOBAL PROPERTY USE_FOLDERS ON )
set_property( GLOBAL PROPERTY PREDEFINED_TARGETS_FOLDER ${DAVA_PREDEFINED_TARGETS_FOLDER} )

######


target_link_libraries( ${PROJECT_NAME} ${LINK_WHOLE_ARCHIVE_FLAG} ${TARGET_LIBRARIES} ${TARGET_MODULES_LIST} ${NO_LINK_WHOLE_ARCHIVE_FLAG} ${LIBRARIES} ${STATIC_LIBRARIES_${DAVA_PLATFORM_CURRENT}} )

foreach ( FILE ${LIBRARIES_DEBUG} ${STATIC_LIBRARIES_${DAVA_PLATFORM_CURRENT}_DEBUG} )
    target_link_libraries  ( ${PROJECT_NAME} debug ${FILE} )
endforeach ()

foreach ( FILE ${LIBRARIES_RELEASE} ${STATIC_LIBRARIES_${DAVA_PLATFORM_CURRENT}_RELEASE} )
    target_link_libraries  ( ${PROJECT_NAME} optimized ${FILE} )
endforeach ()

######


if( DEPLOY )
    message( STATUS "DEPLOY ${PROJECT_NAME} to ${DEPLOY_DIR}")
    execute_process( COMMAND ${CMAKE_COMMAND} -E make_directory ${DEPLOY_DIR} )

    if( DEPLOY_TO_BIN OR DEPLOY_TO_BIN_${DAVA_PLATFORM_CURRENT} )
        file ( GLOB RESOURCES_LIST ${DEPLOY_TO_BIN} ${DEPLOY_TO_BIN_${DAVA_PLATFORM_CURRENT}} )
        if( NOT DEPLOY_DIR_DATA )
            set( DEPLOY_DIR_DATA ${DEPLOY_DIR} )
        endif()
        foreach( ITEM ${RESOURCES_LIST} )
            file(COPY "${ITEM}" DESTINATION "${DEPLOY_DIR_DATA}" )
        endforeach()
    endif()

   append_property( DEPLOY_DIR_${PROJECT_NAME} ${DEPLOY_DIR} )

    if( WIN32 )
        if( APP_DATA )
            foreach( APP_DATA_DIR_ITEM ${APP_DATA})
                get_filename_component( DIR_NAME ${APP_DATA_DIR_ITEM} NAME )

                ADD_CUSTOM_COMMAND( TARGET ${PROJECT_NAME}  POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_directory ${APP_DATA_DIR_ITEM}  ${DEPLOY_DIR}/${DIR_NAME}
                )
            endforeach(APP_DATA_DIR_ITEM)

            ADD_CUSTOM_COMMAND( TARGET ${PROJECT_NAME}  POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E remove  ${DEPLOY_DIR}/${PROJECT_NAME}.ilk
                )
        endif(APP_DATA)

       foreach ( ITEM ${DAVA_THIRD_PARTY_LIBS} )
            execute_process( COMMAND ${CMAKE_COMMAND} -E copy ${DAVA_THIRD_PARTY_LIBRARIES_PATH}/${ITEM}  ${DEPLOY_DIR} )
        endforeach ()

        foreach ( ITEM ${ADDITIONAL_DLL_FILES})
            execute_process( COMMAND ${CMAKE_COMMAND} -E copy ${ITEM}  ${DEPLOY_DIR} )
        endforeach ()

        set( OUTPUT_DIR "${DEPLOY_DIR}" )
        foreach( TARGET ${PROJECT_NAME} ${TARGET_MODULES_LIST} )
            foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
                string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
                set_target_properties ( ${TARGET} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${OUTPUT_DIR} )
            endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )
        endforeach()

    elseif( LINUX )
        if( APP_DATA )
            foreach( APP_DATA_DIR_ITEM ${APP_DATA})
                get_filename_component( DIR_NAME ${APP_DATA_DIR_ITEM} NAME )

                ADD_CUSTOM_COMMAND( TARGET ${PROJECT_NAME}  POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E copy_directory ${APP_DATA_DIR_ITEM}  ${DEPLOY_DIR}/${DIR_NAME} )
            endforeach(APP_DATA_DIR_ITEM)
        endif(APP_DATA)

        foreach ( ITEM ${ADDITIONAL_DLL_FILES})
            execute_process( COMMAND ${CMAKE_COMMAND} -E copy ${ITEM}  ${DEPLOY_DIR} )
        endforeach ()

        set( OUTPUT_DIR "${DEPLOY_DIR}" )
        foreach( TARGET ${PROJECT_NAME} ${TARGET_MODULES_LIST} )
            foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
                string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )
                set_target_properties ( ${TARGET} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${OUTPUT_DIR} )
            endforeach( OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES )
        endforeach()

    elseif( APPLE )
        if( NOT DEPLOY_DIR_EXECUTABLE )
            set( DEPLOY_DIR_EXECUTABLE ${DEPLOY_DIR} )
        endif()

        set_target_properties( ${PROJECT_NAME} PROPERTIES XCODE_ATTRIBUTE_CONFIGURATION_BUILD_DIR  ${DEPLOY_DIR_EXECUTABLE} )

        if( IOS )

            if( NOT IOS_SDK )
                set( IOS_SDK -sdk iphoneos  )
            endif()

            set( XCODERUN_PARAM ${IOS_SDK} PackageApplication -v ${DEPLOY_DIR}/${PROJECT_NAME}.app -o ${DEPLOY_DIR}/${PROJECT_NAME}.ipa )

            if( DEVELOPER_NAME )
                list( APPEND XCODERUN_PARAM  "--sign" "${DEVELOPER_NAME}" )
            endif()

            if( PROVISONING_PROFILE )
                list( APPEND XCODERUN_PARAM  "--embed" "${PROVISONING_PROFILE}" )
            endif()

            add_custom_target ( IOS_DEPLOY_${PROJECT_NAME} ALL COMMAND ${IOS_DEPLOY_CUSTOM_COMAND}
                                                               COMMAND /usr/bin/xcrun ${XCODERUN_PARAM} VERBATIM )

            add_dependencies(  IOS_DEPLOY_${PROJECT_NAME} ${PROJECT_NAME} )

        endif()

        if( MACOS AND (DEPLOY_DIR_LIBS OR MAC_DISABLE_BUNDLE) )
            if( NOT DEPLOY_DIR_LIBS )
                set( DEPLOY_DIR_LIBS ${DEPLOY_DIR}/Libs )
            endif()

            execute_process( COMMAND ${CMAKE_COMMAND} -E make_directory ${DEPLOY_DIR_LIBS} )

            foreach ( ITEM ${DYLIB_FILES})
                execute_process( COMMAND ${CMAKE_COMMAND} -E copy ${ITEM} ${DEPLOY_DIR_LIBS} )
            endforeach ()
        endif()

    endif()

    if( QT5_FOUND AND NOT QT_POST_DEPLOY )
        qt_deploy( )
    endif()

endif()

if( PLUGIN_LIST )
    add_dependencies( ${PROJECT_NAME} ${PLUGIN_LIST} )

    if( DEPLOY )
        set( PLUGIN_OUT_DIR ${DEPLOY_EXECUTE_DIR} )
    else()
        set( PLUGIN_OUT_DIR "$<TARGET_FILE_DIR:${PROJECT_NAME}>" )
    endif()

    foreach( PLUGIN ${PLUGIN_LIST} )

        get_property( ${PLUGIN}_RELATIVE_PATH_TO_FOLDER GLOBAL PROPERTY ${PLUGIN}_RELATIVE_PATH_TO_FOLDER )
        get_property( ${PLUGIN}_PLUGIN_COPY_ADD_FILES GLOBAL PROPERTY ${PLUGIN}_PLUGIN_COPY_ADD_FILES )

        if( APPLE )
            set( PLUGIN_OUT_DIR  "${PLUGIN_OUT_DIR}/../PlugIns" )
        else ()
            set( PLUGIN_OUT_DIR  "${PLUGIN_OUT_DIR}/PlugIns" )
        endif()

        if( ${PLUGIN}_RELATIVE_PATH_TO_FOLDER )
            set( PLUGIN_OUT_DIR ${PLUGIN_OUT_DIR}/${${PLUGIN}_RELATIVE_PATH_TO_FOLDER} )
        endif()

        set_property( GLOBAL PROPERTY ${PLUGIN}_RELATIVE_PATH_TO_FOLDER )

        foreach( OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES} )
            string( TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG )

            if( APPLE )
                set_target_properties( ${PLUGIN} PROPERTIES LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${PLUGIN_OUT_DIR} )
            else()
                set_target_properties( ${PLUGIN} PROPERTIES RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${PLUGIN_OUT_DIR} )
            endif()

        endforeach( )

        if(  ${PLUGIN}_PLUGIN_COPY_ADD_FILES )

            foreach( ITEM ${${PLUGIN}_PLUGIN_COPY_ADD_FILES} )

                if( IS_DIRECTORY ${ITEM} )
                    get_filename_component( FOLDER_NAME ${ITEM}  NAME    )

                    add_custom_command ( TARGET ${PROJECT_NAME}  POST_BUILD
                               COMMAND ${CMAKE_COMMAND} -E copy_directory
                               ${ITEM}
                               ${PLUGIN_OUT_DIR}/${FOLDER_NAME} )

                else()
                    add_custom_command ( TARGET ${PROJECT_NAME}  POST_BUILD
                               COMMAND ${CMAKE_COMMAND} -E copy_if_different
                               ${ITEM}
                               ${PLUGIN_OUT_DIR} )
                endif()

            endforeach( )

        endif()

    endforeach( )

endif()

if( NOT DAVA_MEGASOLUTION )
    file_tree_check()
    dump_module_log()
endif()

coverage_processing()

reset_MAIN_MODULE_VALUES()

endmacro ()

macro( DEPLOY_SCRIPT )

    if( DEPLOY )
        cmake_parse_arguments (ARG "" "" "PYTHON;COPY;COPY_WIN32;COPY_WIN64;COPY_MACOS;COPY_DIR" ${ARGN})

        if( NOT COPY_DIR )
            set( COPY_DIR ${DEPLOY_DIR} )
        endif()

        execute_process( COMMAND ${CMAKE_COMMAND} -E make_directory ${COPY_DIR} )

        if( ARG_PYTHON)
            execute_process( COMMAND python ${ARG_PYTHON} )
        endif()

        if( ARG_COPY )
            list( APPEND COPY_LIST ${ARG_COPY} )
        endif()

        if( ARG_COPY_WIN32 AND WIN32 AND NOT X64_MODE )
            list( APPEND COPY_LIST ${ARG_COPY_WIN32} )
        endif()

        if( ARG_COPY_WIN64 AND WIN32 AND X64_MODE )
            list( APPEND COPY_LIST ${ARG_COPY_WIN64} )
        endif()

        if( ARG_COPY_MACOS AND MACOS )
            list( APPEND COPY_LIST ${ARG_COPY_MACOS} )
        endif()

        foreach ( ITEM ${COPY_LIST} )
            execute_process( COMMAND ${CMAKE_COMMAND} -E copy ${ITEM} ${COPY_DIR} )
        endforeach ()

    endif()
endmacro ()

macro(generate_modules_cpp)
    message(STATUS "Requested modules are:")
    foreach(dvcomp ${DAVA_COMPONENTS})
        message(STATUS "  ${dvcomp}")
    endforeach()

    set( MODULE_GEN_CPP ${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/modules_generated.cpp)
    list( APPEND PROJECT_SOURCE_FILES ${MODULE_GEN_CPP} )

    execute_process( COMMAND python ${DAVA_SCRIPTS_FILES_PATH}/generate_modules_init.py -r ${DAVA_ROOT_DIR} -o ${MODULE_GEN_CPP} ${DAVA_COMPONENTS} )
endmacro()
