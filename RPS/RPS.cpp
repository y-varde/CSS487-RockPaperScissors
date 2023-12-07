// RPS.cpp
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/core/matx.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/objdetect.hpp>
#include <iostream>

using namespace cv;
using namespace std;

RNG rng(12345);
int ROCK_GUESSES = 0;
int PAPER_GUESSES = 0;
int SCISSOR_GUESSES = 0;

/**
* Preconditions: This method should only be called to set the AI's choice
* Postcondition: Returns the AI's choice as a string
*
* For the first 3 rounds of the game, the Ai will choose between rock, paper, or scissors
*   at random. After the 3 rounds, the program will use the user's previous guesses to
*   determine what to play. It will choose the opposite of what the user has chosen the least.
* Ex. if the user has mostly played rock and paper, the program will guess that they will choose
*   scissors next, and play rock.
*/
String getAiChoice() {
    // choose randomly for first 3 games
    if (ROCK_GUESSES + PAPER_GUESSES + SCISSOR_GUESSES <= 3) {
        String choices[3] = { "rock", "paper", "scissors" };
        return choices[rand() % 3];
    }


    String choice = "rock";
    int leastChosen = min(ROCK_GUESSES, min(PAPER_GUESSES, SCISSOR_GUESSES));

    // play opposite of the least chosen
    if (ROCK_GUESSES == leastChosen) {
        choice = "paper";
    }

    if (PAPER_GUESSES == leastChosen) {
        choice = "scissors";
    }

    return choice;
}

//play game
//takes in the text from the image and plays rock paper scissors
void playGame(String text)
{
    String aiChoice = getAiChoice();
    cout << "AI chose " << aiChoice << endl;

    String userChoice = "";
    if (text == "rock" || text == "paper" || text == "scissors")
    {
        userChoice = text;
    }
    else
    {
        cout << "Invalid choice. Please choose either rock, paper, or scissors." << endl;
        return;
    }
    if (userChoice == aiChoice)
    {
        cout << "It's a draw!" << endl;
    }
    else if ((userChoice == "rock" && aiChoice == "scissors") || (userChoice == "scissors" && aiChoice == "paper") || (userChoice == "paper" && aiChoice == "rock"))
    {
        cout << "You win!" << endl;
    }
    else
    {
        cout << "AI wins!" << endl;
    }

    //print out the number of times each choice was chosen
    cout << endl;
    cout << "Rock guesses: " << ROCK_GUESSES << endl;
    cout << "Paper guesses: " << PAPER_GUESSES << endl;
    cout << "Scissors guesses: " << SCISSOR_GUESSES << endl;
}

//image to string
//takes in an image and returns the text in the image (rock, paper, scissors)
String imageToString(Mat image)
{
    String text = "Invalid";

    vector<vector<Point>> contours;
    findContours(image, contours, RETR_LIST, CHAIN_APPROX_NONE);

    vector<vector<Point> >hull(contours.size());
    for (size_t i = 0; i < contours.size(); i++)
    {
        convexHull(contours[i], hull[i]);
    }

    Mat drawing = Mat::zeros(image.size(), CV_8UC3);
    for (size_t i = 0; i < contours.size(); i++)
    {
        Scalar color = Scalar(rng.uniform(0, 256), rng.uniform(0, 256), rng.uniform(0, 256));
        drawContours(drawing, contours, (int)i, color);
        drawContours(drawing, hull, (int)i, color);
    }
    imshow("Hull demo", drawing);
    waitKey(0);

    // analyze contours
    int fingers = 0;
    for (int i = 0; i < contours.size(); i++) {

        // find convexHull and defects
        vector<Vec4i> defects;
        vector<int> indeces;
        vector<int> convHull;

        convexHull(contours[i], convHull, false, false);
        convexityDefects(contours[i], convHull, defects);

        // use defects to find fingers
        for (int j = 0; j < defects.size(); j++) {
            // use distance defect
            if (defects[j][3] > 10000 && defects[j][3] < 100000) { // if gap is 1000, there is a finger in between
                fingers++;
            }
        }
    }
    printf("Fingers detected: %d\n", fingers);

    if (fingers < 2) {
        text = "rock";
        ROCK_GUESSES++;
    }
    else if (fingers < 5) {
        text = "scissors";
        SCISSOR_GUESSES++;
    }
    else {
        text = "paper";
        PAPER_GUESSES++;
    }

    cout << "You chose " << text << endl;

    return text;
}

//computeColorHistogram(Mat foreground, int size);
//compute the color histogram of the foreground image
//foreground - the foreground image
//size - the number of buckets in each dimension of the histogram
//Precondition: foreground is a 3D matrix of integers
//Postcondition: returns the color histogram of the foreground image
Mat computeColorHistogram(Mat foreground, int size)
{
    // create an array of the histogram dimensions
// size is a constant - the # of buckets in each dimension
    int dims[] = { size, size, size };
    // create 3D histogram of integers initialized to zero	
    Mat hist(3, dims, CV_32S, Scalar::all(0));

    //compute the bucket size
    int bucketSize = 256 / size;

    //loop through the foreground image and assign the pixel values to the histogram
    for (int i = 0; i < foreground.rows; i++)
    {
        for (int j = 0; j < foreground.cols; j++)
        {
            // get the pixel value for each channel
            Vec3b pixel = foreground.at<Vec3b>(i, j);
            int blue = pixel[0];
            int green = pixel[1];
            int red = pixel[2];

            // compute the bucket for each channel and increment the histogram accordingly
            int r = red / bucketSize;
            int g = green / bucketSize;
            int b = blue / bucketSize;
            hist.at<int>(b, g, r)++;
        }
    }

    //return the histogram
    return hist;
}

//computeMostCommonColor(Mat hist, int size);
//compute the most common color in the foreground image using the color histogram
//hist - the color histogram of the foreground image
//size - the number of buckets in each dimension of the histogram
//Precondition: hist is a 3D matrix of integers
//Postcondition: returns the most common color in the foreground image
Vec3b computeMostCommonColor(Mat hist, int size)
{
    //initialize the max value and index
    int max = 0;
    int maxIndex = 0;

    //initialize the RGB indexes
    int r = 0;
    int g = 0;
    int b = 0;

    //loop through the histogram and find the bucket with the highest value
    for (int i = 0; i < size; i++)
    {
        for (int j = 0; j < size; j++)
        {
            for (int k = 0; k < size; k++)
            {
                int val = hist.at<int>(i, j, k);

                //update max value and RGB indexes 
                if (val > max)
                {
                    max = hist.at<int>(i, j, k);
                    r = k;
                    g = j;
                    b = i;
                }
            }
        }
    }

    //compute the RGB values of the most common color
    int bucketSize = 256 / size;
    int cRed = r * bucketSize + bucketSize / 2;
    int cGreen = g * bucketSize + bucketSize / 2;
    int cBlue = b * bucketSize + bucketSize / 2;
    return Vec3b(cBlue, cGreen, cRed);
}

//process image
//converts image to an edge image 
Mat processImage(Mat image)
{
    //haar cascade to detect hand
    //taken from https://github.com/Balaje/OpenCV/blob/master/haarcascades/hand.xml
    CascadeClassifier handCascade;
    handCascade.load("hand.xml");

    //detect hand
    vector<Rect> hands;
    handCascade.detectMultiScale(image, hands, 1.1, 2, 0, Size(30, 30));

    //draw rectangle around hand
    for (int i = 0; i < hands.size(); i++)
    {
        rectangle(image, hands[i], Scalar(0, 255, 0), 2);
    }

    imshow("Hand", image);
    waitKey(0);

    //resize image to just the hand
    if (hands.size() > 0)
    {
        image = image(hands[0]);
        imshow("Hand", image);
    }

    // Convert the image to grayscale
    Mat gray;
    cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    imshow("Grayscale", gray);
    waitKey(0);

    // Apply Gaussian blur to the grayscale image
    Mat blur;
    GaussianBlur(gray, blur, cv::Size(5, 5), 0);
    imshow("Gaussian Blur", blur);
    waitKey(0);

    //get edge image
    Mat edges;
    Canny(blur, edges, 0, 50);
    imshow("Edges", edges);
    waitKey(0);

    // Dilate the hand
    Mat dilated;
    dilate(edges, dilated, Mat(), Point(-1, -1), 3);
    imshow("Dilated", dilated);
    waitKey(0);

    //return the dilated image
    return dilated;
}

//capture photo
Mat capturePhoto()
{
    VideoCapture cap(1);
    Mat frame;
    while (true)
    {
        cap >> frame;
        imshow("Press \"c\" to take a photo", frame);
        cout << endl;

        //take photo when c is pressed
        if (waitKey(1) == 'c')
        {
            cap.release();
            destroyAllWindows();
            return frame;
            break;
        }
    }
}

//main for rock paper scissors
int main(int argc, char* argv[])
{
    //use webcam
    bool playAgain = true;
    bool firstTime = true;
    while (playAgain)
    {
        Mat image;
        if (argc > 1 && firstTime)
        {
            //read in image
            image = imread(argv[1]);
            firstTime = false;
        }
        else {
            //capture photo
            image = capturePhoto();
        }

        //process image
        image = processImage(image);

        //get text from image (rock, paper, scissors)
        String text = imageToString(image);

        //play game
        playGame(text);

        //ask to play again
        cout << "Would you like to play again? (y/n)" << endl;
        String answer;
        cin >> answer;
        if (answer == "n")
        {
            cout << "Thanks for playing!" << endl;
            playAgain = false;
        }

        //close all windows
        destroyAllWindows();
    }
}
