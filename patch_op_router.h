/**
 * @file patch_op_router.h
 * @author Simon Prykhodko
 * @brief JSON patch operation router.
 */

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

/** @brief JSON patch operations. */
enum PatchOp { PATCH_OP_ADD = 1, PATCH_OP_REMOVE = 2, PATCH_OP_REPLACE = 4 };

/**
 * @brief Set (bitmask) of JSON patch operations.
 * @details Used in callback assignments.
 */
using PatchOpSet = int;

/** @brief Any of JSON operations. */
static const PatchOpSet PATCH_OP_ANY =
  PATCH_OP_ADD | PATCH_OP_REMOVE | PATCH_OP_REPLACE;

/** @brief Callback to handle JSON patch operation. */
template <typename T, typename T2> using PatchOpCallback =
  function<void(const T&, const smatch&, PatchOp, const T2&)>;

static inline void from_json(const json& j, int& v) { v = j; }
static inline void from_json(const json& j, string& v) { v = j; }

/**
 * @brief JSON patch operation router.
 * @details It allows to assign callbacks to handle different paths of patch
 * operations and to call them accordingly. The router is used by client to
 * process incoming patch operations.
 */
template <typename T> class PatchOpRouter {
 public:
  /**
   * @brief Constructor.
   * @param path_re a regular expression to match the path (can contain groups
   * to be extracted and passed to the corresponding callback).
   * @param ops a set of patch operations to match against (e.g. PATCH_OP_ANY).
   */
  template <typename T2> void AddCallback(const string& path_re,
                                          PatchOpSet ops,
                                          PatchOpCallback<T, T2> cb) {
    auto h = [path_re, ops, cb]
      (const T& data, const smatch& match, PatchOp op, const json& value) {
        T2 typed;
        from_json(value, typed);
        cb(data, match, op, typed);
    };
    conds_.push_back(Case(regex(path_re), ops, h));
  }

  /**
   * @brief Handle patch operation.
   * @details Calls all the callbacks matching the given path and the patch
   * operation.
   * @param data a reference to the top level data to apply patch operation.
   * @param path a patch operation path on.
   * @param op a patch operation.
   * @param val a patch operation value.
   */
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
  using Condition = tuple<regex, PatchOpSet, Callback>;

  vector<Condition> conds_;
};

}

#endif // SYNCER_PATCH_OP_H_
