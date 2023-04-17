#include "game.h"
#include "resource_manager.h"
#include "sprite_renderer.h"
#include "gameobject.h"
#include <ctime>
#include <cstdlib>
#include <vector>
#include <iostream>
#include <unistd.h>
#include "text_renderer.h"
#include "shader.h"
// Game-related State data
SpriteRenderer *Renderer;
TextRenderer *Text;
float bg_pos = 0.0f;
int jetpack_flag = 0;
int no_of_meats[3] = {2 * 500, 2 * 500, 2 * 500};
int meat_space[3] = {80, 80, 80};
// Initial size of the player luffy
const glm::vec2 PLAYER_SIZE(0.8 * 60.25f, 0.8 * 113.5f);
// Initial velocity of the player luffy
float PLAYER_VELOCITY(350.0f);

GameObject *Player;
std::vector<GameObject> meat[3];
int no_of_weapons[3] = {250, 250, 250};
int weapon_space[3] = {800, 750, 700};
// int weapon_space[3] = {550, 540, 520};
int weapon_rotate[3] = {3, 5, 7};
std::vector<GameObject> weapon[3];
int meat_speed[3] = {3, 5, 7};
int weapon_speed[3] = {9, 11, 12};
int level_number = 0;
int level_time = 15;
int meat_nomnom = 0;
int game_end = 0;
int game_won = 0;
int distance = 0;
int time_from_start = 0;
int time_from_restart = 0;
int coll_acc = 10;
int glow_flag = 0;

Game::Game(unsigned int width, unsigned int height)
    : State(GAME_MENU), Keys(), Width(width), Height(height)
{
}

Game::~Game()
{
    delete Renderer;
}

void Game::Init()
{
    srand(time(0));
    // load shaders
    ResourceManager::LoadShader("../shader/vertex.vs", "../shader/fragment.frag", nullptr, "sprite");
    // configure shaders
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(this->Width),
                                      static_cast<float>(this->Height), 0.0f, -1.0f, 1.0f);
    ResourceManager::GetShader("sprite").Use().SetInteger("image", 0);
    ResourceManager::GetShader("sprite").SetMatrix4("projection", projection);
    // set render-specific controls
    // Renderer = new SpriteRenderer(ResourceManager::GetShader("sprite"));
    Shader myShader;
    myShader = ResourceManager::GetShader("sprite");
    Renderer = new SpriteRenderer(myShader);
    // load textures
    ResourceManager::LoadTexture("../textures/Wano.png", true, "background");
    ResourceManager::LoadTexture("../textures/meat.png", true, "meat");
    ResourceManager::LoadTexture("../textures/weapon.png", true, "weapon");
    ResourceManager::LoadTexture("../textures/glowweapon.png", true, "weapon1");
    ResourceManager::LoadTexture("../textures/weapon1.png", true, "weapon2");

    ResourceManager::LoadTexture("../textures/luffy.png", true, "luffy");
    ResourceManager::LoadTexture("../textures/glowluffy.png", true, "glowluffy");

    glm::vec2 playerPos = glm::vec2(5, 450);
    Player = new GameObject(playerPos, PLAYER_SIZE, ResourceManager::GetTexture("luffy"));

    for (int j = 0; j < 3; ++j)
    {
        float prev_x = 400;
        for (int i = 0; i < no_of_meats[j]; ++i)
        {
            float y_pos = 100 + (rand() % 280);
            float x_pos = meat_space[j] + prev_x;

            GameObject onemeat(glm::vec2(x_pos, y_pos), glm::vec2(50.0f, 50.0f), ResourceManager::GetTexture("meat"), glm::vec3(1.0f, 1.0f, 1.0f));
            meat[j].push_back(onemeat);
            prev_x = x_pos;
        }

        float prev_x_w = 400;
        for (int i = 0; i < no_of_weapons[j]; ++i)
        {
            float y_pos = 40 + (rand() % 300);
            float x_pos = weapon_space[j] + prev_x_w;

            GameObject oneweapon(glm::vec2(x_pos, y_pos), glm::vec2(1.5 * 29.5f, 1.5 * 91.25f), ResourceManager::GetTexture("weapon1"), glm::vec3(0.6f, 0.6f, 1.0f));

            weapon[j].push_back(oneweapon);
            prev_x_w = x_pos;
        }
    }
    Text = new TextRenderer(this->Width, this->Height);
    Text->Load("../fonts/font1.ttf", 24);
    time_from_start = glfwGetTime();
}

bool CheckCollision(GameObject &one, GameObject &two) // AABB - AABB collision
{
    // collision x-axis?
    bool collisionX = one.Position.x + one.Size.x >= two.Position.x + coll_acc &&
                      two.Position.x + two.Size.x >= one.Position.x + coll_acc;
    // collision y-axis?
    bool collisionY = one.Position.y + one.Size.y >= two.Position.y + coll_acc &&
                      two.Position.y + two.Size.y >= one.Position.y + coll_acc;
    // collision only if on both axes
    return collisionX && collisionY;
}
void Game::Update(float dt)
{

    if (this->State == GAME_ACTIVE)
    {
        if ((glfwGetTime() - time_from_restart) < level_time)
        {
            if (!game_end)
                distance = (glfwGetTime() - time_from_restart) * meat_speed[0];

            level_number = 0;
        }
        else if ((glfwGetTime() - time_from_restart) < level_time * 2)
        {

            if (!game_end)

                distance = level_time * meat_speed[0] + ((glfwGetTime() - time_from_restart) - level_time) * meat_speed[1];

            level_number = 1;
        }
        else if ((glfwGetTime() - time_from_restart) < level_time * 3)
        {
            if (!game_end)

                distance = level_time * meat_speed[0] + level_time * meat_speed[1] + ((glfwGetTime() - time_from_restart) - 2 * level_time) * meat_speed[2];

            level_number = 2;
        }
        else if ((glfwGetTime() - time_from_restart) < level_time * 4)
        {
            game_won = 1;
            this->State = GAME_MENU;
            for (int i = 0; i < no_of_meats[level_number]; ++i)
            {
                meat[level_number][i].Destroyed = true;
            }
            for (int i = 0; i < no_of_weapons[level_number]; ++i)
            {
                weapon[level_number][i].Destroyed = true;
            }
        }
    }

    if (this->State == GAME_ACTIVE && !game_end && !game_won)
    {
        bg_pos -= meat_speed[level_number];
        if (bg_pos <= -800)
            bg_pos = 0;

        if (jetpack_flag)
        {
            if (Player->Position.y >= 50.0f && jetpack_flag)
                Player->Position.y -= PLAYER_VELOCITY * dt - 5 * dt * dt;
        }
        else
        {
            if (Player->Position.y <= 450.0f && !jetpack_flag)
                Player->Position.y += PLAYER_VELOCITY * dt + 5 * dt * dt;
        }
        for (int i = 0; i < no_of_meats[level_number]; ++i)
        {
            meat[level_number][i].Position.x -= 5;
        }
        for (int i = 0; i < no_of_meats[level_number]; ++i)
        {
            if (!meat[level_number][i].Destroyed)
            {
                GameObject *meat_coll = &meat[level_number][i];
                if (CheckCollision(*Player, *meat_coll))
                {
                    meat[level_number][i].Destroyed = true;
                    meat_nomnom++;
                }
            }
        }
        for (int i = 0; i < no_of_weapons[level_number]; ++i)
        {

            GameObject *weapon_coll = &weapon[level_number][i];
            if (CheckCollision(*Player, *weapon_coll))
            {
                game_end = 1;
                this->State = GAME_MENU;
                for (int j = 0; j < 3; ++j)
                {
                    for (int i = 0; i < no_of_meats[j]; ++i)
                    {
                        meat[j][i].Destroyed = true;
                    }
                    for (int i = 0; i < no_of_weapons[j]; ++i)
                    {
                        weapon[j][i].Destroyed = true;
                    }
                }
            }
        }

        for (int i = 0; i < no_of_weapons[level_number]; ++i)
        {
            weapon[level_number][i].Position.x -= 10;
            int time=glfwGetTime();
            if (time%2==0)           
             {
                weapon[level_number][i].Sprite = ResourceManager::GetTexture("weapon2");
            }
            else
            {
                weapon[level_number][i].Sprite = ResourceManager::GetTexture("weapon1");
            }
            weapon[level_number][i].Rotation -= weapon_rotate[level_number];
        }
    }
}

void Game::ProcessInput(float dt, GLFWwindow *window)
{
    if (this->State == GAME_ACTIVE)
    {
        float velocity = PLAYER_VELOCITY * dt;

        if (this->Keys[GLFW_KEY_SPACE])
        {
            if (!jetpack_flag)
            {
                jetpack_flag = 1;
            }
        }
        else
        {
            jetpack_flag = 0;
        }
    }
    if (this->Keys[GLFW_KEY_ENTER])
    {
        time_from_restart = glfwGetTime();
        if (GAME_MENU)
            this->State = GAME_ACTIVE;
    }
    if (this->Keys[GLFW_KEY_R])
    {
        this->State = GAME_ACTIVE;
        time_from_restart = glfwGetTime();
        level_number = 0;
        distance = 0;
        meat_nomnom = 0;
        for (int j = 0; j < 3; ++j)
        {
            for (int i = 0; i < no_of_meats[j]; ++i)
            {
                meat[j][i].Destroyed = false;
            }
            for (int i = 0; i < no_of_weapons[j]; ++i)
            {
                weapon[j][i].Destroyed = false;
            }
        }
        game_end = 0;
        game_won = 0;
        Player->Position.y = 450;
    }
    if (this->Keys[GLFW_KEY_ESCAPE])
        glfwSetWindowShouldClose(window, true);
}

void Game::Render()
{
    // Renderer->DrawSprite(ResourceManager::GetTexture("background"), glm::vec2(200.0f, 200.0f), glm::vec2(300.0f, 400.0f), 45.0f, glm::vec3(0.0f, 1.0f, 0.0f));
    Texture2D myTexture;
    myTexture = ResourceManager::GetTexture("background");
    Renderer->DrawSprite(myTexture, glm::vec2(bg_pos, 0.0f), glm::vec2(800.0f, 600.0f), 0.0f, glm::vec3(0.8f, 0.5f, 0.7f));
    if (bg_pos != 0)
    {
        Renderer->DrawSprite(myTexture, glm::vec2(800 + bg_pos, 0.0f), glm::vec2(800.0f, 600.0f), 0.0f, glm::vec3(0.8f, 0.5f, 0.7f));
    }

    // Texture2D meatTexture;
    // meatTexture = ResourceManager::GetTexture("meat");

    // Renderer->DrawSprite(meatTexture, glm::vec2(bg_pos+x_pos, y_pos), glm::vec2(80.0f, 60.0f), 0.0f, glm::vec3(1.0f, 1.0f, 1.0f));
    Player->Draw(*Renderer);
    if (this->State == GAME_ACTIVE)
    {
        for (int i = 0; i < no_of_meats[level_number]; ++i)
        {

            meat[level_number][i].Draw(*Renderer);
        }

        for (int i = 0; i < no_of_weapons[level_number]; ++i)
        {

            weapon[level_number][i].Draw(*Renderer);
        }
        char str1[1000];
        char str2[1000];
        char str3[1000];
        sprintf(str1, "Level: %d", level_number + 1);
        sprintf(str2, "Meat Eaten: %d", meat_nomnom);
        sprintf(str3, "Distance: %dm", distance);
        Text->RenderText(str1, 5.0f, 5.0f, 1.0f);
        Text->RenderText(str2, 5.0f, 25.0f, 1.0f);
        Text->RenderText(str3, 5.0f, 45.0f, 1.0f);
    }

    if (this->State == GAME_MENU)
    {
        if (game_won && !game_end)
        {
            Text->RenderText("YOU WON <3", 310.0f, 300.0f, 1.0f);
            Text->RenderText("press R to restart!", 280.0f, 133.0f, 0.8f);

            char str1[1000];
            char str2[1000];
            char str3[1000];
            sprintf(str1, "Level: %d", level_number + 1);
            sprintf(str2, "Meat Eaten: %d", meat_nomnom);
            sprintf(str3, "Distance: %dm", distance);
            Text->RenderText(str1, 5.0f, 5.0f, 1.0f);
            Text->RenderText(str2, 5.0f, 25.0f, 1.0f);
            Text->RenderText(str3, 5.0f, 45.0f, 1.0f);
        }
        else if (!game_end)
        {
            Text->RenderText("eAT tHE mEAt", 310.0f, 300.0f, 1.0f);
            Text->RenderText("press ENTER to start!", 300.0f, 133.0f, 0.8f);
            Text->RenderText("press SPACE to accelerate upwards!", 240.0f, 155.0f, 0.8f);
            Text->RenderText("As soon as Luffy entered the Wano kingdom, he needed meat. ", 160.0f, 70.0f, 0.6f);
            Text->RenderText("Wandering around in the flower city he found lots of it but Kaido keeps throwing his Hassaikai at Luffy :( ", 30.0f, 87.0f, 0.6f);
            Text->RenderText("Let's help Luffy dodge all those and eat as much meat as he can.", 200.0f, 104.0f, 0.6f);
        }
    }
    if (game_end)
    {

        Text->RenderText("YOU LOST :(", 310.0f, 300.0f, 1.0f);
        Text->RenderText("press R to restart!", 280.0f, 133.0f, 0.8f);

        char str1[1000];
        char str2[1000];
        char str3[1000];
        sprintf(str1, "Level: %d", level_number + 1);
        sprintf(str2, "Meat Eaten: %d", meat_nomnom);
        sprintf(str3, "Distance: %dm", distance);
        Text->RenderText(str1, 5.0f, 5.0f, 1.0f);
        Text->RenderText(str2, 5.0f, 25.0f, 1.0f);
        Text->RenderText(str3, 5.0f, 45.0f, 1.0f);
    }
}