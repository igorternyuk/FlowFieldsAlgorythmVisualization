#include "game.hpp"
#include <algorithm>
#include <iostream>

Game::Game():
      mWindow(sf::VideoMode(SCREEN_WIDTH, SCREEN_HEIGHT), TITLE_OF_MAIN_WINDOW,
              sf::Style::Close)
{
    mWindow.setFramerateLimit(FPS);
    centralizeWindow();
    initMap();
    createConnections(Connectivity::FOUR);
    mStart = &mMap[1][1];
    mEnd = &mMap[MAP_HEIGHT - 2][MAP_WIDTH - 2];
    mOldWave.insert(mStart);
}

void Game::run()
{
    sf::Clock clock;
    sf::Time timeSinceLastUpdate = sf::Time::Zero;
    while (mWindow.isOpen())
    {
        auto elapsedTime = clock.restart();
        timeSinceLastUpdate += elapsedTime;
        while (timeSinceLastUpdate > mFrameTime) {
            timeSinceLastUpdate -= mFrameTime;
            inputPhase();
            updatePhase(mFrameTime);
        }
        inputPhase();
        updatePhase(mFrameTime);
        renderPhase();
    }
}

void Game::restart()
{
    resetMap();
    mOldWave.clear();
    mOldWave.insert(mStart);
    mCurrentWave.clear();
    mAgorythmFinished = false;
    mMaxDist = 0;
}

void Game::initMap()
{
    for(int y = 0; y < MAP_HEIGHT; ++y)
    {
        for(int x = 0; x < MAP_WIDTH; ++x)
        {
            mMap[y][x].x = x;
            mMap[y][x].y = y;
            mMap[y][x].distance = 0;
            mMap[y][x].obstacle = x == 0 || x == MAP_WIDTH - 1 || y == 0 || y == MAP_HEIGHT - 1;
        }
    }
}

void Game::resetMap()
{
    for(int y = 0; y < MAP_HEIGHT; ++y)
    {
        for(int x = 0; x < MAP_WIDTH; ++x)
        {
            mMap[y][x].x = x;
            mMap[y][x].y = y;
            mMap[y][x].distance = 0;
        }
    }
}

void Game::createConnections(Game::Connectivity connectivity)
{
    for(int y = 0; y < MAP_HEIGHT; ++y)
    {
        for(int x = 0; x < MAP_WIDTH; ++x)
        {
            mMap[y][x].neighbours.clear();
            if(connectivity == Connectivity::FOUR)
            {
                const int dx[] = { 1, 0, -1, 0 };
                const int dy[] = { 0, 1, 0, -1 };
                for(int dir = 0; dir < 4; ++dir)
                {
                    int nx = x + dx[dir];
                    int ny = y + dy[dir];
                    if(isValidCoords(nx, ny))
                    {
                        mMap[y][x].neighbours.push_back(&mMap[ny][nx]);
                    }
                }
            }
            else if(connectivity == Connectivity::EIGHT)
            {
                for(int dx = -1; dx <= 1; ++dx)
                {
                    for(int dy = -1; dy <= 1; ++dy)
                    {
                        if(dx == 0 && dy == 0) continue;
                        int nx = x + dx;
                        int ny = y + dy;
                        if(isValidCoords(nx, ny))
                        {
                            mMap[y][x].neighbours.push_back(&mMap[ny][nx]);
                        }
                    }
                }
            }
        }
    }
}

bool Game::isValidCoords(int x, int y) const
{
    return x >= 0 && x < MAP_WIDTH && y >= 0 && y < MAP_HEIGHT;
}

void Game::inputPhase()
{
    sf::Event event;
    while (mWindow.pollEvent(event))
    {
        if(event.type == sf::Event::Closed)
        {
            mWindow.close();
        }
        else if(event.type == sf::Event::MouseButtonReleased)
        {
            int mx = event.mouseButton.x / CELL_SIZE;
            int my = event.mouseButton.y / CELL_SIZE;

            if(event.mouseButton.button == sf::Mouse::Left)
            {
                mMap[my][mx].obstacle = false;
                mStart = &mMap[my][mx];
            }
            else if(event.mouseButton.button == sf::Mouse::Right)
            {
                mMap[my][mx].obstacle = false;
                mEnd = &mMap[my][mx];
            }
            else if(event.mouseButton.button == sf::Mouse::Middle)
            {
                if(&mMap[my][mx] != mStart && &mMap[my][mx] != mEnd)
                {
                    mMap[my][mx].obstacle = !mMap[my][mx].obstacle;
                }
            }
            restart();
        }
        else if(event.type == sf::Event::KeyReleased)
        {
            if(event.key.code == sf::Keyboard::Return)
            {
                restart();
            }
            else if(event.key.code == sf::Keyboard::Space)
            {
                togglePause();
            }
        }
    }
}

void Game::updatePhase(sf::Time frameTime)
{
    if(mIsPaused) return;
    if(!mAgorythmFinished && !mOldWave.empty())
    {
        mCurrentWave.clear();
        for(auto &node: mOldWave)
        {
            auto neighbours = node->neighbours;
            for(auto &neighbour: neighbours)
            {
                if(!neighbour->obstacle && neighbour->distance == 0)
                {
                    neighbour->distance = node->distance + 1;
                    mCurrentWave.insert(neighbour);
                    if(mMaxDist < neighbour->distance)
                    {
                        mMaxDist = neighbour->distance;
                    }
                }
            }
        }
        mOldWave.clear();
        for(auto &node: mCurrentWave)
        {
            mOldWave.insert(node);
        }
    }
    else
    {
        mAgorythmFinished = true;
    }

}

void Game::renderPhase()
{
    mWindow.clear();
    for(int y = 0; y < MAP_HEIGHT; ++y)
    {
        for(int x = 0; x < MAP_WIDTH; ++x)
        {
            sf::RectangleShape node;
            node.setPosition(mMap[y][x].x * CELL_SIZE + BORDER_SIZE,
                             mMap[y][x].y * CELL_SIZE + BORDER_SIZE);
            node.setSize({ CELL_SIZE - BORDER_SIZE, CELL_SIZE - BORDER_SIZE });

            float blue = float(mMap[y][x].distance / mMaxDist);
            node.setFillColor(sf::Color(0, 0, int(blue * 255.f)));

            if(mMap[y][x].obstacle)
            {
                node.setFillColor(sf::Color::White);
            }
            if(&mMap[y][x] == mStart)
            {
                node.setFillColor(sf::Color::Green);
            }
            if(&mMap[y][x] == mEnd)
            {
                node.setFillColor(sf::Color::Red);
            }
            mWindow.draw(node);
        }
    }

    for(auto &node: mCurrentWave)
    {
        sf::RectangleShape rect;
        rect.setPosition(node->x * CELL_SIZE + BORDER_SIZE,
                         node->y * CELL_SIZE + BORDER_SIZE);
        rect.setSize({ CELL_SIZE - BORDER_SIZE, CELL_SIZE - BORDER_SIZE });
        rect.setFillColor(sf::Color::Green);
        mWindow.draw(rect);
    }

    mWindow.display();
}

void Game::togglePause()
{
    mIsPaused = !mIsPaused;
}

void Game::centralizeWindow()
{
    const int screenWidth = sf::VideoMode::getDesktopMode().width;
    const int screenHeight = sf::VideoMode::getDesktopMode().height;
    mWindow.setPosition(sf::Vector2i((screenWidth - SCREEN_WIDTH) / 2,
                                         (screenHeight - SCREEN_HEIGHT) / 2));
}
