#include "PluginEditor.h"

//==============================================================================
// FuturisticSliderLookAndFeel
//==============================================================================
FuturisticSliderLookAndFeel::FuturisticSliderLookAndFeel()
{
    setColour (juce::Label::textColourId, juce::Colour (0xffc0c0c0));
}

void FuturisticSliderLookAndFeel::drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                                                     float sliderPos, float minSliderPos, float maxSliderPos,
                                                     juce::Slider::SliderStyle style, juce::Slider& slider)
{
    juce::ignoreUnused (minSliderPos, maxSliderPos, style);

    auto bounds = juce::Rectangle<float> (static_cast<float> (x), static_cast<float> (y),
                                          static_cast<float> (width), static_cast<float> (height));
    
    // Ensure valid bounds
    if (bounds.getWidth() <= 0.0f || bounds.getHeight() <= 0.0f)
        return;
    
    auto trackWidth = 4.0f;
    auto trackX = bounds.getCentreX() - trackWidth * 0.5f;
    auto fillColour = slider.findColour (juce::Slider::trackColourId);

    // Track background
    g.setColour (juce::Colour (0xff151520));
    g.fillRoundedRectangle (trackX - 2.0f, bounds.getY(), trackWidth + 4.0f, bounds.getHeight(), 3.0f);

    float trackHeight = bounds.getHeight() - 4.0f;
    if (trackHeight > 0.0f)
    {
        g.setColour (juce::Colour (0xff000005));
        g.fillRoundedRectangle (trackX, bounds.getY() + 2.0f, trackWidth, trackHeight, 2.0f);
    }

    // Filled portion - ensure sliderPos is within valid bounds
    float clampedSliderPos = juce::jlimit (bounds.getY(), bounds.getBottom(), sliderPos);
    float filledHeight = bounds.getBottom() - clampedSliderPos;

    if (filledHeight > 1.0f)  // Only draw if there's meaningful height
    {
        for (int i = 3; i >= 0; --i)
        {
            float alpha = 0.08f * (1.0f - static_cast<float> (i) * 0.25f);
            float spread = static_cast<float> (i) * 3.0f;
            g.setColour (fillColour.withAlpha (alpha));
            g.fillRoundedRectangle (trackX - spread, clampedSliderPos, trackWidth + spread * 2.0f, filledHeight, 2.0f);
        }

        juce::ColourGradient fillGradient (fillColour.brighter (0.2f), trackX, clampedSliderPos,
                                           fillColour.darker (0.3f), trackX, bounds.getBottom(), false);
        g.setGradientFill (fillGradient);
        g.fillRoundedRectangle (trackX, clampedSliderPos, trackWidth, filledHeight, 2.0f);

        // Only draw highlight if there's enough height
        float highlightHeight = filledHeight - 4.0f;
        if (highlightHeight > 0.0f)
        {
            g.setColour (fillColour.brighter (0.5f).withAlpha (0.6f));
            g.fillRoundedRectangle (trackX + trackWidth * 0.3f, clampedSliderPos + 2.0f,
                                    trackWidth * 0.4f, highlightHeight, 1.0f);
        }
    }

    // Thumb - clamp position to valid range
    float thumbHeight = 10.0f;
    float thumbWidth = 20.0f;
    float thumbY = juce::jlimit (bounds.getY(), bounds.getBottom() - thumbHeight, clampedSliderPos - thumbHeight * 0.5f);
    float thumbX = bounds.getCentreX() - thumbWidth * 0.5f;

    g.setColour (fillColour.withAlpha (0.25f));
    g.fillRoundedRectangle (thumbX - 3.0f, thumbY - 2.0f, thumbWidth + 6.0f, thumbHeight + 4.0f, 4.0f);

    juce::ColourGradient thumbGradient (juce::Colour (0xff252535), thumbX, thumbY,
                                        juce::Colour (0xff151520), thumbX, thumbY + thumbHeight, false);
    g.setGradientFill (thumbGradient);
    g.fillRoundedRectangle (thumbX, thumbY, thumbWidth, thumbHeight, 3.0f);

    g.setColour (fillColour);
    g.fillRoundedRectangle (thumbX + 2.0f, thumbY + thumbHeight * 0.5f - 1.0f, thumbWidth - 4.0f, 2.0f, 1.0f);

    g.setColour (fillColour.withAlpha (0.4f));
    g.drawRoundedRectangle (thumbX, thumbY, thumbWidth, thumbHeight, 3.0f, 1.0f);

    // Tick marks
    g.setColour (juce::Colour (0xff252530));
    for (int i = 0; i <= 10; ++i)
    {
        float tickY = bounds.getY() + (bounds.getHeight() * static_cast<float> (i) / 10.0f);
        float tickW = (i % 5 == 0) ? 6.0f : 3.0f;
        g.fillRect (trackX - tickW - 3.0f, tickY - 0.5f, tickW, 1.0f);
    }
}

juce::Label* FuturisticSliderLookAndFeel::createSliderTextBox (juce::Slider& slider)
{
    auto* label = juce::LookAndFeel_V4::createSliderTextBox (slider);
    label->setColour (juce::Label::textColourId, juce::Colour (0xffa0a0a0));
    label->setColour (juce::Label::backgroundColourId, juce::Colours::transparentBlack);
    label->setColour (juce::Label::outlineColourId, juce::Colours::transparentBlack);
    label->setFont (juce::FontOptions (10.0f));
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

//==============================================================================
// WaveformDisplay
//==============================================================================
WaveformDisplay::WaveformDisplay (PluginProcessor& p) : processor (p)
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

    // Corner accents
    float cs = 12.0f;
    g.setColour (waveformColour.withAlpha (0.35f));
    g.drawLine (bounds.getX(), bounds.getY() + cs, bounds.getX(), bounds.getY(), 1.5f);
    g.drawLine (bounds.getX(), bounds.getY(), bounds.getX() + cs, bounds.getY(), 1.5f);
    g.drawLine (bounds.getRight(), bounds.getY() + cs, bounds.getRight(), bounds.getY(), 1.5f);
    g.drawLine (bounds.getRight(), bounds.getY(), bounds.getRight() - cs, bounds.getY(), 1.5f);
    g.drawLine (bounds.getX(), bounds.getBottom() - cs, bounds.getX(), bounds.getBottom(), 1.5f);
    g.drawLine (bounds.getX(), bounds.getBottom(), bounds.getX() + cs, bounds.getBottom(), 1.5f);
    g.drawLine (bounds.getRight(), bounds.getBottom() - cs, bounds.getRight(), bounds.getBottom(), 1.5f);
    g.drawLine (bounds.getRight(), bounds.getBottom(), bounds.getRight() - cs, bounds.getBottom(), 1.5f);

    if (processor.hasSampleLoaded())
    {
        auto& buffer = processor.getSampleBuffer();
        int numSamples = buffer.getNumSamples();
        int numChannels = buffer.getNumChannels();

        if (numSamples > 0 && numChannels > 0)
        {
            auto* samples = buffer.getReadPointer (0);
            float width = bounds.getWidth() - 20.0f;
            float height = bounds.getHeight() - 20.0f;
            float centerY = bounds.getCentreY();
            float startX = bounds.getX() + 10.0f;

            juce::Path waveformTop, waveformBottom;

            int widthInt = static_cast<int> (width);
            int samplesPerPixel = juce::jmax (1, numSamples / juce::jmax (1, widthInt));

            for (int i = 0; i < widthInt; ++i)
            {
                int startIdx = juce::jlimit (0, numSamples - 1, (i * numSamples) / juce::jmax (1, widthInt));
                int endIdx = juce::jlimit (startIdx, numSamples - 1, startIdx + samplesPerPixel);
                float maxSample = 0.0f;
                for (int j = startIdx; j <= endIdx && j < numSamples; ++j)
                    maxSample = juce::jmax (maxSample, std::abs (samples[j]));

                float x = startX + static_cast<float> (i);
                float yTop = centerY - maxSample * height * 0.42f;
                float yBottom = centerY + maxSample * height * 0.42f;

                if (i == 0) { waveformTop.startNewSubPath (x, yTop); waveformBottom.startNewSubPath (x, yBottom); }
                else { waveformTop.lineTo (x, yTop); waveformBottom.lineTo (x, yBottom); }
            }

            // Fill
            juce::Path fillPath;
            fillPath.addPath (waveformTop);
            for (int i = widthInt - 1; i >= 0; --i)
            {
                int startIdx = juce::jlimit (0, numSamples - 1, (i * numSamples) / juce::jmax (1, widthInt));
                int endIdx = juce::jlimit (startIdx, numSamples - 1, startIdx + samplesPerPixel);
                float maxSample = 0.0f;
                for (int j = startIdx; j <= endIdx && j < numSamples; ++j)
                    maxSample = juce::jmax (maxSample, std::abs (samples[j]));
                fillPath.lineTo (startX + static_cast<float> (i), centerY + maxSample * height * 0.42f);
            }
            fillPath.closeSubPath();

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

            // Grain indicators
            for (size_t i = 0; i < cachedGrainVisuals.size() && i < grainGlowIntensity.size(); ++i)
            {
                if (grainGlowIntensity[i] > 0.01f && std::isfinite (cachedGrainVisuals[i].position))
                {
                    float grainX = startX + juce::jlimit (0.0f, 1.0f, cachedGrainVisuals[i].position) * width;
                    if (std::isfinite (grainX))
                    {
                        drawGrainIndicator (g, grainX, cachedGrainVisuals[i].progress,
                                           cachedGrainVisuals[i].amplitude, grainGlowIntensity[i]);
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

            // Position marker with spray window
            float clampedPosition = juce::jlimit (0.0f, 1.0f, positionMarker);
            float markerX = startX + clampedPosition * width;
            if (std::isfinite (markerX))
            {
                // Draw spray window (shows the range where grains can spawn)
                if (sprayAmount > 0.001f)
                {
                    float sprayWidth = sprayAmount * width;  // Spray is 0-1, convert to pixels
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
    }
    else
    {
        float pulseAlpha = isHovering ? 0.5f : (0.25f + 0.1f * std::sin (animationPhase * 2.0f));
        g.setColour (textColour.withAlpha (pulseAlpha));
        g.setFont (juce::FontOptions (14.0f).withStyle ("Bold"));
        g.drawText (isHovering ? "CLICK TO LOAD" : "DROP OR CLICK", bounds.translated (0, -10), juce::Justification::centred);
        g.setColour (textColour.withAlpha (pulseAlpha * 0.6f));
        g.setFont (juce::FontOptions (10.0f));
        g.drawText ("to load audio file", bounds.translated (0, 10), juce::Justification::centred);

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

void WaveformDisplay::mouseDown (const juce::MouseEvent&)
{
    if (onClickToLoad)
        onClickToLoad();
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

//==============================================================================
// PluginEditor
//==============================================================================
PluginEditor::PluginEditor (PluginProcessor& p)
    : AudioProcessorEditor (&p), processorRef (p), 
      keyboardComponent (keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard),
      waveformDisplay (p)
{
    setLookAndFeel (&sliderLookAndFeel);
    
    // Setup virtual MIDI keyboard
    keyboardState.addListener (this);
    keyboardComponent.setKeyWidth (22.0f);
    keyboardComponent.setScrollButtonWidth (12);
    keyboardComponent.setAvailableRange (36, 96);  // C2 to C7
    keyboardComponent.setOctaveForMiddleC (4);
    keyboardComponent.setColour (juce::MidiKeyboardComponent::keyDownOverlayColourId, accentColour.withAlpha (0.6f));
    keyboardComponent.setColour (juce::MidiKeyboardComponent::whiteNoteColourId, juce::Colour (0xff1a1a22));
    keyboardComponent.setColour (juce::MidiKeyboardComponent::blackNoteColourId, juce::Colour (0xff050508));
    keyboardComponent.setColour (juce::MidiKeyboardComponent::keySeparatorLineColourId, juce::Colour (0xff252530));
    addAndMakeVisible (keyboardComponent);

    // Title
    titleLabel.setText ("WAVERO", juce::dontSendNotification);
    titleLabel.setFont (juce::FontOptions (28.0f).withStyle ("Bold"));
    titleLabel.setColour (juce::Label::textColourId, accentColour);
    titleLabel.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (titleLabel);

    subtitleLabel.setText ("GRANULAR", juce::dontSendNotification);
    subtitleLabel.setFont (juce::FontOptions (9.0f).withStyle ("Bold"));
    subtitleLabel.setColour (juce::Label::textColourId, dimTextColour);
    subtitleLabel.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (subtitleLabel);

    sampleNameLabel.setText ("No sample", juce::dontSendNotification);
    sampleNameLabel.setFont (juce::FontOptions (10.0f));
    sampleNameLabel.setColour (juce::Label::textColourId, dimTextColour);
    sampleNameLabel.setJustificationType (juce::Justification::centredRight);
    addAndMakeVisible (sampleNameLabel);

    // Section labels
    grainLabel.setText ("GRAIN", juce::dontSendNotification);
    grainLabel.setFont (juce::FontOptions (9.0f).withStyle ("Bold"));
    grainLabel.setColour (juce::Label::textColourId, accentColour.withAlpha (0.6f));
    grainLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (grainLabel);

    envelopeLabel.setText ("ENVELOPE", juce::dontSendNotification);
    envelopeLabel.setFont (juce::FontOptions (9.0f).withStyle ("Bold"));
    envelopeLabel.setColour (juce::Label::textColourId, tertiaryColour.withAlpha (0.6f));
    envelopeLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (envelopeLabel);

    waveformDisplay.onFileDropped = [this] { updateSampleDisplay(); };
    waveformDisplay.onClickToLoad = [this] { loadSampleFile(); };
    addAndMakeVisible (waveformDisplay);

    // Buttons
    auto setupButton = [this] (juce::TextButton& btn, juce::Colour col)
    {
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
    pitchSlider.setTextValueSuffix ("x");
    pitchSlider.setNumDecimalPlacesToDisplay (2);
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
    setSize (700, 510);  // Taller to fit keyboard
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
    slider.setColour (juce::Slider::textBoxTextColourId, textColour);
    slider.setColour (juce::Slider::textBoxOutlineColourId, juce::Colours::transparentBlack);
    addAndMakeVisible (slider);

    label.setText (labelText, juce::dontSendNotification);
    label.setFont (juce::FontOptions (8.0f).withStyle ("Bold"));
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

    // Panels
    auto area = getLocalBounds().reduced (12);
    area.removeFromTop (36);
    area.removeFromTop (6);
    area.removeFromTop (140);
    area.removeFromTop (10);

    auto slidersArea = area;

    // Grain panel
    auto grainPanel = slidersArea.removeFromLeft (static_cast<int> (slidersArea.getWidth() * 0.6f)).reduced (2, 0);
    g.setColour (panelColour);
    g.fillRoundedRectangle (grainPanel.toFloat(), 6.0f);
    g.setColour (accentColour.withAlpha (0.08f));
    g.drawRoundedRectangle (grainPanel.toFloat(), 6.0f, 1.0f);

    slidersArea.removeFromLeft (8);

    // Envelope panel
    g.setColour (panelColour);
    g.fillRoundedRectangle (slidersArea.toFloat(), 6.0f);
    g.setColour (tertiaryColour.withAlpha (0.08f));
    g.drawRoundedRectangle (slidersArea.toFloat(), 6.0f, 1.0f);

    // Version
    g.setColour (dimTextColour.withAlpha (0.4f));
    g.setFont (juce::FontOptions (8.0f));
    g.drawText ("v" VERSION " // MIDI Ch 1", getLocalBounds().reduced (8), juce::Justification::bottomRight);
}

void PluginEditor::resized()
{
    auto area = getLocalBounds().reduced (12);
    
    // Reserve space for keyboard at bottom
    auto keyboardArea = area.removeFromBottom (65);
    keyboardComponent.setBounds (keyboardArea);
    area.removeFromBottom (6);

    // Header
    auto header = area.removeFromTop (36);
    auto titleCol = header.removeFromLeft (130);
    titleLabel.setBounds (titleCol.removeFromTop (24));
    subtitleLabel.setBounds (titleCol);

    auto btnArea = header.removeFromRight (180);
    loadButton.setBounds (btnArea.removeFromLeft (55).reduced (2));
    savePresetButton.setBounds (btnArea.removeFromLeft (55).reduced (2));
    loadPresetButton.setBounds (btnArea.reduced (2));

    sampleNameLabel.setBounds (header.reduced (4, 8));

    area.removeFromTop (6);
    waveformDisplay.setBounds (area.removeFromTop (140));
    area.removeFromTop (10);

    auto slidersArea = area;

    // Grain section
    auto grainSection = slidersArea.removeFromLeft (static_cast<int> (slidersArea.getWidth() * 0.6f)).reduced (4);
    grainLabel.setBounds (grainSection.removeFromTop (14));
    int grainSliderW = grainSection.getWidth() / 6;

    auto placeSlider = [&] (juce::Slider& slider, juce::Label& label, juce::Rectangle<int>& row, int w)
    {
        auto col = row.removeFromLeft (w);
        label.setBounds (col.removeFromTop (14));
        col.removeFromBottom (2);
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
