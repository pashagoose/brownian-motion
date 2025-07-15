#include <SFML/Graphics.hpp>
#include <iostream>
#include <chrono>

#include "simulation.h"
#include "fps_counter.h"

constexpr int WINDOW_WIDTH = 1200;
constexpr int WINDOW_HEIGHT = 800;
constexpr int PARTICLE_COUNT = 10000; // Extreme particle count for maximum performance impact

int main() {
    // Debug: Print SFML version
    std::cout << "SFML Version: " << SFML_VERSION_MAJOR << "." << SFML_VERSION_MINOR << "." << SFML_VERSION_PATCH << std::endl;
    
    // Create window - SFML 2.x compatible syntax
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), 
                           "Brownian Motion Simulation - C++ Zero Cost Conf Demo");
    window.setFramerateLimit(0); // No limit - we want to see the real performance
    window.setVerticalSyncEnabled(false); // Disable V-Sync for maximum performance
    
    if (!window.isOpen()) {
        std::cout << "Error: Could not create window!" << std::endl;
        return -1;
    }
    
    // Initialize components
    BrownianSimulation simulation(WINDOW_WIDTH, WINDOW_HEIGHT, PARTICLE_COUNT);
    FPSCounter fps_counter;
    
    if (!fps_counter.initialize()) {
        std::cout << "Warning: Could not load font for FPS counter\n";
    }
    
    // Timing variables
    auto last_time = std::chrono::high_resolution_clock::now();
    
    std::cout << "Brownian Motion Simulation Started\n";
    std::cout << "Particles: " << simulation.getParticleCount() << "\n";
    std::cout << "Window: " << WINDOW_WIDTH << "x" << WINDOW_HEIGHT << "\n";
    std::cout << "Press ESC to exit\n";
    
    // Main game loop
    while (window.isOpen()) {
        // Calculate delta time
        auto current_time = std::chrono::high_resolution_clock::now();
        float delta_time = std::chrono::duration<float>(current_time - last_time).count();
        last_time = current_time;
        
        // Only limit delta time for extremely large jumps (e.g., when debugging/pausing)
        // but allow much higher FPS for performance testing
        if (delta_time > 0.5f) {
            delta_time = 0.016f; // Only cap at 2 FPS to prevent huge simulation jumps
        }
        
        // Handle events - SFML 2.x compatible syntax
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                std::cout << "Window closed by user\n";
                window.close();
            }
            
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Escape) {
                    std::cout << "Escape pressed - exiting\n";
                    window.close();
                } else if (event.key.code == sf::Keyboard::Space) {
                    simulation.resetParticles();
                    std::cout << "Simulation reset\n";
                }
            }
        }
        
        // Update simulation
        simulation.update(delta_time);
        fps_counter.update();
        
        // Render everything
        window.clear(sf::Color::White);
        
        simulation.render(window);
        fps_counter.render(window);
        
        window.display();
    }
    
    std::cout << "Simulation ended\n";
    return 0;
} 