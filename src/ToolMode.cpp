#include <SFML/Graphics.hpp>
enum class ToolMode
{
    Drag,
    Spawn,
    CreateChain
};

class ToolMenu
{
public:
    ToolMenu(const sf::Font &font)
        : items{
              sf::Text(font),
              sf::Text(font),
              sf::Text(font)}
    {
        items[0].setString("1. Drag");
        items[0].setPosition({10.f, 10.f});

        items[1].setString("2. Spawn Object");
        items[1].setPosition({10.f, 40.f});

        items[2].setString("3. Create Chain");
        items[2].setPosition({10.f, 70.f});

        setActive(ToolMode::Drag);
    }

    void setActive(ToolMode mode)
    {
        active = mode;
        for (auto &i : items)
            i.setFillColor(sf::Color::White);

        items[(int)mode].setFillColor(sf::Color::Yellow);
    }

    ToolMode getActive() const { return active; }

    void draw(sf::RenderWindow &window)
    {
        for (auto &i : items)
            window.draw(i);
    }

private:
    sf::Text items[3];
    ToolMode active;
};
