/**
* Author: Jonathan Kim
* Assignment: Pong Clone
* Date Due: 2023-10-21, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

/*
 Pong Game Variation:
 This game allows single player and multiplayer modes with multiplayer mode being default.
 If you press on the key "t", it will switch to single player mode. Item boxes will spawn
 at pseudo-random locations (so not completely random), and once the ball hits the item,
 your paddle will increase while decreasing your opponent's paddle. You can only reach a
 certain size. The ball will speed up and so will your paddle movement. Once the ball hits
 your opponent side, you win and the game is over.
 */
#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#define GL_GLEXT_PROTOTYPES 1

#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"

#include <ctime>
#include "cmath"

#define LOG(argument) std::cout << argument << '\n'

using namespace std;

const int WINDOW_WIDTH = 640,
          WINDOW_HEIGHT = 480;

const float BG_RED = 0.1922f,
            BG_BLUE = 0.549f,
            BG_GREEN = 0.9059f,
            BG_OPACITY = 1.0f;

const int VIEWPORT_X = 0,
          VIEWPORT_Y = 0,
          VIEWPORT_WIDTH = WINDOW_WIDTH,
          VIEWPORT_HEIGHT = WINDOW_HEIGHT;

const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
           F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

const float MILLISECONDS_IN_SECONDS = 1000.0f;

const int NUMBER_OF_TEXTURES = 1;
const GLint LEVEL_OF_DETAIL = 0;
const GLint TEXTURE_BORDER = 0;

const int FONTBANK_SIZE = 16;

const char PLAYER_SPRITE_FILEPATH[] = "/Users/jonathankim/Desktop/2023 Fall/CS 3113/Project 2/CS_3113_Assignment_2/CS 3113 Project 2/CS 3113 Project 2/player1.png";
const char OTHER_SPRITE_FILEPATH[] = "/Users/jonathankim/Desktop/2023 Fall/CS 3113/Project 2/CS_3113_Assignment_2/CS 3113 Project 2/CS 3113 Project 2/player2.png";
const char BALL_SPRITE_FILEPATH[] = "/Users/jonathankim/Desktop/2023 Fall/CS 3113/Project 2/CS_3113_Assignment_2/CS 3113 Project 2/CS 3113 Project 2/ball.png";
const char ITEM_SPRITE_FILEPATH[] = "/Users/jonathankim/Desktop/2023 Fall/CS 3113/Project 2/CS_3113_Assignment_2/CS 3113 Project 2/CS 3113 Project 2/item.png";
const char TEXT_SPRITE_FILEPATH[] = "/Users/jonathankim/Desktop/2023 Fall/CS 3113/Project 2/CS_3113_Assignment_2/CS 3113 Project 2/CS 3113 Project 2/font1.png";

SDL_Window* g_display_window;

bool g_game_is_running = true;

const glm::vec3 PLAYER_INIT_POS = glm::vec3(-4.5f, 0.0f, 0.0f);
const glm::vec3 OTHER_INIT_POS = glm::vec3(4.5f, 0.0f, 0.0f);
glm::vec3 PLAYER_SCALE = glm::vec3(0.2f, 1.0f, 1.0f);
glm::vec3 OTHER_SCALE = glm::vec3(0.2f, 1.0f, 1.0f);


const glm::vec3 BALL_INIT_POS = glm::vec3(0.0f, 0.0f, 0.0f);
const glm::vec3 BALL_INIT_SCALE = glm::vec3(0.2f, 0.2f, 0.2f);

const glm::vec3 ITEM_INIT_SCALE = glm::vec3(0.7f, 0.7f, 0.7f);
const vector<int> ITEM_LIST{0, 1, 2, 3};

float PLAYER_HEIGHT = 1.0f;
const float PLAYER_WIDTH = 0.2f;

const float BALL_DIMENSION = 0.2f;
const float ITEM_DIMENSION = 0.7f;

ShaderProgram g_shader_program;
glm::mat4 g_view_matrix,
          g_player_matrix,
          g_other_matrix,
          g_ball_matrix,
          g_item_matrix,
          g_text_matrix,
          g_projection_matrix,
          g_player_trans_matrix,
          g_other_trans_matrix,
          g_ball_trans_matrix,
          g_item_trans_matrix;

float g_previous_ticks = 0.0f;

glm::vec3 g_player_position = glm::vec3(-4.5f, 0.0f, 0.0f);
glm::vec3 g_player_movement = glm::vec3(0.0f, 0.0f, 0.0f);

glm::vec3 g_other_position = glm::vec3(4.5f, 0.0f, 0.0f);
glm::vec3 g_other_movement = glm::vec3(0.0f, 0.0f, 0.0f);

glm::vec3 g_ball_position = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_ball_movement = glm::vec3(1.0f, 0.0f, 0.0f);

glm::vec3 g_item_position = glm::vec3(10.0f, 10.0f, 0.0f);

float g_player_speed = 2.0f;
float g_ball_speed = 1.5f;

GLuint player_texture_id;
GLuint other_texture_id;
GLuint ball_texture_id;
GLuint item_texture_id;
GLuint text_texture_id;

bool collision = false;
bool single_player = false;
bool game_over = false;
bool gen_item = true;
bool scale_bool = false;

GLuint load_texture(const char* filepath) {
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);
    
    if (image == NULL) {
        LOG("Unable to load image, Make sure the path is correct");
        assert(false);
    }
    
    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    stbi_image_free(image);
    
    return textureID;
}

void initialize() {
    SDL_Init(SDL_INIT_VIDEO);
    g_display_window = SDL_CreateWindow("Pong Clone!",
                                        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                        WINDOW_WIDTH, WINDOW_HEIGHT,
                                        SDL_WINDOW_OPENGL);
    
    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    
    g_shader_program.Load(V_SHADER_PATH, F_SHADER_PATH);
    
    g_view_matrix = glm::mat4(1.0f);
    
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    
    g_player_matrix = glm::mat4(1.0f);
    g_player_matrix = glm::translate(g_player_matrix, PLAYER_INIT_POS);
    g_player_matrix = glm::scale(g_player_matrix, PLAYER_SCALE);

    g_other_matrix = glm::mat4(1.0f);
    g_other_matrix = glm::translate(g_other_matrix, OTHER_INIT_POS);
    g_other_matrix = glm::scale(g_other_matrix, OTHER_SCALE);

    g_ball_matrix = glm::mat4(1.0f);
    g_ball_matrix = glm::translate(g_ball_matrix, BALL_INIT_POS);
    g_ball_matrix = glm::scale(g_ball_matrix, BALL_INIT_SCALE);
    
    g_item_matrix = glm::mat4(1.0f);
    g_item_matrix = glm::translate(g_item_matrix, glm::vec3(10.0f, 10.0f, 0.0f));
    g_item_matrix = glm::scale(g_item_matrix, ITEM_INIT_SCALE);
    
    g_text_matrix = glm::mat4(1.0f);
    g_text_matrix = glm::translate(g_text_matrix, glm::vec3(-2.0f, 0.0f, 0.0f));
    
    g_shader_program.SetProjectionMatrix(g_projection_matrix);
    g_shader_program.SetViewMatrix(g_view_matrix);
    
    glUseProgram(g_shader_program.programID);
    
    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
    
    player_texture_id = load_texture(PLAYER_SPRITE_FILEPATH);
    other_texture_id = load_texture(OTHER_SPRITE_FILEPATH);
    ball_texture_id = load_texture(BALL_SPRITE_FILEPATH);
    item_texture_id = load_texture(ITEM_SPRITE_FILEPATH);
    text_texture_id = load_texture(TEXT_SPRITE_FILEPATH);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void process_input() {
    g_player_movement = glm::vec3(0.0f);
    g_other_movement = glm::vec3(0.0f);
    
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                g_game_is_running = false;
                break;
                
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_t:
                        single_player = true;
                        break;
                        
                    case SDLK_q:
                        g_game_is_running = false;
                        break;
                        
                    default:
                        break;
                }
                
            default:
                break;
        }
    }
    
    const Uint8 *key_state = SDL_GetKeyboardState(NULL);
    
    
    if (key_state[SDL_SCANCODE_W]) {
        if (g_player_position.y <= 3.75 - PLAYER_SCALE.y / 2) {
            g_player_movement.y = 1.0f;
        }
    }
    else if (key_state[SDL_SCANCODE_S]) {
        if (g_player_position.y >= -3.75 + PLAYER_SCALE.y / 2) {
            g_player_movement.y = -1.0f;
        }
        
    }
    
    if (key_state[SDL_SCANCODE_UP]) {
        if (g_other_position.y <= 3.75f - OTHER_SCALE.y / 2) {
            g_other_movement.y = 1.0f;
        }
    }
    else if (key_state[SDL_SCANCODE_DOWN]) {
        if (g_other_position.y >= -3.75f + OTHER_SCALE.y / 2) {
            g_other_movement.y = -1.0f;
        }
    }
    
    if (glm::length(g_player_movement) > 1.0f) {
        g_player_movement = glm::normalize(g_player_movement);
    }
    
    if (glm::length(g_other_movement) > 1.0f) {
        g_other_movement = glm::normalize(g_other_movement);
    }
}

// Checks the collision of the ball and the paddles
bool check_collision(glm::vec3 &position_a, glm::vec3 &position_b, glm::vec3 &scale) {
    float x_distance = fabs(position_a.x - position_b.x) - ((BALL_DIMENSION + scale.x) / 2.0f);
    float y_distance = fabs(position_a.y - position_b.y) - ((BALL_DIMENSION + scale.y) / 2.0f);
    return x_distance <= 0 && y_distance <= 0;
}

// Checks the collision of the item and the ball.
bool item_collision(glm::vec3 &position_a, glm::vec3 &position_b) {
    float x_distance = fabs(position_a.x - position_b.x) - ((BALL_DIMENSION + ITEM_DIMENSION) / 2.0f);
    float y_distance = fabs(position_a.y - position_b.y) - ((BALL_DIMENSION + ITEM_DIMENSION) / 2.0f);
    return x_distance <= 0 && y_distance <= 0;
}

// Randomly generates an unsigned float for x and y coordinates. If the coordinate
// is not within the game scene, it will return the initial position of the item object.
glm::vec3 rand_coord() {
    float x_coord = (float) ((rand() % 500) - 250) / (float) ((rand() % 400) - 200);
    float y_coord = (float) ((rand() % 500) - 250) / (float) ((rand() % 400) - 200);
    if (x_coord > 4.0f || x_coord < -4.0f) {
        return glm::vec3(10.0f, 10.0f, 0.0f);
    }
    if (y_coord > 3.0f || y_coord < -3.0f) {
        return glm::vec3(10.0f, 10.0f, 0.0f);
    }
    return glm::vec3(x_coord, y_coord, 0.0f);
}

// Randomly chooses a number between 0 and 499. If it lands on 1, it will
// position the item on the coordinate that was randomly generated. If
// the random coordinate is not on the screen, then it will skip the turn
// and randomly generate another number.
void generate_item() {
    int random = rand() % 500;
    if (random == 1) {
        if (rand_coord() == glm::vec3(10.0f, 10.0f, 0.0f)) {
            return;
        }
        g_item_matrix = glm::mat4(1.0f);
        g_item_position = rand_coord();
        g_item_matrix = glm::translate(g_item_matrix, g_item_position);
        g_item_matrix = glm::scale(g_item_matrix, ITEM_INIT_SCALE);
        scale_bool = true;
    }
}

void update() {
    if (game_over == false) {
        float ticks = (float) SDL_GetTicks() / MILLISECONDS_IN_SECONDS;
        float delta_time = ticks - g_previous_ticks;
        g_previous_ticks = ticks;
        
        // Wall bounce -- Only want the collision to happen once; therefore, the collision bool
        if(g_ball_position.y >= 3.65f and collision == false) {
            g_ball_movement.y *= -1.0f;
            collision = true;
        }
        if (g_ball_position.y <= -3.65f and collision == false) {
            g_ball_movement.y *= -1.0f;
            collision = true;
        }
        if (g_ball_position.y <= 3.6f and g_ball_position.y >= -3.6f) {
            collision = false;
        }
        
        // Item Generate -- Only generate when item is not on the game plane
        // Once the item is generated, don't generate more
        if (gen_item == true) {
            generate_item();
            if (g_item_position.x != 10.0f) {
                gen_item = false;
            }
        }
        
        // Item Collision -- When the ball collides with the item sprite, it gives
        // effects. Depends on which paddle hit last.
        if (item_collision(g_ball_position, g_item_position) and scale_bool == true) {
            g_item_matrix = glm::mat4(1.0f);
            g_item_position = glm::vec3(10.0f, 10.0f, 0.0f);
            g_item_matrix = glm::translate(g_item_matrix, g_item_position);
            if (g_ball_movement.x < 0) {
                if (OTHER_SCALE.y <= 1.4f) {
                    OTHER_SCALE.y += 0.2f;
                    PLAYER_SCALE.y -= 0.2f;
                }
            }
            else if (g_ball_movement.x > 0) {
                if (OTHER_SCALE.y <= 1.4f) {
                    PLAYER_SCALE.y += 0.2f;
                    OTHER_SCALE.y -= 0.2f;
                }
            }
            gen_item = true;
            scale_bool = false;
        }
        
        // Paddle-Ball Collision -- Ball bounces off the paddle based on the vector
        // direction of the position of the ball and the paddle.
        // When the ball hits the paddle, the speed of the player and the ball increases.
        if (check_collision(g_ball_position, g_player_position, PLAYER_SCALE)) {
            glm::vec3 direction = glm::vec3(0.2f, g_ball_position.y - g_player_position.y, 0.0f);
            g_ball_movement = glm::normalize(direction);
            if (g_ball_speed <= 10.0f) g_ball_speed += 0.1f;
            if (g_player_speed <= 5.0f) g_player_speed += 0.05f;
        }
        if (check_collision(g_ball_position, g_other_position, PLAYER_SCALE)) {
            glm::vec3 direction = glm::vec3(-0.2f, g_ball_position.y - g_other_position.y, 0.0f);
            g_ball_movement = glm::normalize(direction);
            if (g_ball_speed <= 10.0f) g_ball_speed += 0.1f;
            if (g_player_speed <= 5.0f) g_player_speed += 0.05f;
        }
        
        // Player 1 movement
        g_player_position += g_player_movement * g_player_speed * delta_time;
        g_player_matrix = glm::mat4(1.0f);
        g_player_matrix = glm::translate(g_player_matrix, g_player_position);
        g_player_matrix = glm::scale(g_player_matrix, PLAYER_SCALE);
        
        // Player 2 movement
        if (single_player == false) {
            g_other_position += g_other_movement * g_player_speed * delta_time;
            g_other_matrix = glm::mat4(1.0f);
            g_other_matrix = glm::translate(g_other_matrix, g_other_position);
            g_other_matrix = glm::scale(g_other_matrix, OTHER_SCALE);
        }
        
        // Single Player mode -- follows the y position of the ball.
        else {
            if (g_ball_position.y > g_other_position.y and g_other_position.y <= 3.75 - OTHER_SCALE.y / 2) {
                g_other_movement.y = 1.0f;
            }
            else if(g_ball_position.y < g_other_position.y and g_other_position.y >= -3.75f + OTHER_SCALE.y / 2) {
                g_other_movement.y = -1.0f;
            }
            g_other_position += g_other_movement * g_player_speed * delta_time;
            g_other_matrix = glm::mat4(1.0f);
            g_other_matrix = glm::translate(g_other_matrix, g_other_position);
            g_other_matrix = glm::scale(g_other_matrix, OTHER_SCALE);
        }
        
        // Ball movement until game is over
        if (g_ball_position.x <= 4.9f and g_ball_position.x >= -4.9f) {
            g_ball_position += g_ball_movement * g_ball_speed * delta_time;
            g_ball_matrix = glm::mat4(1.0f);
            g_ball_matrix = glm::translate(g_ball_matrix, g_ball_position);
            g_ball_matrix = glm::scale(g_ball_matrix, BALL_INIT_SCALE);
        }
        
        // Game is over if the ball position is at the left-most or right-most side
        // of the screen.
        else {
            game_over = true;
        }
    }
}

void draw_object(glm::mat4 &object_model_matrix, GLuint &object_texture_id) {
    g_shader_program.SetModelMatrix(object_model_matrix);
    glBindTexture(GL_TEXTURE_2D, object_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void DrawText(ShaderProgram *program, GLuint font_texture_id, string text, float screen_size, float spacing, glm::vec3 position)
{
    // Scale the size of the fontbank in the UV-plane
    // We will use this for spacing and positioning
    float width = 1.0f / FONTBANK_SIZE;
    float height = 1.0f / FONTBANK_SIZE;

    // Instead of having a single pair of arrays, we'll have a series of pairs—one for each character
    // Don't forget to include <vector>!
    vector<float> vertices;
    vector<float> texture_coordinates;

    // For every character...
    for (int i = 0; i < text.size(); i++) {
        // 1. Get their index in the spritesheet, as well as their offset (i.e. their position
        //    relative to the whole sentence)
        int spritesheet_index = (int) text[i];  // ascii value of character
        float offset = (screen_size + spacing) * i;
        
        // 2. Using the spritesheet index, we can calculate our U- and V-coordinates
        float u_coordinate = (float) (spritesheet_index % FONTBANK_SIZE) / FONTBANK_SIZE;
        float v_coordinate = (float) (spritesheet_index / FONTBANK_SIZE) / FONTBANK_SIZE;

        // 3. Inset the current pair in both vectors
        vertices.insert(vertices.end(), {
            offset + (-0.5f * screen_size), 0.5f * screen_size,
            offset + (-0.5f * screen_size), -0.5f * screen_size,
            offset + (0.5f * screen_size), 0.5f * screen_size,
            offset + (0.5f * screen_size), -0.5f * screen_size,
            offset + (0.5f * screen_size), 0.5f * screen_size,
            offset + (-0.5f * screen_size), -0.5f * screen_size,
        });

        texture_coordinates.insert(texture_coordinates.end(), {
            u_coordinate, v_coordinate,
            u_coordinate, v_coordinate + height,
            u_coordinate + width, v_coordinate,
            u_coordinate + width, v_coordinate + height,
            u_coordinate + width, v_coordinate,
            u_coordinate, v_coordinate + height,
        });
    }

    // 4. And render all of them using the pairs
    glm::mat4 model_matrix = glm::mat4(1.0f);
    g_text_matrix = glm::translate(g_text_matrix, position);
    
    program->SetModelMatrix(g_text_matrix);
    glUseProgram(program->programID);
    
    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices.data());
    glEnableVertexAttribArray(program->positionAttribute);
    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texture_coordinates.data());
    glEnableVertexAttribArray(program->texCoordAttribute);
    
    glBindTexture(GL_TEXTURE_2D, font_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, (int) (text.size() * 6));
    
    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}

void render() {
    glClear(GL_COLOR_BUFFER_BIT);
    
    float vertices[] =
    {
        -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,
        -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f
    };
    
    float texture_coordinates[] =
    {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f
    };
    
    glVertexAttribPointer(g_shader_program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(g_shader_program.positionAttribute);
    
    glVertexAttribPointer(g_shader_program.texCoordAttribute, 2, GL_FLOAT, false, 0, texture_coordinates);
    glEnableVertexAttribArray(g_shader_program.texCoordAttribute);
    
    draw_object(g_player_matrix, player_texture_id);
    draw_object(g_other_matrix, other_texture_id);
    draw_object(g_item_matrix, item_texture_id);
    draw_object(g_ball_matrix, ball_texture_id);
    if (game_over == true) {
        DrawText(&g_shader_program, text_texture_id, "GAME OVER!", 0.5f, 0.01f, glm::vec3(0.0f, 0.0f, 0.0f));
    }
    
    glDisableVertexAttribArray(g_shader_program.positionAttribute);
    glDisableVertexAttribArray(g_shader_program.texCoordAttribute);
    
    SDL_GL_SwapWindow(g_display_window);
}

void shutdown() {
    SDL_Quit();
}

int main(int argc, const char * argv[]) {
    initialize();
    srand(time_t(NULL)); // For generating pseudo-random number from seeds
    
    while (g_game_is_running) {
        process_input();
        update();
        render();
    }
    shutdown();
    return 0;
}

