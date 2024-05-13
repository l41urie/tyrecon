#include <cstdio>
#include <windows.h>

struct Vehicle{
int wheels;
float maxspeed;

Vehicle(int wheels, float maxspeed) : wheels(wheels), maxspeed(maxspeed)
{ }

int wheel_count()
{
    return wheels;
}

virtual void set_wheels(int num)
{
    wheels = num;
}

float max_speed()
{
    return maxspeed;
}

virtual void set_maxspeed(float speed)
{
    maxspeed = speed;
}

void print_stats()
{
    printf("%d wheels, maxspeed %.2fKM/h\n", wheels, maxspeed);
}
};

struct Car : public Vehicle
{
    Car() : Vehicle(4, 200.f)
    {
    }

    void print()
    {
        printf("hi! i'm a car.\n");
        print_stats();
    }
};

struct Bicycle : public Vehicle
{
    Bicycle() : Vehicle(2, 40.f)
    {
    }

    void print()
    {
        printf("Hi! I'm a bicycle\n");
        print_stats();
    }
};

int argument_test(int a1, int a2, int a3, int a4, int a5, int a6, int a7, int a8, int a9)
{
    return a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9;
}

int main()
{
    while (1) {
        int r = argument_test(1, 2, 3, 4, 5, 6, 7, 8, 9);
        printf("args: %d\n", r);
        Sleep(5000);
    }

    return 0;
}