#include "simulation.h"
#include "matrix_operations.h"
#include <cmath>
#include <algorithm>

Particle::Particle(float x, float y, float r) 
    : position(x, y), velocity(0, 0), radius(r) {
    // Random color using static generator - darker colors for white background
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> color_dist(0, 200); // Darker colors for white background
    
    color = sf::Color(
        color_dist(gen),
        color_dist(gen), 
        color_dist(gen),
        220 // Semi-transparent
    );
}

BrownianSimulation::BrownianSimulation(int width, int height, int particle_count) 
    : rng(std::random_device{}()),
      noise_dist(-50.0f, 50.0f),
      color_dist(0.0f, 1.0f),
      window_width(width), 
      window_height(height) {
    
    // Initialize particles
    particles.reserve(particle_count);
    std::uniform_real_distribution<float> x_dist(10, width - 10);
    std::uniform_real_distribution<float> y_dist(10, height - 10);
    
    for (int i = 0; i < particle_count; ++i) {
        std::uniform_real_distribution<float> radius_dist(1.5f, 3.0f);
        particles.emplace_back(x_dist(rng), y_dist(rng), radius_dist(rng));
        
        // Give particles some initial velocity for more interesting motion
        std::uniform_real_distribution<float> vel_dist(-20.0f, 20.0f);
        particles.back().velocity.x = vel_dist(rng);
        particles.back().velocity.y = vel_dist(rng);
    }
    
    // Initialize matrices for slow computations (this will hurt performance!)
    const int matrix_size = 280; // Extreme matrix size for target ~20 FPS
    transformation_matrix.resize(matrix_size, std::vector<float>(matrix_size));
    position_matrix.resize(matrix_size, std::vector<float>(matrix_size));  
    result_matrix.resize(matrix_size, std::vector<float>(matrix_size));
    
    MatrixOperations::createIdentityMatrix(transformation_matrix, matrix_size);
    MatrixOperations::createRandomMatrix(position_matrix, matrix_size, matrix_size);
}

void BrownianSimulation::update(float delta_time) {
    // INTENTIONALLY SLOW: Perform matrix multiplication every frame!
    // This will be very visible in the profiler
    MatrixOperations::fastMatrixMultiply(transformation_matrix, position_matrix, result_matrix);
    //MatrixOperations::slowMatrixMultiply(transformation_matrix, position_matrix, result_matrix);
    
    // Update each particle
    for (auto& particle : particles) {
        // Add random brownian motion - make it more pronounced
        float noise_x = noise_dist(rng) * delta_time * 2.0f;
        float noise_y = noise_dist(rng) * delta_time * 2.0f;
        
        particle.velocity.x += noise_x;
        particle.velocity.y += noise_y;
        
        // Apply more damping to slow down particles
        particle.velocity.x *= 0.992f; // More damping (was 0.995f)
        particle.velocity.y *= 0.992f;
        
        // Update position with more visible movement (slower)
        particle.position.x += particle.velocity.x * delta_time * 40.0f; // Reduced from 60.0f
        particle.position.y += particle.velocity.y * delta_time * 40.0f;
        
        // Bounce off walls (softer bouncing)
        if (particle.position.x <= particle.radius || particle.position.x >= window_width - particle.radius) {
            particle.velocity.x *= -0.4f; // Softer bounce (was -0.7f)
            particle.position.x = std::max(particle.radius, 
                                         std::min(particle.position.x, window_width - particle.radius));
        }
        
        if (particle.position.y <= particle.radius || particle.position.y >= window_height - particle.radius) {
            particle.velocity.y *= -0.4f; // Softer bounce (was -0.7f)
            particle.position.y = std::max(particle.radius,
                                         std::min(particle.position.y, window_height - particle.radius));
        }
        
        // Slowly change color for visual interest
        if (color_dist(rng) > 0.98f) { // Редко меняем цвет
            std::uniform_int_distribution<> color_change(-10, 10);
            particle.color.r = std::clamp(particle.color.r + color_change(rng), 0, 200);
            particle.color.g = std::clamp(particle.color.g + color_change(rng), 0, 200);
            particle.color.b = std::clamp(particle.color.b + color_change(rng), 0, 200);
        }
    }
}

void BrownianSimulation::render(sf::RenderWindow& window) {
    // Draw particles as circles
    sf::CircleShape circle;
    
    for (const auto& particle : particles) {
        circle.setRadius(particle.radius);
        circle.setPosition(sf::Vector2f(particle.position.x - particle.radius, 
                          particle.position.y - particle.radius));
        circle.setFillColor(particle.color);
        
        window.draw(circle);
    }
}

void BrownianSimulation::resetParticles() {
    std::uniform_real_distribution<float> x_dist(10, window_width - 10);
    std::uniform_real_distribution<float> y_dist(10, window_height - 10);
    
    for (auto& particle : particles) {
        particle.position.x = x_dist(rng);
        particle.position.y = y_dist(rng);
        particle.velocity.x = 0;
        particle.velocity.y = 0;
    }
} 