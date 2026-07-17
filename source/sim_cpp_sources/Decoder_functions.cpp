
#include "Decoder_functions.h"
#include "struct.h"
#include <cmath>
#include <fftw3.h>
#include <limits>
#include <memory>
#include <mutex>
#include <new>
#include <stdexcept>
#include <vector>

namespace
{
class FftCcskMetric
{
  public:
    FftCcskMetric(const std::vector<std::vector<uint16_t>> &bin_symb_seq,
                  const int q,
                  const int q1,
                  const PoAwN::structures::softdata_t sigma)
        : q_(q), q1_(q1), sigma_(sigma), scale_(2.0 / (static_cast<double>(sigma) * sigma))
    {
        input_ = fftw_alloc_complex(q1_);
        output_ = fftw_alloc_complex(q1_);
        pn_input_ = fftw_alloc_complex(q1_);
        pn_fft_ = fftw_alloc_complex(q1_);

        if (!input_ || !output_ || !pn_input_ || !pn_fft_)
        {
            throw std::bad_alloc();
        }

        {
            static std::mutex fftw_plan_mutex;
            std::lock_guard<std::mutex> lock(fftw_plan_mutex);
            forward_plan_ = fftw_plan_dft_1d(q1_, input_, output_, FFTW_FORWARD, FFTW_ESTIMATE);
            reverse_plan_ = fftw_plan_dft_1d(q1_, output_, input_, FFTW_BACKWARD, FFTW_ESTIMATE);
            pn_plan_ = fftw_plan_dft_1d(q1_, pn_input_, pn_fft_, FFTW_FORWARD, FFTW_ESTIMATE);
        }

        for (int i = 0; i < q1_; ++i)
        {
            const int reversed = (i == 0) ? 0 : q1_ - i;
            pn_input_[i][0] = static_cast<double>(bin_symb_seq[0][reversed]);
            pn_input_[i][1] = 0.0;
        }
        fftw_execute(pn_plan_);
    }

    ~FftCcskMetric()
    {
        if (forward_plan_)
            fftw_destroy_plan(forward_plan_);
        if (reverse_plan_)
            fftw_destroy_plan(reverse_plan_);
        if (pn_plan_)
            fftw_destroy_plan(pn_plan_);
        fftw_free(input_);
        fftw_free(output_);
        fftw_free(pn_input_);
        fftw_free(pn_fft_);
    }

    bool matches(const int q, const int q1, const PoAwN::structures::softdata_t sigma) const
    {
        return q_ == q && q1_ == q1 && sigma == sigma_;
    }

    void calculate(const std::vector<PoAwN::structures::softdata_t> &observation,
                   std::vector<PoAwN::structures::softdata_t> &metrics)
    {
        for (int i = 0; i < q1_; ++i)
        {
            input_[i][0] = static_cast<double>(observation[i]);
            input_[i][1] = 0.0;
        }

        fftw_execute(forward_plan_);

        for (int i = 0; i < q1_; ++i)
        {
            const double a = output_[i][0];
            const double b = output_[i][1];
            const double c = pn_fft_[i][0];
            const double d = pn_fft_[i][1];
            output_[i][0] = a * c - b * d;
            output_[i][1] = a * d + b * c;
        }

        fftw_execute(reverse_plan_);

        double min_metric = std::numeric_limits<double>::max();
        const int mask = q1_ - 1;
        for (int j = 0; j < q_; ++j)
        {
            const int fft_index = (q1_ - j) & mask;
            const double metric = (input_[fft_index][0] / q1_) * scale_;
            metrics[j] = static_cast<PoAwN::structures::softdata_t>(metric);
            if (metric < min_metric)
                min_metric = metric;
        }

        for (int j = 0; j < q_; ++j)
        {
            metrics[j] = static_cast<PoAwN::structures::softdata_t>(metrics[j] - min_metric);
        }
    }

  private:
    int q_;
    int q1_;
    PoAwN::structures::softdata_t sigma_;
    double scale_;
    fftw_complex *input_ = nullptr;
    fftw_complex *output_ = nullptr;
    fftw_complex *pn_input_ = nullptr;
    fftw_complex *pn_fft_ = nullptr;
    fftw_plan forward_plan_ = nullptr;
    fftw_plan reverse_plan_ = nullptr;
    fftw_plan pn_plan_ = nullptr;
};

void metrics_to_probabilities(std::vector<PoAwN::structures::softdata_t> &metrics,
                              const int q,
                              std::vector<PoAwN::structures::softdata_t> &probabilities)
{
    PoAwN::structures::softdata_t sum = 0;
    for (int j = 0; j < q; ++j)
    {
        probabilities[j] = std::exp(-metrics[j]);
        sum += probabilities[j];
    }
    for (int j = 0; j < q; ++j)
    {
        probabilities[j] /= sum;
    }
}
} // namespace

void PoAwN::decoding::Channel_LLR(const vector<vector<softdata_t>> &chan_observ,
                                  const vector<vector<uint16_t>> &bin_symb_seq,
                                  uint16_t q,
                                  softdata_t sigma,
                                  vector<vector<softdata_t>> &chan_LLR)
{
    const int N = chan_observ.size();
    const int q1 = chan_observ[0].size();

    if (q != q1 || q1 <= 0 || ((q1 & (q1 - 1)) != 0))
    {
        throw std::runtime_error("FFTW CCSK LLR requires q == chips_per_symbol and a power-of-two length");
    }

    thread_local std::unique_ptr<FftCcskMetric> fft_metric;
    if (!fft_metric || !fft_metric->matches(q, q1, sigma))
    {
        fft_metric = std::make_unique<FftCcskMetric>(bin_symb_seq, q, q1, sigma);
    }

    vector<softdata_t> metrics(q);
    for (int i = 0; i < N; i++)
    {
        fft_metric->calculate(chan_observ[i], metrics);
        metrics_to_probabilities(metrics, q, chan_LLR[i]);
    }
}
