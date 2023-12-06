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

    /*
    int size = 4;
    Mat hist = computeColorHistogram(image, size);
    Vec3b commonColor = computeMostCommonColor(hist, size);

    cout << "Most common color: " << commonColor << endl;
    
    //replace all pixels that are not the most common color with black
    for (int i = 0; i < image.rows; i++)
    {
        for (int j = 0; j < image.cols; j++)
        {
            // get the pixel value for each channel
            Vec3b pixel = image.at<Vec3b>(i, j);
            int blue = pixel[0];
            int green = pixel[1];
            int red = pixel[2];

            //get the color of the most common color
            int cBlue = commonColor[0];
            int cGreen = commonColor[1];
            int cRed = commonColor[2];

            //compute the distance between the pixel and the most common color
            int rDist = abs(red - cRed);
            int gDist = abs(green - cGreen);
            int bDist = abs(blue - cBlue);

            //if the distance is greater than 50, or the pixel is within 50 from white, set the pixel to black
            bool isWhite = (red > 200 && green > 200 && blue > 200);
            if (rDist > 50 || gDist > 50 || bDist > 50 || (isWhite))
            {
                image.at<Vec3b>(i, j) = Vec3b(0, 0, 0);
            }
        }
    }

    imshow("Most Common Color", image);
    waitKey(0);*/

    //resize image to just the hand
    //image = image(hands[0]);

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
    // Load the hand cascade
    CascadeClassifier hand_cascade;
    hand_cascade.load(samples::findFile("hand.xml"));

    //load the palm cascade
    CascadeClassifier palm_cascade;
    palm_cascade.load(samples::findFile("palm.xml"));

    cout << "Will you be using your front or back camera?" << endl;
    int cam = 0;
    String choice;
    cin >> choice;

    if (choice == "b") {
        cam = 1;
    }

    VideoCapture cap(cam);
    Mat frame;
    while (true)
    {
        cap >> frame;

        std::vector<Rect> hand;
        std::vector<Rect> palm;

        // Detect hand
        hand_cascade.detectMultiScale(frame, hand);
        palm_cascade.detectMultiScale(frame, palm);

        //Draw the rectangle that contains the top left of the left hand and the bottom right of the right hand
        if (hand.size() > 0)
        {
            //detect all hands
            for (int i = 0; i < hand.size(); i++)
            {
                //draw rectangle around hand
                Point pt1(hand[i].x, hand[i].y);
                Point pt2(hand[i].x + hand[i].width, hand[i].y + hand[i].height);
                rectangle(frame, pt1, pt2, Scalar(0, 255, 0));
            }

            //resize image to just the hand
            frame = frame(hand[0]);
            //break;
        }
        else if (palm.size() > 0)
        {
            //detect all palms
            for (int i = 0; i < palm.size(); i++)
            {
                //draw rectangle around palm
                Point pt1(palm[i].x, palm[i].y);
                Point pt2(palm[i].x + palm[i].width, palm[i].y + palm[i].height);
                rectangle(frame, pt1, pt2, Scalar(0, 255, 0));
            }

            //resize image to just the palm
            frame = frame(palm[0]);
            //break;
        }

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
    //if user has provided an image, use that image
    if (argc > 1)
    {
        //read in image
        Mat image = imread(argv[1]);

        //process image
        image = processImage(image);

        //get text from image (rock, paper, scissors)
        String text = imageToString(image);

        //play game
        playGame(text);

        //close all windows
        destroyAllWindows();
        return 0;
    }

    //use webcam
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

        //close all windows
        destroyAllWindows();
    }
}
