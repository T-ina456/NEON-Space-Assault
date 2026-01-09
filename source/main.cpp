#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <vector>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <cstdlib>
#include <memory>
#include <algorithm>

// Constants
const float PI = 3.14159265f;

// Enhanced Particle System
struct Particle {
    sf::CircleShape shape;
    sf::Vector2f velocity;
    float lifetime;
    float maxLifetime;
    sf::Color startColor;
    sf::Color endColor;
};

// Power-up types
enum class PowerUpType {
    RAPID_FIRE,
    SHIELD,
    TRIPLE_SHOT
};

struct PowerUp {
    sf::CircleShape shape;
    PowerUpType type;
    float timer;
};

// Enemy with health
struct Enemy {
    sf::RectangleShape shape;
    sf::RectangleShape healthBarBg;
    sf::RectangleShape healthBar;
    int maxHealth;
    int health;
    int type;        // 0 normal, 1 fast, 2 tank, 3 boss
    bool isBoss = false;
};

class Game {
private:
    const unsigned int width = 1000;
    const unsigned int height = 800;
    sf::RenderWindow window;
    
    // Player - Rocket ship
    sf::ConvexShape playerRocket;
    float playerSpeed = 8.0f;
    sf::CircleShape shield;
    bool hasShield = false;
    float shieldTimer = 0.0f;
    
    // Triple shot power-up
    bool hasTripleShot = false;
    float tripleShotTimer = 0.0f;
    
    // Bullets
    std::vector<sf::CircleShape> bullets;
    float bulletSpeed = 12.0f;
    float shootCooldown = 0.0f;
    float fireRate = 0.15f;
    bool rapidFire = false;
    float rapidFireTimer = 0.0f;
    
    // Enemies
    std::vector<Enemy> enemies;
    float baseEnemySpeed = 2.0f;
    float spawnTimer = 0.0f;
    float spawnInterval = 1.2f;
    
    // Power-ups
    std::vector<PowerUp> powerUps;
    float powerUpSpawnTimer = 0.0f;
    
    // Particles
    std::vector<Particle> particles;
    std::vector<sf::CircleShape> trailParticles;
    
    // Level system
    int currentLevel = 1;
    int enemiesKilledInLevel = 0;
    int enemiesNeededForNextLevel = 15;
    bool levelTransition = false;
    float levelTransitionTimer = 0.0f;
    
    // UI
    int score = 0;
    int lives = 3;
    int combo = 0;
    float comboTimer = 0.0f;
    std::shared_ptr<sf::Font> font;
    std::shared_ptr<sf::Text> scoreText;
    std::shared_ptr<sf::Text> livesText;
    std::shared_ptr<sf::Text> levelText;
    std::shared_ptr<sf::Text> comboText;
    std::shared_ptr<sf::Text> gameOverText;
    std::shared_ptr<sf::Text> levelUpText;
    bool fontLoaded = false;
    
    // Stars background
    std::vector<sf::CircleShape> stars;
    std::vector<float> starSpeeds;
    
    // Screen shake
    float screenShake = 0.0f;
    sf::Vector2f shakeOffset;
    
    // Game state
    bool gameOver = false;
    sf::Clock clock;
    
    // Colors
    sf::Color neonCyan = sf::Color(0, 255, 255);
    sf::Color neonPink = sf::Color(255, 0, 255);
    sf::Color neonGreen = sf::Color(0, 255, 100);
    sf::Color neonOrange = sf::Color(255, 150, 0);
    sf::Color neonYellow = sf::Color(255, 255, 0);
    
public:
    Game() : window(sf::VideoMode({width, height}), "NEON SPACE ASSAULT - LEVEL MODE", sf::Style::Close) {
        window.setFramerateLimit(60);
        initializeGame();
    }

   void spawnBoss() {
    Enemy boss;
    boss.isBoss = true;
    boss.type = 3;

    boss.shape.setSize(sf::Vector2f(180.f, 120.f));
    boss.shape.setFillColor(sf::Color(180, 50, 255));
    boss.shape.setOutlineThickness(4.f);
    boss.shape.setOutlineColor(sf::Color::White);

boss.shape.setPosition(sf::Vector2f(
    width / 2.f - boss.shape.getSize().x / 2.f,
    -150.f
));



    boss.maxHealth = 100;
    boss.health = boss.maxHealth;

    // Health bar
    boss.healthBarBg.setSize(sf::Vector2f(boss.shape.getSize().x, 8.f));
    boss.healthBarBg.setFillColor(sf::Color(40, 40, 40));

    boss.healthBar.setSize(sf::Vector2f(boss.shape.getSize().x, 8.f));
    boss.healthBar.setFillColor(sf::Color::Red);

    enemies.push_back(boss);
}


    
    void initializeGame() {
        srand(static_cast<unsigned int>(time(nullptr)));
        
        // Create rocket ship (more detailed)
        playerRocket.setPointCount(7);
        playerRocket.setPoint(0, sf::Vector2f(25, 0));     // Nose
        playerRocket.setPoint(1, sf::Vector2f(15, 25));    // Left body
        playerRocket.setPoint(2, sf::Vector2f(10, 25));    // Left inner
        playerRocket.setPoint(3, sf::Vector2f(0, 50));     // Left fin
        playerRocket.setPoint(4, sf::Vector2f(25, 35));    // Bottom center
        playerRocket.setPoint(5, sf::Vector2f(50, 50));    // Right fin
        playerRocket.setPoint(6, sf::Vector2f(40, 25));    // Right inner
        
        // Create gradient effect with multiple colors
        playerRocket.setFillColor(neonCyan);
        playerRocket.setOutlineThickness(2);
        playerRocket.setOutlineColor(sf::Color::White);
        playerRocket.setPosition(sf::Vector2f(width / 2.0f - 25, height - 100.0f));
        
        // Shield
        shield.setRadius(45);
        shield.setFillColor(sf::Color(0, 200, 255, 50));
        shield.setOutlineThickness(3);
        shield.setOutlineColor(sf::Color(0, 200, 255, 150));
        shield.setOrigin(sf::Vector2f(45.f, 45.f));
        
        // Load font
        font = std::make_shared<sf::Font>();
        if (font->openFromFile("C:/Windows/Fonts/arial.ttf")) {
            fontLoaded = true;
            
            scoreText = std::make_shared<sf::Text>(*font);
            scoreText->setCharacterSize(28);
            scoreText->setFillColor(neonCyan);
            scoreText->setPosition(sf::Vector2f(20, 20));
            scoreText->setStyle(sf::Text::Bold);
            
            livesText = std::make_shared<sf::Text>(*font);
            livesText->setCharacterSize(28);
            livesText->setFillColor(neonPink);
            livesText->setPosition(sf::Vector2f(20, 60));
            livesText->setStyle(sf::Text::Bold);
            
            levelText = std::make_shared<sf::Text>(*font);
            levelText->setCharacterSize(28);
            levelText->setFillColor(neonGreen);
            levelText->setPosition(sf::Vector2f(width - 200.0f, 20));
            levelText->setStyle(sf::Text::Bold);
            
            comboText = std::make_shared<sf::Text>(*font);
            comboText->setCharacterSize(40);
            comboText->setFillColor(neonOrange);
            comboText->setPosition(sf::Vector2f(width / 2.0f - 100, 100));
            comboText->setStyle(sf::Text::Bold);
            
            levelUpText = std::make_shared<sf::Text>(*font);
            levelUpText->setCharacterSize(60);
            levelUpText->setFillColor(neonYellow);
            levelUpText->setStyle(sf::Text::Bold);
            
            gameOverText = std::make_shared<sf::Text>(*font);
            gameOverText->setCharacterSize(70);
            gameOverText->setFillColor(neonPink);
            gameOverText->setString("GAME OVER");
            sf::FloatRect bounds = gameOverText->getGlobalBounds();
            gameOverText->setPosition(sf::Vector2f(width / 2.0f - bounds.size.x / 2, height / 2.0f - 100));
            gameOverText->setStyle(sf::Text::Bold);
        }
        
        // Create parallax starfield
        for (int i = 0; i < 200; i++) {
            float size = static_cast<float>(rand() % 3 + 1);
            sf::CircleShape star(size);
            int brightness = rand() % 200 + 55;
            star.setFillColor(sf::Color(brightness, brightness, 255, rand() % 150 + 100));
            star.setPosition(sf::Vector2f(static_cast<float>(rand() % width), static_cast<float>(rand() % height)));
            stars.push_back(star);
            starSpeeds.push_back(size * 0.5f);
        }
    }
    
    void handleInput() {
        while (const std::optional event = window.pollEvent()) {
            if (event->is<sf::Event::Closed>()) {
                window.close();
            }
            
            if (const auto* keyPressed = event->getIf<sf::Event::KeyPressed>()) {
                if (keyPressed->code == sf::Keyboard::Key::R && gameOver) {
                    resetGame();
                }
            }
        }
        
        if (gameOver || levelTransition) return;
        
        // Player movement
        sf::Vector2f movement(0, 0);
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Left)) {
            movement.x = -playerSpeed;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Right)) {
            movement.x = playerSpeed;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Up)) {
            movement.y = -playerSpeed;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Down)) {
            movement.y = playerSpeed;
        }
        
        sf::Vector2f newPos = playerRocket.getPosition() + movement;
        if (newPos.x > 0 && newPos.x < width - 50)
            playerRocket.move(sf::Vector2f(movement.x, 0));
        if (newPos.y > 0 && newPos.y < height - 60)
            playerRocket.move(sf::Vector2f(0, movement.y));
        
        // Shooting
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Space) && shootCooldown <= 0) {
            shoot();
            float rate = rapidFire ? fireRate * 0.4f : fireRate;
            shootCooldown = rate;
        }
    }
    
    void shoot() {
        sf::Vector2f rocketPos = playerRocket.getPosition();
        
        if (hasTripleShot) {
            // Triple shot - 3 bullets
            for (int i = -1; i <= 1; i++) {
                sf::CircleShape bullet(4);
                bullet.setFillColor(neonYellow);
                bullet.setOutlineThickness(2);
                bullet.setOutlineColor(sf::Color::White);
                bullet.setPosition(sf::Vector2f(rocketPos.x + 22 + (i * 15), rocketPos.y - 10));
                bullets.push_back(bullet);
                createMuzzleFlash(sf::Vector2f(rocketPos.x + 25 + (i * 15), rocketPos.y));
            }
        } else {
            // Single shot
            sf::CircleShape bullet(4);
            bullet.setFillColor(neonCyan);
            bullet.setOutlineThickness(2);
            bullet.setOutlineColor(sf::Color::White);
            bullet.setPosition(sf::Vector2f(rocketPos.x + 22, rocketPos.y - 10));
            bullets.push_back(bullet);
            createMuzzleFlash(sf::Vector2f(rocketPos.x + 25, rocketPos.y));
        }
    }
    
    void spawnEnemy() {
        Enemy enemy;
        
        // Level-based enemy distribution
        int type = rand() % 100;
        
        if (currentLevel == 1) {
            if (type < 70) enemy.type = 0;      // 70% normal
            else if (type < 90) enemy.type = 1; // 20% fast
            else enemy.type = 2;                // 10% tank
        } else if (currentLevel == 2) {
            if (type < 50) enemy.type = 0;      // 50% normal
            else if (type < 80) enemy.type = 1; // 30% fast
            else enemy.type = 2;                // 20% tank
        } else { // Level 3
            if (type < 30) enemy.type = 0;      // 30% normal
            else if (type < 70) enemy.type = 1; // 40% fast
            else enemy.type = 2;                // 30% tank
        }
        
        if (enemy.type == 0) {
            // Normal enemy
            enemy.shape.setSize(sf::Vector2f(40, 30));
            enemy.shape.setFillColor(sf::Color(255, 50, 50));
            enemy.maxHealth = 1;
        } else if (enemy.type == 1) {
            // Fast enemy
            enemy.shape.setSize(sf::Vector2f(30, 25));
            enemy.shape.setFillColor(sf::Color(255, 150, 0));
            enemy.maxHealth = 1;
        } else {
            // Tank enemy
            enemy.shape.setSize(sf::Vector2f(50, 40));
            enemy.shape.setFillColor(sf::Color(150, 0, 255));
            enemy.maxHealth = 3 + currentLevel; // More health in higher levels
        }
        
        enemy.health = enemy.maxHealth;
        enemy.shape.setPosition(sf::Vector2f(static_cast<float>(rand() % (width - 50)), -50));
        enemy.shape.setOutlineThickness(2);
        enemy.shape.setOutlineColor(sf::Color::White);
        
        // Health bar
        enemy.healthBarBg.setSize(sf::Vector2f(enemy.shape.getSize().x, 4));
        enemy.healthBarBg.setFillColor(sf::Color(50, 50, 50));
        
        enemy.healthBar.setSize(sf::Vector2f(enemy.shape.getSize().x, 4));
        enemy.healthBar.setFillColor(sf::Color(0, 255, 0));
        
        enemies.push_back(enemy);
    }
    
    void spawnPowerUp(sf::Vector2f position) {
        if (rand() % 100 < 35) { // 35% chance
            PowerUp powerUp;
            powerUp.shape.setRadius(15);
            powerUp.shape.setOrigin(sf::Vector2f(15.f, 15.f));
            powerUp.shape.setPosition(position);
            
            int type = rand() % 3;
            powerUp.type = static_cast<PowerUpType>(type);
            
            switch (powerUp.type) {
                case PowerUpType::RAPID_FIRE:
                    powerUp.shape.setFillColor(sf::Color(255, 100, 0, 200));
                    break;
                case PowerUpType::SHIELD:
                    powerUp.shape.setFillColor(sf::Color(0, 200, 255, 200));
                    break;
                case PowerUpType::TRIPLE_SHOT:
                    powerUp.shape.setFillColor(sf::Color(255, 255, 0, 200));
                    break;
            }
            
            powerUp.shape.setOutlineThickness(2);
            powerUp.shape.setOutlineColor(sf::Color::White);
            powerUp.timer = 0;
            
            powerUps.push_back(powerUp);
        }
    }
    
    void createExplosion(sf::Vector2f position, sf::Color color) {
        for (int i = 0; i < 25; i++) {
            Particle p;
            p.shape.setRadius(static_cast<float>(rand() % 4 + 2));
            p.shape.setPosition(position);
            
            float angle = (rand() % 360) * PI / 180.0f;
            float speed = static_cast<float>(rand() % 4 + 3);
            p.velocity = sf::Vector2f(cos(angle) * speed, sin(angle) * speed);
            p.lifetime = 0.8f;
            p.maxLifetime = 0.8f;
            p.startColor = color;
            p.endColor = sf::Color(color.r / 2, color.g / 2, color.b / 2, 0);
            p.shape.setFillColor(p.startColor);
            
            particles.push_back(p);
        }
        
        screenShake = 0.3f;
    }
    
    void createMuzzleFlash(sf::Vector2f position) {
        for (int i = 0; i < 5; i++) {
            Particle p;
            p.shape.setRadius(2);
            p.shape.setPosition(position);
            
            float angle = (-90 + (rand() % 40 - 20)) * PI / 180.0f;
            float speed = static_cast<float>(rand() % 2 + 1);
            p.velocity = sf::Vector2f(cos(angle) * speed, sin(angle) * speed);
            p.lifetime = 0.2f;
            p.maxLifetime = 0.2f;
            p.startColor = hasTripleShot ? neonYellow : neonCyan;
            p.endColor = sf::Color(100, 100, 0, 0);
            p.shape.setFillColor(p.startColor);
            
            particles.push_back(p);
        }
    }
    
    void startLevelTransition() {
        levelTransition = true;
        levelTransitionTimer = 3.0f;
        currentLevel++;
        enemiesKilledInLevel = 0;
        
        // Increase difficulty
        if (currentLevel == 2) {
            baseEnemySpeed = 3.5f;
            spawnInterval = 0.9f;
            enemiesNeededForNextLevel = 25;
        } else if (currentLevel == 3) {
            baseEnemySpeed = 5.0f;
            spawnInterval = 0.6f;
            enemiesNeededForNextLevel = 999; // Endless for level 3
        }
        
        // Clear enemies and bullets
        enemies.clear();
        bullets.clear();

        if (currentLevel == 3) {
    spawnBoss();
}

    }
    
    void update(float deltaTime) {
        if (gameOver) return;
        
        // Level transition
        if (levelTransition) {
            levelTransitionTimer -= deltaTime;
            if (levelTransitionTimer <= 0) {
                levelTransition = false;
            }
            return;
        }
        
        shootCooldown -= deltaTime;
        spawnTimer += deltaTime;
        comboTimer -= deltaTime;
        powerUpSpawnTimer += deltaTime;
        
        if (comboTimer <= 0) combo = 0;
        
        // Power-up timers
        if (rapidFire) {
            rapidFireTimer -= deltaTime;
            if (rapidFireTimer <= 0) rapidFire = false;
        }
        if (hasShield) {
            shieldTimer -= deltaTime;
            if (shieldTimer <= 0) hasShield = false;
        }
        if (hasTripleShot) {
            tripleShotTimer -= deltaTime;
            if (tripleShotTimer <= 0) hasTripleShot = false;
        }
        
        // Screen shake
        if (screenShake > 0) {
            screenShake -= deltaTime;
            shakeOffset = sf::Vector2f(
                (rand() % 20 - 10) * screenShake,
                (rand() % 20 - 10) * screenShake
            );
        } else {
            shakeOffset = sf::Vector2f(0, 0);
        }
        
        // Spawn enemies
      if (spawnTimer >= spawnInterval) {
    if (!(currentLevel == 3 && !enemies.empty() && enemies[0].isBoss)) {
        spawnEnemy();
    }
    spawnTimer = 0;
}

        
        // Update bullets
        for (auto it = bullets.begin(); it != bullets.end();) {
            it->move(sf::Vector2f(0, -bulletSpeed));
            
            // Create trail
            if (rand() % 3 == 0) {
                sf::CircleShape trail(2);
                sf::Color trailColor = hasTripleShot ? sf::Color(255, 255, 0, 100) : sf::Color(0, 255, 255, 100);
                trail.setFillColor(trailColor);
                trail.setPosition(it->getPosition());
                trailParticles.push_back(trail);
            }
            
            if (it->getPosition().y < -20) {
                it = bullets.erase(it);
            } else {
                ++it;
            }
        }
        
        // Update trail particles
        for (auto it = trailParticles.begin(); it != trailParticles.end();) {
            sf::Color color = it->getFillColor();
            color.a = static_cast<uint8_t>(color.a * 0.9f);
            it->setFillColor(color);
            
            if (color.a < 10) {
                it = trailParticles.erase(it);
            } else {
                ++it;
            }
        }
        
        // Update enemies
for (auto it = enemies.begin(); it != enemies.end();)
{
    float speed = baseEnemySpeed;

    // ----- SPEED TUNING -----
    if (it->isBoss)
    {
        speed = 0.9f;                 // Boss: slow & heavy
    }
    else
    {
        if (it->type == 1) speed *= 1.3f;   // Fast enemy
        if (it->type == 2) speed *= 0.6f;   // Tank enemy
    }

    // ---- MOVE ONLY ONCE ----
    it->shape.move(sf::Vector2f(0.f, speed));

    // Health bar position
    
sf::Vector2f pos = it->shape.getPosition();
it->healthBarBg.setPosition(sf::Vector2f(pos.x, pos.y - 8.f));
it->healthBar.setPosition(sf::Vector2f(pos.x, pos.y - 8.f));


    // Health bar size
    float healthPercent = static_cast<float>(it->health) / it->maxHealth;
    it->healthBar.setSize(
        sf::Vector2f(it->shape.getSize().x * healthPercent, 4)
    );

    // Health bar color
    if (healthPercent > 0.6f)
        it->healthBar.setFillColor(sf::Color::Green);
    else if (healthPercent > 0.3f)
        it->healthBar.setFillColor(sf::Color::Yellow);
    else
        it->healthBar.setFillColor(sf::Color::Red);

    // Enemy reached bottom
    if (it->shape.getPosition().y > height)
    {
        if (!hasShield) lives--;
        createExplosion(it->shape.getPosition(), sf::Color::Red);
        it = enemies.erase(it);

        if (lives <= 0) gameOver = true;
    }
    else
    {
        ++it;
    }
}

        
        // Update power-ups
        for (auto it = powerUps.begin(); it != powerUps.end();) {
            it->shape.move(sf::Vector2f(0, 2));
            it->timer += deltaTime;
            
            // Pulsing effect
            float scale = 1.0f + sin(it->timer * 10) * 0.2f;
            it->shape.setScale(sf::Vector2f(scale, scale));
            
            if (it->shape.getPosition().y > height) {
                it = powerUps.erase(it);
            } else {
                ++it;
            }
        }
        
        // Collision: bullets vs enemies
       for (auto bulletIt = bullets.begin(); bulletIt != bullets.end();) {
    bool bulletHit = false;
    sf::FloatRect bulletBounds = bulletIt->getGlobalBounds();

    for (auto& enemy : enemies) {
        sf::FloatRect enemyBounds = enemy.shape.getGlobalBounds();

        if (bulletBounds.findIntersection(enemyBounds).has_value()) {
            bulletHit = true;
            enemy.health--;

            if (enemy.health <= 0) {
                if (enemy.isBoss) {
    score += 5000;
    gameOver = true;   // OR create victory screen
}

                int points = 10;
                if (enemy.type == 1) points = 15;
                else if (enemy.type == 2) points = 30;

                points *= currentLevel;
                score += points * (combo + 1);

                combo++;
                comboTimer = 2.f;
                enemiesKilledInLevel++;

                createExplosion(enemy.shape.getPosition(),
                                enemy.shape.getFillColor());
                spawnPowerUp(enemy.shape.getPosition());

                enemy.health = -999; // mark for removal
            }
            break;
        }
    }

    if (bulletHit)
        bulletIt = bullets.erase(bulletIt);
    else
        ++bulletIt;
}

// Remove dead enemies AFTER bullet loop
enemies.erase(
    std::remove_if(enemies.begin(), enemies.end(),
        [](const Enemy& e) { return e.health < 0; }),
    enemies.end()
);

  // ===== LEVEL PROGRESSION CHECK =====
if (!levelTransition &&
    currentLevel < 3 &&
    enemiesKilledInLevel >= enemiesNeededForNextLevel) {
    startLevelTransition();
}
      
        // Collision: player vs power-ups
        sf::FloatRect playerBounds = playerRocket.getGlobalBounds();
        for (auto it = powerUps.begin(); it != powerUps.end();) {
            if (playerBounds.findIntersection(it->shape.getGlobalBounds()).has_value()) {
                switch (it->type) {
                    case PowerUpType::RAPID_FIRE:
                        rapidFire = true;
                        rapidFireTimer = 8.0f;
                        break;
                    case PowerUpType::SHIELD:
                        hasShield = true;
                        shieldTimer = 10.0f;
                        break;
                    case PowerUpType::TRIPLE_SHOT:
                        hasTripleShot = true;
                        tripleShotTimer = 12.0f;
                        break;
                }
                it = powerUps.erase(it);
            } else {
                ++it;
            }
        }
        
        // Update particles
        for (auto it = particles.begin(); it != particles.end();) {
            it->lifetime -= deltaTime;
            it->shape.move(it->velocity);
            
            float progress = 1.0f - (it->lifetime / it->maxLifetime);
            sf::Color currentColor(
                it->startColor.r + (it->endColor.r - it->startColor.r) * progress,
                it->startColor.g + (it->endColor.g - it->startColor.g) * progress,
                it->startColor.b + (it->endColor.b - it->startColor.b) * progress,
                it->startColor.a + (it->endColor.a - it->startColor.a) * progress
            );
            it->shape.setFillColor(currentColor);
            
            if (it->lifetime <= 0) {
                it = particles.erase(it);
            } else {
                ++it;
            }
        }
        
        // Animate stars
        for (size_t i = 0; i < stars.size(); i++) {
            stars[i].move(sf::Vector2f(0, starSpeeds[i] * currentLevel * 0.5f));
            if (stars[i].getPosition().y > height) {
                stars[i].setPosition(sf::Vector2f(static_cast<float>(rand() % width), -5));
            }
        }
        
        // Update UI
         if (fontLoaded) {
            std::stringstream ss;
            ss << "SCORE: " << std::setw(8) << std::setfill('0') << score;
            scoreText->setString(ss.str());
            
            livesText->setString("LIVES: " + std::string(lives, '♥'));
            levelText->setString("LEVEL: " + std::to_string(currentLevel));
            
            if (combo > 1) {
                comboText->setString("COMBO x" + std::to_string(combo));
            }
        }
    }
    
    void render() {
        window.clear(sf::Color(5, 5, 20));
        
        sf::View view = window.getDefaultView();
        view.move(shakeOffset);
        window.setView(view);
        
        // Draw stars
        for (const auto& star : stars) {
            window.draw(star);
        }
        
        // Draw trail particles
        for (const auto& trail : trailParticles) {
            window.draw(trail);
        }
        
        // Draw particles
        for (const auto& particle : particles) {
            window.draw(particle.shape);
        }
        
        // Draw power-ups
        for (const auto& powerUp : powerUps) {
            window.draw(powerUp.shape);
        }
        
        // Draw player with shield
        if (hasShield) {
            shield.setPosition(playerRocket.getPosition() + sf::Vector2f(25, 25));
            window.draw(shield);
        }
        
        // Draw rocket exhaust flames
        if (!levelTransition && !gameOver) {
            sf::CircleShape flame1(6);
            flame1.setFillColor(sf::Color(255, 150, 0, 180));
            flame1.setPosition(playerRocket.getPosition() + sf::Vector2f(19, 45));
            window.draw(flame1);
            
            sf::CircleShape flame2(4);
            flame2.setFillColor(sf::Color(255, 50, 0, 200));
            flame2.setPosition(playerRocket.getPosition() + sf::Vector2f(21, 50));
            window.draw(flame2);
        }
        
        window.draw(playerRocket);
        
        // Draw bullets
        for (const auto& bullet : bullets) {
            window.draw(bullet);
        }
        
        // Draw enemies with health bars
        for (const auto& enemy : enemies) {
            window.draw(enemy.shape);
            if (enemy.maxHealth > 1) {
                window.draw(enemy.healthBarBg);
                window.draw(enemy.healthBar);
            }
        }
        
        // Reset view for UI
        window.setView(window.getDefaultView());
        
        // Draw UI
        if (fontLoaded) {
            window.draw(*scoreText);
            window.draw(*livesText);
            window.draw(*levelText);
            
            if (combo > 1) {
                window.draw(*comboText);
            }
            
            // Draw power-up indicators
            float indicatorY = 110;
            if (rapidFire) {
                sf::RectangleShape indicator(sf::Vector2f(150, 30));
                indicator.setPosition(sf::Vector2f(20, indicatorY));
                indicator.setFillColor(sf::Color(255, 100, 0, 100));
                indicator.setOutlineThickness(2);
                indicator.setOutlineColor(sf::Color(255, 100, 0));
                window.draw(indicator);
                
                sf::Text text(*font);
                text.setCharacterSize(18);
                text.setFillColor(sf::Color::White);
                text.setString("RAPID FIRE");
                text.setPosition(sf::Vector2f(30, indicatorY + 5));
                window.draw(text);
                indicatorY += 40;
            }
            
            if (hasShield) {
                sf::RectangleShape indicator(sf::Vector2f(150, 30));
                indicator.setPosition(sf::Vector2f(20, indicatorY));
                indicator.setFillColor(sf::Color(0, 200, 255, 100));
                indicator.setOutlineThickness(2);
                indicator.setOutlineColor(sf::Color(0, 200, 255));
                window.draw(indicator);
                
                sf::Text text(*font);
                text.setCharacterSize(18);
                text.setFillColor(sf::Color::White);
                text.setString("SHIELD ACTIVE");
                text.setPosition(sf::Vector2f(30, indicatorY + 5));
                window.draw(text);
                indicatorY += 40;
            }
            
            if (hasTripleShot) {
                sf::RectangleShape indicator(sf::Vector2f(150, 30));
                indicator.setPosition(sf::Vector2f(20, indicatorY));
                indicator.setFillColor(sf::Color(255, 255, 0, 100));
                indicator.setOutlineThickness(2);
                indicator.setOutlineColor(sf::Color(255, 255, 0));
                window.draw(indicator);
                
                sf::Text text(*font);
                text.setCharacterSize(18);
                text.setFillColor(sf::Color::White);
                text.setString("TRIPLE SHOT");
                text.setPosition(sf::Vector2f(30, indicatorY + 5));
                window.draw(text);
            }
            
            // Level transition screen
            if (levelTransition) {
                sf::RectangleShape overlay(sf::Vector2f(static_cast<float>(width), static_cast<float>(height)));
                overlay.setFillColor(sf::Color(0, 0, 0, 200));
                window.draw(overlay);
                
                levelUpText->setString("LEVEL " + std::to_string(currentLevel));
                sf::FloatRect bounds = levelUpText->getGlobalBounds();
                levelUpText->setPosition(sf::Vector2f(width / 2.0f - bounds.size.x / 2, height / 2.0f - 50));
                window.draw(*levelUpText);
                
                sf::Text readyText(*font);
                readyText.setCharacterSize(40);
                readyText.setFillColor(sf::Color::White);
                readyText.setString("GET READY!");
                bounds = readyText.getGlobalBounds();
                readyText.setPosition(sf::Vector2f(width / 2.0f - bounds.size.x / 2, height / 2.0f + 50));
                window.draw(readyText);
            }
            
            if (gameOver) {
                // Dark overlay
                sf::RectangleShape overlay(sf::Vector2f(static_cast<float>(width), static_cast<float>(height)));
                overlay.setFillColor(sf::Color(0, 0, 0, 180));
                window.draw(overlay);
                
                window.draw(*gameOverText);
                
                sf::Text restartText(*font);
                restartText.setCharacterSize(35);
                restartText.setFillColor(sf::Color::White);
                restartText.setString("Press R to Restart");
                sf::FloatRect bounds = restartText.getGlobalBounds();
                restartText.setPosition(sf::Vector2f(width / 2.0f - bounds.size.x / 2, height / 2.0f));
                window.draw(restartText);
                
                sf::Text finalScoreText(*font);
                finalScoreText.setCharacterSize(30);
                finalScoreText.setFillColor(neonCyan);
                finalScoreText.setString("FINAL SCORE: " + std::to_string(score));
                bounds = finalScoreText.getGlobalBounds();
                finalScoreText.setPosition(sf::Vector2f(width / 2.0f - bounds.size.x / 2, height / 2.0f + 80));
                window.draw(finalScoreText);
                
                sf::Text levelReachedText(*font);
                levelReachedText.setCharacterSize(25);
                levelReachedText.setFillColor(neonGreen);
                levelReachedText.setString("Level Reached: " + std::to_string(currentLevel));
                bounds = levelReachedText.getGlobalBounds();
                levelReachedText.setPosition(sf::Vector2f(width / 2.0f - bounds.size.x / 2, height / 2.0f + 130));
                window.draw(levelReachedText);
            }
        }
        
        window.display();
    }
    
    void resetGame() {
        bullets.clear();
        enemies.clear();
        particles.clear();
        trailParticles.clear();
        powerUps.clear();
        score = 0;
        lives = 3;
        currentLevel = 1;
        enemiesKilledInLevel = 0;
        enemiesNeededForNextLevel = 15;
        combo = 0;
        gameOver = false;
        levelTransition = false;
        baseEnemySpeed = 2.0f;
        spawnInterval = 1.2f;
        rapidFire = false;
        hasShield = false;
        hasTripleShot = false;
        playerRocket.setPosition(sf::Vector2f(width / 2.0f - 25, height - 100.0f));
    }
    
    void run() {
        while (window.isOpen()) {
            float deltaTime = clock.restart().asSeconds();
            
            handleInput();
            update(deltaTime);
            render();
        }
    }
};

// =======================
// MAIN FUNCTION
// =======================
int main() {
    Game game;
    game.run();
    return 0;
}
