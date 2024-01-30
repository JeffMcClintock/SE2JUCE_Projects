#include "SE2JUCE_Processor.h"

// Here you can specify a Processor class.
// the default is SE2JUCE_Processor, but you can substitute your own Processor class for example if you need a custom Editor.

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SE2JUCE_Processor();
}
