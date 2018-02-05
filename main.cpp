#include <iostream>
#include <sstream>
#include <direct.h>
#include <fstream>
#include <vector>

#define USE_FACE
#include "NtKinect.h"

#include "DxLib.h"
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

using namespace std;

string faceProp[] = {
	"happy", "engaed", "glass", "leftEyeClosed",
	"rightEyeClosed", "mouseOpen", "mouseMoved", "lookingAway"};
string dstate[] = { "unknown", "no", "maybe", "yes" };

#include <time.h>
string now() {
	char s[1024];
	time_t t = time(NULL);
	struct tm lnow;
	localtime_s(&lnow, &t);
	sprintf_s(s, "%04d-%02d-%02d_%02d-%02d-%02d", lnow.tm_year + 1900, lnow.tm_mon + 1, lnow.tm_mday, lnow.tm_hour, lnow.tm_min, lnow.tm_sec);
	return string(s);
}

string decideMovie(){
	std::vector<string> movie_tittle = {"narratage.mp4", "sensei.mp4", "doraemon.mp4", "starwars.mp4", "marvel.mp4"};
	int video_number = rand() % movie_tittle.size();
	return "video\\" + movie_tittle[video_number];
}

int write2csv(UINT64 ID, string movie_ID, double attention_time){
	FILE *fp;
	char str;

	fp = fopen("E:\log\\result.csv","a");

	fprintf(fp, "%d,%s,%f\n", ID, movie_ID.c_str(), attention_time);
	

	fclose(fp);
	return 0;
}

int WINAPI main(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	NtKinect kinect;
	cv::VideoWriter vw;
	int scale = 1;
	bool check_frag = true;
	cv::Size sz(1920 / scale, 1080 / scale);
	bool onSave = false;
	cv::Mat img;
	bool start_flag = false;
	int miss_flame = 0;
	double sum_looking_time = 0.0;
	string filename;
	string video_name;
	string log;
	string movie_name;
	int MovieGraphHandle;
	int video_number = 0;
	std::vector<UINT64> human_id;
	std::vector<clock_t> human_start_time;
	std::vector<clock_t> human_end_time;
	std::vector<double> human_total_time;
	std::vector<bool> human_start_flag;
	std::vector<int> human_miss_flame;
	std::vector<string> human_movie_name;
	std::vector<string> human_image_pass;



	//ChangeWindowMode(TRUE);

	if (DxLib_Init() == -1)
	{
		return -1;    // finish if ellor cause
	}

	//initial movie flame
	MovieGraphHandle = LoadGraph("video\\narratage.mp4");
	PlayMovieToGraph(MovieGraphHandle);
	DrawExtendGraph(0, 0, 640, 480, MovieGraphHandle, FALSE);
	DeleteGraph(MovieGraphHandle);

	MovieGraphHandle = LoadGraph("video\\black.mp4");
	PlayMovieToGraph(MovieGraphHandle);


	while (CheckHitKey(KEY_INPUT_ESCAPE) == 0) {
		if (onSave) {
			cv::resize(kinect.rgbImage, img, sz, 0, 0);
			cv::cvtColor(img, img, CV_BGRA2BGR);
			vw << img;
		}

		kinect.setRGB();
		kinect.setSkeleton();
		for (auto person : kinect.skeleton) {
			for (auto joint : person) {
				if (joint.TrackingState == TrackingState_NotTracked) continue;
				ColorSpacePoint cp;
				kinect.coordinateMapper->MapCameraPointToColorSpace(joint.Position, &cp);
			}
		}
		kinect.setFace();
		for (int p = 0; p < kinect.skeletonTrackingId.size(); p++) {
			if (!onSave) {
				DeleteGraph(MovieGraphHandle);
				filename = "E:\log\\" + now();
				//_mkdir(filename.c_str());
				vw = cv::VideoWriter("E:\log\\video\\" + now() + ".avi", CV_FOURCC_MACRO('X', 'V', 'I', 'D'), 20.0, sz);

				if (!vw.isOpened()) throw runtime_error("cannot create video file");
				onSave = true;

				// load movie file
				movie_name = decideMovie();
				MovieGraphHandle = LoadGraph(movie_name.c_str());

				//log = filename + "\n" + video_name + "\n";

				// play movie
				PlayMovieToGraph(MovieGraphHandle);
			}

			vector< UINT64 >::iterator cIter = find(human_id.begin(), human_id.end(), kinect.skeletonTrackingId[p]);
			if (cIter == human_id.end()) {
				string image_pass = "E:\log\\image\\" + now() + ".jpg";
				cv::imwrite(image_pass, kinect.rgbImage);
				human_image_pass.push_back(image_pass);
				human_id.push_back(kinect.skeletonTrackingId[p]);
				human_start_time.push_back(clock());
				human_end_time.push_back(clock());
				human_total_time.push_back(0.0);
				human_start_flag.push_back(false);
				human_miss_flame.push_back(0);
				human_movie_name.push_back(movie_name);
			}
		}
		for (int p = 0; p < human_id.size(); p++) {
			check_frag = true;
			for (int i = 0; i < kinect.faceDirection.size(); i++) {
				if (human_id[p] == kinect.skeletonTrackingId[i]) {
					cv::Vec3f dir = kinect.faceDirection[i];
					//cv::putText(kinect.rgbImage, "pitch : " + to_string(dir[0]), cv::Point(200 * p + 50, 30), cv::FONT_HERSHEY_SIMPLEX, 1.2, cv::Scalar(0, 230, 0), 1, CV_AA);
					//cv::putText(kinect.rgbImage, "yaw : " + to_string(dir[1]), cv::Point(200 * p + 50, 60), cv::FONT_HERSHEY_SIMPLEX, 1.2, cv::Scalar(0, 230, 0), 1, CV_AA);
					//cv::putText(kinect.rgbImage, "roll : " + to_string(dir[2]), cv::Point(200 * p + 50, 90), cv::FONT_HERSHEY_SIMPLEX, 1.2, cv::Scalar(0, 230, 0), 1, CV_AA);

					if (!(dir[0] == 0.0 && dir[1] == 0.0 && dir[2] == 0.0)) {


						if (dir[1] >= -20 && dir[1] <= 20 && !human_start_flag[p]) {
							human_start_time[p] = clock();
							human_start_flag[p] = true;
							human_end_time[p] = clock();
						}

						if (dir[1] >= -20 && dir[1] <= 20 && human_start_flag[p]) {
							human_end_time[p] = clock();
							//cv::putText(kinect.rgbImage, "look : " + to_string(sum_looking_time + (time_from_start - start) / 1000.0), cv::Point(200 * p + 50, 120), cv::FONT_HERSHEY_SIMPLEX, 1.2, cv::Scalar(0, 230, 255), 1, CV_AA);
						}

						if ((dir[1] < -20 || dir[1] > 20) && human_start_flag[p]) {
							human_start_flag[p] = false;
							//log = log + to_string(kinect.skeletonTrackingId[0]) + "looking_time : " + to_string((time_from_start - start) / 1000.0) + "\n";
							//log = log + "looking_time : " + to_string((time_from_start - start) / 1000.0) + "\n";
							human_total_time[p] = human_total_time[p] + ((human_end_time[p] - human_start_time[p]) / 1000.0);
						}

						if ((dir[1] < -20 || dir[1] > 20) && !human_start_flag[p]) {
							//cv::putText(kinect.rgbImage, "look : " + to_string(sum_looking_time), cv::Point(200 * p + 50, 120), cv::FONT_HERSHEY_SIMPLEX, 1.2, cv::Scalar(0, 230, 255), 1, CV_AA);
						}
					}
					human_miss_flame[p] = 0;
					check_frag = false;
				}

			}
			if (check_frag) {
				human_miss_flame[p] = human_miss_flame[p] + 1;
			}

			if (human_miss_flame[p] >= 30) {
				if (human_start_flag[p] == true) {
					//human_start_flag[p] = false;
					//log = log + "looking_time : " + to_string((time_from_start - start) / 1000.0) + "\n";
					human_total_time[p] = human_total_time[p] + ((human_end_time[p] - human_start_time[p]) / 1000.0);
				}

				write2csv(human_id[p], human_movie_name[p], human_total_time[p]);

				human_id.erase(human_id.begin() + p);
				human_start_time.erase(human_start_time.begin() + p);
				human_end_time.erase(human_end_time.begin() + p);
				human_total_time.erase(human_total_time.begin() + p);
				human_start_flag.erase(human_start_flag.begin() + p);
				human_miss_flame.erase(human_miss_flame.begin() + p);
				human_movie_name.erase(human_movie_name.begin() + p);

			}



			if (onSave && human_id.size() == 0) {
				vw.release();
				onSave = false;

				cv::destroyAllWindows();
				// Delete the graphic handle of the included movie file
				DeleteGraph(MovieGraphHandle);

				// rode movie file
				MovieGraphHandle = LoadGraph("video\\black.mp4");

				// play movie
				PlayMovieToGraph(MovieGraphHandle);
			}


			// change movie size

			//auto key = cv::waitKey(1);
			//if (key == 'q') break;
		}

		DrawExtendGraph(0, 0, 640, 480, MovieGraphHandle, FALSE);
	}
	cv::destroyAllWindows();
	DxLib_End(); 

	return 0;
}