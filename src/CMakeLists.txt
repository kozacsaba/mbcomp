# Plugin #######################################################################

# juce_set_vst2_sdk_path(...)

juce_add_plugin(MBComp
    PRODUCT_NAME "MBComp"
    COMPANY_NAME "Csaba Koza"
    FORMATS 
        Standalone
        VST3
    PLUGIN_NAME "MBComp"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    VST3_CATEGORIES
        Fx
        Dynamics
    BUNDLE_ID "com.Csaba_Koza.MBComp"
)

target_include_directories(MBComp PRIVATE 
    ${CMAKE_SOURCE_DIR}/src
    
    ${CMAKE_SOURCE_DIR}/src/frame
    ${CMAKE_SOURCE_DIR}/src/containers
    ${CMAKE_SOURCE_DIR}/src/gui
    ${CMAKE_SOURCE_DIR}/src/processors
    
    # ...
    )

target_sources(MBComp PRIVATE

    ./frame/PluginEditor.cpp
    ./frame/PluginProcessor.cpp

    ./gui/EditorComponent.cpp

    # ...
    )

target_compile_definitions(MBComp PRIVATE
    JUCE_WEB_BROWSER=0  # If you remove this, add `NEEDS_WEB_BROWSER TRUE` to the `juce_add_gui_app` call
    JUCE_USE_CURL=0     # If you remove this, add `NEEDS_CURL TRUE` to the `juce_add_gui_app` call
    JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:MBComp,JUCE_PRODUCT_NAME>"
    JUCE_APPLICATION_VERSION_STRING="$<TARGET_PROPERTY:MBComp,JUCE_VERSION>"
)

# juce_add_binary_data(GuiAppData SOURCES ...)

target_link_libraries(MBComp PRIVATE
    # GuiAppData
    juce::juce_core
    juce::juce_audio_basics
    juce::juce_gui_basics
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_audio_devices
)

target_compile_features(MBComp PUBLIC cxx_std_20)
