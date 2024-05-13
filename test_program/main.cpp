#include <cstdio>
#include <windows.h>

struct Vehicle {
  int wheels;
  float maxspeed;

  Vehicle(int wheels, float maxspeed) : wheels(wheels), maxspeed(maxspeed) {}

  int wheel_count() { return wheels; }

  virtual void set_wheels(int num) { wheels = num; }

  float max_speed() { return maxspeed; }

  virtual void set_maxspeed(float speed) { maxspeed = speed; }

  void print_stats() {
    printf("%d wheels, maxspeed %.2fKM/h\n", wheels, maxspeed);
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

void use_after_free(void *a) {
  printf("I received %p\n", a);
}

int main() {
  while (1) {
    void *a = malloc(100);
    free(a);
    use_after_free(a);
    Sleep(50000);
  }
  return 0;
}