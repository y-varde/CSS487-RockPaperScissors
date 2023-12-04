// main.cpp
#include <opencv2/imgproc.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/core/matx.hpp>
#include <opencv2/core.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>

using namespace cv;
using namespace std;

//play game
//takes in the text from the image and plays rock paper scissors
void playGame(String text)
{
    String choices[3] = { "rock", "paper", "scissors" };
    String aiChoice = choices[rand() % 3];
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
}

//image to string
//takes in an image and returns the text in the image
String imageToString(Mat image)
{
    String text = "Invalid";

    vector<vector<Point>> contours;
    findContours(image, contours, RETR_LIST, CHAIN_APPROX_NONE);
    
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
            if (defects[j][3] > 10000) { // if gap is 1000, there is a finger in between
                fingers++;
            }
        }
    }

    
    if (fingers < 2) {
        text = "rock";
    }
    else if (fingers < 5) {
        text = "scissors";
    }
    else {
        text = "paper";
    }

    cout << "Your vote " << text << endl;

    return text;
}

//process image
Mat processImage(Mat image)
{
    Mat gray;
    cvtColor(image, gray, COLOR_BGR2GRAY);
    Mat thresholded;
    threshold(gray, thresholded, 30, 255, THRESH_BINARY);
    imshow("img", thresholded);
    waitKey(0);
    return thresholded;
}

//capture photo
Mat capturePhoto()
{
    VideoCapture cap(0);
    Mat frame;
    while (true)
    {
        cap >> frame;
        imshow("img", frame);
        if (waitKey(1) == 'q')
        {
            break;
        }
    }
    cap.release();
    destroyAllWindows();
    return frame;
}

//main for rock paper scissors
int main(int argc, char* argv[])
{
    bool playAgain = true;
    while (playAgain)
    {
        Mat image = capturePhoto();
        image = processImage(image);
        String text = imageToString(image);
        playGame(text);
        cout << "Would you like to play again? (y/n)" << endl;
        String answer;
        cin >> answer;
        if (answer == "n")
        {
            playAgain = false;
        }
    }

}



    
