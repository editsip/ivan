#include "PluginEditor.h"
#include "WebUIData.h"

// ============================================================================
// Editor implementation
// ============================================================================

IvanAudioProcessorEditor::IvanAudioProcessorEditor(IvanAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    setSize(kDefaultWidth, kDefaultHeight);
    setResizable(true, true);
    setResizeLimits(700, 500, 1400, 1000);

    // Create WebBrowserComponent
    webView = std::make_unique<juce::WebBrowserComponent>(false);
    addAndMakeVisible(*webView);

    // Load the UI from binary resources
    juce::String htmlContent = buildHTMLContent();

    // Use a data URL to load the HTML
    // WebBrowserComponent can load from a temp file or data URL
    auto tempDir = juce::File::getSpecialLocation(juce::File::tempDirectory);
    auto htmlFile = tempDir.getChildFile("ivan_ui.html");
    htmlFile.replaceWithText(htmlContent);
    webView->goToURL(htmlFile.getFullPathName());

    // Set up parameter listener
    paramListener = std::make_unique<ParameterListener>(*this);
    auto& apvts = processorRef.getAPVTS();
    for (auto* param : apvts.processor.getParameters()) {
        if (auto* paramWithID = dynamic_cast<juce::AudioProcessorParameterWithID*>(param))
            apvts.addParameterListener(paramWithID->paramID, paramListener.get());
    }

    // Start timer for periodic UI updates (metering, etc.)
    startTimerHz(30);
}

IvanAudioProcessorEditor::~IvanAudioProcessorEditor() {
    stopTimer();

    auto& apvts = processorRef.getAPVTS();
    for (auto* param : apvts.processor.getParameters()) {
        if (auto* paramWithID = dynamic_cast<juce::AudioProcessorParameterWithID*>(param))
            apvts.removeParameterListener(paramWithID->paramID, paramListener.get());
    }
}

void IvanAudioProcessorEditor::paint(juce::Graphics& g) {
    g.fillAll(juce::Colour(0xff0a0a0a));
}

void IvanAudioProcessorEditor::resized() {
    if (webView)
        webView->setBounds(getLocalBounds());
}

void IvanAudioProcessorEditor::timerCallback() {
    // Push metering data to the UI
    auto& chain = processorRef.getSignalChain();
    float gr = chain.getCompressor().getGainReduction();
    bool gateOpen = chain.getNoiseGate().isOpen();

    juce::String js = "if(window.updateMeters)window.updateMeters("
                    + juce::String(gr, 1) + ","
                    + (gateOpen ? "true" : "false") + ");";
    webView->goToURL("javascript:" + js);
}

void IvanAudioProcessorEditor::sendParameterUpdate(const juce::String& paramId, float value) {
    juce::String js = "if(window.onParamChange)window.onParamChange('"
                    + paramId + "'," + juce::String(value, 4) + ");";
    webView->goToURL("javascript:" + js);
}

void IvanAudioProcessorEditor::sendAllParameters() {
    auto& apvts = processorRef.getAPVTS();
    for (auto* param : apvts.processor.getParameters()) {
        if (auto* p = dynamic_cast<juce::AudioProcessorParameterWithID*>(param))
            sendParameterUpdate(p->paramID, p->getValue());
    }
}

juce::String IvanAudioProcessorEditor::buildHTMLContent() {
    // Load from binary resources
    auto htmlData = juce::String::createStringFromData(
        WebUI::index_html, WebUI::index_htmlSize);
    auto cssData = juce::String::createStringFromData(
        WebUI::style_css, WebUI::style_cssSize);
    auto jsData = juce::String::createStringFromData(
        WebUI::app_js, WebUI::app_jsSize);

    // Inline CSS and JS into HTML
    htmlData = htmlData.replace("<!-- INLINE_CSS -->",
        "<style>" + cssData + "</style>");
    htmlData = htmlData.replace("<!-- INLINE_JS -->",
        "<script>" + jsData + "</script>");

    return htmlData;
}

void IvanAudioProcessorEditor::ParameterListener::parameterChanged(
    const juce::String& parameterID, float newValue)
{
    // Thread-safe: post to message thread
    juce::MessageManager::callAsync([&editor = this->editor, parameterID, newValue]() {
        editor.sendParameterUpdate(parameterID, newValue);
    });
}
