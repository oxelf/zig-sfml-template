// Project: SpinGame
// Author: Andrej Geller
// Created: 1/18/2025 6:53:28 PM

#define _USE_MATH_DEFINES
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <fstream>
#include <iostream>
#include <math.h>

#ifdef __APPLE__
#include "TargetConditionals.h"
#if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
#include <SFML/Main.hpp>
#endif
#endif

void handleDirectionChange(int &score, int &coins, int &highscore, float &speed,
                           float &coin_angle, float &direction,
                           sf::RectangleShape &playerHitbox, sf::Sprite &coin,
                           sf::Text &scoreText, sf::Text &coinCount,
                           sf::Text &highscoreText, sf::Sound &sound) {
  direction *= -1;

  if (playerHitbox.getGlobalBounds().findIntersection(coin.getGlobalBounds())) {
    // Hit the coin!
    score++;
    coins++;

    // Save coins
    std::ofstream coinsSave("coins.spin");
    if (coinsSave.is_open()) {
      coinsSave << coins;
      coinsSave.close();
    }

    // Update UI
    coinCount.setString(std::to_string(coins));
    sound.play();
    sound.setPitch(sound.getPitch() + 0.03f);
    scoreText.setString(std::to_string(score));
    scoreText.setOrigin(scoreText.getLocalBounds().getCenter());

    // Check and update highscore
    if (score > highscore) {
      highscore = score;
      highscoreText.setString("Highscore: " + std::to_string(highscore));
      highscoreText.setOrigin(highscoreText.getLocalBounds().getCenter());

      std::ofstream highscoreSave("highscore.spin");
      if (highscoreSave.is_open()) {
        highscoreSave << highscore;
        highscoreSave.close();
      }
    }

    speed += 0.15f;
    coin_angle = (rand() % 360) * (M_PI / 180.0f);
  } else {
    // Missed!
    score = 0;
    scoreText.setString(std::to_string(score));
    scoreText.setOrigin(scoreText.getLocalBounds().getCenter());
    speed = 1.0f;
    sound.setPitch(1.0f);
  }
}

int main() {
  // Window setup
  sf::ContextSettings settings;
  settings.antiAliasingLevel = 8;

  sf::RenderWindow window(sf::VideoMode({1280, 720}), "SpinGame",
                          sf::Style::Default, sf::State::Windowed, settings);
  window.setIcon(sf::Image("assets/textures/coin.png"));
  window.setFramerateLimit(120);
  window.setVerticalSyncEnabled(true);
  window.setKeyRepeatEnabled(false);

  sf::Vector2f center(window.getSize().x / 2.0f, window.getSize().y / 2.0f);

  // Game state
  float rotation = 0.0f;
  float speed = 1.0f;
  float direction = 1.0f;
  float coin_angle = -90.0f * (M_PI / 180.0f);
  float hitboxAngle = -90.0f * (M_PI / 180.0f);
  int score = 0;
  int coins = 0;
  int highscore = 0;

  // Load saved data
  std::ifstream highscoreLoad("highscore.spin");
  if (highscoreLoad.is_open()) {
    highscoreLoad >> highscore;
    highscoreLoad.close();
  }

  std::ifstream coinsLoad("coins.spin");
  if (coinsLoad.is_open()) {
    coinsLoad >> coins;
    coinsLoad.close();
  }

  // Background circles
  sf::CircleShape bg_circle(200.0f, 128);
  bg_circle.setFillColor(sf::Color::Black);
  bg_circle.setOrigin(
      sf::Vector2f(bg_circle.getRadius(), bg_circle.getRadius()));
  bg_circle.setPosition(center);

  sf::CircleShape circle(120.0f, 128);
  circle.setFillColor(sf::Color::White);
  circle.setOrigin(sf::Vector2f(circle.getRadius(), circle.getRadius()));
  circle.setPosition(center);

  // Player bar
  sf::RectangleShape playerBar({200.0f, 20.0f});
  playerBar.setFillColor(sf::Color::Red);
  playerBar.setOrigin({0.0f, 10.0f});
  playerBar.setPosition(center);
  playerBar.setRotation(sf::degrees(-90.0f));

  // Player hitbox (invisible in game)
  sf::RectangleShape playerHitbox({40.0f, 40.0f});
  playerHitbox.setOrigin({20.0f, 20.0f});

  // Coin sprite
  sf::Texture coinTexture("assets/textures/coin.png");
  sf::Sprite coin(coinTexture);
  coin.setOrigin({8.0f, 8.0f});
  coin.setScale({35.0f / 16.0f, 35.0f / 16.0f});

  // Fonts
  sf::Font silkscreen("assets/fonts/Silkscreen-Regular.ttf");

  // Score text
  sf::Text scoreText(silkscreen);
  scoreText.setCharacterSize(100);
  scoreText.setFillColor(sf::Color::Black);
  scoreText.setString(std::to_string(score));
  scoreText.setOrigin(scoreText.getGlobalBounds().getCenter());
  scoreText.setPosition(center);

  // Highscore text
  sf::Text highscoreText(silkscreen);
  highscoreText.setCharacterSize(48);
  highscoreText.setFillColor(sf::Color::Black);
  highscoreText.setString("Highscore: " + std::to_string(highscore));
  highscoreText.setOrigin(highscoreText.getGlobalBounds().getCenter());
  highscoreText.setPosition({center.x, 85.0f});

  // Sound
  sf::SoundBuffer buffer("assets/sfx/score.wav");
  sf::Sound sound(buffer);

  // Coin counter UI
  sf::Sprite coinIcon(coinTexture);
  coinIcon.setOrigin({8.0f, 8.0f});
  coinIcon.setScale({50.0f / 16.0f, 50.0f / 16.0f});
  coinIcon.setPosition({45.0f, 45.0f});

  sf::Text coinCount(silkscreen);
  coinCount.setString(std::to_string(coins));
  coinCount.setPosition({90.0f, 30.0f});
  coinCount.setFillColor(sf::Color::Black);
  coinCount.setCharacterSize(32);

  // FPS tracking
  int fps = 0;
  time_t last_frame_check = time(NULL);
  sf::Clock clock;
  float frameDuration;
  const float frameSpeed = 60.0f;

  // Random initial coin position
  std::srand(static_cast<unsigned>(time(0)));
  coin_angle = (rand() % 360) * (M_PI / 180.0f);

  // Main game loop
  while (window.isOpen()) {
    // Event handling
    while (const std::optional event = window.pollEvent()) {
      // Handle window resize
      if (const auto *resized = event->getIf<sf::Event::Resized>()) {
        window.setView(sf::View(center, sf::Vector2f(window.getSize())));
      }

      // Close window
      if (event->is<sf::Event::Closed>()) {
        window.close();
      }

      // Keyboard input
      if (const auto *keyPressed = event->getIf<sf::Event::KeyPressed>()) {
        if (keyPressed->scancode == sf::Keyboard::Scancode::Escape) {
          window.close();
        }

        if (keyPressed->scancode == sf::Keyboard::Scancode::Space ||
            keyPressed->scancode == sf::Keyboard::Scancode::E) {
          handleDirectionChange(score, coins, highscore, speed, coin_angle,
                                direction, playerHitbox, coin, scoreText,
                                coinCount, highscoreText, sound);
        }
      }

      // Mouse input
      if (const auto *mouseButton =
              event->getIf<sf::Event::MouseButtonPressed>()) {
        if (mouseButton->button == sf::Mouse::Button::Left) {
          handleDirectionChange(score, coins, highscore, speed, coin_angle,
                                direction, playerHitbox, coin, scoreText,
                                coinCount, highscoreText, sound);
        }
      }

      // Touch input (iOS)
      if (event->is<sf::Event::TouchBegan>()) {
        handleDirectionChange(score, coins, highscore, speed, coin_angle,
                              direction, playerHitbox, coin, scoreText,
                              coinCount, highscoreText, sound);
      }
    }

    // Update game state
    frameDuration = clock.restart().asSeconds();

    coin.setPosition({center.x + 160.0f * cos(coin_angle),
                      center.y + 160.0f * sin(coin_angle)});

    playerHitbox.setPosition({center.x + 160.0f * cos(hitboxAngle),
                              center.y + 160.0f * sin(hitboxAngle)});

    playerBar.setRotation(sf::degrees(rotation - 90.0f));
    rotation += speed * direction * (frameDuration * frameSpeed);
    hitboxAngle = (rotation - 90.0f) * (M_PI / 180.0f);

    // Render
    window.clear(sf::Color(80, 165, 250));
    window.draw(bg_circle);
    window.draw(playerBar);
    window.draw(circle);
    window.draw(coin);
    window.draw(coinIcon);
    window.draw(coinCount);
    window.draw(highscoreText);
    window.draw(scoreText);
    window.display();

    // FPS counter
    fps++;
    if ((time(NULL) - last_frame_check) >= 1) {
      window.setTitle("SpinGame | FPS: " + std::to_string(fps));
      fps = 0;
      last_frame_check = time(NULL);
    }
  }

  return 0;
}
