file(GLOB_RECURSE AssetFiles CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/../Resources2/*")

# Don't edit manualy. These are any binary resources automatically exported by SynthEdit
juce_add_binary_data(BinaryData SOURCES 
${AssetFiles}
"dsp.se.xml"
"gui.se.json"
"factory.se.xml"
"database.se.xml"
"parameters.se.xml"
"PD303____fontMetrics.txt"
"PD303__background.png"
"PD303__button_sm.png"
"PD303__button_sm.txt"
"PD303__global.txt"
"PD303__knob_med.png"
"PD303__knob_med.txt"
"default2__global.txt"
"default__button_sm.png"
"default__button_sm.txt"
"default__knob_sm.png"
"default__knob_sm.txt"
"default__led.png"
"default__led.txt"
)
