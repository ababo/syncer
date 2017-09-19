/** ZeroMQ client implementation. */

#ifndef SYNCER_PATCH_OP_H_
#define SYNCER_PATCH_OP_H_

#include <regex>
#include <string>
#include <tuple>
#include <vector>

#include "json.hpp"

namespace syncer {

using namespace nlohmann;
using namespace std;

enum PatchOp { PATCH_OP_ADD = 1, PATCH_OP_REMOVE = 2, PATCH_OP_REPLACE = 4 };

using PatchOpMask = int;

static const PatchOpMask PATCH_OP_ANY =
  PATCH_OP_ADD | PATCH_OP_REMOVE | PATCH_OP_REPLACE;

template <typename T, typename T2> using PatchOpCallback =
  function<void(const T&, const smatch&, PatchOp, const T2&)>;

static inline void from_json(const json& j, int& v) { v = j; }

static inline void from_json(const json& j, string& v) { v = j; }

template <typename T> class PatchOpHandler {
 public:
  template <typename T2> void AddCallback(const string& path_re,
                                          PatchOpMask ops,
                                          PatchOpCallback<T, T2> cb) {
    auto h = [path_re, ops, cb]
      (const T& data, const smatch& match, PatchOp op, const json& value) {
        T2 typed;
        from_json(value, typed);
        cb(data, match, op, typed);
    };
    conds_.push_back(Case(regex(path_re), ops, h));
  }

  void HandleOp(const T& data,
                const string& path,
                PatchOp op,
                const json& val) {
    smatch match;
    for (auto c : conds_) {
      if (get<1>(c) & op && regex_match(path, match, get<0>(c))) {
        get<2>(c)(data, match, op, val);
      }
    }
  }

 private:
  using Callback =
    function<void(const T& data, smatch, PatchOp, const json& value)>;
  using Condition = tuple<regex, PatchOpMask, Callback>;

  vector<Condition> conds_;
};

}

#endif // SYNCER_PATCH_OP_H_
