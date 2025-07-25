#include <SFML/Graphics.hpp>
#include <iostream>
#include <chrono>
#include <string>
#include <iomanip>
#include <signal.h>
#include <thread>

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
    float total_frame_time = 0.0f;
    
    // Run until interrupted
    while (running) {
        auto frame_start = std::chrono::high_resolution_clock::now();
        auto current_time = frame_start;
        float delta_time = std::chrono::duration<float>(current_time - last_time).count();
        last_time = current_time;
        
        // Update simulation (this is where the matrix operations happen)
        simulation.update(delta_time);
        
        auto frame_end = std::chrono::high_resolution_clock::now();
        float frame_time = std::chrono::duration<float>(frame_end - frame_start).count();
        total_frame_time += frame_time;
        
        frame_count++;
        total_frames++;
        
        // Print FPS every second (simple frames per second calculation)
        auto fps_duration = std::chrono::duration<float>(current_time - last_fps_time).count();
        if (fps_duration >= 1.0f) {
            float fps = frame_count / fps_duration;
            float avg_frame_time = total_frame_time / frame_count;
            std::cout << "FPS: " << std::fixed << std::setprecision(1) << fps 
                     << " | Avg frame time: " << std::setprecision(3) << (avg_frame_time * 1000.0f) << "ms"
                     << " | Total frames: " << total_frames << std::endl;
            frame_count = 0;
            total_frame_time = 0.0f;
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
    
    // Create window with version-specific API
#if SFML_VERSION_MAJOR >= 3
    // SFML 3.x API
    sf::RenderWindow window(sf::VideoMode(sf::Vector2u(WINDOW_WIDTH, WINDOW_HEIGHT)), 
                           "Brownian Motion Simulation - C++ Zero Cost Conf Demo");
#else
    // SFML 2.x API  
    sf::RenderWindow window(sf::VideoMode(WINDOW_WIDTH, WINDOW_HEIGHT), 
                           "Brownian Motion Simulation - C++ Zero Cost Conf Demo");
#endif
    
    window.setFramerateLimit(120); // Lock to 120 FPS for consistent performance comparison
    window.setVerticalSyncEnabled(false); // Disable V-Sync
    
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
    std::cout << "Press ESC to exit, SPACE to reset\n";
    
    // Main game loop
    while (window.isOpen()) {
        // Calculate delta time
        auto current_time = std::chrono::high_resolution_clock::now();
        float delta_time = std::chrono::duration<float>(current_time - last_time).count();
        last_time = current_time;
        
        // Handle events with version-specific API
#if SFML_VERSION_MAJOR >= 3
        // SFML 3.x event handling
        while (auto event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                std::cout << "Window closed by user\n";
                window.close();
            }
            
            if (event->is<sf::Event::KeyPressed>()) {
                auto key_event = event->getIf<sf::Event::KeyPressed>();
                if (key_event->code == sf::Keyboard::Key::Escape) {
                    std::cout << "Escape pressed - exiting\n";
                    window.close();
                } else if (key_event->code == sf::Keyboard::Key::Space) {
                    simulation.resetParticles();
                    std::cout << "Simulation reset\n";
                }
            }
        }
#else
        // SFML 2.x event handling
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
#endif
        
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