#include "common.h"

/*
Polygon ReadPolygon(istream &is) {
  int num_points;
  is >> num_points;
  vector<Point> ps(num_points);
  rep (j, num_points) ps[j] = ReadPoint(is);

  Polygon p;
  ps.emplace_back(ps[0]);
  bg::assign_points(p, ps);
  return p;
}
*/

// Input
vector<Polygon> silhouette_polygons;
vector<bool> silhouette_flags;
vector<Segment> skeleton;

// Points and adjacency
vector<Point> coords;
vector<vector<int>> adj;

// Regions
vector<vector<int>> regions;

void Input() {
  istringstream ss(ReadAllAndRemoveComma(cin));

  int num_polygons;
  ss >> num_polygons;
  rep (i, num_polygons) {
    int num_points;
    ss >> num_points;
    vector<Point> ps(num_points);
    rep (j, num_points) ps[j] = ReadPoint(ss);

    Polygon p;
    ps.emplace_back(ps[0]);
    bg::assign_points(p, ps);

    bool f = bg::area(p) > 0;
    if (!f) {
      reverse(all(ps));
      bg::assign_points(p, ps);
    }

    assert(bg::area(p) > 0);
    silhouette_polygons.emplace_back(p);
    silhouette_flags.push_back(f);
  }

  int num_segments;
  ss >> num_segments;
  rep (i, num_segments) skeleton.emplace_back(ReadSegment(ss));
}

//
// Segment Arrangement
//

void SegmentArrangement() {
  // Enumerating points
  coords.clear();
  for (const auto &s : skeleton) {
    coords.emplace_back(s.first);
    coords.emplace_back(s.second);
  }
  rep (i, skeleton.size()) {
    rep (j, i) {
      vector<Point> ps;
      bg::intersection(skeleton[i], skeleton[j], ps);
      // cout << x.size() << endl;
      for (const auto &p : ps) {
        // cout << bg::dsv(skeleton[i]) << bg::dsv(skeleton[j]) << bg::dsv(a) << endl;
        coords.emplace_back(p);
      }
    }
  }
  sort(all(coords), [](const Point &a, const Point &b) {
      return a.x() == b.x() ? a.y() < b.y() : a.x() < b.x();
    });
  coords.erase(std::unique(
      coords.begin(), coords.end(),
      [](const Point &a, const Point &b) {
        return a.x() == b.x() && a.y() == b.y();
      }), coords.end());

  cout << bg::comparable_distance(coords[0], coords[1]) << endl;

  // Constructing the graph
  adj.assign(coords.size(), vector<int>());
  for (const auto &s : skeleton) {
    vector<pair<Bigrat, int>> ps;
    rep (j, coords.size()) {
      const auto &p = coords[j];
      if (bg::intersects(p, s)) {
        ps.emplace_back(make_pair(bg::comparable_distance(s.first, p), j));
      }
    }
    sort(all(ps));
    assert(ps.size() >= 2);
    rep (j, ps.size() - 1) {
      int a = ps[j].second, b = ps[j + 1].second;
      adj[a].emplace_back(b);
      adj[b].emplace_back(a);
    }
  }
}

void PrintGraph() {
  rep (i, coords.size()) {
    cerr << bg::dsv(coords[i]) << ": ";
    for (int x : adj[i]) cerr << " " << bg::dsv(coords[x]);
    cerr << endl;
  }
}

//
// Regions
//

Polygon RegionPolygon(int k) {
  const auto &region = regions[k];
  vector<Point> ps(region.size());
  rep (i, region.size()) ps[i] = coords[region[i]];
  ps.emplace_back(ps[0]);
  Polygon p;
  bg::assign_points(p, ps);
  return p;
}

void EnumerateRegions() {
  using pii = pair<int, int>;
  map<pii, pii> nxt;
  rep (i, coords.size()) {
    const Point &p = coords[i];
    vector<int> ord = adj[i];
    sort(all(ord), [&] (const int a, const int b) -> bool {
        Point av = coords[a], bv = coords[b];
        bg::subtract_point(av, p);
        bg::subtract_point(bv, p);
        int aq = Quadrant(av), bq = Quadrant(bv);
        if (aq != bq) return aq < bq;
        else return Det(av, bv) > 0;
      });

    rep (j, ord.size()) {
      nxt[mp(ord[(j + 1) % ord.size()], i)] = mp(i, ord[j]);
    }
  }

  set<pii> usd;
  rep (i, coords.size()) {
    for (int j : adj[i]) {
      if (usd.count(mp(i, j))) continue;

      vector<int> reg;
      int a = i, b = j;
      while (usd.count(mp(a, b)) == 0) {
        reg.emplace_back(a);
        usd.insert(mp(a, b));
        tie(a, b) = nxt.at(mp(a, b));
      }
      regions.emplace_back(reg);
    }
  }
}

void PrintRegions() {
  cerr << "[[[ " << regions.size() << " Regions ]]]" << endl;
  rep (i, regions.size()) {
    Polygon p = RegionPolygon(i);
    cerr << bg::dsv(p) << ":" << bg::area(p) << endl;
  }
}

void FilterRegions() {
  vector<vector<int>> filtered_regions;
  rep (k, regions.size()) {
    Polygon p = RegionPolygon(k);

    if (bg::area(p) < 0) continue;

    rep (i, silhouette_polygons.size()) {
      if (!silhouette_flags[i] && bg::within(p, silhouette_polygons[i])) {
        goto dmp;
      }
    }

    filtered_regions.emplace_back(regions[k]);
 dmp:;
  }
  regions.swap(filtered_regions);
}

void Output() {
  cout << coords.size() << endl;
  for (const auto &p : coords) {
    cout
        << bm::numerator(p.x()) << "/" << bm::denominator(p.x()) << " "
        << bm::numerator(p.y()) << "/" << bm::denominator(p.y()) << endl;
  }
  cout << regions.size() << endl;
  for (const auto &r : regions) {
    cout << r.size() << " ";
    for (auto i : r) cout << " " << i;
    cout << endl;
  }
}

//
// Verify
//

void Verify() {
  // Sum of area
  {
    Bigrat expected = 0;
    rep (i, silhouette_polygons.size()) {
      expected += (silhouette_flags[i] ? +1 : -1) *
          bg::area(silhouette_polygons[i]);
    }
    Bigrat actual = 0;
    rep (i, regions.size()) {
      actual += bg::area(RegionPolygon(i));
    }
    assert(expected == actual);
  }

  cerr << "!!! PASSED SYSTEM TEST !!!" << endl;
}

//
// Entry point
//

int main() {
  Input();
  SegmentArrangement();
  //PrintGraph();
  EnumerateRegions();
  //PrintRegions();
  FilterRegions();
  //PrintRegions();
  Verify();
  Output();
}
