/**
* Author: Louisa Liu
* Assignment: Pong Clone
* Date due: 2024-10-12, 11:59pm
* I pledge that I have completed this assignment without
* collaborating with anyone else, in conformance with the
* NYU School of Engineering Policies and Procedures on
* Academic Misconduct.
**/

#define GL_SILENCE_DEPRECATION
#define STB_IMAGE_IMPLEMENTATION
#define LOG(argument) std::cout << argument << '\n'
#define GL_GLEXT_PROTOTYPES 1

#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "stb_image.h"

enum AppStatus { RUNNING, TERMINATED };

constexpr int   WINDOW_WIDTH = 640 * 2,
                WINDOW_HEIGHT = 480 * 2;

constexpr float BG_RED = 0.9765625f,
                BG_GREEN = 0.97265625f,
                BG_BLUE = 0.9609375f,
                BG_OPACITY = 1.0f;

constexpr int   VIEWPORT_X = 0,
                VIEWPORT_Y = 0,
                VIEWPORT_WIDTH = WINDOW_WIDTH,
                VIEWPORT_HEIGHT = WINDOW_HEIGHT;

constexpr char  V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
                F_SHADER_PATH[] = "shaders/fragment_textured.glsl";

constexpr float MILLISECONDS_IN_SECOND = 1000.0;

constexpr GLint NUMBER_OF_TEXTURES = 1,                 // to be generated, that is
                LEVEL_OF_DETAIL = 0,                    // mipmap reduction image level
                TEXTURE_BORDER = 0;                     // this value MUST be zero

constexpr char  EMAIL_SPRITE_FILEPATH[] = "email.png",
                PERSON_SPRITE_FILEPATH[] = "person.png",
                LEFTWIN_SPRITE_FILEPATH[] = "left-win.png",
                RIGHTWIN_SPRITE_FILEPATH[] = "right-win.png";

// TODO: GET RID OF ROT INCREMENT (AFTER UNDERSTANDING)
// constexpr float ROT_INCREMENT = 1.0f;

SDL_Window* g_display_window;
AppStatus g_app_status = RUNNING;
ShaderProgram g_shader_program = ShaderProgram();

glm::mat4   g_view_matrix,
            g_person1_matrix,
            g_person2_matrix,
            g_email1_matrix,
            g_projection_matrix,
            g_leftwin_matrix,
            g_rightwin_matrix;

constexpr float BASE_SCALE = 1.0f,
                MAX_AMPLITUDE = 0.1f,
                PULSE_SPEED = 10.0f;

float g_previous_ticks = 0.0f;
float g_pulse_time = 0.0f;


constexpr glm::vec3 INIT_PERSON_SCALE = glm::vec3(1.5f, 1.75f, 0.0f),
                    INIT_PERSON1_POSITION = glm::vec3(3.5f, 0.0f, 0.0f),
                    INIT_PERSON2_POSITION = glm::vec3(-3.5f, 0.0f, 0.0f),
                    INIT_EMAIL_SCALE = glm::vec3(0.7f, 0.7f, 0.0f),
                    INIT_EMAIL_POSITION = glm::vec3(0.0f, 0.0f, 0.0f),
                    INIT_WIN_SCALE = glm::vec3(12.0f, 8.0f, 0.0f);

glm::vec3 g_person1_position = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_person1_movement = glm::vec3(0.0f, 0.0f, 0.0f);

glm::vec3 g_person2_position = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_person2_movement = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_person2_velocity = glm::vec3(0.0f, 0.5f, 0.0f);

glm::vec3 g_email_position = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_email_movement = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 g_email_velocity = glm::vec3(1.5f, 1.0f, 0.0f);

float g_person_speed = 20.0f;                
float g_email_speed = 1.0f;

bool no_friend = false;
int curr_balls = 1;
bool end_game = false;
int who_win = 0;
int player2_direction = 1.0;        // DONT NEED

GLuint  g_email_texture_id,
        g_person_texture_id,
        g_leftwin_texture_id,
        g_rightwin_texture_id;



GLuint load_texture(const char* filepath)
{
    // STEP 1: Loading the image file
    int width, height, number_of_components;
    unsigned char* image = stbi_load(filepath, &width, &height, &number_of_components, STBI_rgb_alpha);

    if (image == NULL)
    {
        LOG("Unable to load image. Make sure the path is correct.");
        assert(false);
    }

    // STEP 2: Generating and binding a texture ID to our image
    GLuint textureID;
    glGenTextures(NUMBER_OF_TEXTURES, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, LEVEL_OF_DETAIL, GL_RGBA, width, height, TEXTURE_BORDER, GL_RGBA, GL_UNSIGNED_BYTE, image);

    // STEP 3: Setting our texture filter parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    // STEP 4: Releasing our file from memory and returning our texture id
    stbi_image_free(image);

    return textureID;
}



void initialise()
{
    SDL_Init(SDL_INIT_VIDEO);

    g_display_window = SDL_CreateWindow("PleaseWork!",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        WINDOW_WIDTH, WINDOW_HEIGHT,
        SDL_WINDOW_OPENGL);

    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);

    if (g_display_window == nullptr)
    {
        std::cerr << "Error: SDL window could not be created.\n";
        SDL_Quit();
        exit(1);

    }

#ifdef _WINDOWS
    glewInit();
#endif

    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);

    g_shader_program.load(V_SHADER_PATH, F_SHADER_PATH);

    // MATRIXES
    g_person1_matrix = glm::mat4(1.0f);
    g_person2_matrix = glm::mat4(1.0f);
    g_email1_matrix = glm::mat4(1.0f);
    g_leftwin_matrix = glm::mat4(1.0f);
    g_rightwin_matrix = glm::mat4(1.0f);

    // 

    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);

    g_shader_program.set_projection_matrix(g_projection_matrix);
    g_shader_program.set_view_matrix(g_view_matrix);

    glUseProgram(g_shader_program.get_program_id());

    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);

    // LOAD TEXTURE
    g_email_texture_id = load_texture(EMAIL_SPRITE_FILEPATH);
    g_person_texture_id = load_texture(PERSON_SPRITE_FILEPATH);
    g_leftwin_texture_id = load_texture(LEFTWIN_SPRITE_FILEPATH);
    g_rightwin_texture_id = load_texture(RIGHTWIN_SPRITE_FILEPATH);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}



void process_input()
{
    // NOTHING PRESS, NOWHERE GO
    g_person1_movement = glm::vec3(0.0f);
    g_person2_movement = glm::vec3(0.0f);

    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
            // end game
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                g_app_status = TERMINATED;
                break;

            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {

                case SDLK_1:
                    // 1 ball
                    curr_balls = 1;
                    break;

                case SDLK_2:
                    // 2 ball
                    curr_balls = 2;
                    break;

                case SDLK_3:
                    // 3 ball
                    curr_balls = 3;
                    break;

                case SDLK_q:
                    // peace out
                    g_app_status = TERMINATED; break;

                case SDLK_t:
                    // toggle 1 / 2 player mode
                    no_friend = !no_friend;
                    
                 

                default: break;

                }

        }
    }

    const Uint8 *key_state = SDL_GetKeyboardState(NULL);

    /* PLAYER 1 CONTROLS */
    if (key_state[SDL_SCANCODE_UP]) {
        // player 1 move up
        g_person1_movement.y = 1.0f;
    }
    else if (key_state[SDL_SCANCODE_DOWN]) {
        // player 1 move down
        g_person1_movement.y = -1.0f;
    }
    else {
        g_person1_movement.y = 0;
    }

    /* PLAYER 2 CONTROLS */
    if (key_state[SDL_SCANCODE_W] && no_friend == false) {
        // double player, player 2 move up
        g_person2_movement.y = 1.0f;
    } else if (key_state[SDL_SCANCODE_S] && no_friend == false) {
        // double player, player 2 move down
        g_person2_movement.y = -1.0f;
    }
    else {
        g_person2_movement.y = 0;
    }


    // prevent move faster
    if (glm::length(g_person1_movement) > 1.0f){
        g_person1_movement = glm::normalize(g_person1_movement);
        g_person2_movement = glm::normalize(g_person2_movement);
    }

}



void update() {

    //vec3 = 3D vector

    /* Delta time calculations */
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;   // get curr number of ticks
    float delta_time = ticks - g_previous_ticks;                    // delta time = difference from the last frame
    g_previous_ticks = ticks;

    /* AUTO MOVEMENTS*/
    if (no_friend == true) {
        // 1 player
        g_person2_movement.y = g_person2_velocity.y;
    }

    // EMAIL MOVEMENT
    g_email_movement.x = g_email_velocity.x;
    g_email_movement.y = g_email_velocity.y;
    if (end_game == false)
    {
        g_email_position += g_email_movement * delta_time;
    }

    /* ACCUMULATOR LOGIC */
    // Add: direction * units per second * elapsed time
    g_person1_position += g_person1_movement * g_person_speed * delta_time;
    g_person2_position += g_person2_movement * g_person_speed * delta_time;

    /* INIT & FOLLOW TRANSLATION*/
    g_person1_matrix = glm::mat4(1.0f);
    g_person1_matrix = glm::translate(g_person1_matrix, INIT_PERSON1_POSITION);
    g_person1_matrix = glm::translate(g_person1_matrix, g_person1_position);

    g_person2_matrix = glm::mat4(1.0f);
    g_person2_matrix = glm::translate(g_person2_matrix, INIT_PERSON2_POSITION);
    g_person2_matrix = glm::translate(g_person2_matrix, g_person2_position);

    g_leftwin_matrix = glm::mat4(1.0f);
    g_rightwin_matrix = glm::mat4(1.0f);

    // AUTO MOVEMENTS 
    g_email1_matrix = glm::mat4(1.0f);
    g_email1_matrix = glm::translate(g_email1_matrix, g_email_position);

    /* INIT SCALING */          
    g_person1_matrix = glm::scale(g_person1_matrix, INIT_PERSON_SCALE);
    g_person2_matrix = glm::scale(g_person2_matrix, INIT_PERSON_SCALE);
    g_email1_matrix = glm::scale(g_email1_matrix, INIT_EMAIL_SCALE);

    g_leftwin_matrix = glm::scale(g_leftwin_matrix, INIT_WIN_SCALE);
    g_rightwin_matrix = glm::scale(g_rightwin_matrix, INIT_WIN_SCALE);


    /* COLLISION */

    // calculate box to box collision values
    float x_distance1 = fabs(g_email_position.x + INIT_EMAIL_POSITION.x - g_person1_position.x - INIT_PERSON1_POSITION.x) -
        ((INIT_PERSON_SCALE.x + INIT_EMAIL_SCALE.x) / 2.0f);

    float y_distance1 = fabs(g_email_position.y + INIT_EMAIL_POSITION.x - g_person1_position.y - INIT_PERSON1_POSITION.y) -
        ((INIT_PERSON_SCALE.y + INIT_EMAIL_SCALE.y) / 2.0f);

    float x_distance2 = fabs(g_email_position.x + INIT_EMAIL_POSITION.x - g_person2_position.x - INIT_PERSON2_POSITION.x) -
        ((INIT_PERSON_SCALE.x + INIT_EMAIL_SCALE.x) / 2.0f);

    float y_distance2 = fabs(g_email_position.y + INIT_EMAIL_POSITION.x - g_person2_position.y - INIT_PERSON2_POSITION.y) -
        ((INIT_PERSON_SCALE.y + INIT_EMAIL_SCALE.y) / 2.0f);

    // collision: email & person
    if ((x_distance1 < 0.0f && y_distance1 < 0.0f) && (g_email_position.x < INIT_PERSON1_POSITION.x))
    {
        g_email_velocity.x = -g_email_velocity.x;
        g_email_velocity.y = -g_email_velocity.y;
    }
    else if (x_distance2 < 0.0f && y_distance2 < 0.0f)
    {
        g_email_velocity.x = -g_email_velocity.x;
        g_email_velocity.y = -g_email_velocity.y;
    }


    /* BORDER */
    if (g_person1_position.y < -2.5) {
        // border at bottom
        g_person1_position.y = -2.5;  
    }
    else if (g_person1_position.y > 2.5) {
        // border at top
        g_person1_position.y = 2.5;
    }

    if (g_person2_position.y < -2.5) {
        // border at bottom
        g_person2_position.y = -2.5;

        if (no_friend) { g_person2_velocity.y = -g_person2_velocity.y; }
    }
    else if (g_person2_position.y > 2.5) {
        // border at top
        g_person2_position.y = 2.5;

        if (no_friend) { g_person2_velocity.y = -g_person2_velocity.y; }
    }

    if (g_email_position.y <= -2.5 || g_email_position.y >= 2.5) {
        // ball reach up or down
        g_email_velocity.y = -g_email_velocity.y;
    }

    /* TERMINATION FOR EMAIL REACH END */
    if (g_email_position.x < -4.5f) {
        // right win
        end_game = true;
        who_win = 1;
    }
    else if (g_email_position.x > 4.5f)
    {
        // left win
        end_game = true;
        who_win = 0;
    }
    // TODO: SHOW END GAME SCREEN

}

void draw_object(glm::mat4& object_g_model_matrix, GLuint& object_texture_id)
{
    g_shader_program.set_model_matrix(object_g_model_matrix);
    glBindTexture(GL_TEXTURE_2D, object_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, 6); // we are now drawing 2 triangles, so use 6, not 3
}

// TODO: what does render do?
void render()
{
    glClear(GL_COLOR_BUFFER_BIT);

    // Vertices
    float vertices[] =
    {
        -0.5f, -0.5f, 0.5f, -0.5f, 0.5f, 0.5f,  // triangle 1
        -0.5f, -0.5f, 0.5f, 0.5f, -0.5f, 0.5f   // triangle 2
    };

    // Textures
    float texture_coordinates[] =
    {
        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f,     // triangle 1
        0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f,     // triangle 2
    };

    glVertexAttribPointer(g_shader_program.get_position_attribute(), 2, GL_FLOAT, false,
        0, vertices);
    glEnableVertexAttribArray(g_shader_program.get_position_attribute());

    glVertexAttribPointer(g_shader_program.get_tex_coordinate_attribute(), 2, GL_FLOAT,
        false, 0, texture_coordinates);
    glEnableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    // Bind texture
    draw_object(g_person1_matrix, g_person_texture_id);
    draw_object(g_person2_matrix, g_person_texture_id);
    draw_object(g_email1_matrix, g_email_texture_id);
    //draw_object(g_leftwin_matrix, g_leftwin_texture_id);
    if (end_game && who_win == 0)
    {
        // left win
        draw_object(g_leftwin_matrix, g_leftwin_texture_id);
    }
    else if (end_game && who_win == 1)
    {
        // right win
        draw_object(g_rightwin_matrix, g_rightwin_texture_id);
    }

    // We disable two attribute arrays now
    glDisableVertexAttribArray(g_shader_program.get_position_attribute());
    glDisableVertexAttribArray(g_shader_program.get_tex_coordinate_attribute());

    SDL_GL_SwapWindow(g_display_window);
}



void shutdown() { SDL_Quit(); }

int main(int argc, char* argv[])
{
    initialise();

    while (g_app_status == RUNNING)
    {
        process_input();    // if player do smt, then process
        update();           // with new input, update game's state
        render();           // render updates onto screen
    }

    shutdown();
    return 0;
}