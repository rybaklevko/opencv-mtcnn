#include <boost/filesystem.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/timer/timer.hpp>

#include <opencv2/dnn.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>

#include <mtcnn/detector.h>

namespace fs = boost::filesystem;

using rectPoints = std::pair<cv::Rect, std::vector<cv::Point>>;

static cv::Mat drawRectsAndPoints(const cv::Mat &img,
                                  const std::vector<rectPoints> data) {
  cv::Mat outImg;
  img.convertTo(outImg, CV_8UC3);

  for (auto &d : data) {
    cv::rectangle(outImg, d.first, cv::Scalar(0, 0, 255));
    auto pts = d.second;
    for (size_t i = 0; i < pts.size(); ++i) {
      cv::circle(outImg, pts[i], 3, cv::Scalar(0, 0, 255));
    }
  }
  return outImg;
}

int main(int argc, char **argv) {

  if (argc < 3) {
    std::cerr << "Usage " << argv[0] << ": "
              << "<model-dir> "
              << " "
              << "<test-image>\n";
    return -1;
  }

  fs::path modelDir = fs::path(argv[1]);

  ProposalNetwork::Config pConfig;
  pConfig.caffeModel = (modelDir / "det1.caffemodel").string();
  pConfig.protoText = (modelDir / "det1.prototxt").string();
  pConfig.threshold = 0.6f;

  RefineNetwork::Config rConfig;
  rConfig.caffeModel = (modelDir / "det2.caffemodel").string();
  rConfig.protoText = (modelDir / "det2.prototxt").string();
  rConfig.threshold = 0.7f;

  OutputNetwork::Config oConfig;
  oConfig.caffeModel = (modelDir / "det3.caffemodel").string();
  oConfig.protoText = (modelDir / "det3.prototxt").string();
  oConfig.threshold = 0.7f;

  MTCNNDetector detector(pConfig, rConfig, oConfig);
  cv::Mat img = cv::imread(argv[2]);

  std::vector<Face> faces;

  static constexpr int cycleNumber = 10;
  static constexpr double secondsCaster = 1000000000.0L;
  long double summary = 0;

  for (int i(0); i < cycleNumber; ++i)
  {

    boost::timer::cpu_timer t;//(3, "%w seconds\n");
    faces = detector.detect(img, 10.f, 0.709f);

    boost::timer::cpu_times times = t.elapsed();
    std::cout << "Time used for face recognition : " << static_cast<double>(times.wall) /secondsCaster << std::endl;
    summary +=  static_cast<double>(times.wall) /secondsCaster;
  }

  std::cout << "Average face recognition time: " << double(summary/cycleNumber) <<  std::endl;
  std::cout << "Number of faces found in the supplied image - " << faces.size()<< std::endl;


  std::vector<rectPoints> data;

  // show the image with faces in it
  for (size_t i = 0; i < faces.size(); ++i) {
    std::vector<cv::Point> pts;
    for (int p = 0; p < NUM_PTS; ++p) {
      pts.push_back(
          cv::Point(faces[i].ptsCoords[2 * p], faces[i].ptsCoords[2 * p + 1]));
    }

    auto rect = faces[i].bbox.getRect();
    auto d = std::make_pair(rect, pts);
    data.push_back(d);
  }

  auto resultImg = drawRectsAndPoints(img, data);

  std::string fpath = "output.jpg";
  cv::imwrite( fpath, resultImg );

  // cv::imshow("test-oc", resultImg);
  // cv::waitKey(0);

  return 0;
}
