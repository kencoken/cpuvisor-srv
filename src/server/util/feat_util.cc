#include "feat_util.h"

#include "classification/svm/liblinear.h"
#ifdef MATEXP_DEBUG
  #include "server/util/debug/matfileutils_cpp.h"
#endif

namespace cpuvisor {

  cv::Mat computeFeat(const std::string& full_path,
                      featpipe::CaffeEncoder& encoder) {

    cv::Mat im = cv::imread(full_path, CV_LOAD_IMAGE_COLOR);
    im.convertTo(im, CV_32FC3);

    std::vector<cv::Mat> ims;
    ims.push_back(im);

    #ifndef MATEXP_DEBUG

    return encoder.compute(ims);

    #else // DEBUG
    std::vector<cv::Mat> debug_input_images;
    cv::Mat feat = encoder.compute(ims, &debug_input_images);
    std::cout << full_path << std::endl;
    if (full_path.find("JPEGImages/000001.jpg") != string::npos) {
      MatFile mat_file("sample_feat.mat", true);
      mat_file.writeFloatMat("feat", (float*)feat.data, feat.rows, feat.cols);

      std::vector<cv::Mat> bgrChannels(3);
      split(debug_input_images[0], bgrChannels);
      mat_file.writeFloatMat("im_b", (float*)bgrChannels[0].data, bgrChannels[0].rows, bgrChannels[0].cols);
      mat_file.writeFloatMat("im_g", (float*)bgrChannels[1].data, bgrChannels[1].rows, bgrChannels[1].cols);
      mat_file.writeFloatMat("im_r", (float*)bgrChannels[2].data, bgrChannels[2].rows, bgrChannels[2].cols);
    }

    return feat;
    #endif

  }

  cv::Mat trainLinearSvm(const cv::Mat pos_feats, const cv::Mat neg_feats,
                         std::vector<std::string> _debug_pos_paths,
                         std::vector<std::string> _debug_neg_paths) {

    CHECK_EQ(pos_feats.type(), CV_32F);
    CHECK_EQ(neg_feats.type(), CV_32F);
    cv::Mat feats;
    cv::vconcat(pos_feats, neg_feats, feats);
    CHECK_EQ(feats.type(), CV_32F);

    DLOG(INFO) << "pos_feats size: " << pos_feats.rows << "x" << pos_feats.cols;
    DLOG(INFO) << "neg_feats size: " << neg_feats.rows << "x" << neg_feats.cols;
    DLOG(INFO) << "comb feats size: " << feats.rows << "x" << feats.cols;
    DLOG(INFO) << "pos count: " << pos_feats.rows;

    // train using Liblinear

    std::vector<std::vector<size_t> > labels(1);
    labels[0].resize(pos_feats.rows);
    for (size_t i = 0; i < labels[0].size(); ++i) {
      labels[0][i] = i;
    }

    featpipe::Liblinear svm;
    svm.set_c(1.0);
    //svm.set_eps(0.001);
    svm.train((float*)feats.data, feats.cols, feats.rows, labels);
    float* w_ptr = svm.get_w();

    #ifdef MATEXP_DEBUG // DEBUG
    MatFile mat_file("pretrain.mat", true);
    mat_file.writeFloatMat("feats", (float*)feats.data, feats.rows, feats.cols);
    mat_file.writeFloatMat("pos_feats", (float*)pos_feats.data, pos_feats.rows, pos_feats.cols);
    mat_file.writeFloatMat("neg_feats", (float*)neg_feats.data, neg_feats.rows, neg_feats.cols);
    if (!_debug_pos_paths.empty()) {
      mat_file.writeVectOfStrs("pos_paths", _debug_pos_paths);
    }
    if (!_debug_neg_paths.empty()) {
      mat_file.writeVectOfStrs("neg_paths", _debug_neg_paths);
    }
    mat_file.writeFloatMat("w_vect", w_ptr, 1, feats.cols);
    #endif

    return cv::Mat(feats.cols, 1, CV_32F, w_ptr);

  }

  void rankUsingModel(const cv::Mat model, const cv::Mat dset_feats,
                      cv::Mat* scores, cv::Mat* sortIdxs) {
    DLOG(INFO) << "Applying model";
    (*scores) = dset_feats*model;

    DLOG(INFO) << "Getting sort indexes...";
    size_t dset_sz = scores->rows;
    CHECK_EQ(scores->cols, 1);
    CHECK_EQ(dset_sz, dset_feats.rows);

    cv::sortIdx((*scores), (*sortIdxs),
                CV_SORT_EVERY_COLUMN + CV_SORT_DESCENDING);

    #ifdef MATEXP_DEBUG // DEBUG
    MatFile mat_file("posttrain.mat", true);
    mat_file.writeFloatMat("dset_feats", (float*)dset_feats.data, dset_feats.rows, dset_feats.cols);
    mat_file.writeFloatMat("w_vect", (float*)model.data, model.rows, model.cols);
    mat_file.writeFloatMat("multiplied_ocv", (float*)scores->data, scores->rows, scores->cols);
    #endif

  }

}
