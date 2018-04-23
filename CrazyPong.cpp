///including the openGL and
#include <glad/glad.h>
#include <glfw3.h>

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <stdlib.h>
#include <math.h>
#include <time.h> ///cpp header for time
#include <chrono> ///cpp 11 header for times, used to get the milliseconds

///defining the GLSL fragment shader preset used for this program to make code easier to read
#define FRAGMENT_SHADER(NAME,COLOUR) const char *NAME = "#version 330 core\nout vec4 FragColor;\nvoid main()\n{\nFragColor = vec4(" COLOUR ");\n" "}\n\0";

///declaring functions
    void framebuffer_size_callback(GLFWwindow* window, int width, int height);
    void processInput(GLFWwindow *window);
   float cap(float value, float lowerCap, float upperCap);
    void move_ball(float vectorX,float vectorY);
long int msTime();
     int randomnumber(int maximum);
    void startball(float x,float y, float vecx, float vecy);

///window size
const unsigned int SCR_WIDTH = 980;
const unsigned int SCR_HEIGHT = (SCR_WIDTH/16)*9;
///vertex shader used for everything
const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec4 aPos;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\0";
///fragment shader (defining the color shading) using the define from above
FRAGMENT_SHADER(fragmentShader1Source, " 1.0f, 1.0f, 1.0f, 1.0f ")//" 0.95f, 0.95f, 0.55f, 1.0f ")  ///cream
FRAGMENT_SHADER(fragmentShader2Source, " 0.8f, 0.8f, 0.8f, 0.8f ")//" 1.0f, 1.0f, 0.3f, 1.0f ")  ///LightGrey

///declaring global variables
///every variable that should be easily modifiable all the time is declared global here
float translatePlayerOneY=0.0f; ///vertical movement player 1
float translatePlayerTwoY=0.0f; ///vertical movement player 2
float ballPositionX=0.0f; ///horizontal position of the ball
float ballPositionY=0.0f; ///vertical position of the ball
float newBallPositionX=0.0f; ///horizontal buffer position (handy for collision detection and avoiding glitches)
float newBallPositionY=0.0f; ///vertical  buffer position
float translateBallX=0.0f; ///horizontal difference from one position to the next
float translateBallY=0.0f; ///vertical difference from one position to the next
float vectorX=0.0f; ///horizontal movement value
float vectorY=0.0f; ///vertical movement value
bool pause=true; ///defines when the game is on pause
bool pressed=false; ///pause toggle

///The following function is called after each frame and runs a loop while a certain amount of time has not passed.
///Used to limit frame rates on powerful hardware, without the game would play incredibly fast
void wait_for_next_frame(long int startFrame)
{while(startFrame >= (msTime()-10) ){};}

int main() ///main program
{
    srand(time(0)); ///initializes random seed using the current time()
    startball(0,0,(randomnumber(2)-2.5f) * (9),(randomnumber(2)-0.5f) * (3.0f)); ///starts the ball in the center of the window.
    ///initializing the GLFW library
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    ///creating the window with width, height and name.
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Crazy Pong", NULL, NULL);
    if (window == NULL) ///in case an error occurs console will print this error
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1; ///returning -1 as error
    }
    ///making the window usable by creating the context and size callback for input and window scaling
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    ///loading GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) ///in case an error occurs console will print this error
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1; ///returning -1 as error
    }

    /// now building and compiling the shade programs used by openGL
    ///vertex shader
    unsigned int vertexShader               = glCreateShader(GL_VERTEX_SHADER); ///building the vertex shader
    ///fragment shader
    unsigned int ShadeColourWhite           = glCreateShader(GL_FRAGMENT_SHADER); /// the first fragment shader using the color White
    unsigned int ShadeColourLightGrey       = glCreateShader(GL_FRAGMENT_SHADER); /// the second fragment shader using the color LightGrey
    ///assigning shader programs
    unsigned int shaderProgramWhite         = glCreateProgram();
    unsigned int shaderProgramLightGrey     = glCreateProgram();
    unsigned int shaderProgramBall          = glCreateProgram();
    ///compiling shader programs
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    glShaderSource(ShadeColourWhite, 1, &fragmentShader1Source, NULL);
    glCompileShader(ShadeColourWhite);
    glShaderSource(ShadeColourLightGrey, 1, &fragmentShader2Source, NULL);
    glCompileShader(ShadeColourLightGrey);
    ///linking the first program object using the colour white
    glAttachShader(shaderProgramWhite, vertexShader);
    glAttachShader(shaderProgramWhite, ShadeColourWhite);
    glLinkProgram(shaderProgramWhite);
    ///linking the first program object using the colour Light Grey (only used for the objects while game is paused)
    glAttachShader(shaderProgramLightGrey, vertexShader);
    glAttachShader(shaderProgramLightGrey, ShadeColourLightGrey);
    glLinkProgram(shaderProgramLightGrey);

    ///generating the according Vertex Array Objects (VAOs), Element Buffer Objects (EBOs) and Vertex Buffer Objects (VBOs)
    unsigned int VBOs[3], VAOs[3], EBO[3];  ///declaring
    glGenVertexArrays(3, VAOs);
    glGenBuffers(3, VBOs);
    glGenBuffers(3, EBO);


/// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    while (!glfwWindowShouldClose(window)){ ///MAIN GAME LOOP///
/// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    long int startFrame = msTime(); ///gets the system time when entering the frame (used for frame cap at the end of the loop)
    ///PLAYER 1---------------------------------------------------
    ///declaring position values now.
    float PosX=-0.8f; ///initial horizontal position of Player one
    float PosY=0.25f; ///initial vertical position of Player one
    ///set up of vertex data and configure vertex attributes
    float firstSquare[] = { ///building all coordinates
        PosX,       PosY+translatePlayerOneY,      0.0f,  ///top right corner
        PosX-0.05f, PosY+translatePlayerOneY,      0.0f,  ///top left corner
        PosX,       PosY+translatePlayerOneY-0.5f, 0.0f,  ///bottom right corner
        PosX-0.05f, PosY+translatePlayerOneY-0.5f, 0.0f,};///bottom left corner
    unsigned int firstSquareIndices[] = { ///building all indices
    0, 1, 3,  /// first triangle
    0, 2, 3}; /// second triangle
    ///first square setup - generating buffers and binding vertex data
    glBindVertexArray(VAOs[0]);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[0]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(firstSquare), firstSquare, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(firstSquareIndices), firstSquareIndices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    ///PLAYER 1 END-----------------------------------------------

    ///PLAYER 2---------------------------------------------------
    PosX=0.8f;  ///initial horizontal position of Player one
    PosY=0.25f; ///initial vertical position of Player one
    ///set up of vertex data and configure vertex attributes
    float secondSquare[] = { ///building all coordinates
        PosX,       PosY+translatePlayerTwoY,      0.0f,  ///top left corner
        PosX+0.05f, PosY+translatePlayerTwoY,      0.0f,  ///top right corner
        PosX,       PosY+translatePlayerTwoY-0.5f, 0.0f,  ///bottom left corner
        PosX+0.05f, PosY+translatePlayerTwoY-0.5f, 0.0f,};///bottom right corner
    unsigned int secondSquareIndices[] = { ///building all indices
    0, 2, 3,  /// first triangle
    0, 1, 3}; /// second triangle
    ///second square setup - generating buffers and binding vertex data
    glBindVertexArray(VAOs[1]);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(secondSquare), secondSquare, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(secondSquareIndices), secondSquareIndices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);
    ///PLAYER 2 END-----------------------------------------------

    ///BALL-------------------------------------------------------
    PosX=0.0f; ///initial horizontal position
    PosY=0.0f; ///initial vertical position
    float height = 0.02f; ///height of the ball
    float width  = (height/16.0f)*9.0f; ///suitable width for standard aspect ratio
    float ballSquare[] = { ///building all coordinates
        PosX-width+ballPositionX, PosY-height+ballPositionY, 0.0f,  ///bottom left corner
        PosX+width+ballPositionX, PosY-height+ballPositionY, 0.0f,  ///bottom right corner
        PosX-width+ballPositionX, PosY+height+ballPositionY, 0.0f,  ///top left corner
        PosX+width+ballPositionX, PosY+height+ballPositionY, 0.0f,};///top right corner
    unsigned int ballSquareIndices[] = {
    0, 2, 3,  ///first triangle
    0, 1, 3}; ///second triangle


    /// ball setup
    glBindVertexArray(VAOs[2]);
    glBindBuffer(GL_ARRAY_BUFFER, VBOs[2]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(ballSquare), ballSquare, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(ballSquareIndices), ballSquareIndices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(0);
    ///BALL END---------------------------------------------------

    ///RENDER
    processInput(window); ///getting the input from keyboard and process it
        ///
        switch (pause){ ///switching code in relation to the pause boolean
        case 0:{ ///case false: game is running
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f); ///set background to black
            glClear(GL_COLOR_BUFFER_BIT);
            glUseProgram(shaderProgramWhite);     ///using the color white for all rendered objects
            move_ball(vectorX, vectorY);          ///moving the ball in this frame
        break;}
        case 1:{ ///case true: game is paused
            glClearColor(0.2f, 0.2f, 0.2f, 1.0f); ///set background to a dark gray
            glClear(GL_COLOR_BUFFER_BIT);
            glUseProgram(shaderProgramLightGrey); ///using light gray for rendered objects
            ///Note: The changed colors act as indicator for the user: the classy low contrast, gray overlay picture
        }}
        ///drawing the elements
        ///Player 1: first VAO
        glBindVertexArray(VAOs[0]);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        ///Player 2: second VAO
        glBindVertexArray(VAOs[1]);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        ///Ball: third VAO
        glBindVertexArray(VAOs[2]);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        glfwSwapBuffers(window); ///swapping the frame buffers is crucial for proper window content display
        glfwPollEvents(); ///poll events, IO events (keys pressed/released, mouse moved etc.)
        wait_for_next_frame(startFrame); ///waiting for the next frame (frame rate limit)

} ///MAIN GAME LOOP END
/// ////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///De-allocate all resources :
    glDeleteVertexArrays(2, VAOs);
    glDeleteBuffers(2, VBOs);
    glDeleteBuffers(2, EBO);
    glfwTerminate(); ///clearing all resources allocated by GLFW
    return 0; ///program closes
}

long int msTime() ///returns the system time in milliseconds
{
    /// getting the system time
    SYSTEMTIME systemTime;
    GetSystemTime(&systemTime);
    /// receiving the current file time
    FILETIME fileTime;
    SystemTimeToFileTime(&systemTime,&fileTime);
    /// file time in a resolution of 100 nanosecond
    long int fileTimeNano100;
    fileTimeNano100 = (((long int)fileTime.dwHighDateTime)<<32) + fileTime.dwLowDateTime;
    ///converting into milliseconds and removing windows epochal offset
    long int posixTime = fileTimeNano100/10000 - 11644473600000;
    return posixTime; ///returns the value
};

float cap(float value, float lowerCap, float upperCap) ///position cap for the platforms of player 1 and 2
{
    if (value<lowerCap){return lowerCap;}; ///when a player position is beyond the allowed range, the position is reset to the nearest allowed position
    if (value>upperCap){return upperCap;};
    return value; ///returns the input value when in allowed range
}

void processInput(GLFWwindow *window) ///this function determines if a key has been pressed
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true); ///pressing escape ends the program

    switch (pause) ///during pause, key input for movement is disabled.
    {
    case 0:{ ///Movements: Every movement control checks the position with cap()
    /// movement (up) for player 1.
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        {translatePlayerOneY+=0.025; translatePlayerOneY=cap(translatePlayerOneY,-0.75f, 0.75f);}
    /// movement (down) for player 1
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        {translatePlayerOneY-=0.025; translatePlayerOneY=cap(translatePlayerOneY,-0.75f, 0.75f);}
    /// movement (up) for player 2
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
        {translatePlayerTwoY+=0.025; translatePlayerTwoY=cap(translatePlayerTwoY,-0.75f, 0.75f);}
    /// movement (down) for player 2
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
        {translatePlayerTwoY-=0.025; translatePlayerTwoY=cap(translatePlayerTwoY,-0.75f, 0.75f);}
    }} ///pause switch end

    ///pressing space toggles the pause
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        {
            if ((pause==true)&&(pressed==false)) ///entering the pause
                {pause=false;
                pressed=true;}
            else if ((pause==false)&&(pressed==false)) ///exiting the pause
                {pause=true;
                pressed=true;}
        }
    if (glfwGetKey(window, GLFW_KEY_SPACE) != GLFW_PRESS)
        pressed = false; ///changing the pause toggle

}

int randomnumber(int maximum)  ///gives a random number between 0 and n
{
    int value;      ///declaring integer to return
    double r;       ///declaring integer to return
    if (maximum<=0)
        maximum+=2*maximum; ///forms the value of the input if it is negative, still returns a positive integer
    value = rand()%maximum; ///creates the random value and transforms into an integer to return (rounds down)
    return value;
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height) ///calls back the window size
{
    ///changing the openGL content size after the window has been scaled
    glViewport(0, 0, width, height);
}

void startball(float x, float y, float vecx, float vecy) ///sets the ball position and vector
{
    ///Note: after realizing that always changing the balls vector on reflection is tiresome and the ball needs to be restarted after each goal anyway,
    ///      combining both problems and solving them with the same function appeared neater.
    newBallPositionX=x;
    newBallPositionY=y;
    vectorX = vecx;
    vectorY = vecy;
}

void move_ball(float vectorX,float vectorY) ///each frame, the ball is being moved with this function
{
    translateBallX = vectorX/1000.0f; ///since screen positions range from 0 to 1 and the movement has to be really slow, scaling down is required
    translateBallY = vectorY/1000.0f;
    newBallPositionX = ballPositionX+translateBallX; ///adds position change to the new position
    newBallPositionY = ballPositionY+translateBallY;
    ///the new position is needed for checking the balls movement on validity
    if (newBallPositionY >= 1.0f || newBallPositionY <= -1.0f) ///Collision detection with upper and lower edges and reflection of the ball
    {
        vectorY  =  vectorY*(-1.1f); ///inverting the y-vector (reflection)
        startball(ballPositionX-translateBallX,ballPositionY-translateBallY,vectorX,vectorY);
    }
    if ((newBallPositionX >=  1.0f) || ((newBallPositionX <= -1.0f))) ///collision detection and reaction of goals (contact with right or left edge)
    {
        newBallPositionX=0.0f;
        newBallPositionY=0.0f;
        startball(0,0,(randomnumber(2)-2.5f) * (9),(randomnumber(2)-0.5f) * (3.0f));
    }

    if (((newBallPositionX <= -0.80f)  &&
         (newBallPositionX >= -0.85f)) &&
         (newBallPositionY >= translatePlayerOneY-0.25f) &&
         (newBallPositionY <= translatePlayerOneY+0.25f)) ///conditions for ball collision with Player 1
        {
        vectorX  =  (vectorX)*(-1.01f); ///inverting the x-vector (reflection on player panel) and slightly increasing the movement speed
        vectorY  += ((randomnumber(8000)-randomnumber(16000))/1000.0f); ///randomizes the y-vector of the ball
        startball( -0.79f, ballPositionY, vectorX,vectorY);
        }

    if (((newBallPositionX >=  0.80f)  &&
         (newBallPositionX <=  0.85f)) &&
        (newBallPositionY  >= translatePlayerTwoY-0.25f) &&
        (newBallPositionY  <= translatePlayerTwoY+0.25f)) ///conditions for ball collision with Player 2
        {
        vectorX  =  (vectorX)*(-1.01f); ///inverting the x-vector (reflection on player panel) and slightly increasing the movement speed
        vectorY  +=  ((randomnumber(8000)-randomnumber(16000))/1000.0f); ///randomizes the y-vector of the ball
        startball( 0.79f, ballPositionY, vectorX,vectorY);
        }
    ///after problem detection, sets coordinates to new coordinates
    ballPositionX=newBallPositionX;
    ballPositionY=newBallPositionY;
}

