
#include <JuceHeader.h>
#include "BinaryData.h"

#define INIT_STATIC_FILE(filename) void se_static_library_init_##filename(); se_static_library_init_##filename();

/*
Please add a reference below to any additional modules that you need.
and also do so in CmakeLists.txt just below "Source/ExtraModules.cpp"
*/

extern void initialise_synthedit_extra_modules([[maybe_unused]]bool passFalse)
{
#ifndef _DEBUG
    // NOTE: We don't have to actually call these functions (they do nothing), only *reference* them to
    // cause them to be linked into plugin such that static objects get initialised.
    if (!passFalse)
        return;
#endif

    INIT_STATIC_FILE(ADSR4);
}

// Provide the framework with access to plugin-specific Binary Resources
extern const char* se2juce_getNamedResource(const char* name, int& returnBytes)
{
	returnBytes = 0; // defensive

	return BinaryData::getNamedResource(name, returnBytes);
}

#if 0
extern const char* se2juce_getIndexedResource(int index, int& returnBytes)
{
	returnBytes = 0; // defensive

	if (index < 0 || BinaryData::namedResourceListSize >= index)
		return {};

	return BinaryData::getNamedResource(BinaryData::namedResourceList[index], returnBytes);
}
#endif