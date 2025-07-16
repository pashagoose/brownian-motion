#include "obstacle_system.h"
#include <cmath>
#include <algorithm>

Obstacle::Obstacle(float x, float y, float w, float h) 
    : position(x, y), velocity(0, 0), size(w, h), rotation(0), angular_velocity(0) {
    
    // Random color for visual distinction
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> color_dist(50, 255);
    
    color = sf::Color(
        color_dist(gen),
        color_dist(gen), 
        color_dist(gen),
        180 // Semi-transparent
    );
    
    // Random angular velocity
    static std::uniform_real_distribution<> angular_dist(-2.0f, 2.0f);
    angular_velocity = angular_dist(gen);
}

ObstacleSystem::ObstacleSystem(int width, int height, int obstacle_count) 
    : rng(std::random_device{}()),
      direction_dist(-1.0f, 1.0f),
      window_width(width), 
      window_height(height) {
    
    obstacles.reserve(obstacle_count);
    
    // Create obstacles with random properties
    std::uniform_real_distribution<float> x_dist(100, width - 100);
    std::uniform_real_distribution<float> y_dist(100, height - 100);
    std::uniform_real_distribution<float> size_dist(30.0f, 80.0f);
    std::uniform_real_distribution<float> vel_dist(-30.0f, 30.0f);
    
    for (int i = 0; i < obstacle_count; ++i) {
        float w = size_dist(rng);
        float h = size_dist(rng);
        
        obstacles.emplace_back(x_dist(rng), y_dist(rng), w, h);
        
        // Give obstacles initial velocity
        obstacles.back().velocity.x = vel_dist(rng);
        obstacles.back().velocity.y = vel_dist(rng);
    }
}

void ObstacleSystem::update(float delta_time) {
    // Deep call hierarchy for interesting flame graph
    updateObstacleMovement(delta_time);
    handleObstacleBoundaries();
}

void ObstacleSystem::updateObstacleMovement(float delta_time) {
    for (auto& obstacle : obstacles) {
        // Update position
        obstacle.position.x += obstacle.velocity.x * delta_time;
        obstacle.position.y += obstacle.velocity.y * delta_time;
        
        // Update rotation
        obstacle.rotation += obstacle.angular_velocity * delta_time;
        
        // Keep rotation in [0, 2π] range
        if (obstacle.rotation > 2.0f * M_PI) {
            obstacle.rotation -= 2.0f * M_PI;
        } else if (obstacle.rotation < 0) {
            obstacle.rotation += 2.0f * M_PI;
        }
        
        // Add slight random velocity changes for more interesting movement
        if (direction_dist(rng) > 0.95f) {
            std::uniform_real_distribution<float> vel_change(-10.0f, 10.0f);
            obstacle.velocity.x += vel_change(rng);
            obstacle.velocity.y += vel_change(rng);
            
            // Limit velocity
            float max_velocity = 50.0f;
            float velocity_magnitude = sqrt(obstacle.velocity.x * obstacle.velocity.x + 
                                          obstacle.velocity.y * obstacle.velocity.y);
            if (velocity_magnitude > max_velocity) {
                obstacle.velocity.x = (obstacle.velocity.x / velocity_magnitude) * max_velocity;
                obstacle.velocity.y = (obstacle.velocity.y / velocity_magnitude) * max_velocity;
            }
        }
    }
}

void ObstacleSystem::handleObstacleBoundaries() {
    for (auto& obstacle : obstacles) {
        // Bounce off walls with some randomness
        bool bounced = false;
        
        if (obstacle.position.x - obstacle.size.x/2 <= 0 || 
            obstacle.position.x + obstacle.size.x/2 >= window_width) {
            obstacle.velocity.x *= -0.8f; // Some energy loss
            obstacle.position.x = std::max(obstacle.size.x/2, 
                                         std::min(obstacle.position.x, window_width - obstacle.size.x/2));
            bounced = true;
        }
        
        if (obstacle.position.y - obstacle.size.y/2 <= 0 || 
            obstacle.position.y + obstacle.size.y/2 >= window_height) {
            obstacle.velocity.y *= -0.8f; // Some energy loss
            obstacle.position.y = std::max(obstacle.size.y/2,
                                         std::min(obstacle.position.y, window_height - obstacle.size.y/2));
            bounced = true;
        }
        
        // Change angular velocity slightly when bouncing
        if (bounced) {
            std::uniform_real_distribution<float> angular_change(-0.5f, 0.5f);
            obstacle.angular_velocity += angular_change(rng);
            
            // Limit angular velocity
            obstacle.angular_velocity = std::max(-3.0f, std::min(obstacle.angular_velocity, 3.0f));
        }
    }
}

bool ObstacleSystem::handleParticleCollision(sf::Vector2f& particle_pos, sf::Vector2f& particle_velocity, float particle_radius) {
    sf::Vector2f original_pos = particle_pos;
    bool any_collision = false;
    
    for (const auto& obstacle : obstacles) {
        // First check if particle is currently inside obstacle
        CollisionInfo point_collision = checkPointRectangleCollision(particle_pos, obstacle, particle_radius);
        
        if (point_collision.has_collision) {
            // Push particle out of obstacle
            particle_pos = particle_pos + point_collision.collision_normal * point_collision.penetration_depth;
            
            // Reflect velocity
            particle_velocity = reflectVelocity(particle_velocity, point_collision.collision_normal);
            
            // Dampen velocity to prevent jittering
            particle_velocity = particle_velocity * 0.7f;
            
            any_collision = true;
        } else {
            // Check trajectory collision
            sf::Vector2f next_pos = particle_pos; // particle_pos is already updated position
            CollisionInfo trajectory_collision = checkLineRectangleCollision(original_pos, next_pos, obstacle, particle_radius);
            
            if (trajectory_collision.has_collision) {
                // Stop particle at collision point
                particle_pos = trajectory_collision.collision_point - trajectory_collision.collision_normal * particle_radius;
                
                // Reflect velocity
                particle_velocity = reflectVelocity(particle_velocity, trajectory_collision.collision_normal);
                
                // Dampen velocity
                particle_velocity = particle_velocity * 0.8f;
                
                any_collision = true;
            }
        }
    }
    
    return any_collision;
}

ObstacleSystem::CollisionInfo ObstacleSystem::checkPointRectangleCollision(const sf::Vector2f& point, const Obstacle& obstacle, float particle_radius) {
    CollisionInfo info;
    info.has_collision = false;
    
    // Transform point to obstacle's local coordinate system
    sf::Vector2f local_point = point - obstacle.position;
    local_point = rotateVector(local_point, -obstacle.rotation);
    
    // Expanded rectangle dimensions (to account for particle radius)
    float half_width = obstacle.size.x / 2 + particle_radius;
    float half_height = obstacle.size.y / 2 + particle_radius;
    
    // Check if point is inside expanded rectangle
    if (std::abs(local_point.x) <= half_width && std::abs(local_point.y) <= half_height) {
        info.has_collision = true;
        
        // Calculate penetration depth and normal
        float penetration_x = half_width - std::abs(local_point.x);
        float penetration_y = half_height - std::abs(local_point.y);
        
        sf::Vector2f local_normal;
        if (penetration_x < penetration_y) {
            // Horizontal collision
            local_normal.x = (local_point.x > 0) ? 1.0f : -1.0f;
            local_normal.y = 0.0f;
            info.penetration_depth = penetration_x;
        } else {
            // Vertical collision
            local_normal.x = 0.0f;
            local_normal.y = (local_point.y > 0) ? 1.0f : -1.0f;
            info.penetration_depth = penetration_y;
        }
        
        // Transform normal back to world coordinates
        info.collision_normal = rotateVector(local_normal, obstacle.rotation);
        info.collision_point = point;
    }
    
    return info;
}

ObstacleSystem::CollisionInfo ObstacleSystem::checkLineRectangleCollision(const sf::Vector2f& line_start, const sf::Vector2f& line_end, const Obstacle& obstacle, float particle_radius) {
    CollisionInfo info;
    info.has_collision = false;
    
    // For simplicity, we'll check if the line passes through the expanded rectangle
    // Transform both points to obstacle's local coordinate system
    sf::Vector2f local_start = line_start - obstacle.position;
    local_start = rotateVector(local_start, -obstacle.rotation);
    
    sf::Vector2f local_end = line_end - obstacle.position;
    local_end = rotateVector(local_end, -obstacle.rotation);
    
    // Expanded rectangle dimensions
    float half_width = obstacle.size.x / 2 + particle_radius;
    float half_height = obstacle.size.y / 2 + particle_radius;
    
    // Simple AABB line intersection check
    // If start is outside and end is inside, or vice versa, there's a collision
    bool start_inside = (std::abs(local_start.x) <= half_width && std::abs(local_start.y) <= half_height);
    bool end_inside = (std::abs(local_end.x) <= half_width && std::abs(local_end.y) <= half_height);
    
    if (!start_inside && end_inside) {
        // Particle is entering the obstacle
        info.has_collision = true;
        
        // Find collision point along the line (simplified)
        float t = 0.5f; // Midpoint approximation
        sf::Vector2f local_collision = local_start + (local_end - local_start) * t;
        
        // Calculate normal based on which edge was hit
        sf::Vector2f local_normal;
        if (std::abs(local_collision.x) / half_width > std::abs(local_collision.y) / half_height) {
            // Hit vertical edge
            local_normal.x = (local_collision.x > 0) ? 1.0f : -1.0f;
            local_normal.y = 0.0f;
        } else {
            // Hit horizontal edge
            local_normal.x = 0.0f;
            local_normal.y = (local_collision.y > 0) ? 1.0f : -1.0f;
        }
        
        // Transform back to world coordinates
        info.collision_normal = rotateVector(local_normal, obstacle.rotation);
        info.collision_point = obstacle.position + rotateVector(local_collision, obstacle.rotation);
        info.penetration_depth = 0.0f;
    }
    
    return info;
}

sf::Vector2f ObstacleSystem::reflectVelocity(const sf::Vector2f& velocity, const sf::Vector2f& normal) {
    // Reflection formula: v' = v - 2(v·n)n
    float dot_product = velocity.x * normal.x + velocity.y * normal.y;
    return sf::Vector2f(
        velocity.x - 2.0f * dot_product * normal.x,
        velocity.y - 2.0f * dot_product * normal.y
    );
}

sf::Vector2f ObstacleSystem::rotateVector(const sf::Vector2f& vec, float angle) {
    float cos_a = cos(angle);
    float sin_a = sin(angle);
    
    return sf::Vector2f(
        vec.x * cos_a - vec.y * sin_a,
        vec.x * sin_a + vec.y * cos_a
    );
}

sf::Vector2f ObstacleSystem::normalizeVector(const sf::Vector2f& vec) {
    float magnitude = sqrt(vec.x * vec.x + vec.y * vec.y);
    
    if (magnitude < 0.0001f) {
        return sf::Vector2f(0, 0);
    }
    
    return sf::Vector2f(vec.x / magnitude, vec.y / magnitude);
}

void ObstacleSystem::render(sf::RenderWindow& window) {
    sf::RectangleShape rectangle;
    
    for (const auto& obstacle : obstacles) {
        rectangle.setSize(obstacle.size);
        rectangle.setOrigin(sf::Vector2f(obstacle.size.x / 2, obstacle.size.y / 2));
        rectangle.setPosition(obstacle.position);
        rectangle.setRotation(sf::degrees(obstacle.rotation * 180.0f / M_PI)); // Convert to degrees
        rectangle.setFillColor(obstacle.color);
        rectangle.setOutlineThickness(2.0f);
        rectangle.setOutlineColor(sf::Color(0, 0, 0, 100));
        
        window.draw(rectangle);
    }
}

void ObstacleSystem::addObstacle(float x, float y, float w, float h) {
    obstacles.emplace_back(x, y, w, h);
}

void ObstacleSystem::resetObstacles() {
    for (auto& obstacle : obstacles) {
        std::uniform_real_distribution<float> x_dist(100, window_width - 100);
        std::uniform_real_distribution<float> y_dist(100, window_height - 100);
        std::uniform_real_distribution<float> vel_dist(-30.0f, 30.0f);
        
        obstacle.position.x = x_dist(rng);
        obstacle.position.y = y_dist(rng);
        obstacle.velocity.x = vel_dist(rng);
        obstacle.velocity.y = vel_dist(rng);
        obstacle.rotation = 0;
    }
} 