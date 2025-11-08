// Project: SpinGame
//
// Author: Andrej Geller
// Created: 1/18/2025 6:53:28 PM
#define _USE_MATH_DEFINES
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <fstream>
#include <iostream>
#include <math.h>

int main() {
  sf::ContextSettings settings;
  settings.antiAliasingLevel = 8;
  bool fullscreen = false;

  sf::RenderWindow window(sf::VideoMode({1280, 720}), "SpinGame",
                          sf::Style::Default, sf::State::Windowed, settings);
  window.setIcon(sf::Image("resources/textures/coin.png"));
  window.setFramerateLimit(120);
  window.setVerticalSyncEnabled(true);
  window.setKeyRepeatEnabled(false);

  sf::Vector2f center =
      (sf::Vector2f(window.getSize().x / 2, window.getSize().y / 2));

  float rotation = 0;
  float speed = 1;
  float direction = 1;
  float coin_angle = -90 * (M_PI / 180.0f); // 0 degrees
  float hitboxAngle = -90 * (M_PI / 180.0f);
  int score = 0;
  int coins = 0;
  int highscore = 0;

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

  sf::CircleShape bg_circle(200.f, 128);
  bg_circle.setFillColor(sf::Color::Red);
  // bg_circle.setOutlineThickness(20);
  // bg_circle.setOutlineColor(sf::Color::White);
  bg_circle.setOrigin(
      sf::Vector2f(bg_circle.getRadius(), bg_circle.getRadius()));
  bg_circle.setPosition(center);

  sf::CircleShape circle(120.f, 128);
  circle.setFillColor(sf::Color::White);
  circle.setOrigin(sf::Vector2f(circle.getRadius(), circle.getRadius()));
  circle.setPosition(center);

  sf::RectangleShape playerBar({200.f, 20.f});
  playerBar.setFillColor(sf::Color::Red);
  playerBar.setOrigin({0, 10.f});
  playerBar.setPosition(center);
  playerBar.setRotation(sf::degrees(-90));

  sf::RectangleShape playerHitbox({40.f, 40.f});
  playerHitbox.setFillColor(sf::Color(0, 0, 220, 150));
  playerHitbox.setOutlineColor(sf::Color(0, 0, 220));
  playerHitbox.setOutlineThickness(-2);
  playerHitbox.setOrigin({20.f, 20.f});

  // Coin
  sf::Texture coinTexture("resources/textures/coin.png");
  sf::Sprite coin(coinTexture);
  coin.scale({35.f, 35.f});
  coin.setOrigin({8.f, 8.f});
  coin.setScale({35.f / 16.f, 35.f / 16.f});

  // Fonts && Text
  sf::Font arial("resources/fonts/arial.ttf");
  sf::Font silkscreen("resources/fonts/Silkscreen-Regular.ttf");
  sf::Text scoreText(silkscreen);
  scoreText.setCharacterSize(100);
  scoreText.setFillColor(sf::Color::Black);
  scoreText.setString(std::to_string(score));
  scoreText.setOrigin(scoreText.getGlobalBounds().getCenter());
  scoreText.setPosition({center.x, center.y});

  sf::Text highscoreText(silkscreen);
  highscoreText.setCharacterSize(48);
  highscoreText.setFillColor(sf::Color::Black);
  highscoreText.setString("Highscore: " + std::to_string(highscore));
  highscoreText.setOrigin(highscoreText.getGlobalBounds().getCenter());
  highscoreText.setPosition({center.x, 85});

  // SFX
  sf::SoundBuffer buffer("resources/sfx/score.wav");
  sf::Sound sound(buffer);

  // Coin anzeige
  sf::Sprite coinIcon(coinTexture);
  coinIcon.scale({50.f, 50.f});
  coinIcon.setOrigin({8.f, 8.f});
  coinIcon.setScale({50.f / 16.f, 50.f / 16.f});
  coinIcon.setPosition({45, 45});

  sf::Text coinCount(silkscreen);
  coinCount.setString(std::to_string(coins));
  coinCount.setPosition({45, 45});
  coinCount.setFillColor(sf::Color::Black);

  // Shop
  sf::RectangleShape shopBG({250.f, 595.f});
  shopBG.setFillColor(sf::Color(0, 92, 204));
  shopBG.setPosition({15.f, 95.f});

  sf::Text shopTitle(silkscreen);
  shopTitle.setOrigin(shopTitle.getGlobalBounds().getCenter());
  shopTitle.setPosition({70.f, 110.f});
  shopTitle.setString("SHOP");
  shopTitle.setCharacterSize(45);

  // Buttons
  sf::RectangleShape shopBtn[6];
  shopBtn[0].setSize({230.f, 75.f});
  shopBtn[0].setFillColor(sf::Color(64, 150, 255));
  shopBtn[0].setPosition({25.f, 180.f});

  for (int i = 1; i < 6; i++) {
    shopBtn[i].setSize({230.f, 75.f});
    shopBtn[i].setFillColor(shopBtn[0].getFillColor());
    shopBtn[i].setPosition(sf::Vector2f(
        shopBtn[0].getPosition().x,
        shopBtn[0].getPosition().y + (shopBtn[0].getSize().y + 10.f) * i + 1));
  }

  int fps = 0;
  time_t last_frame_check = time(NULL);
  sf::Clock clock;
  float fd;        // Frame duration
  float fs = 60.f; // Frame speed

  std::srand(time(0));
  coin_angle = (rand() % 360) * (M_PI / 180.0f);

  while (window.isOpen()) {
    while (const std::optional event = window.pollEvent()) {
      // Scale content properly
      if (const auto *resized = event->getIf<sf::Event::Resized>()) {
        window.setView(sf::View(center, sf::Vector2f(window.getSize())));
      }

      // Close Game
      if (event->is<sf::Event::Closed>()) {
        window.close();
      } else if (const auto *keyPressed =
                     event->getIf<sf::Event::KeyPressed>()) {
        if (keyPressed->scancode == sf::Keyboard::Scancode::Escape) {
          window.close();
        }
        if (keyPressed->scancode == sf::Keyboard::Scancode::F11) {
          if (!fullscreen) {
            // Resize content properly
            window.create(sf::VideoMode::getDesktopMode(), "SpinGame",
                          sf::State::Fullscreen, settings);
            fullscreen = true;
            window.setIcon(sf::Image("resources/textures/coin.png"));
            window.setFramerateLimit(120);
            window.setKeyRepeatEnabled(false);
            window.setView(sf::View(center, sf::Vector2f(window.getSize())));
          } else {
            window.create(sf::VideoMode({1280, 720}), "SpinGame",
                          sf::Style::Default, sf::State::Windowed, settings);
            fullscreen = false;
            window.setIcon(sf::Image("resources/textures/coin.png"));
            window.setFramerateLimit(120);
            window.setKeyRepeatEnabled(false);
            window.setView(sf::View(center, sf::Vector2f(window.getSize())));
          }
        }
      }

      // Keyboard Controls
      if (const auto *keyPressed = event->getIf<sf::Event::KeyPressed>()) {
        if (keyPressed->scancode == sf::Keyboard::Scancode::Space ||
            keyPressed->scancode == sf::Keyboard::Scancode::E) {
          direction = direction * -1;
          if (playerHitbox.getGlobalBounds().findIntersection(
                  coin.getGlobalBounds())) {
            std::cout << "true" << std::endl;
            score++;
            coins++;
            std::ofstream coinsSave("coins.spin");
            if (coinsSave.is_open()) {
              coinsSave << coins;
              coinsSave.close();
            }
            coinCount.setString(std::to_string(coins));
            sound.play();
            sound.setPitch(sound.getPitch() + 0.03f);
            scoreText.setString(std::to_string(score));
            scoreText.setOrigin(scoreText.getLocalBounds().getCenter());

            if (score > highscore) {
              highscore = score;
              highscoreText.setString("Highscore: " +
                                      std::to_string(highscore));
              highscoreText.setOrigin(
                  highscoreText.getLocalBounds().getCenter());
              std::ofstream highscoreSave("highscore.spin");
              if (highscoreSave.is_open()) {
                highscoreSave << highscore;
                highscoreSave.close();
              }
            }

            speed += 0.15;
            coin_angle = (rand() % 360) * (M_PI / 180.0f);
          } else {
            score = 0;
            scoreText.setString(std::to_string(score));
            scoreText.setOrigin(scoreText.getLocalBounds().getCenter());
            speed = 1;
            sound.setPitch(1.0f);
          }
        }
      }

      // Mouse Controls
      if (const auto *mouseButton =
              event->getIf<sf::Event::MouseButtonPressed>()) {
        if (mouseButton->button == sf::Mouse::Button::Left) {
          direction = direction * -1;
          if (playerHitbox.getGlobalBounds().findIntersection(
                  coin.getGlobalBounds())) {
            std::cout << "true" << std::endl;
            score++;
            coins++;
            std::ofstream coinsSave("coins.spin");
            if (coinsSave.is_open()) {
              coinsSave << coins;
              coinsSave.close();
            }
            coinCount.setString(std::to_string(coins));
            sound.play();
            sound.setPitch(sound.getPitch() + 0.03f);
            scoreText.setString(std::to_string(score));
            scoreText.setOrigin(scoreText.getLocalBounds().getCenter());

            if (score > highscore) {
              highscore = score;
              highscoreText.setString("Highscore: " +
                                      std::to_string(highscore));
              highscoreText.setOrigin(
                  highscoreText.getLocalBounds().getCenter());

              std::ofstream highscoreSave("highscore.spin");
              if (highscoreSave.is_open()) {
                highscoreSave << highscore;
                highscoreSave.close();
              }
            }

            speed += 0.15;
            coin_angle = (rand() % 360) * (M_PI / 180.0f);
          } else {
            score = 0;
            scoreText.setString(std::to_string(score));
            scoreText.setOrigin(scoreText.getLocalBounds().getCenter());
            speed = 1;
            sound.setPitch(1.0f);
          }
        }
      }
    }

    // Calculate frame duration
    fd = clock.restart().asSeconds();
    // 0.0165

    coin.setPosition({center.x + 160.f * cos(coin_angle),
                      center.y + 160.f * sin(coin_angle)});
    // coin.setRotation(sf::radians(coin_angle));

    playerHitbox.setPosition({center.x + 160.f * cos(hitboxAngle),
                              center.y + 160.f * sin(hitboxAngle)});

    // Shop hover
    std::cout << "X: " << sf::Mouse::getPosition(window).x
              << " | Y: " << sf::Mouse::getPosition(window).y
              << "\nX-Shop: " << shopBtn[0].getPosition().x
              << "Y - Shop: " << shopBtn[0].getPosition().y;
    /*if (shopBtn.getGlobalBounds().contains(sf::Mouse::getPosition()))
    {
            cout << true;
    }*/

    window.clear(sf::Color(80, 165, 250));
    window.draw(coinIcon);
    window.draw(coinCount);
    window.draw(shopBG);
    for (sf::RectangleShape button : shopBtn) {
      window.draw(button);
    }
    window.draw(shopTitle);
    window.draw(bg_circle);
    window.draw(playerBar);
    window.draw(circle);
    // window.draw(playerHitbox);
    window.draw(coin);
    window.draw(highscoreText);
    window.draw(scoreText);
    window.display();

    playerBar.setRotation(sf::degrees(rotation - 90));
    rotation += speed * direction * (fd * fs);
    hitboxAngle = (rotation - 90) * (M_PI / 180.0f);
    // coin_angle += (1) * (M_PI / 180.0f);

    // FPS Counter
    fps++;
    if ((time(NULL) - last_frame_check) >= 1) {
      window.setTitle("SpinGame | FPS: " + std::to_string(fps));
      std::cout << "FPS: " << fps << std::endl;
      fps = 0;
      last_frame_check = time(NULL);
    }
    // std::cout << fd << std::endl;
  }
}
