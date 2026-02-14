#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class IvanLookAndFeel : public juce::LookAndFeel_V4 {
public:
    IvanLookAndFeel();

    // Colors
    static constexpr juce::uint32 bgPrimary    = 0xff0a0a0a;
    static constexpr juce::uint32 bgSecondary  = 0xff111111;
    static constexpr juce::uint32 bgElevated   = 0xff161616;
    static constexpr juce::uint32 bgCard       = 0xff1a1a1a;
    static constexpr juce::uint32 borderSubtle  = 0xff222222;
    static constexpr juce::uint32 borderNormal  = 0xff2a2a2a;
    static constexpr juce::uint32 textPrimary   = 0xffe8e8e8;
    static constexpr juce::uint32 textSecondary = 0xff888888;
    static constexpr juce::uint32 textTertiary  = 0xff555555;
    static constexpr juce::uint32 amber         = 0xffc4883a;
    static constexpr juce::uint32 amberDim      = 0xff8a6028;
    static constexpr juce::uint32 amberGlow     = 0x28c4883a;
    static constexpr juce::uint32 green         = 0xff4a9e6e;
    static constexpr juce::uint32 red           = 0xffb04a4a;
    static constexpr juce::uint32 knobTrack     = 0xff2a2a2a;

    void drawRotarySlider(juce::Graphics&, int x, int y, int width, int height,
                          float sliderPosProportional, float rotaryStartAngle,
                          float rotaryEndAngle, juce::Slider&) override;

    void drawComboBox(juce::Graphics&, int width, int height, bool isButtonDown,
                      int buttonX, int buttonY, int buttonW, int buttonH,
                      juce::ComboBox&) override;

    void drawPopupMenuItem(juce::Graphics&, const juce::Rectangle<int>& area,
                           bool isSeparator, bool isActive, bool isHighlighted,
                           bool isTicked, bool hasSubMenu,
                           const juce::String& text, const juce::String& shortcutKeyText,
                           const juce::Drawable* icon, const juce::Colour* textColour) override;

    juce::Font getComboBoxFont(juce::ComboBox&) override;
    juce::Font getPopupMenuFont() override;

    void drawToggleButton(juce::Graphics&, juce::ToggleButton&,
                          bool shouldDrawButtonAsHighlighted,
                          bool shouldDrawButtonAsDown) override;

    void drawLabel(juce::Graphics&, juce::Label&) override;
};
