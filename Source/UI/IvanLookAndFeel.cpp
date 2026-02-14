#include "IvanLookAndFeel.h"

IvanLookAndFeel::IvanLookAndFeel() {
    setColour(juce::ResizableWindow::backgroundColourId, juce::Colour(bgPrimary));
    setColour(juce::ComboBox::backgroundColourId, juce::Colour(bgCard));
    setColour(juce::ComboBox::outlineColourId, juce::Colour(borderNormal));
    setColour(juce::ComboBox::textColourId, juce::Colour(textPrimary));
    setColour(juce::ComboBox::arrowColourId, juce::Colour(textSecondary));
    setColour(juce::PopupMenu::backgroundColourId, juce::Colour(bgElevated));
    setColour(juce::PopupMenu::textColourId, juce::Colour(textPrimary));
    setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colour(amberGlow));
    setColour(juce::PopupMenu::highlightedTextColourId, juce::Colour(amber));
    setColour(juce::Label::textColourId, juce::Colour(textSecondary));
    setColour(juce::ToggleButton::textColourId, juce::Colour(textSecondary));
    setColour(juce::ToggleButton::tickColourId, juce::Colour(amber));
}

void IvanLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                        float sliderPos, float rotaryStartAngle,
                                        float rotaryEndAngle, juce::Slider& slider)
{
    auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat().reduced(4.0f);
    auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) / 2.0f;
    auto centreX = bounds.getCentreX();
    auto centreY = bounds.getCentreY();
    auto rx = centreX - radius;
    auto ry = centreY - radius;
    auto rw = radius * 2.0f;
    auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

    // Track circle
    g.setColour(juce::Colour(knobTrack));
    juce::Path trackArc;
    trackArc.addCentredArc(centreX, centreY, radius - 2, radius - 2,
                           0.0f, rotaryStartAngle, rotaryEndAngle, true);
    g.strokePath(trackArc, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved,
                                                  juce::PathStrokeType::rounded));

    // Value arc
    g.setColour(juce::Colour(amber));
    juce::Path valueArc;
    valueArc.addCentredArc(centreX, centreY, radius - 2, radius - 2,
                           0.0f, rotaryStartAngle, angle, true);
    g.strokePath(valueArc, juce::PathStrokeType(2.0f, juce::PathStrokeType::curved,
                                                  juce::PathStrokeType::rounded));

    // Dot indicator
    float dotRadius = 3.0f;
    float dotX = centreX + (radius - 2) * std::sin(angle);
    float dotY = centreY - (radius - 2) * std::cos(angle);
    g.setColour(juce::Colour(amber));
    g.fillEllipse(dotX - dotRadius, dotY - dotRadius, dotRadius * 2, dotRadius * 2);

    // Subtle glow behind dot
    juce::ColourGradient glow(juce::Colour(amber).withAlpha(0.15f), dotX, dotY,
                               juce::Colour(amber).withAlpha(0.0f), dotX + 10, dotY + 10, true);
    g.setGradientFill(glow);
    g.fillEllipse(dotX - 8, dotY - 8, 16, 16);
}

void IvanLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height, bool,
                                    int, int, int, int, juce::ComboBox& box)
{
    auto bounds = juce::Rectangle<int>(0, 0, width, height).toFloat();
    g.setColour(juce::Colour(bgCard));
    g.fillRoundedRectangle(bounds, 6.0f);
    g.setColour(juce::Colour(borderNormal));
    g.drawRoundedRectangle(bounds.reduced(0.5f), 6.0f, 1.0f);

    // Arrow
    auto arrowArea = juce::Rectangle<float>(float(width - 20), 0.0f, 16.0f, float(height));
    juce::Path arrow;
    arrow.addTriangle(arrowArea.getCentreX() - 4, arrowArea.getCentreY() - 2,
                      arrowArea.getCentreX() + 4, arrowArea.getCentreY() - 2,
                      arrowArea.getCentreX(), arrowArea.getCentreY() + 3);
    g.setColour(juce::Colour(textSecondary));
    g.fillPath(arrow);
}

void IvanLookAndFeel::drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
                                         bool, bool isActive, bool isHighlighted, bool isTicked,
                                         bool, const juce::String& text, const juce::String&,
                                         const juce::Drawable*, const juce::Colour*)
{
    if (isHighlighted) {
        g.setColour(juce::Colour(amberGlow));
        g.fillRect(area);
    }

    g.setColour(isHighlighted ? juce::Colour(amber) :
                isTicked ? juce::Colour(amber) : juce::Colour(textPrimary));
    g.setFont(12.0f);
    g.drawText(text, area.reduced(10, 0), juce::Justification::centredLeft);
}

juce::Font IvanLookAndFeel::getComboBoxFont(juce::ComboBox&)  { return juce::Font(12.0f); }
juce::Font IvanLookAndFeel::getPopupMenuFont()                { return juce::Font(12.0f); }

void IvanLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                                        bool highlighted, bool)
{
    auto bounds = button.getLocalBounds().toFloat();
    auto toggled = button.getToggleState();

    // Draw pill-shaped toggle
    auto toggleBounds = juce::Rectangle<float>(0, bounds.getCentreY() - 8, 32, 16);
    g.setColour(toggled ? juce::Colour(amber).withAlpha(0.3f) : juce::Colour(bgCard));
    g.fillRoundedRectangle(toggleBounds, 8.0f);
    g.setColour(toggled ? juce::Colour(amber) : juce::Colour(borderNormal));
    g.drawRoundedRectangle(toggleBounds.reduced(0.5f), 8.0f, 1.0f);

    // Dot
    float dotX = toggled ? toggleBounds.getRight() - 10.0f : toggleBounds.getX() + 10.0f;
    g.setColour(toggled ? juce::Colour(amber) : juce::Colour(textTertiary));
    g.fillEllipse(dotX - 5, toggleBounds.getCentreY() - 5, 10, 10);

    // Label
    g.setColour(juce::Colour(toggled ? textPrimary : textSecondary));
    g.setFont(11.0f);
    g.drawText(button.getButtonText(), toggleBounds.getRight() + 6, 0,
               int(bounds.getWidth() - toggleBounds.getRight() - 6), int(bounds.getHeight()),
               juce::Justification::centredLeft);
}

void IvanLookAndFeel::drawLabel(juce::Graphics& g, juce::Label& label) {
    g.setColour(label.findColour(juce::Label::textColourId));
    g.setFont(label.getFont());
    g.drawText(label.getText(), label.getLocalBounds(), label.getJustificationType());
}
