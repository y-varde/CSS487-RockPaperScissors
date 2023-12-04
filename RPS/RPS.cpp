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

    cout << "You chose " << text << endl;

    return text;
}

//process image
//converts image to an edge image 
Mat processImage(Mat image)
{
    // Convert the image to grayscale
    Mat gray;
    cvtColor(image, gray, cv::COLOR_BGR2GRAY);

    // Apply Gaussian blur to the grayscale image
    Mat blur;
    GaussianBlur(gray, blur, cv::Size(5, 5), 0);

    // Apply Canny edge detection
    Mat edges;
    Canny(blur, edges, 50, 150);

    // Dilate the edges
    Mat dilatedEdges;
    dilate(edges, dilatedEdges, Mat());

    // Display the edge image
    namedWindow("Edges", cv::WINDOW_NORMAL);
    imshow("Edges", dilatedEdges);
    waitKey(0);

    return dilatedEdges;
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
        if (waitKey(1) == 'c')
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
        //capture photo
        Mat image = capturePhoto();

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
    }

}



    
