#include "element.hpp"

struct Azimuth {
  double val{0.0};
};
struct Elevation {
  double val{0.0};
};
struct Distance {
  double val{1.0};
};

struct SETag{};
struct SomethingElse {
    std::string s{"who knows?"};
    using tag = SETag;
};

using PositionImpl =
    ElementImpl<Azimuth, OptionalParam<Elevation>, DefaultedParam<Distance>>;

// ElementBase provides get(), has(), unset(), isDefault() and variadic ctor
class Position : public ElementBase<Position, PositionImpl> {
 public:
  // remove this and define ctor to mandate some parameters are passed on construction
  using ElementBase::ElementBase;

  // set is manually defined, partly so autocomplete works, partly so you can
  // make parameters read only
  void set(Azimuth const& az) {
    impl_.set(az);
  }
  void set(Elevation const& el) {
    impl_.set(el);
  }
  void set(Distance const& d) {
    impl_.set(d);
  }

  void set(SomethingElse const& s) {
      something = s;
  }

  // to let the base class mess with impl_
  friend ElementBase<Position, PositionImpl>;
 private:
  PositionImpl impl_;
  SomethingElse something;

  // if not in impl, will try tag dispatch
  SomethingElse get_(SomethingElse::tag) const {
      return something;
  }
};

struct BadTag {};
struct Bad {using tag = BadTag;};

int main() {
  Position sPos;
  std::cout << (sPos.has<Elevation>() ? "" : "do not ") << "have Elevation" << '\n';
  std::cout << (sPos.has<Azimuth>() ? "" : "do not ") << "have Azimuth" << '\n';
  std::cout << (sPos.has<Distance>() ? "" : "do not ") << "have Distance" << '\n';
  std::cout << sPos.get<Azimuth>().val << '\n';
  sPos.set(Azimuth{30.0});
  std::cout << sPos.get<Azimuth>().val << '\n';
  sPos.set(Elevation{15.0});
  std::cout << sPos.get<Elevation>().val << '\n';
  auto otherPos = sPos;
  std::cout << otherPos.get<Elevation>().val << '\n';
  std::cout << sPos.get<Distance>().val << '\n';
  std::cout << "Distance " << (sPos.isDefault<Distance>() ? "is" : "is not") << " still set to default" << '\n';
  sPos.set(Distance{2.0});
  std::cout << "Distance " << (sPos.isDefault<Distance>() ? "is" : "is not") << " still set to default" << '\n';
  sPos.unset<Elevation>();
  sPos.unset<Distance>();
  assert(!sPos.has<Elevation>());
  assert(sPos.isDefault<Distance>());
  auto pos = Position{Azimuth{3.0}};
  std::cout << sPos.get<SomethingElse>().s << "\n";
//  sPos.get<Bad>();
}
