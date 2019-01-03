#ifndef GAME_HPP
#define GAME_HPP

#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <vector>
#include <set>

class Game
{
public:
    explicit Game();
    void run();
private:
    enum
    {
        FPS = 5,
        MAP_WIDTH = 16,
        MAP_HEIGHT = 16,
        CELL_SIZE = 40,
        BORDER_SIZE = 5,
        SCREEN_WIDTH = MAP_WIDTH * CELL_SIZE + BORDER_SIZE,
        SCREEN_HEIGHT = MAP_HEIGHT * CELL_SIZE + BORDER_SIZE
    };

    enum class Connectivity { FOUR, EIGHT };

    const std::string TITLE_OF_MAIN_WINDOW { "FlowFields" };
    const sf::Time mFrameTime { sf::seconds(1.0f /FPS) };
    sf::RenderWindow mWindow;
    struct Node
    {
        int x, y;
        int distance;
        bool obstacle;
        std::vector<Node*> neighbours;
    };
    Node mMap[MAP_WIDTH][MAP_HEIGHT];
    Node *mStart;
    Node *mEnd;
    struct NodeCompare
    {
        bool operator()(Node* lhs, Node* rhs)
        {
            auto index = [](int x, int y){ return y * MAP_WIDTH + x; };
            return index(lhs->x, lhs->y) < index(rhs->x, rhs->y);
        }
    };
    std::set<Node*, NodeCompare> mOldWave;
    std::set<Node*, NodeCompare> mCurrentWave;
    bool mAgorythmFinished = false;
    bool mIsPaused = false;
    float mMaxDist = 0;

    void restart();
    void initMap();
    void resetMap();
    void createConnections(Connectivity connectivity);
    bool isValidCoords(int x, int y) const;
    void inputPhase();
    void updatePhase(sf::Time frameTime);
    void renderPhase();
    void togglePause();
    void centralizeWindow();
};

#endif // GAME_HPP
