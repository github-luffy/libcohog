#include <libcohog/Trainer.hpp>

namespace libcohog
{

parameter default_liblinear_parameter()
{
    parameter par;
    par.solver_type  = L2R_L2LOSS_SVC_DUAL;
    par.eps          = 0.1;
    par.C            = 1;
    par.nr_weight    = 0;
    par.weight_label = NULL;
    par.weight       = NULL;
    return par;
}

model* train_liblinear(const std::vector<std::vector<feature_node> >& positive_features, const std::vector<std::vector<feature_node> >& negative_features, int dim, parameter param)
{
    std::vector<double> responses(positive_features.size() + negative_features.size());
    std::fill(responses.begin(), responses.begin() + positive_features.size(), 1);
    std::fill(responses.begin() + positive_features.size(), responses.end(),  -1);

    //liblinear問題構築
    problem prob;
    prob.n = dim;               //次元数
    prob.y = responses.data();  //ラベル配列
    prob.l = responses.size();  //学習データ数
    prob.bias = 0;

    std::vector<feature_node*> features_ptr;
    for(int i = 0; i < positive_features.size(); ++i) features_ptr.push_back(const_cast<feature_node*>(positive_features[i].data()));
    for(int i = 0; i < negative_features.size(); ++i) features_ptr.push_back(const_cast<feature_node*>(negative_features[i].data()));
    prob.x = features_ptr.data();

    //liblinear学習
    model *m = train(&prob, &param);

    //識別テスト
    int cnt = 0;
    for(int i = 0; i < features_ptr.size(); ++i)
        if(predict(m, features_ptr[i]) != prob.y[i])
            ++cnt;
    std::cerr << "misses:" << cnt << " (" << (100 * cnt / responses.size()) << "%)" << std::endl;

    return m;
}


model* train_liblinear(const std::vector<std::vector<float> >& positive_features, const std::vector<std::vector<float> >& negative_features, parameter param)
{
    const int dim = positive_features[0].size();

    std::vector<std::vector<float> > features;
    features.insert(features.end(), positive_features.begin(), positive_features.end());
    features.insert(features.end(), negative_features.begin(), negative_features.end());

    std::vector<double> responses(features.size());
    std::fill(responses.begin(), responses.begin() + positive_features.size(), 1);
    std::fill(responses.begin() + positive_features.size(), responses.end(),  -1);

    //liblinear問題構築
    problem prob;
    prob.n = dim;               //次元数
    prob.y = responses.data();  //ラベル配列
    prob.l = responses.size();  //学習データ数
    prob.bias = 0;

    std::vector<std::vector<feature_node> > features_liblinear(responses.size());
    std::vector<feature_node*> features_ptr;
    for(int i = 0; i < responses.size(); ++i)
    {
        for(int k = 0; k < dim; ++k)
            features_liblinear[i].push_back(feature_node{k + 1, features[i][k]});
        features_liblinear[i].push_back(feature_node{-1, 0});
        features_ptr.push_back(features_liblinear[i].data());
    }
    prob.x = features_ptr.data();

    //check looop
    {
        for(int i = 0; i < responses.size(); ++i)
        {
            for(int k = 0; k < dim; ++k)
            {
                feature_node node = features_ptr[i][k];
                if(node.index != k + 1 || node.value != features[i][k])
                    throw;
            }
        }
    }

    //liblinear学習
    model *m = train(&prob, &param);

    //識別テスト
    int cnt = 0;
    for(int i = 0; i < features.size(); ++i)
        if(predict(m, features_ptr[i]) != prob.y[i])
            ++cnt;
    std::cerr << "misses:" << cnt << " (" << (100 * cnt / responses.size()) << "%)" << std::endl;

    return m;
}

}

