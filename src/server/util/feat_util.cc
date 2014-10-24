#include "feat_util.h"

#include "classification/svm/liblinear.h"

namespace cpuvisor {

  cv::Mat computeFeat(const std::string& full_path,
                      featpipe::CaffeEncoder& encoder) {

    cv::Mat im = cv::imread(full_path, CV_LOAD_IMAGE_COLOR);
    im.convertTo(im, CV_32FC3);

    std::vector<cv::Mat> ims;
    ims.push_back(im);
    return encoder.compute(ims);

  }

  cv::Mat trainLinearSvm(const cv::Mat pos_feats, const cv::Mat neg_feats) {

    cv::Mat feats;
    cv::vconcat(pos_feats, neg_feats, feats);

    DLOG(INFO) << "pos_feats size: " << pos_feats.rows << "x" << pos_feats.cols;
    DLOG(INFO) << "neg_feats size: " << neg_feats.rows << "x" << neg_feats.cols;
    DLOG(INFO) << "comb feats size: " << feats.rows << "x" << feats.cols;
    DLOG(INFO) << "pos count: " << pos_feats.rows;

    std::vector<std::vector<size_t> > labels(1);
    labels[0].resize(pos_feats.rows);
    for (size_t i = 0; i < labels[0].size(); ++i) {
      labels[0][i] = i;
    }

    featpipe::Liblinear svm;
    svm.set_c(0.1);
    svm.train((float*)feats.data, feats.cols, feats.rows, labels);
    float* w_ptr = svm.get_w();

    return cv::Mat(feats.cols, 1, CV_32F, w_ptr);

  }

  void rankUsingModel(const cv::Mat model, const cv::Mat dset_feats,
                      cv::Mat* scores, cv::Mat* sortIdxs) {
    (*scores) = dset_feats*model;

    size_t dset_sz = scores->rows;
    CHECK_EQ(scores->cols, 1);
    CHECK_EQ(dset_sz, dset_feats.rows);

    cv::sortIdx((*scores), (*sortIdxs),
                CV_SORT_EVERY_COLUMN + CV_SORT_DESCENDING);

  }

}
