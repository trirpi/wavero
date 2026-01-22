#include "PluginEditor.h"

//==============================================================================
// FuturisticSliderLookAndFeel
//==============================================================================
FuturisticSliderLookAndFeel::FuturisticSliderLookAndFeel()
{
    setColour (juce::Label::textColourId, juce::Colour (0xffb8b8c0));
}

void FuturisticSliderLookAndFeel::drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                                                     float sliderPos, float minSliderPos, float maxSliderPos,
                                                     juce::Slider::SliderStyle style, juce::Slider& slider)
{
    juce::ignoreUnused (minSliderPos, maxSliderPos, style);

    auto bounds = juce::Rectangle<float> (static_cast<float> (x), static_cast<float> (y),
                                          static_cast<float> (width), static_cast<float> (height));
    
    if (bounds.getWidth() <= 0.0f || bounds.getHeight() <= 0.0f)
        return;
    
    auto trackWidth = 6.0f;
    auto trackX = bounds.getCentreX() - trackWidth * 0.5f;
    auto fillColour = slider.findColour (juce::Slider::trackColourId);
    float centreX = bounds.getCentreX();

    // Draw tick marks FIRST (behind everything)
    for (int i = 0; i <= 10; ++i)
    {
        float tickY = bounds.getY() + (bounds.getHeight() * static_cast<float> (i) / 10.0f);
        bool isMajor = (i % 5 == 0);
        
        if (isMajor)
        {
            // Chevron tick for major positions
            g.setColour (juce::Colour (0xff303040));
            juce::Path chevron;
            chevron.startNewSubPath (trackX - 10.0f, tickY);
            chevron.lineTo (trackX - 5.0f, tickY - 3.0f);
            chevron.lineTo (trackX - 5.0f, tickY + 3.0f);
            chevron.closeSubPath();
            g.fillPath (chevron);
        }
        else
        {
            // Simple line for minor ticks
            g.setColour (juce::Colour (0xff202028));
            g.fillRect (trackX - 6.0f, tickY - 0.5f, 4.0f, 1.0f);
        }
    }

    // Art Deco stepped track background
    g.setColour (juce::Colour (0xff0a0a12));
    g.fillRect (trackX - 1.0f, bounds.getY(), trackWidth + 2.0f, bounds.getHeight());
    
    // Inner track
    g.setColour (juce::Colour (0xff050508));
    g.fillRect (trackX, bounds.getY() + 2.0f, trackWidth, bounds.getHeight() - 4.0f);

    // Filled portion
    float clampedSliderPos = juce::jlimit (bounds.getY(), bounds.getBottom(), sliderPos);
    float filledHeight = bounds.getBottom() - clampedSliderPos;

    if (filledHeight > 1.0f)
    {
        // Stepped glow effect (Art Deco style)
        for (int i = 2; i >= 0; --i)
        {
            float alpha = 0.06f * (1.0f - static_cast<float> (i) * 0.3f);
            float spread = static_cast<float> (i) * 4.0f;
            g.setColour (fillColour.withAlpha (alpha));
            g.fillRect (trackX - spread, clampedSliderPos, trackWidth + spread * 2.0f, filledHeight);
        }

        // Main fill with gradient
        juce::ColourGradient fillGradient (fillColour.brighter (0.3f), trackX, clampedSliderPos,
                                           fillColour.darker (0.2f), trackX, bounds.getBottom(), false);
        g.setGradientFill (fillGradient);
        g.fillRect (trackX, clampedSliderPos, trackWidth, filledHeight);

        // Highlight line
        g.setColour (fillColour.brighter (0.6f).withAlpha (0.5f));
        g.fillRect (trackX + 1.0f, clampedSliderPos + 2.0f, 2.0f, juce::jmax (0.0f, filledHeight - 4.0f));
    }

    // Art Deco geometric thumb (diamond shape)
    float thumbCentreY = juce::jlimit (bounds.getY() + 8.0f, bounds.getBottom() - 8.0f, clampedSliderPos);
    
    // Thumb glow
    g.setColour (fillColour.withAlpha (0.2f));
    juce::Path glowPath;
    glowPath.addTriangle (centreX - 14.0f, thumbCentreY, centreX, thumbCentreY - 10.0f, centreX, thumbCentreY + 10.0f);
    glowPath.addTriangle (centreX + 14.0f, thumbCentreY, centreX, thumbCentreY - 10.0f, centreX, thumbCentreY + 10.0f);
    g.fillPath (glowPath);

    // Main diamond thumb
    juce::Path thumbPath;
    thumbPath.addTriangle (centreX - 11.0f, thumbCentreY, centreX, thumbCentreY - 7.0f, centreX, thumbCentreY + 7.0f);
    thumbPath.addTriangle (centreX + 11.0f, thumbCentreY, centreX, thumbCentreY - 7.0f, centreX, thumbCentreY + 7.0f);
    
    juce::ColourGradient thumbGradient (juce::Colour (0xff252535), centreX, thumbCentreY - 7.0f,
                                        juce::Colour (0xff101018), centreX, thumbCentreY + 7.0f, false);
    g.setGradientFill (thumbGradient);
    g.fillPath (thumbPath);

    // Thumb border
    g.setColour (fillColour.withAlpha (0.6f));
    g.strokePath (thumbPath, juce::PathStrokeType (1.0f));

    // Centre accent line on thumb
    g.setColour (fillColour);
    g.fillRect (centreX - 6.0f, thumbCentreY - 0.5f, 12.0f, 1.0f);
}

juce::Label* FuturisticSliderLookAndFeel::createSliderTextBox (juce::Slider& slider)
{
    auto* label = juce::LookAndFeel_V4::createSliderTextBox (slider);
    label->setColour (juce::Label::textColourId, juce::Colour (0xffb8b8c0));  // Same as dimTextColour
    label->setColour (juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    label->setColour (juce::Label::outlineColourId, juce::Colours::transparentBlack);
    if (customFonts != nullptr)
        label->setFont (customFonts->getLabelFont (11.0f));
    else
        label->setFont (juce::FontOptions (11.0f));
    return label;
}

void FuturisticSliderLookAndFeel::drawButtonBackground (juce::Graphics& g, juce::Button& button,
                                                         const juce::Colour& backgroundColour,
                                                         bool isMouseOver, bool isButtonDown)
{
    auto bounds = button.getLocalBounds().toFloat().reduced (0.5f);

    auto baseColour = backgroundColour;
    if (isButtonDown)
        baseColour = baseColour.brighter (0.2f);
    else if (isMouseOver)
        baseColour = baseColour.brighter (0.1f);

    g.setColour (baseColour);
    g.fillRoundedRectangle (bounds, 4.0f);

    g.setColour (button.findColour (juce::TextButton::textColourOffId).withAlpha (0.3f));
    g.drawRoundedRectangle (bounds, 4.0f, 1.0f);
}

juce::Font FuturisticSliderLookAndFeel::getTextButtonFont (juce::TextButton&, int buttonHeight)
{
    float fontSize = juce::jmax (11.0f, static_cast<float> (buttonHeight) * 0.5f);
    if (customFonts != nullptr)
        return customFonts->getTitleFont (fontSize);
    return juce::Font (juce::FontOptions (fontSize));
}

juce::Font FuturisticSliderLookAndFeel::getLabelFont (juce::Label& label)
{
    if (customFonts != nullptr)
        return customFonts->getLabelFont (label.getFont().getHeight());
    return label.getFont();
}

void FuturisticSliderLookAndFeel::drawButtonText (juce::Graphics& g, juce::TextButton& button,
                                                   bool isMouseOverButton, bool isButtonDown)
{
    juce::ignoreUnused (isMouseOverButton, isButtonDown);
    
    auto font = getTextButtonFont (button, button.getHeight());
    g.setFont (font);
    g.setColour (button.findColour (button.getToggleState() ? juce::TextButton::textColourOnId
                                                            : juce::TextButton::textColourOffId)
                       .withMultipliedAlpha (button.isEnabled() ? 1.0f : 0.5f));

    auto bounds = button.getLocalBounds().toFloat();
    g.drawText (button.getButtonText(), bounds, juce::Justification::centred, true);
}

//==============================================================================
// WaveformDisplay
//==============================================================================
WaveformDisplay::WaveformDisplay (PluginProcessor& p, const CustomFonts& fonts) 
    : processor (p), customFonts (fonts)
{
    startTimerHz (60);
}

WaveformDisplay::~WaveformDisplay()
{
    stopTimer();
}

void WaveformDisplay::timerCallback()
{
    animationPhase += 0.05f;
    if (animationPhase > juce::MathConstants<float>::twoPi)
        animationPhase -= juce::MathConstants<float>::twoPi;

    cachedGrainVisuals = processor.getGrainVisuals();

    for (size_t i = 0; i < cachedGrainVisuals.size() && i < grainGlowIntensity.size(); ++i)
    {
        if (cachedGrainVisuals[i].active)
        {
            // Fade in quickly
            grainGlowIntensity[i] += (1.0f - grainGlowIntensity[i]) * 0.4f;
            
            // Spawn particles for active grains
            if (random.nextFloat() < 0.2f && std::isfinite (cachedGrainVisuals[i].position))
            {
                auto bounds = getLocalBounds().toFloat();
                float clampedPos = juce::jlimit (0.0f, 1.0f, cachedGrainVisuals[i].position);
                float xPos = bounds.getX() + 10.0f + clampedPos * (bounds.getWidth() - 20.0f);
                float yPos = bounds.getCentreY() + (random.nextFloat() - 0.5f) * bounds.getHeight() * 0.3f;
                if (std::isfinite (xPos) && std::isfinite (yPos))
                    spawnParticles (xPos, yPos, 1);
            }
        }
        else
        {
            // Fade out faster when inactive
            grainGlowIntensity[i] *= 0.85f;
            // Force to zero when very small to prevent lingering
            if (grainGlowIntensity[i] < 0.005f)
                grainGlowIntensity[i] = 0.0f;
        }
    }

    updateParticles();
    repaint();
}

void WaveformDisplay::spawnParticles (float x, float y, int count)
{
    for (int i = 0; i < count; ++i)
    {
        GrainParticle p;
        p.x = x;
        p.y = y;
        p.vx = (random.nextFloat() - 0.5f) * 2.0f;
        p.vy = -random.nextFloat() * 2.0f - 0.5f;
        p.life = 1.0f;
        p.maxLife = 0.4f + random.nextFloat() * 0.4f;
        p.size = 1.5f + random.nextFloat() * 2.5f;
        p.colour = particleColour.interpolatedWith (grainColour, random.nextFloat());
        particles.push_back (p);
    }
    while (particles.size() > 120u)
        particles.erase (particles.begin());
}

void WaveformDisplay::updateParticles()
{
    for (auto it = particles.begin(); it != particles.end();)
    {
        it->x += it->vx;
        it->y += it->vy;
        it->vy += 0.06f;
        it->life -= 1.0f / (60.0f * it->maxLife);
        if (it->life <= 0.0f)
            it = particles.erase (it);
        else
            ++it;
    }
}

void WaveformDisplay::drawGrainIndicator (juce::Graphics& g, float x, float progress,
                                           float amplitude, float glowIntensity)
{
    auto bounds = getLocalBounds().toFloat();
    float topLimit = bounds.getY() + 12.0f;      // Stay away from top edge
    float bottomLimit = bounds.getBottom() - 12.0f;  // Stay away from bottom edge
    float centerY = bounds.getCentreY();
    float availableHeight = bottomLimit - topLimit;
    float pulse = 0.7f + 0.3f * std::sin (animationPhase * 3.0f + x * 0.1f);
    float effectiveGlow = glowIntensity * pulse;

    // Calculate heights that stay within bounds
    float maxHalfHeight = availableHeight * 0.45f;
    float glowHalfHeight = maxHalfHeight * 0.9f;
    float lineHalfHeight = maxHalfHeight * 0.75f * (0.3f + 0.7f * (1.0f - progress));

    // Soft glow - use filled rectangles with gradient instead of thick lines
    // This gives us soft edges that don't clip
    for (int i = 2; i >= 0; --i)
    {
        float alpha = effectiveGlow * 0.06f * (1.0f - static_cast<float> (i) * 0.25f);
        float spread = 3.0f + static_cast<float> (i) * 4.0f;
        
        // Top half glow - fades to transparent at top
        juce::ColourGradient glowTop (grainColour.withAlpha (alpha), x, centerY,
                                      grainColour.withAlpha (0.0f), x, centerY - glowHalfHeight, false);
        g.setGradientFill (glowTop);
        g.fillRect (x - spread * 0.5f, centerY - glowHalfHeight, spread, glowHalfHeight);
        
        // Bottom half glow - fades to transparent at bottom
        juce::ColourGradient glowBottom (grainColour.withAlpha (alpha), x, centerY,
                                         grainColour.withAlpha (0.0f), x, centerY + glowHalfHeight, false);
        g.setGradientFill (glowBottom);
        g.fillRect (x - spread * 0.5f, centerY, spread, glowHalfHeight);
    }

    // Main line with gradient fade at ends
    float lineWidth = 2.0f + amplitude * 2.0f;
    
    // Top half of main line
    juce::ColourGradient lineTop (grainColour.withAlpha (effectiveGlow * 0.9f), x, centerY,
                                  grainColour.withAlpha (0.0f), x, centerY - lineHalfHeight, false);
    g.setGradientFill (lineTop);
    g.fillRect (x - lineWidth * 0.5f, centerY - lineHalfHeight, lineWidth, lineHalfHeight);
    
    // Bottom half of main line  
    juce::ColourGradient lineBottom (grainColour.withAlpha (effectiveGlow * 0.9f), x, centerY,
                                     grainColour.withAlpha (0.0f), x, centerY + lineHalfHeight, false);
    g.setGradientFill (lineBottom);
    g.fillRect (x - lineWidth * 0.5f, centerY, lineWidth, lineHalfHeight);

    // Bright center highlight
    float highlightHalf = lineHalfHeight * 0.25f;
    g.setColour (juce::Colours::white.withAlpha (effectiveGlow * 0.7f));
    g.fillRect (x - 0.5f, centerY - highlightHalf, 1.0f, highlightHalf * 2.0f);

    // Center dot instead of end caps (no clipping issues)
    float dotSize = 3.0f + amplitude * 2.0f;
    g.setColour (grainColour.withAlpha (effectiveGlow));
    g.fillEllipse (x - dotSize * 0.5f, centerY - dotSize * 0.5f, dotSize, dotSize);
    
    // Smaller dots at line ends (well within bounds)
    float endDotSize = 2.0f;
    g.setColour (grainColour.withAlpha (effectiveGlow * 0.5f));
    g.fillEllipse (x - endDotSize * 0.5f, centerY - lineHalfHeight * 0.7f - endDotSize * 0.5f, endDotSize, endDotSize);
    g.fillEllipse (x - endDotSize * 0.5f, centerY + lineHalfHeight * 0.7f - endDotSize * 0.5f, endDotSize, endDotSize);
}

void WaveformDisplay::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();

    juce::ColourGradient bgGradient (backgroundColour.brighter (0.02f), bounds.getCentreX(), bounds.getY(),
                                     backgroundColour, bounds.getCentreX(), bounds.getBottom(), false);
    g.setGradientFill (bgGradient);
    g.fillRoundedRectangle (bounds, 5.0f);

    // Scanlines
    g.setColour (juce::Colour (0x06ffffff));
    for (float yPos = 0.0f; yPos < bounds.getHeight(); yPos += 3.0f)
        g.fillRect (bounds.getX(), bounds.getY() + yPos, bounds.getWidth(), 1.0f);

    float borderAlpha = (isDragOver || isHovering) ? 0.5f : (0.12f + 0.04f * std::sin (animationPhase));
    g.setColour (waveformColour.withAlpha (borderAlpha));
    g.drawRoundedRectangle (bounds.reduced (0.5f), 5.0f, (isDragOver || isHovering) ? 1.5f : 1.0f);

    // Art Deco corner accents - inset to clear the 5px rounded corners
    float cs = 14.0f;
    float inset = 8.0f;
    g.setColour (waveformColour.withAlpha (0.35f));
    // Top-left
    g.drawLine (bounds.getX() + inset, bounds.getY() + inset, bounds.getX() + inset + cs, bounds.getY() + inset, 1.5f);
    g.drawLine (bounds.getX() + inset, bounds.getY() + inset, bounds.getX() + inset, bounds.getY() + inset + cs, 1.5f);
    // Top-right
    g.drawLine (bounds.getRight() - inset, bounds.getY() + inset, bounds.getRight() - inset - cs, bounds.getY() + inset, 1.5f);
    g.drawLine (bounds.getRight() - inset, bounds.getY() + inset, bounds.getRight() - inset, bounds.getY() + inset + cs, 1.5f);
    // Bottom-left
    g.drawLine (bounds.getX() + inset, bounds.getBottom() - inset, bounds.getX() + inset + cs, bounds.getBottom() - inset, 1.5f);
    g.drawLine (bounds.getX() + inset, bounds.getBottom() - inset, bounds.getX() + inset, bounds.getBottom() - inset - cs, 1.5f);
    // Bottom-right
    g.drawLine (bounds.getRight() - inset, bounds.getBottom() - inset, bounds.getRight() - inset - cs, bounds.getBottom() - inset, 1.5f);
    g.drawLine (bounds.getRight() - inset, bounds.getBottom() - inset, bounds.getRight() - inset, bounds.getBottom() - inset - cs, 1.5f);

    if (processor.hasSampleLoaded())
    {
        auto& buffer = processor.getSampleBuffer();
        int numSamples = buffer.getNumSamples();
        int numChannels = buffer.getNumChannels();

        if (numSamples > 0 && numChannels > 0)
        {
            auto* samples = buffer.getReadPointer (0);
            float availableWidth = bounds.getWidth() - 20.0f;
            float height = bounds.getHeight() - 20.0f;
            float centerY = bounds.getCentreY();
            float startX = bounds.getX() + 10.0f;
            
            // Calculate sample duration
            double sampleRate = processor.getSampleSampleRate();
            double durationSeconds = static_cast<double> (numSamples) / sampleRate;
            cachedSampleDuration = durationSeconds;
            
            // Determine display mode based on duration
            // Short sample (<1s): fixed scale, don't stretch
            // Normal sample (1s-60s): stretch to fit
            // Long sample (>60s): scrollable, show 60s at a time
            
            float waveformWidth;
            int viewStartSample = 0;
            int viewEndSample = numSamples;
            
            constexpr float pixelsPerSecond = 200.0f;  // For short samples
            constexpr double maxViewDuration = 60.0;   // Show 60 seconds max before scrolling
            
            if (durationSeconds < 1.0)
            {
                // Short sample: use fixed pixel-per-second scale
                waveformWidth = static_cast<float> (durationSeconds * pixelsPerSecond);
                waveformWidth = juce::jmin (waveformWidth, availableWidth);  // Don't exceed available
                needsScroll = false;
            }
            else if (durationSeconds <= maxViewDuration)
            {
                // Normal sample: stretch to fit
                waveformWidth = availableWidth;
                needsScroll = false;
            }
            else
            {
                // Long sample: scrollable view
                waveformWidth = availableWidth;
                needsScroll = true;
                
                // Calculate visible portion based on scroll offset
                double viewDuration = maxViewDuration;
                double maxScrollTime = durationSeconds - viewDuration;
                double viewStartTime = scrollOffset * maxScrollTime;
                
                viewStartSample = static_cast<int> (viewStartTime * sampleRate);
                viewEndSample = static_cast<int> ((viewStartTime + viewDuration) * sampleRate);
                viewEndSample = juce::jmin (viewEndSample, numSamples);
            }

            // Find peak amplitude for normalization (across entire sample for consistency)
            float peakAmplitude = 0.0f;
            for (int i = 0; i < numSamples; ++i)
                peakAmplitude = juce::jmax (peakAmplitude, std::abs (samples[i]));
            
            // Normalize factor (avoid division by zero, cap at reasonable gain)
            float normalizeFactor = (peakAmplitude > 0.001f) ? (1.0f / peakAmplitude) : 1.0f;
            normalizeFactor = juce::jmin (normalizeFactor, 10.0f);  // Cap at 10x gain for very quiet samples

            juce::Path waveformTop, waveformBottom;

            int widthInt = static_cast<int> (waveformWidth);
            int viewNumSamples = viewEndSample - viewStartSample;
            int samplesPerPixel = juce::jmax (1, viewNumSamples / juce::jmax (1, widthInt));

            for (int i = 0; i < widthInt; ++i)
            {
                int relIdx = (i * viewNumSamples) / juce::jmax (1, widthInt);
                int startIdx = juce::jlimit (0, numSamples - 1, viewStartSample + relIdx);
                int endIdx = juce::jlimit (startIdx, numSamples - 1, startIdx + samplesPerPixel);
                float maxSample = 0.0f;
                for (int j = startIdx; j <= endIdx && j < numSamples; ++j)
                    maxSample = juce::jmax (maxSample, std::abs (samples[j]));

                // Apply normalization
                float normalizedSample = maxSample * normalizeFactor;
                
                float x = startX + static_cast<float> (i);
                float yTop = centerY - normalizedSample * height * 0.42f;
                float yBottom = centerY + normalizedSample * height * 0.42f;

                if (i == 0) { waveformTop.startNewSubPath (x, yTop); waveformBottom.startNewSubPath (x, yBottom); }
                else { waveformTop.lineTo (x, yTop); waveformBottom.lineTo (x, yBottom); }
            }

            // Fill
            juce::Path fillPath;
            fillPath.addPath (waveformTop);
            for (int i = widthInt - 1; i >= 0; --i)
            {
                int relIdx = (i * viewNumSamples) / juce::jmax (1, widthInt);
                int startIdx = juce::jlimit (0, numSamples - 1, viewStartSample + relIdx);
                int endIdx = juce::jlimit (startIdx, numSamples - 1, startIdx + samplesPerPixel);
                float maxSample = 0.0f;
                for (int j = startIdx; j <= endIdx && j < numSamples; ++j)
                    maxSample = juce::jmax (maxSample, std::abs (samples[j]));
                
                float normalizedSample = maxSample * normalizeFactor;
                fillPath.lineTo (startX + static_cast<float> (i), centerY + normalizedSample * height * 0.42f);
            }
            fillPath.closeSubPath();
            
            // Use waveformWidth for the actual width variable going forward
            float width = waveformWidth;

            juce::ColourGradient fillGradient (waveformColour.withAlpha (0.15f), bounds.getCentreX(), centerY - height * 0.4f,
                                               waveformColour.withAlpha (0.02f), bounds.getCentreX(), centerY, false);
            g.setGradientFill (fillGradient);
            g.fillPath (fillPath);

            g.setColour (waveformColour.withAlpha (0.06f));
            g.strokePath (waveformTop, juce::PathStrokeType (4.0f));
            g.strokePath (waveformBottom, juce::PathStrokeType (4.0f));

            g.setColour (waveformColour.withAlpha (0.55f));
            g.strokePath (waveformTop, juce::PathStrokeType (1.2f));
            g.strokePath (waveformBottom, juce::PathStrokeType (1.2f));

            g.setColour (waveformColour.withAlpha (0.08f));
            g.drawLine (startX, centerY, startX + width, centerY, 1.0f);
            
            // Calculate view range for scrollable waveforms
            float viewStartNorm = 0.0f;
            float viewEndNorm = 1.0f;
            if (needsScroll)
            {
                double maxScrollTime = durationSeconds - maxViewDuration;
                double viewStartTime = scrollOffset * maxScrollTime;
                viewStartNorm = static_cast<float> (viewStartTime / durationSeconds);
                viewEndNorm = static_cast<float> ((viewStartTime + maxViewDuration) / durationSeconds);
            }
            float viewRange = viewEndNorm - viewStartNorm;

            // Grain indicators (adjusted for scroll view)
            for (size_t i = 0; i < cachedGrainVisuals.size() && i < grainGlowIntensity.size(); ++i)
            {
                if (grainGlowIntensity[i] > 0.01f && std::isfinite (cachedGrainVisuals[i].position))
                {
                    float grainPos = cachedGrainVisuals[i].position;
                    // Check if grain is in visible range
                    if (grainPos >= viewStartNorm && grainPos <= viewEndNorm)
                    {
                        float relativePos = (grainPos - viewStartNorm) / viewRange;
                        float grainX = startX + juce::jlimit (0.0f, 1.0f, relativePos) * width;
                        if (std::isfinite (grainX))
                        {
                            drawGrainIndicator (g, grainX, cachedGrainVisuals[i].progress,
                                               cachedGrainVisuals[i].amplitude, grainGlowIntensity[i]);
                        }
                    }
                }
            }

            // Particles
            for (const auto& p : particles)
            {
                float alpha = p.life * 0.6f;
                g.setColour (p.colour.withAlpha (alpha));
                g.fillEllipse (p.x - p.size * 0.5f, p.y - p.size * 0.5f, p.size, p.size);
                g.setColour (p.colour.withAlpha (alpha * 0.25f));
                g.fillEllipse (p.x - p.size, p.y - p.size, p.size * 2.0f, p.size * 2.0f);
            }

            // Position marker with spray window (adjusted for scroll view)
            float clampedPosition = juce::jlimit (0.0f, 1.0f, positionMarker);
            
            // Check if marker is in visible range
            bool markerVisible = (clampedPosition >= viewStartNorm && clampedPosition <= viewEndNorm);
            if (markerVisible)
            {
                float relativeMarkerPos = (clampedPosition - viewStartNorm) / viewRange;
                float markerX = startX + relativeMarkerPos * width;
                
                if (std::isfinite (markerX))
                {
                    // Draw spray window (shows the range where grains can spawn)
                    if (sprayAmount > 0.001f)
                    {
                        float sprayInView = sprayAmount / viewRange;  // Adjust spray for view zoom
                        float sprayWidth = sprayInView * width;
                        float sprayLeft = juce::jmax (startX, markerX - sprayWidth);
                        float sprayRight = juce::jmin (startX + width, markerX + sprayWidth);
                        float sprayRectWidth = sprayRight - sprayLeft;
                        
                        if (sprayRectWidth > 0.0f)
                        {
                            // Soft gradient fill for spray region
                            juce::ColourGradient sprayGradient (markerColour.withAlpha (0.0f), sprayLeft, centerY,
                                                               markerColour.withAlpha (0.12f), markerX, centerY, false);
                            g.setGradientFill (sprayGradient);
                            g.fillRect (sprayLeft, bounds.getY() + 8.0f, markerX - sprayLeft, bounds.getHeight() - 16.0f);
                            
                            juce::ColourGradient sprayGradient2 (markerColour.withAlpha (0.12f), markerX, centerY,
                                                                markerColour.withAlpha (0.0f), sprayRight, centerY, false);
                            g.setGradientFill (sprayGradient2);
                            g.fillRect (markerX, bounds.getY() + 8.0f, sprayRight - markerX, bounds.getHeight() - 16.0f);
                            
                            // Edge lines for spray window
                            g.setColour (markerColour.withAlpha (0.2f));
                            g.drawLine (sprayLeft, bounds.getY() + 10.0f, sprayLeft, bounds.getBottom() - 10.0f, 1.0f);
                            g.drawLine (sprayRight, bounds.getY() + 10.0f, sprayRight, bounds.getBottom() - 10.0f, 1.0f);
                        }
                    }
                    
                    // Main position marker line
                    for (int i = 2; i >= 0; --i)
                    {
                        g.setColour (markerColour.withAlpha (0.1f * (1.0f - static_cast<float> (i) * 0.3f)));
                        g.drawLine (markerX, bounds.getY() + 10.0f, markerX, bounds.getBottom() - 10.0f, 3.0f + static_cast<float> (i) * 2.0f);
                    }
                    g.setColour (markerColour);
                    g.drawLine (markerX, bounds.getY() + 10.0f, markerX, bounds.getBottom() - 10.0f, 2.0f);

                    // Triangle head marker
                    juce::Path head;
                    head.addTriangle (markerX - 4.0f, bounds.getY() + 5.0f, markerX + 4.0f, bounds.getY() + 5.0f, markerX, bounds.getY() + 12.0f);
                    g.fillPath (head);
                }
            }
            
            // Draw scroll indicator for long samples
            if (needsScroll)
            {
                float scrollBarY = bounds.getBottom() - 6.0f;
                float scrollBarHeight = 3.0f;
                float scrollBarWidth = availableWidth * 0.3f;  // Scroll thumb width
                float scrollBarX = startX + scrollOffset * (availableWidth - scrollBarWidth);
                
                // Track
                g.setColour (waveformColour.withAlpha (0.1f));
                g.fillRoundedRectangle (startX, scrollBarY, availableWidth, scrollBarHeight, 1.5f);
                
                // Thumb
                g.setColour (waveformColour.withAlpha (0.4f));
                g.fillRoundedRectangle (scrollBarX, scrollBarY, scrollBarWidth, scrollBarHeight, 1.5f);
            }
        }
    }
    else
    {
        float pulseAlpha = isHovering ? 0.6f : (0.35f + 0.1f * std::sin (animationPhase * 2.0f));
        g.setColour (textColour.withAlpha (pulseAlpha));
        g.setFont (customFonts.getTitleFont (18.0f));
        g.drawText (isHovering ? "CLICK TO LOAD" : "DROP OR CLICK", bounds.translated (0, -10), juce::Justification::centred);
        g.setColour (textColour.withAlpha (pulseAlpha * 0.7f));
        g.setFont (customFonts.getLabelFont (12.0f));
        g.drawText ("to load audio file", bounds.translated (0, 12), juce::Justification::centred);

        // Hover glow effect
        if (isHovering)
        {
            g.setColour (waveformColour.withAlpha (0.05f));
            g.fillRoundedRectangle (bounds.reduced (2.0f), 4.0f);
        }
    }
}

void WaveformDisplay::resized() {}

bool WaveformDisplay::isInterestedInFileDrag (const juce::StringArray& files)
{
    for (auto& file : files)
        if (file.endsWithIgnoreCase (".wav") || file.endsWithIgnoreCase (".mp3") ||
            file.endsWithIgnoreCase (".aif") || file.endsWithIgnoreCase (".aiff") ||
            file.endsWithIgnoreCase (".flac") || file.endsWithIgnoreCase (".ogg"))
            return true;
    return false;
}

void WaveformDisplay::filesDropped (const juce::StringArray& files, int x, int y)
{
    juce::ignoreUnused (x, y);
    isDragOver = false;
    for (auto& filePath : files)
    {
        juce::File file (filePath);
        if (file.existsAsFile())
        {
            processor.loadSampleAsync (file);
            if (onFileDropped) onFileDropped();
            break;
        }
    }
    repaint();
}

void WaveformDisplay::fileDragEnter (const juce::StringArray&, int, int) { isDragOver = true; repaint(); }
void WaveformDisplay::fileDragExit (const juce::StringArray&) { isDragOver = false; repaint(); }

void WaveformDisplay::mouseDown (const juce::MouseEvent& event)
{
    lastDragX = static_cast<float> (event.x);
    
    // Only trigger file load if not in scroll mode, or if no sample loaded
    if (!needsScroll || !processor.hasSampleLoaded())
    {
        if (onClickToLoad)
            onClickToLoad();
    }
}

void WaveformDisplay::mouseEnter (const juce::MouseEvent&)
{
    isHovering = true;
    setMouseCursor (juce::MouseCursor::PointingHandCursor);
    repaint();
}

void WaveformDisplay::mouseExit (const juce::MouseEvent&)
{
    isHovering = false;
    setMouseCursor (juce::MouseCursor::NormalCursor);
    repaint();
}

void WaveformDisplay::mouseDrag (const juce::MouseEvent& event)
{
    if (needsScroll && processor.hasSampleLoaded())
    {
        float dragDelta = lastDragX - static_cast<float> (event.x);
        float width = static_cast<float> (getWidth()) - 20.0f;
        
        // Scroll by drag amount relative to visible portion
        scrollOffset += dragDelta / width * 0.5f;  // 0.5 makes scrolling feel natural
        scrollOffset = juce::jlimit (0.0f, 1.0f, scrollOffset);
        
        lastDragX = static_cast<float> (event.x);
        repaint();
    }
}

void WaveformDisplay::mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails& wheel)
{
    if (needsScroll && processor.hasSampleLoaded())
    {
        // Scroll horizontally with mouse wheel
        scrollOffset -= wheel.deltaX * 0.1f;  // Horizontal scroll
        scrollOffset -= wheel.deltaY * 0.05f; // Vertical scroll also works (for mice without horizontal)
        scrollOffset = juce::jlimit (0.0f, 1.0f, scrollOffset);
        repaint();
    }
}

//==============================================================================
// PluginEditor
//==============================================================================
PluginEditor::PluginEditor (PluginProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p), 
      keyboardComponent (keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard),
      waveformDisplay (p, customFonts)
{
    sliderLookAndFeel.setCustomFonts (&customFonts);
    setLookAndFeel (&sliderLookAndFeel);
    
    // Setup virtual MIDI keyboard
    keyboardState.addListener (this);
    keyboardComponent.setKeyWidth (21.0f);
    keyboardComponent.setAvailableRange (36, 96);  // C2 to C7 (5 octaves)
    keyboardComponent.setOctaveForMiddleC (4);
    keyboardComponent.setColour (juce::MidiKeyboardComponent::keyDownOverlayColourId, accentColour.withAlpha (0.6f));
    keyboardComponent.setColour (juce::MidiKeyboardComponent::whiteNoteColourId, juce::Colour (0xff1a1a22));
    keyboardComponent.setColour (juce::MidiKeyboardComponent::blackNoteColourId, juce::Colour (0xff050508));
    keyboardComponent.setColour (juce::MidiKeyboardComponent::keySeparatorLineColourId, juce::Colour (0xff252530));
    addAndMakeVisible (keyboardComponent);

    // Title
    titleLabel.setText ("WAVERO", juce::dontSendNotification);
    titleLabel.setFont (customFonts.getTitleFont (34.0f));
    titleLabel.setColour (juce::Label::textColourId, accentColour);
    titleLabel.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (titleLabel);

    subtitleLabel.setText ("GRANULAR", juce::dontSendNotification);
    subtitleLabel.setFont (customFonts.getLabelFont (11.0f));
    subtitleLabel.setColour (juce::Label::textColourId, dimTextColour);
    subtitleLabel.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (subtitleLabel);

    sampleNameLabel.setText ("No sample", juce::dontSendNotification);
    sampleNameLabel.setFont (customFonts.getLabelFont (12.0f));
    sampleNameLabel.setColour (juce::Label::textColourId, dimTextColour);
    sampleNameLabel.setJustificationType (juce::Justification::centredRight);
    addAndMakeVisible (sampleNameLabel);

    // Section labels
    grainLabel.setText ("GRAIN", juce::dontSendNotification);
    grainLabel.setFont (customFonts.getTitleFont (12.0f));
    grainLabel.setColour (juce::Label::textColourId, accentColour.withAlpha (0.7f));
    grainLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (grainLabel);

    envelopeLabel.setText ("ENVELOPE", juce::dontSendNotification);
    envelopeLabel.setFont (customFonts.getTitleFont (12.0f));
    envelopeLabel.setColour (juce::Label::textColourId, tertiaryColour.withAlpha (0.7f));
    envelopeLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (envelopeLabel);

    waveformDisplay.onFileDropped = [this] { updateSampleDisplay(); };
    waveformDisplay.onClickToLoad = [this] { loadSampleFile(); };
    addAndMakeVisible (waveformDisplay);

    // Buttons
    auto setupButton = [this] (juce::TextButton& btn, juce::Colour col)
    {
        btn.setLookAndFeel (&sliderLookAndFeel);
        btn.setColour (juce::TextButton::buttonColourId, panelColour.brighter (0.05f));
        btn.setColour (juce::TextButton::textColourOffId, col);
        addAndMakeVisible (btn);
    };

    setupButton (loadButton, accentColour);
    setupButton (savePresetButton, secondaryColour);
    setupButton (loadPresetButton, tertiaryColour);

    loadButton.onClick = [this] { loadSampleFile(); };
    savePresetButton.onClick = [this] { savePreset(); };
    loadPresetButton.onClick = [this] { loadPreset(); };

    // Sliders - grain section (0 = accent, 1 = secondary, 2 = tertiary)
    setupSlider (positionSlider, positionLabel, "POS", 0);
    setupSlider (grainSizeSlider, grainSizeLabel, "SIZE", 1);
    setupSlider (densitySlider, densityLabel, "DENS", 0);
    setupSlider (pitchSlider, pitchLabel, "PITCH", 1);
    setupSlider (spraySlider, sprayLabel, "SPRAY", 0);
    setupSlider (gainSlider, gainLabel, "GAIN", 1);

    // ADSR sliders
    setupSlider (attackSlider, attackLabel, "A", 2);
    setupSlider (decaySlider, decayLabel, "D", 2);
    setupSlider (sustainSlider, sustainLabel, "S", 2);
    setupSlider (releaseSlider, releaseLabel, "R", 2);

    // Attachments
    positionAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        processorRef.getParameters(), PluginProcessor::POSITION_ID, positionSlider);
    grainSizeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        processorRef.getParameters(), PluginProcessor::GRAIN_SIZE_ID, grainSizeSlider);
    densityAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        processorRef.getParameters(), PluginProcessor::DENSITY_ID, densitySlider);
    pitchAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        processorRef.getParameters(), PluginProcessor::PITCH_ID, pitchSlider);
    sprayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        processorRef.getParameters(), PluginProcessor::SPRAY_ID, spraySlider);
    gainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        processorRef.getParameters(), PluginProcessor::GAIN_ID, gainSlider);
    attackAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        processorRef.getParameters(), PluginProcessor::ATTACK_ID, attackSlider);
    decayAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        processorRef.getParameters(), PluginProcessor::DECAY_ID, decaySlider);
    sustainAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        processorRef.getParameters(), PluginProcessor::SUSTAIN_ID, sustainSlider);
    releaseAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (
        processorRef.getParameters(), PluginProcessor::RELEASE_ID, releaseSlider);
    
    // Add units/suffixes to sliders
    positionSlider.setTextValueSuffix ("");
    positionSlider.setNumDecimalPlacesToDisplay (2);
    grainSizeSlider.setTextValueSuffix (" ms");
    grainSizeSlider.setNumDecimalPlacesToDisplay (0);
    densitySlider.setTextValueSuffix (" Hz");
    densitySlider.setNumDecimalPlacesToDisplay (1);
    // Display pitch in semitones instead of multiplier
    pitchSlider.textFromValueFunction = [] (double value)
    {
        if (value <= 0.0) return juce::String ("0 st");
        double semitones = 12.0 * std::log2 (value);
        int rounded = static_cast<int> (std::round (semitones));
        if (rounded > 0)
            return juce::String ("+") + juce::String (rounded) + " st";
        return juce::String (rounded) + " st";
    };
    pitchSlider.valueFromTextFunction = [] (const juce::String& text)
    {
        // Parse semitones back to multiplier
        double semitones = text.trimCharactersAtEnd ("st ").getDoubleValue();
        return std::pow (2.0, semitones / 12.0);
    };
    pitchSlider.setNumDecimalPlacesToDisplay (0);
    spraySlider.setTextValueSuffix ("");
    spraySlider.setNumDecimalPlacesToDisplay (2);
    gainSlider.setTextValueSuffix ("");
    gainSlider.setNumDecimalPlacesToDisplay (2);
    attackSlider.setTextValueSuffix (" s");
    attackSlider.setNumDecimalPlacesToDisplay (2);
    decaySlider.setTextValueSuffix (" s");
    decaySlider.setNumDecimalPlacesToDisplay (2);
    sustainSlider.setTextValueSuffix ("");
    sustainSlider.setNumDecimalPlacesToDisplay (2);
    releaseSlider.setTextValueSuffix (" s");
    releaseSlider.setNumDecimalPlacesToDisplay (2);

#if JUCE_DEBUG
    inspectButton.setColour (juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    inspectButton.setColour (juce::TextButton::textColourOffId, dimTextColour);
    inspectButton.onClick = [&] {
        if (!inspector) { inspector = std::make_unique<melatonin::Inspector> (*this); inspector->onClose = [this]() { inspector.reset(); }; }
        inspector->setVisible (true);
    };
    addAndMakeVisible (inspectButton);
#endif

    startTimerHz (30);
    setSize (820, 635);  // Larger window to fit keyboard, sliders with labels, and footer
}

PluginEditor::~PluginEditor()
{
    keyboardState.removeListener (this);
    stopTimer();
    setLookAndFeel (nullptr);
}

void PluginEditor::handleNoteOn (juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity)
{
    auto message = juce::MidiMessage::noteOn (midiChannel, midiNoteNumber, velocity);
    processorRef.addMidiMessage (message);
}

void PluginEditor::handleNoteOff (juce::MidiKeyboardState*, int midiChannel, int midiNoteNumber, float velocity)
{
    auto message = juce::MidiMessage::noteOff (midiChannel, midiNoteNumber, velocity);
    processorRef.addMidiMessage (message);
}

void PluginEditor::setupSlider (juce::Slider& slider, juce::Label& label, const juce::String& labelText, int colourIndex)
{
    juce::Colour col = (colourIndex == 0) ? accentColour : (colourIndex == 1) ? secondaryColour : tertiaryColour;

    slider.setSliderStyle (juce::Slider::LinearVertical);
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 50, 14);
    slider.setColour (juce::Slider::trackColourId, col);
    slider.setColour (juce::Slider::textBoxTextColourId, dimTextColour);
    slider.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible (slider);

    label.setText (labelText, juce::dontSendNotification);
    label.setFont (customFonts.getTitleFont (10.0f));
    label.setColour (juce::Label::textColourId, dimTextColour);
    label.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (label);
}

void PluginEditor::loadSampleFile()
{
    auto chooser = std::make_shared<juce::FileChooser> (
        "Select audio file...",
        juce::File::getSpecialLocation (juce::File::userMusicDirectory),
        "*.wav;*.mp3;*.aif;*.aiff;*.flac;*.ogg");

    chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
        [this, chooser] (const juce::FileChooser& fc) {
            auto file = fc.getResult();
            if (file.existsAsFile()) { processorRef.loadSampleAsync (file); updateSampleDisplay(); }
        });
}

void PluginEditor::savePreset()
{
    auto presetDir = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory).getChildFile ("Wavero/Presets");
    presetDir.createDirectory();

    auto chooser = std::make_shared<juce::FileChooser> ("Save Preset...", presetDir, "*.wvpreset");
    chooser->launchAsync (juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
        [this, chooser] (const juce::FileChooser& fc) {
            auto file = fc.getResult();
            if (file != juce::File()) processorRef.savePreset (file.withFileExtension ("wvpreset"));
        });
}

void PluginEditor::loadPreset()
{
    auto presetDir = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory).getChildFile ("Wavero/Presets");
    auto chooser = std::make_shared<juce::FileChooser> ("Load Preset...", presetDir, "*.wvpreset");
    chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
        [this, chooser] (const juce::FileChooser& fc) {
            auto file = fc.getResult();
            if (file.existsAsFile()) { processorRef.loadPreset (file); updateSampleDisplay(); }
        });
}

void PluginEditor::updateSampleDisplay()
{
    if (processorRef.hasSampleLoaded())
        sampleNameLabel.setText (processorRef.getSampleName(), juce::dontSendNotification);
    waveformDisplay.repaint();
}

void PluginEditor::timerCallback()
{
    waveformDisplay.setPositionMarker (static_cast<float> (positionSlider.getValue()));
    waveformDisplay.setSprayAmount (static_cast<float> (spraySlider.getValue()));
    
    // Always check if sample name needs updating (handles async loading from presets)
    if (processorRef.hasSampleLoaded())
    {
        juce::String currentSampleName = processorRef.getSampleName();
        if (sampleNameLabel.getText() != currentSampleName)
            sampleNameLabel.setText (currentSampleName, juce::dontSendNotification);
    }
    else if (sampleNameLabel.getText() != "No sample")
    {
        sampleNameLabel.setText ("No sample", juce::dontSendNotification);
    }
}

void PluginEditor::paint (juce::Graphics& g)
{
    g.fillAll (backgroundColour);

    juce::ColourGradient gradient (juce::Colour (0x06ffffff), 0, 0,
                                   juce::Colours::transparentBlack,
                                   static_cast<float> (getWidth()),
                                   static_cast<float> (getHeight()), true);
    g.setGradientFill (gradient);
    g.fillAll();

    g.setColour (juce::Colour (0x04ffffff));
    for (int i = 0; i < getWidth(); i += 25)
        g.drawVerticalLine (i, 0.0f, static_cast<float> (getHeight()));
    for (int i = 0; i < getHeight(); i += 25)
        g.drawHorizontalLine (i, 0.0f, static_cast<float> (getWidth()));

    // Panels - must match resized() calculations exactly
    auto area = getLocalBounds().reduced (14);
    area.removeFromBottom (16);  // footer
    area.removeFromBottom (75);  // keyboard
    area.removeFromBottom (14);  // gap between keyboard and sliders
    area.removeFromTop (42);     // header
    area.removeFromTop (8);      // gap
    area.removeFromTop (165);    // waveform
    area.removeFromTop (12);     // gap

    auto slidersArea = area;

    // Art Deco corner accent helper - inset enough to clear the 6px rounded corners
    auto drawDecoCorners = [&g] (juce::Rectangle<float> bounds, juce::Colour col, float size = 12.0f)
    {
        g.setColour (col);
        float inset = 10.0f;  // Increased inset to clear rounded corners
        // Top-left
        g.drawLine (bounds.getX() + inset, bounds.getY() + inset, bounds.getX() + inset + size, bounds.getY() + inset, 1.5f);
        g.drawLine (bounds.getX() + inset, bounds.getY() + inset, bounds.getX() + inset, bounds.getY() + inset + size, 1.5f);
        // Top-right
        g.drawLine (bounds.getRight() - inset, bounds.getY() + inset, bounds.getRight() - inset - size, bounds.getY() + inset, 1.5f);
        g.drawLine (bounds.getRight() - inset, bounds.getY() + inset, bounds.getRight() - inset, bounds.getY() + inset + size, 1.5f);
        // Bottom-left
        g.drawLine (bounds.getX() + inset, bounds.getBottom() - inset, bounds.getX() + inset + size, bounds.getBottom() - inset, 1.5f);
        g.drawLine (bounds.getX() + inset, bounds.getBottom() - inset, bounds.getX() + inset, bounds.getBottom() - inset - size, 1.5f);
        // Bottom-right
        g.drawLine (bounds.getRight() - inset, bounds.getBottom() - inset, bounds.getRight() - inset - size, bounds.getBottom() - inset, 1.5f);
        g.drawLine (bounds.getRight() - inset, bounds.getBottom() - inset, bounds.getRight() - inset, bounds.getBottom() - inset - size, 1.5f);
    };

    // Grain panel
    auto grainPanel = slidersArea.removeFromLeft (static_cast<int> (slidersArea.getWidth() * 0.6f)).reduced (2, 0);
    g.setColour (panelColour);
    g.fillRoundedRectangle (grainPanel.toFloat(), 6.0f);
    g.setColour (accentColour.withAlpha (0.08f));
    g.drawRoundedRectangle (grainPanel.toFloat(), 6.0f, 1.0f);
    drawDecoCorners (grainPanel.toFloat(), accentColour.withAlpha (0.25f));

    slidersArea.removeFromLeft (8);

    // Envelope panel
    auto envPanel = slidersArea.toFloat();
    g.setColour (panelColour);
    g.fillRoundedRectangle (envPanel, 6.0f);
    g.setColour (tertiaryColour.withAlpha (0.08f));
    g.drawRoundedRectangle (envPanel, 6.0f, 1.0f);
    drawDecoCorners (envPanel, tertiaryColour.withAlpha (0.25f));

    // Version - at the very bottom below keyboard
    g.setColour (dimTextColour.withAlpha (0.5f));
    g.setFont (customFonts.getLabelFont (10.0f));
    auto versionArea = getLocalBounds().reduced (14).removeFromBottom (14);
    g.drawText ("v" VERSION " // MIDI Ch 1", versionArea, juce::Justification::centredRight);
}

void PluginEditor::resized()
{
    auto area = getLocalBounds().reduced (14);
    
    // Reserve space for version footer at very bottom
    area.removeFromBottom (16);
    
    // Reserve space for keyboard at bottom
    auto keyboardArea = area.removeFromBottom (75);
    keyboardComponent.setBounds (keyboardArea);
    area.removeFromBottom (14);  // Gap between keyboard and sliders

    // Header
    auto header = area.removeFromTop (42);
    auto titleCol = header.removeFromLeft (150);
    titleLabel.setBounds (titleCol.removeFromTop (28));
    subtitleLabel.setBounds (titleCol);

    auto btnArea = header.removeFromRight (200);
    loadButton.setBounds (btnArea.removeFromLeft (62).reduced (2));
    savePresetButton.setBounds (btnArea.removeFromLeft (62).reduced (2));
    loadPresetButton.setBounds (btnArea.reduced (2));

    sampleNameLabel.setBounds (header.reduced (4, 10));

    area.removeFromTop (8);
    waveformDisplay.setBounds (area.removeFromTop (165));
    area.removeFromTop (12);

    auto slidersArea = area;

    // Grain section
    auto grainSection = slidersArea.removeFromLeft (static_cast<int> (slidersArea.getWidth() * 0.6f)).reduced (4);
    grainLabel.setBounds (grainSection.removeFromTop (14));
    int grainSliderW = grainSection.getWidth() / 6;

    auto placeSlider = [&] (juce::Slider& slider, juce::Label& label, juce::Rectangle<int>& row, int w)
    {
        auto col = row.removeFromLeft (w);
        label.setBounds (col.removeFromTop (14));
        col.removeFromTop (2);   // Small gap after label
        col.removeFromBottom (8); // Keep slider value text above corner accents
        slider.setBounds (col.reduced (4, 0));
    };

    placeSlider (positionSlider, positionLabel, grainSection, grainSliderW);
    placeSlider (grainSizeSlider, grainSizeLabel, grainSection, grainSliderW);
    placeSlider (densitySlider, densityLabel, grainSection, grainSliderW);
    placeSlider (pitchSlider, pitchLabel, grainSection, grainSliderW);
    placeSlider (spraySlider, sprayLabel, grainSection, grainSliderW);
    placeSlider (gainSlider, gainLabel, grainSection, grainSliderW);

    slidersArea.removeFromLeft (8);

    // Envelope section
    auto envSection = slidersArea.reduced (4);
    envelopeLabel.setBounds (envSection.removeFromTop (14));
    int envSliderW = envSection.getWidth() / 4;

    placeSlider (attackSlider, attackLabel, envSection, envSliderW);
    placeSlider (decaySlider, decayLabel, envSection, envSliderW);
    placeSlider (sustainSlider, sustainLabel, envSection, envSliderW);
    placeSlider (releaseSlider, releaseLabel, envSection, envSliderW);

#if JUCE_DEBUG
    inspectButton.setBounds (getWidth() - 40, getHeight() - 90, 32, 14);
#endif
}
