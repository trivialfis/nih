#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

#include <string>
#include <vector>
#include <system_error>
#include <cmath>

namespace tungsten {
class Pipe {
  int _pfd[2];
  std::error_code _s;
  std::string _buffer;

  std::error_code readChild(std::string* buffer) {
    char c;
    unsigned i = 0;
    int bytes = 0;
    do {
      bytes = read(_pfd[0], &c, 1);
      buffer->push_back(c);
      i++;
    } while(bytes > 0) ;
    return std::make_error_code(std::errc());
  }

  int execvp_wrapper(std::string const& command, std::vector<std::string> const& argvs) {
    std::vector<char const*> c_argvs;
    c_argvs.push_back(command.c_str());
    for (auto const& v : argvs) {
      c_argvs.push_back(v.c_str());
    }
    c_argvs.push_back(nullptr);
    int s = execvp(command.c_str(), const_cast<char**>(c_argvs.data()));
    return s;
  }
  int execvpe_wrapper(std::string const& command,
                      std::vector<std::string> const& argvs,
                      std::vector<std::string> const& envs) {
    std::vector<char const*> c_argvs;
    for (auto const& v : argvs) {
      c_argvs.push_back(v.c_str());
    }
    c_argvs.push_back(nullptr);
    std::vector<char const*> c_envs (envs.size());
    for (auto const& v : envs) {
      c_envs.push_back(v.c_str());
    }
    int s = execvpe(command.c_str(),
                    const_cast<char**>(c_argvs.data()),
                    const_cast<char**>(c_envs.data()));
    return s;
  }

 public:
  explicit operator bool() const {
    return bool(_s);
  }
  std::string const& getBuffer() { return _buffer; }

  Pipe() : _s{std::error_code()}{
    if (pipe(_pfd) < 0) {
      _s = std::error_code(errno, std::system_category());
    }
  }
  std::error_code call(std::string const& command, std::vector<std::string> const& argvs, bool blocking=true) {
    pid_t pid = fork();
    if (pid < 0) {
      return {errno, std::system_category()};
    }
    if (pid == 0) {
      close(_pfd[0]);
      dup2(_pfd[1], STDOUT_FILENO);
      dup2(_pfd[1], STDERR_FILENO);
      int s = execvp_wrapper(command, argvs);
      exit(s);
    } else {
      close(_pfd[1]);
      auto s = readChild(&_buffer);
      int e {0};
      if (blocking) {
        waitpid(pid, &e, 0);
      }
    }
    return _s;
  }
};

/*!
 * \brief Linear congruential generator.
 *
 * The distribution defined in std is not portable. Given the same seed, it
 * migth produce different outputs on different platforms or with different
 * compilers.  The SimpleLCG implemented here is to make sure all tests are
 * reproducible.
 */
class SimpleLCG {
 private:
  using StateType = int64_t;
  static StateType constexpr kDefaultInit = 3;
  static StateType constexpr default_alpha_ = 61;
  static StateType constexpr max_value_ = ((StateType)1 << 32) - 1;

  StateType state_;
  StateType const alpha_;
  StateType const mod_;

  StateType seed_;

 public:
  SimpleLCG() : state_{kDefaultInit},
                alpha_{default_alpha_}, mod_{max_value_}, seed_{state_}{}
  SimpleLCG(SimpleLCG const& that) = default;
  SimpleLCG(SimpleLCG&& that) = default;

  void Seed(StateType seed) {
    seed_ = seed;
  }
  /*!
   * \brief Initialize SimpleLCG.
   *
   * \param state  Initial state, can also be considered as seed. If set to
   *               zero, SimpleLCG will use internal default value.
   * \param alpha  multiplier
   * \param mod    modulo
   */
  explicit SimpleLCG(StateType state,
                     StateType alpha=default_alpha_, StateType mod=max_value_)
      : state_{state == 0 ? kDefaultInit : state},
        alpha_{alpha}, mod_{mod} , seed_{state} {}

  StateType operator()() {
    state_ = (alpha_ * state_) % mod_;
    return state_;
  }
  StateType Min() const { return seed_ * alpha_; }
  StateType Max() const { return max_value_; }
};

template <typename ResultT>
class SimpleRealUniformDistribution {
 private:
  ResultT const lower_;
  ResultT const upper_;

  /*! \brief Over-simplified version of std::generate_canonical. */
  template <size_t Bits, typename GeneratorT>
  ResultT GenerateCanonical(GeneratorT* rng) const {
    static_assert(std::is_floating_point<ResultT>::value,
                  "Result type must be floating point.");
    long double const r = (static_cast<long double>(rng->Max())
                           - static_cast<long double>(rng->Min())) + 1.0L;
    auto const log2r = static_cast<size_t>(std::log(r) / std::log(2.0L));
    size_t m = std::max<size_t>(1UL, (Bits + log2r - 1UL) / log2r);
    ResultT sum_value = 0, r_k = 1;

    for (size_t k = m; k != 0; --k) {
      sum_value += ResultT((*rng)() - rng->Min()) * r_k;
      r_k *= r;
    }

    ResultT res = sum_value / r_k;
    return res;
  }

 public:
  SimpleRealUniformDistribution(ResultT l, ResultT u) :
      lower_{l}, upper_{u} {}

  template <typename GeneratorT>
  ResultT operator()(GeneratorT* rng) const {
    ResultT tmp = GenerateCanonical<std::numeric_limits<ResultT>::digits,
                                    GeneratorT>(rng);
    auto ret = (tmp * (upper_ - lower_)) + lower_;
    // Correct floating point error.
    return std::max(ret, lower_);
  }
};

constexpr float kRtEps = 1e-6;
}  // namespace tungsten
