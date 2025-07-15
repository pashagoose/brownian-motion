#include <SFML/Graphics.hpp>
#include <iostream>
#include <chrono>
#include <string>
#include <iomanip>
#include <signal.h>

#include "simulation.h"
#include "fps_counter.h"

constexpr int WINDOW_WIDTH = 1200;
constexpr int WINDOW_HEIGHT = 800;
constexpr int PARTICLE_COUNT = 10000; // Extreme particle count for maximum performance impact

// Global variables for signal handling
static volatile bool running = true;
static int total_frames = 0;
static std::chrono::high_resolution_clock::time_point start_time;

void signalHandler(int signal) {
    if (signal == SIGINT) {
        std::cout << "\n\n=== INTERRUPTED BY USER ===" << std::endl;
        auto end_time = std::chrono::high_resolution_clock::now();
        float total_duration = std::chrono::duration<float>(end_time - start_time).count();
        float avg_fps = total_frames / total_duration;
        
        std::cout << "Average FPS: " << std::fixed << std::setprecision(1) << avg_fps << std::endl;
        std::cout << "Total frames: " << total_frames << std::endl;
        std::cout << "Duration: " << std::fixed << std::setprecision(1) << total_duration << " seconds" << std::endl;
        
        running = false;
    }
}

void runHeadlessMode() {
    std::cout << "=== HEADLESS MODE ===" << std::endl;
    std::cout << "Particles: " << PARTICLE_COUNT << std::endl;
    std::cout << "Matrix operations: 280x280 per frame" << std::endl;
    std::cout << "Running indefinitely... Press Ctrl+C to stop and see results" << std::endl;
    
    // Setup signal handler
    signal(SIGINT, signalHandler);
    
    // Initialize simulation (no window needed)
    BrownianSimulation simulation(WINDOW_WIDTH, WINDOW_HEIGHT, PARTICLE_COUNT);
    
    start_time = std::chrono::high_resolution_clock::now();
    auto last_time = start_time;
    auto last_fps_time = start_time;
    
    int frame_count = 0;
    total_frames = 0;
    
    // Run until interrupted
    while (running) {
        auto current_time = std::chrono::high_resolution_clock::now();
        float delta_time = std::chrono::duration<float>(current_time - last_time).count();
        last_time = current_time;
        
        // Limit delta time for consistent simulation
        if (delta_time > 0.1f) {
            delta_time = 0.016f;
        }
        
        // Update simulation (this is where the matrix operations happen)
        simulation.update(delta_time);
        
        frame_count++;
        total_frames++;
        
        // Print FPS every second
        auto fps_duration = std::chrono::duration<float>(current_time - last_fps_time).count();
        if (fps_duration >= 1.0f) {
            float fps = frame_count / fps_duration;
            std::cout << "FPS: " << std::fixed << std::setprecision(1) << fps 
                     << " | Total frames: " << total_frames << std::endl;
            frame_count = 0;
            last_fps_time = current_time;
        }
    }
}

int main(int argc, char* argv[]) {
    // Parse command line arguments
    bool headless = false;
    for (int i = 1; i < argc; i++) {
        if (std::string(argv[i]) == "-no-visualize" || std::string(argv[i]) == "--no-visualize") {
            headless = true;
            break;
        }
    }
    
    if (headless) {
        runHeadlessMode();
        return 0;
    }
    
    // Original graphical mode
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