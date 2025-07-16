#pragma once

#include <vector>
#include <random>
#include <SFML/Graphics.hpp>
#include "obstacle_system.h"

struct Particle {
    sf::Vector2f position;
    sf::Vector2f velocity;
    sf::Color color;
    float radius;
    
    Particle(float x, float y, float r = 2.0f);
};

class BrownianSimulation {
private:
    std::vector<Particle> particles;
    std::mt19937 rng;
    std::uniform_real_distribution<float> noise_dist;
    std::uniform_real_distribution<float> color_dist;
    
    int window_width;
    int window_height;
    
    // For demonstration: we'll do some matrix operations each frame
    std::vector<std::vector<float>> transformation_matrix;
    std::vector<std::vector<float>> position_matrix;
    std::vector<std::vector<float>> result_matrix;
    
    // Obstacle system for particle interactions
    ObstacleSystem obstacle_system;
    
public:
    BrownianSimulation(int width, int height, int particle_count = 1000);
    
    void update(float delta_time);
    void render(sf::RenderWindow& window);
    
    void resetParticles();
    int getParticleCount() const { return particles.size(); }
}; 