#include "fps_counter.h"
#include <sstream>
#include <iomanip>
#include <numeric>
#include <memory>

FPSCounter::FPSCounter() : font_loaded(false) {
}

bool FPSCounter::initialize() {
    // Try to load system font (this might fail on some systems)
    if (font.loadFromFile("/System/Library/Fonts/Helvetica.ttc") ||  // macOS
        font.loadFromFile("/Library/Fonts/Arial.ttf") ||  // macOS alternative
        font.loadFromFile("/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf") ||  // Linux
        font.loadFromFile("C:\\Windows\\Fonts\\arial.ttf")) {  // Windows
        
        font_loaded = true;
        fps_text = std::make_unique<sf::Text>("FPS: --", font, 24);
        fps_text->setFillColor(sf::Color::Black); // Black text on white background
        fps_text->setStyle(sf::Text::Bold);
        fps_text->setPosition(sf::Vector2f(10, 10));
        return true;
    }
    
    // If we can't load system fonts, we'll proceed without text display
    return false;
}

void FPSCounter::update() {
    auto now = std::chrono::high_resolution_clock::now();
    
    // Add current time to the queue
    frame_times.push_back(now);
    
    // Remove old samples
    while (frame_times.size() > MAX_SAMPLES) {
        frame_times.pop_front();
    }
    
    // Update FPS text if font is loaded
    if (font_loaded && fps_text) {
        float current_fps = getCurrentFPS();
        
        std::ostringstream oss;
        oss << std::fixed << std::setprecision(1);
        oss << "FPS: " << current_fps;
        oss << " | Particles: 10000";
        oss << "\nMatrix ops per frame: 280x280";
        
        // Show which optimization is active
#ifdef USE_SLOW_MATRIX
        oss << " | SLOW";
#elif defined(USE_FAST_MATRIX)
        oss << " | FAST";
#else
        oss << " | DEFAULT";
#endif
        
        fps_text->setString(oss.str());
    }
}

void FPSCounter::render(sf::RenderWindow& window) {
    if (font_loaded && fps_text) {
        // Draw semi-transparent background
        sf::RectangleShape background(sf::Vector2f(250, 70));
        background.setPosition(sf::Vector2f(5, 5));
        background.setFillColor(sf::Color(255, 255, 255, 200)); // Light background for better readability
        window.draw(background);
        
        // Draw FPS text
        window.draw(*fps_text);
    }
}

float FPSCounter::getCurrentFPS() const {
    if (frame_times.size() < 2) {
        return 0.0f;
    }
    
    auto duration = frame_times.back() - frame_times.front();
    auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
    
    if (milliseconds <= 0) {
        return 0.0f;
    }
    
    float fps = (frame_times.size() - 1) * 1000.0f / milliseconds;
    return fps;
} 