#pragma once

#include <vector>
#include <random>
#include <SFML/Graphics.hpp>

struct Obstacle {
    sf::Vector2f position;        // Center position
    sf::Vector2f velocity;        // Movement velocity
    sf::Vector2f size;           // Width and height
    float rotation;              // Current rotation angle (radians)
    float angular_velocity;      // Rotation speed (radians/second)
    sf::Color color;            // Visual color
    
    Obstacle(float x, float y, float w, float h);
};

class ObstacleSystem {
private:
    std::vector<Obstacle> obstacles;
    std::mt19937 rng;
    std::uniform_real_distribution<float> direction_dist;
    
    int window_width;
    int window_height;
    
    // Helper functions for collision detection and force calculation
    sf::Vector2f rotateVector(const sf::Vector2f& vec, float angle);
    sf::Vector2f normalizeVector(const sf::Vector2f& vec);
    bool isPointNearRotatedRectangle(const sf::Vector2f& point, const Obstacle& obstacle, float& distance);
    sf::Vector2f calculateRepulsionForce(const sf::Vector2f& particle_pos, const Obstacle& obstacle);
    
public:
    ObstacleSystem(int width, int height, int obstacle_count = 5);
    
    void update(float delta_time);
    void render(sf::RenderWindow& window);
    
    // Collision detection and response
    bool handleParticleCollision(sf::Vector2f& particle_pos, sf::Vector2f& particle_velocity, float particle_radius);
    
    // Obstacle management
    void addObstacle(float x, float y, float w, float h);
    void resetObstacles();
    int getObstacleCount() const { return obstacles.size(); }
    
private:
    void updateObstacleMovement(float delta_time);
    void handleObstacleBoundaries();
    
    // Collision detection helpers
    struct CollisionInfo {
        bool has_collision;
        sf::Vector2f collision_point;
        sf::Vector2f collision_normal;
        float penetration_depth;
    };
    
    CollisionInfo checkLineRectangleCollision(const sf::Vector2f& line_start, const sf::Vector2f& line_end, 
                                            const Obstacle& obstacle, float particle_radius);
    CollisionInfo checkPointRectangleCollision(const sf::Vector2f& point, const Obstacle& obstacle, float particle_radius);
    sf::Vector2f reflectVelocity(const sf::Vector2f& velocity, const sf::Vector2f& normal);
}; 