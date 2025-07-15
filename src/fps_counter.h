#pragma once

#include <SFML/Graphics.hpp>
#include <chrono>
#include <deque>
#include <memory>

class FPSCounter {
private:
    std::deque<std::chrono::high_resolution_clock::time_point> frame_times;
    sf::Font font;
    std::unique_ptr<sf::Text> fps_text;
    bool font_loaded;
    
    static constexpr int MAX_SAMPLES = 60;
    
public:
    FPSCounter();
    
    bool initialize(); // Load font and setup text
    void update(); // Call this every frame
    void render(sf::RenderWindow& window);
    
    float getCurrentFPS() const;
}; 