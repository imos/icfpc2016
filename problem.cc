#include <complex>
#include <vector>

#include "base/base.h"
#include "boost/multiprecision/cpp_int.hpp"
#include "boost/rational.hpp"

DEFINE_bool(expand_viewbox, true,
            "Expand viewbox to covert the entire silhouette.");
DEFINE_bool(shrink_viewbox, true,
            "Shrink viewbox to fit silhouette and hide the original rect.");

using boost::rational;
using boost::rational_cast;
using namespace std;

typedef boost::multiprecision::cpp_rational Q;
typedef std::complex<Q> C;

namespace std {
// Dummy functions for std::complex
bool isnan(const Q& q) { return false; }
bool isinf(const Q& q) { return false; }
Q copysign(const Q& x, const Q& y) { return x.sign() != y.sign() ? -x : x; }
}

struct Vertex {
  Q x;
  Q y;
};

typedef vector<Vertex> Polygon;

bool is_ccw(const Polygon& p) {
  vector<C> v(p.size());
  for (int i = 0; i < p.size(); ++i) {
    v[i] = C(p[i].x - p[0].x, p[i].y - p[0].y);
  }
  Q area;
  for (int i = 1; i < v.size() - 1; ++i) {
    area += (v[i] * std::conj(v[i + 1])).imag();
  }
  LOG_IF(ERROR, area == 0) << "Unexpected zero area";
  return area < 0;
}

Q consume_rational(std::istream& is) {
  while (isspace(is.peek())) {
    is.get();
  }
  string buf;
  while (true) {
    int c = is.peek();
    if (isdigit(c) || c == '-' || c == '/') {
      buf += c;
      is.get();
    } else {
      break;
    }
  }
  return Q(buf);
}

std::istream& operator>>(std::istream& is, Vertex& v) {
  v.x = consume_rational(is);
  int c = is.get();
  if (c != ',') {
    LOG(ERROR) << "Expected comma but was " << (char)c << ":" << c;
  }
  v.y = consume_rational(is);
  return is;
}

std::istream& operator>>(std::istream& is, Polygon& p) {
  int n_verts;
  is >> n_verts;
  p.resize(n_verts);
  for (int i = 0; i < n_verts; ++i) {
    is >> p[i];
  }
  return is;
}

int main(int argc, char** argv) {
  ParseCommandLineFlags(&argc, &argv);
  int n_polys;
  cin >> n_polys;
  vector<Polygon> polys(n_polys);
  for (int i = 0; i < n_polys; ++i) {
    cin >> polys[i];
  }
  int n_edges;
  cin >> n_edges;
  vector<pair<Vertex, Vertex>> edges(n_edges);
  for (int i = 0; i < n_edges; ++i) {
    cin >> edges[i].first >> edges[i].second;
  }

  // viewbox size
  Q min_x = 0, min_y = 0, max_x = 1, max_y = 1;
  if (FLAGS_shrink_viewbox) {
    min_x = max_x = polys[0][0].x;
    min_y = max_y = polys[0][0].y;
  }
  if (FLAGS_expand_viewbox) {
    for (int i = 0; i < n_polys; ++i) {
      for (const auto& v : polys[i]) {
        if (v.x < min_x) min_x = v.x;
        if (max_x < v.x) max_x = v.x;
        if (v.y < min_y) min_y = v.y;
        if (max_y < v.y) max_y = v.y;
      }
    }
  }
  if (FLAGS_shrink_viewbox) {
    // Translate to reasonable coordinates
    for (auto& p : polys) {
      for (auto& v : p) {
        v.x -= min_x;
        v.y -= min_y;
      }
    }
    for (auto& e : edges) {
      e.first.x -= min_x;
      e.first.y -= min_y;
      e.second.x -= min_x;
      e.second.y -= min_y;
    }
    LOG(INFO) << "Translate " << min_x << "," << min_y;
    max_x -= min_x;
    max_y -= min_y;
    min_x = min_y = 0;
  }
  printf(
      R"(<svg xmlns="http://www.w3.org/2000/svg" xmlns:xlink="http://www.w3.org/1999/xlink" width="400px" height="400px" viewBox="%.3f %.3f %.3f %.3f" stroke-linejoin="round" stroke-linecap="round">)",
      min_x.convert_to<double>() - 0.005, min_y.convert_to<double>() - 0.005,
      Q(max_x - min_x).convert_to<double>() + 0.01,
      Q(max_y - min_y).convert_to<double>() + 0.01);
  if (!FLAGS_shrink_viewbox) {
    printf(
        R"(<rect x="0" y="0" width="1" height="1" fill="none" stroke="blue" stroke-width="0.005"/>)");
  }
  printf(
      R"(<path fill="silver" stroke="gray" stroke-width="0.005" fill-rule="nonzero" d=")");
  for (int i = 0; i < n_polys; ++i) {
    for (int j = 0; j < polys[i].size(); ++j) {
      printf("%c%.3f %.3f", j == 0 ? 'M' : 'L',
             polys[i][j].x.convert_to<double>(),
             polys[i][j].y.convert_to<double>());
    }
    printf(R"(Z)");
  }
  printf(
      R"("/><g fill="none" stroke="purple" stroke-width="0.003">)");
  for (const auto& e : edges) {
    printf(
        R"(<path d="M%.3f %.3fL%.3f %.3f"/>)", e.first.x.convert_to<double>(),
        e.first.y.convert_to<double>(), e.second.x.convert_to<double>(),
        e.second.y.convert_to<double>());
  }
  printf("</g></svg>");

  return 0;
}
