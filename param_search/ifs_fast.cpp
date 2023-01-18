// g++ param_search/ifs_fast.cpp -o a.exe -std=gnu++1y -O2; ./a.exe

#include <bits/stdc++.h>
#include <time.h>

using namespace std;

typedef long long int ll;
ll MOD = 1000000007;
ll INFL = 1ll << 60;
ll INF = 1 << 28;

vector<vector<float>> params{{0.58378481, 0.5710405, 0.74313123, -0.55115729,
                              -0.06844673, 0.06882748, 0.63922813},
                             {0.69770033, -0.0544094, -0.02932284, -0.60126404,
                              0.35812325, -0.38601142, 0.36077187}};

class ifs_function {
  float prev_x, prev_y, next_x, next_y;
  vector<vector<float>> function;
  vector<float> xs, ys;
  vector<int> cash;
  uint32_t x, y, z, w, t;

 public:
  ifs_function(float input_prev_x, float input_prev_y) {
    float prev_x = input_prev_x;
    float prev_y = input_prev_y;
    x = 123456789, y = 362436069, z = 521288629, w = 88675123;  // seed. すべて0でなければよい
  }

  // 重みを渡すとchoose結果を返す
  int weighted_choose(vector<float> probs) {
    // **xor_shift** 0~1
    t = x ^ (x << 11), x = y, y = z, z = w;
    float value = float((w = (w ^ (w >> 19)) ^ (t ^ (t >> 8)))) / UINT32_MAX;

    for (int i = 0; i < probs.size(); i++)
      if (value < probs[i]) return i;
    return -1;
  }

  // a~b間の小数を生成する
  float random_float(float min_val, float max_val) {
    t = x ^ (x << 11), x = y, y = z, z = w;
    float value = float((w = (w ^ (w >> 19)) ^ (t ^ (t >> 8)))) / UINT32_MAX;
    return value * (max_val - min_val) + min_val;
  }

  // 画像に含まれる1の割合を計算する
  float cal_pix(vector<vector<int>> image) {
    float count = 0.0;
    for (auto y : image)
      for (auto x : y)
        count += x;
    // cout << "cal_pix ... count:" << count << " size:" << (image.size() * image[0].size()) << endl;
    return count / (image.size() * image[0].size());
  }

  vector<vector<int>> calculate(int iteration, int width, int height, int pad_x, int pad_y, vector<vector<float>> params) {
    function = params;
    xs.resize(iteration);
    ys.resize(iteration);

    vector<float> weights;  // 各functionの発生確率の重み
    for (auto x : function)
      weights.push_back(x[6]);

    // 重み付けchoose用の階段を作る
    vector<float> probs(weights.size());
    probs[0] = weights[0];
    for (int i = 1; i < probs.size(); i++)
      probs[i] = probs[i - 1] + weights[i];

    // cout << "start " << iteration << endl;

    // 重みのついた確率で適用する関数を選択していく
    float xmax = 0.0, xmin = 0.0, ymax = 0.0, ymin = 0.0;
    prev_x = 0.0, prev_y = 0.0, next_x = 0.0, next_y = 0.0;
    for (int i = 0; i < iteration; i++) {
      // random float using xorshift
      t = x ^ (x << 11), x = y, y = z, z = w;
      float value = float((w = (w ^ (w >> 19)) ^ (t ^ (t >> 8)))) / UINT32_MAX;
      // weighted_choose
      int j = 0;
      for (j; j < probs.size() - 1; j++)
        if (value < probs[j]) break;

      // 33135
      // if (i == 33135) {
      //   cout << prev_x << " " << prev_y << " " << j << " " << probs.size() << endl;
      //   for (int k = 0; k < 6; k++) {
      //     cout << function[j][k] << " ";
      //   }
      // }

      next_x = prev_x * function[j][0] + prev_y * function[j][1] + function[j][4];
      next_y = prev_x * function[j][2] + prev_y * function[j][3] + function[j][5];
      prev_x = xs[i] = next_x;
      prev_y = ys[i] = next_y;
      if (xmax < xs[i]) xmax = xs[i];
      if (xmin > xs[i]) xmin = xs[i];
      if (ymax < ys[i]) ymax = ys[i];
      if (ymin > ys[i]) ymin = ys[i];
    }

    // cout << "finish" << endl;

    // RESCALE
    for (int i = 0; i < xs.size(); i++) {
      xs[i] = uint16_t((xs[i] - xmin) / (xmax - xmin) * float(width - 2 * pad_x) + float(pad_x));
      ys[i] = uint16_t((ys[i] - ymin) / (ymax - ymin) * float(height - 2 * pad_y) + float(pad_y));
    }

    // DRAW IMAGE
    vector<vector<int>> image(height, vector<int>(width, 0));
    for (int i = 0; i < xs.size(); i++) image[ys[i]][xs[i]] = 1;
    return image;
  }

  void ifs_search(float threshold = 0.2, int n_category = 100) {
    // float threshold = 0.2;
    // int n_category = 100;

    for (int class_num = 0; class_num < n_category;) {
      int param_size = int(random_float(2, 9));
      float sum_proba = 0.0;

      vector<vector<float>> funcs(param_size, vector<float>(7));
      for (int i = 0; i < param_size; i++) {
        for (int j = 0; j < 7; j++) { funcs[i][j] = random_float(-1, 1); }
        funcs[i][6] = abs(funcs[i][0] * funcs[i][3] + funcs[i][1] * funcs[i][2]);
        sum_proba += funcs[i][6];
      }

      // 足したら1になるように
      for (int i = 0; i < param_size; i++)
        funcs[i][6] /= sum_proba;

      vector<vector<int>> image = calculate(100000, 512, 512, 6, 6, funcs);
      float pixels = cal_pix(image);

      // cout << pixels << "   param size" << param_size << endl;

      // もし画像の2割以上がフラクタルで埋められていたら、採用
      if (pixels > threshold) {
        cout << "---CLASS " << class_num << "---" << endl;
        for (int i = 0; i < funcs.size(); i++) {
          for (int j = 0; j < funcs[i].size(); j++) {
            cout << funcs[i][j] << " ";
          }
          cout << endl;
        }
        cout << endl;
        class_num++;
      }
    }
  }
};

// ====================================================================

int main() {
  cout << "Start." << endl;

  ofstream outputfile("output_image.txt");
  outputfile << "hogemaru" << endl;

  clock_t start = clock();
  vector<vector<int>> image(512, vector<int>(512, 0));

  ifs_function ifs_fast(0, 0);
  ifs_fast.ifs_search(0.2, 100);

  clock_t end = clock();

  // for (int i = 0; i < 512; i++) {
  //   for (int j = 0; j < 512; j++) outputfile << (image[i][j] ? 'o' : '_');
  //   outputfile << endl;
  // }

  cout << "duration = " << (double)(end - start) / CLOCKS_PER_SEC << "sec." << endl;
  outputfile.close();
}
