#include <cstdio>
#include <cstdlib>
#include <vector>

struct Vehicle {
  int wheels;
  float maxspeed;

  Vehicle(int wheels, float maxspeed) : wheels(wheels), maxspeed(maxspeed) {}

  float max_speed() { return maxspeed; }
  int wheel_count() { return wheels; }

  virtual void set_wheels(int num) { wheels = num; }
  virtual void set_speed(float speed) { maxspeed = speed; }

  void print_stats() {
    printf("%d wheels, maxspeed %.2fKM/h\n", wheel_count(), max_speed());
  }
};

struct Car : public Vehicle {
  Car() : Vehicle(4, 200.f) {}

  void print() {
    printf("hi! i'm a car.\n");
    print_stats();
  }
};

struct Bicycle : public Vehicle {
  Bicycle() : Vehicle(2, 40.f) {}

  void print() {
    printf("Hi! I'm a bicycle\n");
    print_stats();
  }
};

void type_discovery_test() {
  printf("--- type discovery ---\n");

  Bicycle *b = new Bicycle;
  b->print();
  delete b;

  Car *c = new Car;
  c->set_wheels(3);
  c->print();
  delete c;
}

struct VehicleManager {
  std::vector<Vehicle *> v;

  void new_random_vehicle() {
    if ((rand() % 2) == 0) {
      Car *c = new Car;
      c->set_speed(160 + rand() % 40);
      v.emplace_back(c);
    } else {
      auto *c = new Bicycle;
      c->set_speed(15 + rand() % 5);
      v.emplace_back(c);
    }
  }

  VehicleManager() {
    for (auto i = 0; i < 100; ++i) {
      new_random_vehicle();
    }
  }
};

bool vehicle_matches_requirements(Vehicle *v) {
  auto count = v->wheel_count();
  if (count % 2 != 0)
    return false;

  if (v->max_speed() < 17)
    return false;

  return true;
}

void type_discovery2() {
  printf("--- type discovery 2 ---\n");
  auto count = 0;
  VehicleManager m;
  for (auto v : m.v) {
    if (vehicle_matches_requirements(v))
      count++;
  }

  printf("found %d vehicles matching our requirements\n", count);
}

int main() {
  type_discovery_test();
  system("pause");
  type_discovery2();
  system("pause");
  return 0;
}