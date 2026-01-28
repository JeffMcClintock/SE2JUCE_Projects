This repo will build a demo plugin called PD303.

To build your own plugin:

Copy entire PD303 folder, name it to your plugin name

Edit CmakeLists.txt

project(PD303 VERSION 0.0.1)

Change plugin name 'PD303'

change manufacturer code from Xxxx (keeping same format)

change PLUGIN_CODE from Yyyy (keeping the same 4-digit format)

change PRODUCT_NAME

In VS Configuration Manager, untick LV2 Helper under Debug and Release

JUCE_APP_VERSION

