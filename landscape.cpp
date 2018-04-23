///including every required library and header
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <stdlib.h>
#include <math.h>
#include <time.h>

///debug: to compile the release file, set to 0 to decrease file size, turn off LOGs and speed up the executive
#define DEBUG 0
#if DEBUG
#define LOG(X) std::cout<< X <<std::endl ///Everything inside the braces of LOG will be show in the console.
                                         ///Every function that uses input is only compiled when DEBUG is 1.
                                         ///DEBUG 0 means the console will not show any message except output file name and run automatically.
                                         ///The build of the file usually takes 3 seconds.
#else
#define LOG(X) ///LOG does nothing and everything inside the braces is ignored
#endif

#define LANDSCAPE_SIZE 260 ///defines the maximum size of the Matrix. important for initializing and making sure all counters have enough space to loop.
#define DEFAULT_HEIGHT 0   ///example of a value that could be set to change basic properties of the generation.

float landscape[LANDSCAPE_SIZE][LANDSCAPE_SIZE]={0}; ///[x]-coordinate, [y]-coordinate, height is the double value
int maximum; ///the maximum count in any direction of coordinates (usually 256 =2^8). used in several functions, thus global.

std::string int_to_string(int number) ///converts an integer into a string
{
    ///to convert an integer into a string, every digit of the number is written into the string.

    std::string output;         ///declares the string to return
    for (int counter=0;number>0;counter++) ///loops for each digit as long as the number is bigger than 0
    {
        int digit       = (number%10);  ///returns the last digit only
        char converter  = digit+48;     ///converts the number into the according ASCII number character
        output = converter + output;    ///adds the character to the string
        number          = number/10;    ///divides the number by 10 (deleting last digit)
    }
    return output;  ///returns the string
}

bool check_if_valid(int x, int y) ///This function controls a coordinate on its validity. if this coordinate is not element of the current matrix, it returns false.
{
    ///This function controls a coordinate on its validity. if this coordinate is not element of the current matrix, it returns false.
    ///This is mainly to avoid crashes and artifacts at the edge of the landscape
    if (x<0)return false;
    if (y<0)return false;
    if (x>maximum)return false;
    if (x>maximum)return false;
    return true; ///returns true when the given coordinate is part of the landscape
}

bool check_circle(int center_x, int center_y, int posx, int posy, double b_size) ///checks, if the coordinate is inside the circle, returns true if so
    {
        /// This is essentially a circle approximation.
        /// Using Pythagoras, and being given the center coordinate of the circle and the radius as well as the coordinate to check,
        /// the square of the hypotenuse and both the tangents are calculated and then compared.
        /// Is the squared hypotenuse larger than the sum of both squared tangents, the coordinate is outside of range of the circle.
        /// Tis function returns false only if the coordinate is outside of the circle.
        /// Note: This function has to check on the given vales, occasionally having to change negatives to positives.
        /// Note: The condition checks every triangle from both axes, making sure one of them returns the correct value. (better safe than sorry)

        int delta_x = center_x-posx; ///defines the delta of x
        int delta_y = center_y-posy; ///defines the delta of y

        double compare_withy = (( delta_x*delta_x )-( b_size*b_size )); ///gets the square of radius and delta x to compare with y squared later
        double compare_withx = (( delta_y*delta_y )-( b_size*b_size )); ///gets the square of radius and delta y to compare with x squared later

        if(delta_y<0)delta_y = (delta_y - delta_y)- delta_y; ///checks if the delta is negative and gets the value of it
        if(delta_x<0)delta_x = (delta_x - delta_x)- delta_x;

        if(compare_withy<0)compare_withy = (compare_withy - compare_withy)- compare_withy; ///checks if the compare square is negative and gets the value of it
        if(compare_withx<0)compare_withx = (compare_withx - compare_withx)- compare_withx;

        if((( delta_y*delta_y ) <= compare_withy) || (( delta_x*delta_x ) <= compare_withx)) return true; ///checking via Pythagoras, if inside the circle, returns true
        return false; ///not inside the circle, returning false
    }

class brush ///more brushes like smoothing, adding or stretching could be implemented
{           ///as of now, there is only one method, which is fully sufficient
public:

    void make_height(int posx, int posy, int height, double range) ///Method to set a circular area with r=range to a value=height (aka building plateaus)
    {
        for (int xcord= posx-range; xcord<= posx+range; xcord++) ///external loop
        {
        for (int ycord= posy-range; ycord<= posy+range; ycord++) ///internal loop
        {
        if (check_if_valid(xcord,ycord)==false)break;{           ///checking in each iteration, if the current coordinate is an actual part of the matrix
        if ((check_circle(posx, posy, xcord, ycord, range))==true) ///checking if the current coordinate is inside the circle
        {
            ///those multipliers always have to add up to 1, they describe the quote of plateau to original terrain.
            double pos_multiplier = 0;    ///if 1: original landscape is being kept
            double height_multiplier = 1; ///if 1: plateau is built everywhere
            double current_range = ((posx-xcord)*(posx-xcord)) + ((posy-ycord)*(posy-ycord)); ///calculates the current length between midpoint and coordinates Pythagoras
            current_range = sqrt(current_range);                                              ///thus, we need to get the square root

            pos_multiplier = (current_range*current_range) / ((range)*(range)); ///gets the quadratic relation of current range to absolute range (smoothing the brush edges)
            height_multiplier = 1-pos_multiplier; ///setting up the plateau  multiplier


            landscape[xcord][ycord]=   (height_multiplier *(height) + (pos_multiplier *landscape[xcord][ycord]))  ; ///sets the height
        }
    }}}};
};

int randomnumber(int maximum)  ///gives a random number between 0 and n
{
    int value;      ///declaring integer to return
    double r;       ///declaring integer to return
    if (maximum<=0)
    {
        maximum+=2*maximum; ///forms the value of the input if it is negative, still returns a positive integer
    }
    value = rand()%maximum; ///creates the random value and transforms into an integer to return (rounds down)
    return value;
}

void clearlandscape() ///sets every height in the matrix to zero
{
    for(int m=0;m<LANDSCAPE_SIZE;m++){for(int n=0;n<LANDSCAPE_SIZE;n++){
    landscape[m][n]=0.0f;
    }}
}

void saveasobj() ///writing the Matrix into a .obj file to use in your favorite 3D-Program
{
    std::string name;                   ///going to be the name of the file
    std::string number={0};             ///using the current seconds system time as unique ID for the filename
    std::string prefix = "landscape";   ///defines the prefix of the filename
    const std::string suffix = ".obj";  ///defines the suffix, including the file type for the file.
                                        ///this data can only be interpret as an .obj, so this should not be changed
    number= int_to_string(time(0));     ///converts the current time into a string of digit characters and assigns it
    name = prefix + number + suffix;    ///constructs the filename, putting the strings together
    LOG("writing into file: "<< name);
    std::fstream file (name,std::ios_base::in|std::ios_base::out|std::ios_base::app);  ///creates the file and opens it for editing

    int posx=1; ///initiating the counter for the current x-position
    int posy=1; ///initiating the counter for the current y-position

    LOG("  saving vertices...");
    ///The following loop will write each vertex with x and y and height value into the file in a specific order.
    ///It is necessary to use the same order for building the faces later.
    ///Every vertex gets its own line. the term "face" here means one triangle.

    while (posx<maximum+1) ///outer loop
    {
        while (posy<maximum+1) ///inner loop
        {
            float x_coordinate = (posy-1)/100.0f; /// assigns a fraction of the x-position as coordinate (scaling down the huge landscape)
            float y_coordinate = (posx-1)/100.0f; /// assigns a fraction of the y-position as coordinate (scaling down the huge landscape)

            file << "v " << x_coordinate << " " << landscape[posy][posx]/200.0f << " " << y_coordinate << "\n" ;
            ///writing the index of this vertex into the file, compatible to .obj syntax
            ///also scaling the height down to a reasonable size, shrinking the whole landscape (makes editing with 3D-graphic software easier)
            posy++; ///counter up
        }
        posx++; ///counter up
        posy=0; ///counter reset
    }

    posx=1;                   ///reseting the x-position-counter
    posy=1;                   ///reseting the y-position-counter
    bool even=true;           ///every second triangle uses partially the same vertices. to keep this organized, we need a binary counter.
    int index_position;       ///the index position is the line number of the desired vertex
    int arraysize=maximum+1;  ///size of the counter array has to be 1 higher than the array index counter
    int counter=1;            ///initializing the counter as second index position

    LOG("  saving faces...");
    while (posx<(maximum))  ///outer loop, going through x-coordinates
    {
        while(posy<(maximum)) ///inner loop, goign through all y-coordinated for each x-coordinate
        {
            index_position=posy+((posx-1)*arraysize); ///assigns the relative index of the desired vertex

            if (even)   ///toggled for every second face
            {
                file << "f " << index_position   << "//" << counter << " "      ///
                             << index_position+1 << "//" << counter << " "      ///
                     << index_position+arraysize << "//" << counter << " \n" ;  /// This line makes one face

                file << "f " << index_position+1 << "//" << counter << " "      ///
                     << index_position+arraysize << "//" << counter << " "      ///
                   << index_position+arraysize+1 << "//" << counter << " \n" ;  /// This line makes another face
                counter++;
                even=false;                                                     ///Both triangular faces make up one square.
            }
            else        ///toggled for every other face, works the same just with a mirrored cut of the square into 2 triangular faces
            {
                file << "f " << index_position << "//" << counter << " "
                   << index_position+arraysize << "//" << counter << " "
                 << index_position+arraysize+1 << "//" << counter << " \n" ;

                file << "f " << index_position << "//" << counter << " "
                           << index_position+1 << "//" << counter << " "
                 << index_position+arraysize+1 << "//" << counter << " \n" ;
                 counter++;
                 even=true;
            }
            counter++;
            posy++;
        }
        posx++;
        posy=1;
    } ///both lops end here and the file has finished writing.
    std::cout<<"File successfully created and saved as '"<<name<<"'"<<std::endl;
}

#define FULLLOG 0
void outputlandscape() ///only for special debug, prints out every single height value of the matrix
{
    #if FULLLOG
    int n=0; int m=0;
    std::cout<<"\n"<<std::endl;
    while (m<maximum+1)
    {
        while (n<maximum+1)
        {

            int s=intlenght(landscape[n][m]);
            if (landscape[n][m]>=0)
            {
                std::cout << " ";
            }
            switch (s)
            {
                case 0:{std::cout<<"   ";break;}
                case 1:{std::cout<<"   ";break;}
                case 2:{std::cout<<"  ";break;}
                case 3:{std::cout<<" ";break;}
                case 4:{std::cout<<"";break;}
            }
            std::cout << (landscape[n][m]);
            s=0;
            n++;
        }
        m++;
        n=0;
        std::cout << "\n\n";
    }
    #endif // FULLLOG
}

int control_input(std::string input, int highest)
{
///-------------------------------------------------------------------------------------------------------------------------------------///
/// This function controls the input on validity                                                                                        ///
/// Input will always mean entering a number to chose a program point or enter values for the generator.                                ///
/// A string is required to be the input to stop the console from crashing when the user enters a wrong number or not a number at all.  ///
/// Normally, when the console expects an integer, and the user enters a character, the console crashes.                                ///
/// This way, the control translates the input into a valid choice to use.                                                              ///
/// Every Number from 1 to "highest" is valid, due to the function this can only be 1 to 9, which is sufficient for this program.       ///
///-------------------------------------------------------------------------------------------------------------------------------------///
///Downside: entering "11" will be interpret as "1", because only the first char of the string will be translated.

    int choice;             ///declaring the return variable (integer)
    choice = input[0]-48;   ///translates the first character into a number
                            ///the ASCII value of 0 is 48. by converting a char into an int, the ASCII value is taken
                            ///i.e. the input of "6" actually has a value of 54 behind the scenes, so this line corrects it.
    if (choice<=0 || choice>highest)    ///checks, if the input is inside the valid range
    {
        LOG("wrong entry, try again!"); ///error message
        return 0;                       ///returns 0 (false)
    }
    return choice;  ///returns the correct value
}

void diamond_square(int n) ///square-diamond- algorithm
{   ///---------------------------------------------------------------------------------------------------------------------------------------------///
    /// This is the Core Function. The Diamond-Square algorithm generates a landscape recursively.                                                  ///
    /// first, the corners and the middle are randomly generated. normally, those values would then be given to the function.                       ///
    /// In this case, I have decided to use a global matrix and directly change the values and read from the Matrix again.                          ///
    ///(This makes the function thinner and the writing direct)                                                                                     ///
    /// The Diamond-Square algorithm goes in deeper and deeper, calculating each corner and middle in each square.                                  ///
    /// The Squares become smaller and smaller until every single point has a value, then it will stop calling itself.                              ///
    /// Because of the size of the squares being halved each time, I restricted the size in coordinates of this landscape to be a power of 2 only.  ///
    ///(due to the limited (still more than sufficient) size, the .obj will have a maximum of (2^8)^2 vertices - 65536 and be less than 7MB)        ///
    /// This whole function uses the current power as counter and tracker and also as reference on adding ransom values of appropriate value.       ///
    ///---------------------------------------------------------------------------------------------------------------------------------------------///

    int xcounter=0, ycounter=0;     ///setting the counters to coordinate origin

    while(xcounter<maximum)         ///external loop with x-coordinates
    {
        while(ycounter<maximum)     ///internal loop with y-coordinates
        {

                landscape[(int(pow(2,n))/2)+ycounter][xcounter] =(
                (   landscape[ycounter]                 [xcounter]
                +   landscape[ycounter+(int(pow(2,n)))] [xcounter])
                /2) ///adds the Values of the according, opposing coordinates and divides them to get the average height.
                + randomnumber(n+1) - randomnumber(n+1) + randomnumber(n*n+1) - randomnumber(n*n+1)
                    ///then adds randomness to this height (could be disabled or manipulated in any way
                + DEFAULT_HEIGHT;
                    ///adds other Values to shape the landscape, will remain useless for now


                ///does the same thing with the other 4 coordinates in this loop of this iteration (I'm sorry the code looks messy, those are some long lines)
                  landscape[ycounter][(int(pow(2,n))/2)+xcounter] =
               (( landscape[ycounter][xcounter]
                + landscape[ycounter][xcounter+int(pow(2,n))]
               )/2)
                + randomnumber(n+1) - randomnumber(n+1) + randomnumber(n*n+1) - randomnumber(n*n+1)

                + DEFAULT_HEIGHT;


                  landscape[ycounter+int(pow(2,n))][(int(pow(2,n))/2)+xcounter] =
               (( landscape[ycounter+int(pow(2,n))][xcounter]
                + landscape[ycounter+int(pow(2,n))][xcounter+int(pow(2,n))]
               )/2)
                + randomnumber(n+1) - randomnumber(n+1) + randomnumber(n*n+1) - randomnumber(n*n+1)

                + DEFAULT_HEIGHT;

                 landscape[(int(pow(2,n))/2)+ycounter][xcounter+int(pow(2,n))] =
               (( landscape[ycounter]                 [xcounter+int(pow(2,n))]
                + landscape[ycounter+int(pow(2,n))]   [xcounter+int(pow(2,n))]
               )/2)
                + randomnumber(n+1) - randomnumber(n+1) + randomnumber(n*n+1) - randomnumber(n*n+1)

                + DEFAULT_HEIGHT;

                  landscape[(int(pow(2,n))/2)+ycounter][(int(pow(2,n))/2)+xcounter] =
                ((landscape[ycounter]                  [xcounter]
                + landscape[ycounter]                  [xcounter+int(pow(2,n))]
                + landscape[ycounter+int(pow(2,n))]    [xcounter]
                + landscape[ycounter+int(pow(2,n))]    [xcounter+int(pow(2,n))]
                )/4)
                + randomnumber(n+1) - randomnumber(n+1) + randomnumber(n*n+1) - randomnumber(n*n+1)

                + DEFAULT_HEIGHT;

            ycounter+=(pow(2,n)); ///adds to the counter depending on the power we are in (this loop takes longer, the deeper we go)
        }
        ycounter=0;           ///resets the counter for the next loop
        xcounter+=(pow(2,n)); ///again, adds to the counter depending on the power we are in (this loop takes longer, the deeper we go)
    }
    n--;        ///after this iteration, the power will be incremented
    if(n>=0)    ///as long as the power is a positive value, the function calls itself
    {
        LOG("building landscape... " << n << " steps to go  "); ///only to display power progress
        diamond_square(n); ///calls itself with the incremented power for the next loop
    }
    ///as soon as the Power is 0, this function will end.
}

void randlandscape() ///this function initiates the main algorithm with basic data and showcase the progress
{
    int power_of_two=0; ///initializes the Power before input (better safe than sorry)
    #if DEBUG
    std::string input;  ///declares the input

    std::cout << "\nEnter the power of 2 (1 to 8): "; ///orders the user
    std::cin >> input;                              ///processes input
    power_of_two = control_input(input, 8);         ///controls the input on validity (see function for details)
    if(!power_of_two)return;                        ///stops right here when the input is invalid
    #else
    power_of_two = 8; ///during release, it will only generate the required sized landscapes
    #endif // DEBUG
    clearlandscape(); ///clears any data inside the matrix ((better safe than sorry)

    maximum=(pow(2,power_of_two)); ///sets the maximum size of the array, important to know the coordinates for the square-diamond- algorithm
    srand(time(0));                ///initializes random seed using the current time()

    ///The following gives each corner of the matrix a random integer from 0 to the power of two as starting values
    landscape[0][0]             = randomnumber(pow(2,power_of_two)) ;
    landscape[0][maximum]       = randomnumber(pow(2,power_of_two)) ;
    landscape[maximum][0]       = randomnumber(pow(2,power_of_two)) ;
    landscape[maximum][maximum] = randomnumber(pow(2,power_of_two)) ;

    ///the following calls each function and displays messages of their order
    LOG("building vertices... ");
    diamond_square(power_of_two); ///actual building of the vertices

    brush Postprocessor; ///creating the new object
    int numberOfLoops;   ///declaring loop amount

    ///The post processing loop is calling a method after the whole matrix is filled with a rough landscape
    ///This loop does not create something new, but rather changes the values in certain areas
    ///Adding some random shapes and actually smoothing the terrain makes the landscape more interesting and less jagged

    LOG("building plateaus... ");
    numberOfLoops=randomnumber(20)+20; ///sets a random amount of loops for the current brush
    for(int a=0; a<numberOfLoops; a++) ///brush loop
    {
        int posx = randomnumber(maximum)+1; ///calls a random x coordinate for the brush center
        int posy = randomnumber(maximum)+1; ///calls a random y coordinate for the brush center
        double range = randomnumber((maximum/5)*2.5f)/2.0f; ///sets a random (but still pretty) size of the brush
        Postprocessor.make_height( posx, posy  , (landscape[ posx][posy]), range); ///builds the plateau
    }
    LOG("\tplateaus built!");

    LOG("saving data... ");
    saveasobj();                  ///writing the data into a .obj file

    outputlandscape();            ///displays every single coordinate, function is empty on default (FULLLOG=0).
                                  ///WARNING: Do not activate with powers greater than 3, it takes ages to display.

    LOG("clearing buffer... ");
    clearlandscape();             ///sets every single value inside the Matrix to 0 (better safe than sorry)
    LOG("process successfully finished!\n");

}

int main() ///main menu
{
    bool run_loop=true; ///keeps the console running until the user chose to end the program or closes the console

    #if DEBUG
    std::cout << "\n\tWECLOME TO THE RANDOM LANDSCAPE GENERATOR!" << std::endl; ///welcome message
    std::cout << "\t  type a number and confirm with enter" << std::endl;
    while (run_loop==true)                                                      ///menu loop
    {
    std::cout << "\tWhat do you want to do?\n"               << std::endl;      ///-------------------------------
    std::cout << "    1-generate random landscape"            << std::endl;     ///  main function of the program
    std::cout << "    2-end "                                 << std::endl;     ///  ends and exits the loop
    std::cout << "\nconfirm your choice with enter"          << std::endl;      ///-------------------------------

    std::string input;  ///declares the input
    std::cin >> input;  ///processes input
    int choice = control_input(input, 2);   ///controls the input on validity (see function for details)

    switch (choice) ///based on the input a different part of the program will be run:
    {
        case 1: ///main feature - the 100% random generator
        {
            randlandscape();    ///calls random generator
            break;
        }
        case 2:
        {
            run_loop=false;     ///sets the boolean to false, so the main loop will stop running
            return 0;           ///returning 0 means the program ran without problems
        }}}
        #else
        randlandscape();
        #endif // DEBUG
    return 0; ///returning 0 means the program ran without problems
}


