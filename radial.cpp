#include <cassert>
#include <cmath>
#include <iostream>
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>

float k_1 = -0.05436;
float k_2 = -0.45894;
float k_3 =  0.04944;

sf::Texture         texture;
sf::RectangleShape  rect1;
sf::RectangleShape  rect2;

float dot(const sf::Vector2f& v1, const sf::Vector2f& v2)
{
  return (v1.x * v2.x) + (v1.y * v2.y);
}

float length(const sf::Vector2f& v)
{
  return std::sqrt(dot(v, v));
}

void draw_texture(sf::RenderWindow& renderWindow)
{
  renderWindow.draw(rect1);
  renderWindow.draw(rect2);
}

sf::Image distort(const sf::Image& in)
{
  sf::Image ret;
  ret.create(in.getSize().x, in.getSize().y, sf::Color(0, 0, 0));

  auto center = sf::Vector2f(in.getSize().x, in.getSize().y) / 2.f;
  for(std::size_t x = 0, x_size = in.getSize().x; x < x_size; ++x)
  {
    for(std::size_t y = 0, y_size = in.getSize().y; y < y_size; ++y)
    {
      // formula for distortion:
      // https://cgcooke.github.io/Blog/optimisation/computer%20vision/2020/04/05/Inverse-Radial-Distortion.pynb.html
      // x_d = x * (1 + (k_1 * r) + (k_2 * r^2) + (k_3 * r^3))
      // y_d = y * (1 + (k_1 * r) + (k_2 * r^2) + (k_3 * r^3))
      // 
      // solving for x and y, we have:
      // x = x_d / (1 + (k_1 * r) + (k_2 * r^2) + (k_3 * r^3))
      // y = y_d / (1 + (k_1 * r) + (k_2 * r^2) + (k_3 * r^3))
      auto p = sf::Vector2f(x, y);

      // The formula requires that the image is from [-1, 1] in both the x-y coordinates, where the center is at (0, 0), so
      // we map the coordinates to [-0.5, 0.5]
      sf::Transform m1(1.f / float(x_size), 0.f,                  -(center.x / float(x_size)),
                       0.f,                 1.f / float(y_size),  -(center.y / float(y_size)),
                       0.f,                 0.f,                   1.f);
      auto np = m1.transformPoint(p);

      float r = length(np);
      float d = (1.f + (k_1 * r) + (k_2 * r * r) + (k_3 * r * r * r));

      sf::Transform m2(x_size / float(d),     0.f,                   center.x,
                       0.f,                   y_size / float(d),     center.y,
                       0.f,                   0.f,                   1.f);
      auto up = m2.transformPoint(np);
      if(up.x < 0 || up.x > in.getSize().x ||
         up.y < 0 || up.y > in.getSize().y)
      {
        continue;
      }

      ret.setPixel(x, y, in.getPixel(up.x, up.y));
    }
  }

  return ret;
}

int main()
{
  sf::RenderWindow renderWindow(sf::VideoMode(800, 600), "Distorted Image");

  sf::Image image;
  image.loadFromFile("image.jpg");

  auto distorted_image = distort(image);
  texture.loadFromImage(distorted_image);

  rect1.setSize(sf::Vector2f(400, 400));
  rect1.setPosition((renderWindow.getSize().x / 2) - (rect1.getSize().x / 2),
                   (renderWindow.getSize().y / 2) - (rect1.getSize().y / 2));
  rect1.setTexture(&texture);
  rect1.setPosition(rect1.getPosition().x - (rect1.getSize().x / 2), rect1.getPosition().y);

  rect2.setSize(sf::Vector2f(400, 400));
  rect2.setPosition((renderWindow.getSize().x / 2) - (rect2.getSize().x / 2),
                    (renderWindow.getSize().y / 2) - (rect2.getSize().y / 2));
  rect2.setTexture(&texture);
  rect2.setPosition(rect2.getPosition().x + (rect2.getSize().x / 2), rect2.getPosition().y);

  while(renderWindow.isOpen())
  {
    sf::Event event;
    while(renderWindow.pollEvent(event))
    {
      switch(event.type)
      {
      case sf::Event::Closed:
        renderWindow.close();
      default:
        break;
      }
    }

    renderWindow.clear();
    draw_texture(renderWindow);
    renderWindow.display();
  }

  return 0;
}
